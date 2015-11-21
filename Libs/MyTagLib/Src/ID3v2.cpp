/***************************************************************
 * Name:      ID3v2.cpp
 * Purpose:   Implement the ID3 v2.3 tagging format
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-18
 **************************************************************/
#include "StdAfx.h"
#include "ID3v2.h"
#include "Charset.h"

#include <wchar.h> // for wcstol

namespace MyTagLib {
struct ID3v2::Header {
    // Construct a whole new ID3v2.3 header
    Header();

    // Constructor
    Header(std::istream &istrm);

    // Write back to stream
    void save(std::ostream &ostrm);

    // Returns true if the header is valid.
    bool isValid() const;

    enum {
        RAW_SIZE = 10, /// No padding
    };

public:

    // The "ID3" identifier
    char identifier[3];
    // The version, should be 3(for ID3 v2.3) or 4(for ID3 v2.4)
    char version;
    char dummy[2];
    int bodySize;
};

ID3v2::Header::Header() {
    memcpy(identifier, "ID3", 3);
    version = 3;
    dummy[0] = dummy[1] = 0;
    bodySize = 0;
}

ID3v2::Header::Header(std::istream &istrm) {
    istrm.seekg(0);
    if (istrm) {
        // The "ID3" identifier
        istrm.read(identifier, 3);

        if (strncmp(identifier, "ID3", 3) == 0) {

            // The version, should be 3(for ID3 v2.3) or 4(for ID3 v2.4)
            istrm.read(&version, 1);

            // Skip something we aren't interested in
            istrm.read(dummy, 2);

            // Read the raw size integer
            istrm.read((char *) &bodySize, sizeof(bodySize));

            // Calculate the size of data body.
            unsigned char *bodySizeVec = (unsigned char *) &bodySize;
            bodySize = ((bodySizeVec[0] & 0x7F) << 21) +
                       ((bodySizeVec[1] & 0x7F) << 14) +
                       ((bodySizeVec[2] & 0x7F) << 07) +
                       ((bodySizeVec[3]));
        }
    }
}

bool ID3v2::Header::isValid() const {
    return (strncmp(identifier, "ID3", 3) == 0) &&
           ((version == 3) || (version == 4));
}

void ID3v2::Header::save(std::ostream &ostrm) {
    assert(isValid());

    ostrm.write(identifier, 3);
    ostrm.write(&version, 1);
    ostrm.write(dummy, sizeof(dummy));

    // Calculate size
    int bodySizeToWrite = 0;
    unsigned char *bodySizeVec = (unsigned char *) &bodySizeToWrite;
    bodySizeVec[0] = (bodySize >> 21) & 0x7F;
    bodySizeVec[1] = (bodySize >> 14) & 0x7F;
    bodySizeVec[2] = (bodySize >>  7) & 0x7F;
    bodySizeVec[3] = (bodySize) & 0x7F;

    ostrm.write((char *) &bodySizeToWrite, sizeof(bodySizeToWrite));
}

//////////////////////////////////////////////////////////////////////////

struct ID3v2::Frame {
public:

    enum {
        HEADER_SIZE = 10,
    };

    // Constructor
    Frame(std::istream &istrm);

    // Virtual dtor
    virtual ~Frame();

    // Write back to stream
    void save(std::ostream &ostrm);

protected:

    // Default constructor, used to create a whole new frame
    Frame(const char *name);

    // Load frame header(10 bytes)
    void loadHeader(std::istream &istrm);

public:

    char name[4]; // No terminated zero.
    size_t contentSize;
    unsigned short flags;

