/***************************************************************
 * Name:      APEv2.cpp
 * Purpose:   Implement the APE v2 tagging format
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-3-19
 **************************************************************/
#include "StdAfx.h"
#include "APEv2.h"
#include "Charset.h"

#include <stdlib.h> // for qsort()

namespace MyTagLib {
struct APEv2::Item {
    // Default ctor
    Item();

    // Create a whole new item
    Item(const std::string &k, size_t valueSize);

    // Dtor
    ~Item();

    // Load from an input stream
    bool load(std::istream &istrm);

    // Write back to stream
    bool save(std::ostream &ostrm);

    // Get item size
    size_t getSize() const;

    // For sorting
    static int compare(const void *arg1, const void *arg2);

public:

    int flags;
    std::string key;
    size_t valSize;
    char *val;
};

APEv2::Item::Item()
    : flags(0), valSize(0), val(NULL) {

}

APEv2::Item::Item(const std::string &k, size_t valueSize)
    : flags(0), key(k), valSize(valueSize), val(NULL) {
    if (valSize) {
        val = new char[valSize];
    }
}

bool APEv2::Item::load(std::istream &istrm) {
    istrm.read((char *) &valSize, sizeof(valSize));
    istrm.read((char *) &flags, sizeof(flags));

    char ch;
    while (istrm && (ch = istrm.get())) { // TODO:
        if (!istrm) {
            // Invalid item: endless key
            return false;
        }

        key.push_back(ch);
    }

    // TODO: Max size?
    if (valSize) {
        val = new char[valSize];
        istrm.read(val, valSize);
    }

    return true;
}

bool APEv2::Item::save(std::ostream &ostrm) {
    ostrm.write((char *) &valSize, sizeof(valSize));
    ostrm.write((char *) &flags, sizeof(flags));

    ostrm.write(key.c_str(), key.length());
    char ch = 0;
    ostrm.write(&ch, 1); // The terminated zero.

    if (valSize) {
        assert(val);
        ostrm.write(val, valSize);
    }

    return true;
}

APEv2::Item::~Item() {
    if (val) {
        delete [] val;
    }
}

size_t APEv2::Item::getSize() const {
    return 8 + key.length() + 1 + valSize;
}

/*static*/
int APEv2::Item::compare(const void *arg1, const void *arg2) {
    const Item *item1 = *((const Item **) arg1);
    const Item *item2 = *((const Item **) arg2);

    return item1->valSize - item2->valSize;
}

//////////////////////////////////////////////////////////////////////////

APEv2::HeaderFooter::HeaderFooter()
    : version(2000), tagSize(sizeof(HeaderFooter)),
      numItems(0), tagsFlags(0x80000000),
      reserved1(0), reserved2(0) {
    memset(preamble, 0, sizeof(preamble));
}

void APEv2::HeaderFooter::init() {
    assert(!isValid(HFST_FOOTER));

    memcpy(preamble, "APETAGEX", sizeof(preamble));
    version = 2000;
    tagSize = sizeof(HeaderFooter);
    numItems = 0;
    tagsFlags = 0x80000000; // footer
    reserved1 = reserved2 = 0;
}

bool APEv2::HeaderFooter::save(std::ostream &ostrm, StructType st) {
    // Write premble, version, tagSize, numItems (20 bytes in total)
    ostrm.write(preamble, 20);

    int flags = tagsFlags;
    if (st == HFST_HEADER) {
        flags |= 0x20000000;
    }

    ostrm.write((char *) &flags, sizeof(flags));

    // Write reserved1, reserved2
    ostrm.write((char *) &reserved1, sizeof(reserved1) * 2);

    return true;
}

bool APEv2::HeaderFooter::isValid(StructType st) const {
    return (strncmp(preamble, "APETAGEX", 8) == 0) &&
           (version == 2000) &&
           (tagSize >= 0) &&
           (numItems >= 0) &&
           ((st == HFST_FOOTER) ?
            (tagsFlags == 0x80000000) :
            (tagsFlags == (0x80000000 | 0x20000000))) &&
           (reserved1 == 0) &&
           (reserved2 == 0);
}

//////////////////////////////////////////////////////////////////////////

APEv2::APEv2()
    : m_hasHeader(false) {
    getRange().setOrder(Range::ORDER_FILE_END - 10000);
}

APEv2::~APEv2() {
    clear();
}

void APEv2::doClear() {
    ItemVec::const_iterator iter(m_items.begin());
    for (; iter != m_items.end(); ++iter) {
        delete *iter;
    }

    m_items.clear();

    //----------------------------------------------

    // bad trick
    m_footer.preamble[0] = 0;
    m_hasHeader = false;
}

void APEv2::decreasePaddings(size_t delta) {
    Item *dummy = findOrCreateItem("Dummy");
    if (dummy->valSize == 0) {
        enum {
            PADDING_SIZE = 256,
        };

        dummy->valSize = PADDING_SIZE;
        dummy->val = new char[PADDING_SIZE];
        memset(dummy->val, 0, PADDING_SIZE);

        getRange().resize(dummy->getSize());
    }

    if (delta <= dummy->valSize) {
        dummy->valSize -= delta;
    } else {
        getRange().resize(delta);
    }
}

bool APEv2::doLoad(std::istream &istrm) {
    if (!seekFooter(istrm)) {
        return false;
    }

    return readItems(istrm);
}

bool APEv2::seekFooter(std::istream &istrm) {
    enum {
        // Searching the last 10 KB at most.
        _10KB = 10 * 1024,
    };

    char buf[_10KB];

    istrm.seekg(0, std::ios::end);
    int maxSizeToRead = (int) istrm.tellg();
    if (maxSizeToRead > _10KB) {
        maxSizeToRead = _10KB;
    }

    istrm.seekg(- maxSizeToRead, std::ios::end);
    istrm.read(buf, maxSizeToRead);

    // Prepare for first matching
    const char *p = buf + maxSizeToRead - sizeof(HeaderFooter);

    while (p >= buf) {
        HeaderFooter *footer = (HeaderFooter *) p;
        if (footer->isValid(HeaderFooter::HFST_FOOTER)) {

            m_footer = *footer;
            // Check whether exists a header.
            m_hasHeader = hasHeader(p);

            //---------------------------------------------
            // Calculate the range

            int footerBegins =
                (int) istrm.tellg() - (maxSizeToRead - (p - buf));

            int rangeStart =
                // The tag size field in footer includes the footer itself
                // but excludes the header.
                footerBegins - m_footer.tagSize + sizeof(HeaderFooter);

            int rangeLength = m_footer.tagSize;

            // Seek to the tag beginning
            istrm.seekg(rangeStart);

            if (m_hasHeader) {
                rangeStart -= sizeof(HeaderFooter);
                rangeLength += sizeof(HeaderFooter);
            }

            getRange().assign(rangeStart, rangeLength);
            return true;
        }

        --p;
    }

    return false;
}

bool APEv2::hasHeader(const char *p) {
    assert(m_footer.isValid(HeaderFooter::HFST_FOOTER));

    p -= m_footer.tagSize;
    HeaderFooter *header = (HeaderFooter *) p;

    return header->isValid(HeaderFooter::HFST_HEADER);
}

bool APEv2::readItems(std::istream &istrm) {
    assert(m_footer.isValid(HeaderFooter::HFST_FOOTER));

    for (int i = 0; i < m_footer.numItems; i++) {
        // TODO: Badly written code.
        Item *item = new Item;
        if (item->load(istrm)) {
            m_items.push_back(item);
        } else {
            // Clear all items read.
            delete item;
            clear(); // TODO: 这里是有必要的？

            return false;
        }
    }

    return true;
}

Tag::WriteResult APEv2::doSave(std::ostream &ostrm) {
    // TODO:
    assert(isOk());

    if (!m_items.empty()) {
        // APE tag items should be sorted ascending by size
        qsort(&m_items[0], m_items.size(), sizeof(Item *), Item::compare);
        updateHeader();
    }

    if (m_hasHeader) {
        m_footer.save(ostrm, HeaderFooter::HFST_HEADER);
    }

    ItemVec::iterator iter(m_items.begin());
    for (; iter != m_items.end(); ++iter) {
        (*iter)->save(ostrm);
    }

    m_footer.save(ostrm, HeaderFooter::HFST_FOOTER);

    //================================================

    // TODO:
    return WR_OK;
}

bool APEv2::doInit() {
    assert(m_items.empty());

    m_hasHeader = true;
    m_footer.init();
    // No paddings at first
    getRange().resize(sizeof(HeaderFooter) * 2);

    return true;
}

void APEv2::updateHeader() {
    // Tag size in bytes including footer and all tag items
    // excluding the header
    m_footer.tagSize = sizeof(HeaderFooter);
    ItemVec::const_iterator iter(m_items.begin());
    for (; iter != m_items.end(); ++iter) {
        m_footer.tagSize += (*iter)->getSize();
    }

    m_footer.numItems = m_items.size();
}

bool APEv2::isOk() const {
    return m_footer.isValid(HeaderFooter::HFST_FOOTER);
}

APEv2::Item *APEv2::findItem(const char *key) const {
    ItemVec::const_iterator iter(m_items.begin());
    for (; iter != m_items.end(); ++iter) {
        if ((*iter)->key == key) {
            return *iter;
        }
    }

    return NULL;
}

APEv2::Item *APEv2::findOrCreateItem(const char *key) {
    Item *item = findItem(key);
    if (!item) {
        item = new Item(key, 0);
        m_items.push_back(item);
    }

    return item;
}

String APEv2::getString(const char *key) const {
    Item *item = findItem(key);
    if (item && item->val) {
        return Charset::getInstance().utf8ToWide(item->val, item->valSize);
    } else {
        return EmptyString;
    }
}

bool APEv2::setString(const char *key, const String &val) {
    Charset &stringcvt = Charset::getInstance();
    char *utf8s = stringcvt.wideToUtf8(val);

    // Charset conversion failed.
    if (!utf8s) {
        return false;
    }

    Item *item = findOrCreateItem(key);
    int oldSize = item->valSize;
    int newSize = strlen(utf8s);
    int delta = newSize - oldSize;

    if (delta > 0) {
        if (item->val) {
            delete [] item->val;
        }

        item->val = utf8s;
        item->valSize = newSize;

        decreasePaddings((oldSize == 0) ? item->getSize() : delta);
    } else {
        // The content should not be null nor empty.
        if (item->val) {
            // Add paddings in place, no changing the original valSize
            memset(item->val, 0, oldSize);
            memcpy(item->val, utf8s, newSize);
        }

        delete [] utf8s;
        utf8s = NULL;
    }

    return true;
}

int APEv2::getInteger(const char *key, int defaultVal) const {
    Item *item = findItem(key);
    if (item) {
        assert(item->valSize == sizeof(int));
    }

    // TODO: What the form of a so-called `integer` in APEv2 standard?
    return item ? (int) item->val : defaultVal;
}

bool APEv2::setInteger(const char *key, int val) {
    Item *item = findItem(key);
    if (item) {
        assert(item->valSize == sizeof(int));
    } else {
        item = new Item(key, sizeof(int));
        m_items.push_back(item);

        decreasePaddings(item->getSize());
    }

    // Assign new value
    *((int *) item->val) = val;

    return true;
}

String APEv2::doGetArtist() const {
    return getString("Artist");
}

bool APEv2::doSetArtist(const String &Artist) {
    return setString("Artist", Artist);
}

String APEv2::doGetTitle() const {
    return getString("Title");
}

bool APEv2::doSetTitle(const String &Title) {
    return setString("Title", Title);
}

String APEv2::doGetAlbum() const {
    return getString("Album");
}

bool APEv2::doSetAlbum(const String &Album) {
    return setString("Album", Album);
}

String APEv2::doGetYear() const {
    return getString("Year");
}

bool APEv2::doSetYear(const String &Year) {
    return setString("Year", Year);
}

String APEv2::doGetComment() const {
    return getString("Comment");
}

bool APEv2::doSetComment(const String &Comment) {
    return setString("Comment", Comment);
}

String APEv2::doGetLyric() const {
    return EmptyString;
}

bool APEv2::doSetLyric(const String &Lyric) {
    return false;
}

int APEv2::doGetTrackNumber() const {
    return wcstol(getString("Track").c_str(), NULL, 10);
}

bool APEv2::doSetTrackNumber(int TrackNumber) {
    wchar_t buf[4];
    swprintf(buf, sizeof(buf), L"%d", TrackNumber);

    return setString("Track", buf);
}

Genre APEv2::doGetGenre() const {
    return getString("Genre");
}

bool APEv2::doSetGenre(const Genre &genre) {
    return setString("Genre", genre.getName());
}

}