    char *content;
};

ID3v2::Frame::Frame(const char *name)
    : contentSize(0), flags(0), content(NULL) {
    assert(name);
    assert(strlen(name) == 4);

    memcpy(this->name, name, 4);
}

ID3v2::Frame::Frame(std::istream &istrm)
    : content(NULL) {
    loadHeader(istrm);

    if (contentSize > 0) {
        content = new char[contentSize];
        istrm.read(content, contentSize);
    }
}

ID3v2::Frame::~Frame() {
    if (content) {
        delete [] content;
        content = NULL;
    }
}

void ID3v2::Frame::loadHeader(std::istream &istrm) {
    istrm.read(name, 4);

    // Read frame size
    char *sizeVec = (char *) &contentSize;
    istrm.read(sizeVec, sizeof(contentSize));
    std::swap(sizeVec[0], sizeVec[3]);
    std::swap(sizeVec[1], sizeVec[2]);

    // Skip 2 bytes of flags
    istrm.read((char *) &flags, sizeof(flags));
}

void ID3v2::Frame::save(std::ostream &ostrm) {
    ostrm.write(name, 4);

    // Write frame size
    size_t contentSizeToWrite = contentSize;
    char *sizeVec = (char *) &contentSizeToWrite;
    std::swap(sizeVec[0], sizeVec[3]);
    std::swap(sizeVec[1], sizeVec[2]);

    ostrm.write(sizeVec, sizeof(contentSizeToWrite));
    ostrm.write((char *) &flags, sizeof(flags));

    // Write content
    if (content) {
        assert(contentSize > 0);

        ostrm.write(content, contentSize);
    }
}

// Frames that store text and encoding specification.
struct TextFrame : public ID3v2::Frame {
public:

    // Constructor, used to create a hole new text frame
    TextFrame(const char *name);

    // Constructor
    TextFrame(std::istream &istrm);

    enum RoughTextEncoding {
        RTE_ANSI,
        RTE_UTF16_BOM,
        RTE_UTF8_BOM,
        RTE_UTF8_WT_BOM,
    };

    // Get frame text
    virtual String getText() const;

    // Set frame text
    virtual void setText(const String &val);

protected:

    // Get frame text and handle all the encoding issues
    static String getText(char encoding, const char *p, size_t size);
};

TextFrame::TextFrame(const char *name)
    : Frame(name) {

}

TextFrame::TextFrame(std::istream &istrm)
    : Frame(istrm) {

}

String TextFrame::getText() const {
    if (content) {
        return getText(content[0], content + 1, contentSize - 1);
    } else {
        return EmptyString;
    }
}

/*static*/
String TextFrame::getText(char encoding, const char *p, size_t size) {
    if (!p || (size == 0)) {
        return EmptyString;
    }

    //------------------------------------------------------

    Charset &stringcvt = Charset::getInstance();

    // We will always detect the BOM of the string instead of
    // using the encoding param directly.

    // UTF16(LE)
    if (Charset::hasUtf16LE_BOM(p)) {
        return stringcvt.utf16ToWide(p + 2, size - 2);
    }

    // UTF-8
    // Charset::isUtf8() will modify the args.
    if (Charset::isUtf8(p, size)) {
        return stringcvt.utf8ToWide(p, size);
    }

    switch (encoding) {
    case RTE_UTF8_WT_BOM:

        return stringcvt.utf8ToWide(p, size);

    default:

        break;
    }

    // The fallback
    return stringcvt.multibyteToWide(p, size);
}

void TextFrame::setText(const String &val) {
    char *raw = Charset::getInstance().wideToUtf16(val);

    // Charset conversion failed.
    if (!raw) {
        return;
    }

    // No terminated null character.
    size_t stringSize = val.length() * 2;
    // 1 for the encoding mark and 2 for the UTF-16 BOM
    size_t newContentSize = 1 + 2 + stringSize;

    if (newContentSize < contentSize) {
        newContentSize = contentSize;
    } else {
        if (content) {
            delete [] content;
        }

        // newContentSize is surely not 0.
        content = new char[newContentSize];
    }

    contentSize = newContentSize;

    // Clear the buffer
    memset(content, 0, contentSize);

    //=====================================================
    // Submit changes

    // We always write UFT-16LE with BOM strings to the tag.
    unsigned char *bomWriter = (unsigned char *) content;
    bomWriter[0] = RTE_UTF16_BOM;
    bomWriter[1] = 0xFF;
    bomWriter[2] = 0xFE;

    // Copy to new buffer
    if (stringSize) {
        memcpy(content + 3, raw, stringSize);
    }

    delete [] raw;
}

/// Text frames that with language tags (Multilingual)
class MultilingualTextFrame : public TextFrame {
public:

    // Default constructor, used to create a hole new frame.
    MultilingualTextFrame(const char *name);

    // Constructor
    MultilingualTextFrame(std::istream &istrm);

    // Get frame text
    virtual String getText() const;
    // Set frame text
    virtual void setText(const String &val);

private:

    // Create default language entry and description for the tag.
    void init();

private:

    int m_textBegin;
};

MultilingualTextFrame::MultilingualTextFrame(const char *name)
    : TextFrame(name) {
    init();
}

MultilingualTextFrame::MultilingualTextFrame(std::istream &istrm)
    : TextFrame(istrm), m_textBegin(0) {
    char lang[4]= { 0 };
    if (content) {
        memcpy(lang, content + 1, 3);
    }

    // Incorrect format
    if (strlen(lang) != 3) {
        init();
    } else {
        // Skip the language name and discription parts

        const char *p = content + 4; // #(language name) == 4
        const char *pEnd = content + contentSize;

        // Skip the description string
        while ((p < pEnd) && *p) {
            ++p;
        }
        // Skip the terminated zeros of description string
        while ((p < pEnd) && !(*p)) {
            ++p;
        }

        m_textBegin = p - content;
    }
}

void MultilingualTextFrame::init() {
    const static unsigned char stdLangAndDesc[] =
    { RTE_UTF16_BOM, 'e', 'n', 'g', 0xFF, 0xFE, 0, 0 };

    if (contentSize < sizeof(stdLangAndDesc)) {
        if (content) {
            assert(contentSize);
            delete [] content;
        }

        contentSize = sizeof(stdLangAndDesc);
        content = new char[sizeof(stdLangAndDesc)];
    }

    memcpy(content, stdLangAndDesc, sizeof(stdLangAndDesc));
    m_textBegin = sizeof(stdLangAndDesc);
}

String MultilingualTextFrame::getText() const {
    assert(m_textBegin > 0);
    assert(content);

    const char *p = content + m_textBegin;
    return TextFrame::getText(content[0], p, contentSize - m_textBegin);
}

void MultilingualTextFrame::setText(const String &val) {
    assert(m_textBegin > 0);
    assert(content);

    //-----------------------------------------------------

    char *raw = Charset::getInstance().wideToUtf16(val);

    // Charset conversion failed.
    if (!raw) {
        return;
    }

    // No terminated null character.
    size_t stringSize = val.length() * 2;
    // 2 for the UTF-16 BOM
    size_t newContentSize = m_textBegin + 2 + stringSize;

    if (newContentSize < contentSize) {
        newContentSize = contentSize;
    } else {
        char *newStorage = new char[newContentSize];
        memcpy(newStorage, content, m_textBegin);
        delete [] content;

        content = newStorage;
    }

    contentSize = newContentSize;

    // Clear the buffer
    memset(content + m_textBegin, 0, contentSize - m_textBegin);

    //=====================================================
    // Submit changes

    // We always write UFT-16LE with BOM strings to the tag.
    unsigned char *bomWriter = (unsigned char *) content;
    bomWriter[0] = RTE_UTF16_BOM;
    bomWriter[m_textBegin] = 0xFF;
    bomWriter[m_textBegin + 1] = 0xFE;

    // Copy to new buffer
    if (stringSize) {
        memcpy(content + (m_textBegin + 2), raw, stringSize);
    }

    delete [] raw;
}

//////////////////////////////////////////////////////////////////////////

ID3v2::ID3v2()
    : m_header(NULL), m_paddings(0) {
    getRange().setOrder(Range::ORDER_FILE_BEG);
}

ID3v2::~ID3v2() {
    clear();
}

void ID3v2::doClear() {
    if (m_header) {
        delete m_header;
        m_header = NULL;
    }

    FrameVec::const_iterator iter(m_frames.begin());
    for (; iter != m_frames.end(); ++iter) {
        delete *iter;
    }

    m_frames.clear();
    m_paddings = 0;
}

bool ID3v2::doLoad(std::istream &istrm) {
    m_header = new Header(istrm);
    if (!m_header->isValid()) {
        clear(); // TODO: 这里是有必要的？
        return false;
    }

    getRange().assign(0, Header::RAW_SIZE + m_header->bodySize);
    return loadFrames(istrm, m_header->bodySize);
}

bool ID3v2::loadFrames(std::istream &istrm, int bodySize) {
    const int dataBodyEnd = (int) istrm.tellg() + bodySize;
    // Given some tag's format is incorrect, we try to handle it.
    size_t updatedSize = 0;

    do {
        // Test and skip all zero bytes(which all are intent for padding)
        if (istrm.peek() == 0) {
            istrm.get();
            m_paddings++;

            continue;
        }

        if (!readFrame(istrm)) {
            clear(); // TODO: 这里是有必要的？
            return false;
        }

        updatedSize += Frame::HEADER_SIZE + m_frames.back()->contentSize;

    } while (istrm && (int) istrm.tellg() < dataBodyEnd);

    // Update size
    updatedSize += m_paddings;
    int delta = updatedSize - bodySize;
    if (delta < 0) {
        m_paddings += - delta;
    } else if (delta > 0) {
        handleFrameResize(delta);
    }

    return true;
}

bool isValidFrameId(char *frameId) {
    for (int i = 0; i < 4; i++) {
        if (frameId[i] == 0) {
            return false;
        }
    }

    if (frameId[0] == 'T' || frameId[0] == 'W') {
        return true;
    }

    static const char *sValidFrameIDs[] = {
        "AENC","APIC","ASPI","COMM","COMR",
        "ENCR","EQU2","EQUA","ETCO","GEOB",
        "GRID","IPLS","LINK","MCDI","MLLT",
        "OWNE","PCNT","POPM","POSS","PRIV",
        "RBUF","RVA2","RVAD","RVRB","SEEK",
        "SIGN","SYLT","SYTC","UFID","USER",
        "USLT"
    };

    size_t count = sizeof(sValidFrameIDs) / sizeof(sValidFrameIDs[0]);
    for (size_t i = 0; i < count; i++) {
        if (memcmp(frameId, sValidFrameIDs[i], 4) == 0) {
            return true;
        }
    }

    return false;
}

bool ID3v2::readFrame(std::istream &istrm) {
    char frameId[5] = { 0 };
    istrm.read(frameId, 4);

    if (!isValidFrameId(frameId)) {
        return false;
    }

    istrm.seekg(-4, std::ios::cur);

    Frame *frame;
    if (frameId[0] == 'T') {
        frame = new TextFrame(istrm);
    }
    // Multilingual text frame
    else if (strcmp(frameId, "COMM") == 0 ||
             strcmp(frameId, "USLT") == 0) {
        frame = new MultilingualTextFrame(istrm);
    } else {
        frame = new Frame(istrm);
    }

    m_frames.push_back(frame);
    return true;
}

Tag::WriteResult ID3v2::doSave(std::ostream &ostrm) {
    assert(m_header);

    // Save header
    m_header->save(ostrm);

    // Save frames
    FrameVec::iterator iter(m_frames.begin());
    for (; iter != m_frames.end(); ++iter) {
        (*iter)->save(ostrm);
    }

    // Rewrite paddings
    if (m_paddings) {
        char *zero = new char[m_paddings];
        memset(zero, 0, m_paddings);
        ostrm.write(zero, m_paddings);
        delete [] zero;
    }

    return WR_OK;
}

bool ID3v2::doInit() {
    assert(m_header == NULL);

    m_header = new Header;
    // Always add some paddings
    m_paddings = _2KB;
    m_header->bodySize = _2KB;

    getRange().resize(Header::RAW_SIZE + m_paddings);
    return true;
}

bool ID3v2::isOk() const {
    return m_header && m_header->isValid()/* && !m_frames.empty()*/;
}

void ID3v2::handleFrameResize(size_t deltaSize) {
    assert(m_header);
    assert(deltaSize >= 0); // Should not shrink the tag.

    if (deltaSize == 0) {
        return;
    }

    // Make use of the paddings
    if (deltaSize <= m_paddings) {
        m_paddings -= deltaSize;
        deltaSize = 0;
    } else {
        // Add more paddings for later modifications
        int morePaddings = (m_paddings == 0) ? _2KB : _1KB;

        // The current paddings is not enough to fit.
        deltaSize -= m_paddings;
        deltaSize += morePaddings;

        m_paddings = morePaddings;
    }

    // Update body size
    m_header->bodySize += deltaSize;
    // Update range length (but not commit)
    getRange().resize(deltaSize);
}

//////////////////////////////////////////////////////////////////////////

template<class FrameT>
FrameT *ID3v2::findFrame(const char *name) const {
    FrameVec::const_iterator iter(m_frames.begin());
    for (; iter != m_frames.end(); ++iter) {
        if (strncmp((*iter)->name, name, 4) == 0) {
            return (FrameT *) *iter;
        }
    }

    return NULL;
}

String ID3v2::getFrameText(const char *name) const {
    TextFrame *frame = findFrame<TextFrame>(name);
    return frame ? frame->getText() : EmptyString;
}

bool ID3v2::setFrameText(const char *name, const String &val) {
    int deltaSize = 0;
    TextFrame *frame = findFrame<TextFrame>(name);
    if (!frame) {
        if (name[0] == 'T') {
            frame = new TextFrame(name);
        } else {
            frame = new MultilingualTextFrame(name);
        }

        deltaSize += Frame::HEADER_SIZE + frame->contentSize;
        m_frames.push_back(frame);
    }

    size_t oldSize = frame->contentSize;
    frame->setText(val);
    size_t newSize = frame->contentSize;

    deltaSize += newSize - oldSize;
    handleFrameResize(deltaSize);

    return true;
}

String ID3v2::doGetArtist() const {
    return getFrameText("TPE1");
}

bool ID3v2::doSetArtist(const String &Artist) {
    return setFrameText("TPE1", Artist);
}

String ID3v2::doGetTitle() const {
    return getFrameText("TIT2");
}

bool ID3v2::doSetTitle(const String &Title) {
    return setFrameText("TIT2", Title);
}

String ID3v2::doGetAlbum() const {
    return getFrameText("TALB");
}

bool ID3v2::doSetAlbum(const String &Album) {
    return setFrameText("TALB", Album);
}

String ID3v2::doGetYear() const {
    String year(getFrameText("TYER"));
    if (year.empty()) {
        year = getFrameText("TDRC");
    }

    return year;
}

bool ID3v2::doSetYear(const String &Year) {
    bool ok1 = setFrameText("TYER", Year);
    bool ok2 = setFrameText("TDRC", Year);

    return ok1 || ok2;
}

String ID3v2::doGetComment() const {
    return getFrameText("COMM");
}

bool ID3v2::doSetComment(const String &Comment) {
    return setFrameText("COMM", Comment);
}

String ID3v2::doGetLyric() const {
    return getFrameText("USLT");
}

bool ID3v2::doSetLyric(const String &Lyric) {
    return setFrameText("USLT", Lyric);
}

int ID3v2::doGetTrackNumber() const {
    String val(getFrameText("TRCK"));
    return wcstol(val.c_str(), NULL, 10);
}

bool ID3v2::doSetTrackNumber(int TrackNumber) {
    wchar_t buf[8];
    swprintf(buf, sizeof(buf), L"%d", TrackNumber);

    return setFrameText("TIT2", buf);
}

Genre ID3v2::doGetGenre() const {
    return getFrameText("TCON");
}

bool ID3v2::doSetGenre(const Genre &genre) {
    return setFrameText("TCON", genre.getName());
}

}
