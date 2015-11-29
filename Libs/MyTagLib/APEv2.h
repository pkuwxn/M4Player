/***************************************************************
 * Name:      APEv2.cpp
 * Purpose:   Implement the APE v2 tagging format
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-3-19
 **************************************************************/
#pragma once
#include "Tag.h"

#include <vector>

namespace MyTagLib {
/// Implement the APE v2 tagging format
class APEv2 : public Tag {
public:

    /// Default constructor
    APEv2();

    /// Dtor
    virtual ~APEv2();

    /// Returns true if the tag has been read successfully.
    virtual bool isOk() const;

private:

    OVERRIDE_ALL_ACCESSORS();

    virtual bool doLoad(std::istream &istrm);
    virtual WriteResult doSave(std::ostream &ostrm);
    virtual bool doInit();
    virtual void doClear();

    // Find APEv2 footer from the end.
    // Return true if footer was found, and the file pointer will point
    // to the next byte of the structure.
    bool seekFooter(std::istream &istrm);

    // Determine whether exists the header structure after the footer
    // is known and read.
    // @a p The raw data pointer which points to the loaded footer.
    bool hasHeader(const char *p);

    // Read all APEv2 items
    bool readItems(std::istream &istrm);

    // Read an APEv2 item
    void readItem(std::istream &istrm);

    // Ajust the paddings (decrease)
    void decreasePaddings(size_t delta);

    // Update tag size for writing.
    void updateHeader();

private:

    struct Item;

    // Find the specified item, if not found, then an new item with the
    // specified @key will be created.
    //
    // This function does not maintain the total tag size, so the caller
    // should handle it.
    Item *findOrCreateItem(const char *key);

    // Find the specified item.
    // @return If not found, NULL will be returned.
    Item *findItem(const char *key) const;

    // Get the specified item named @a key
    String getString(const char *key) const;

    // Assign new value to item named @a key
    bool setString(const char *key, const String &val);

    // Get the specified item named @a key
    //
    // If item not found, the @a defaultVal will be returned.
    int getInteger(const char *key, int defaultVal) const;

    // Assign a new integer
    bool setInteger(const char *key, int val);

private:

    struct HeaderFooter {
        char preamble[8]; // No terminated NULL character.

        /* 1000 = Version 1.000 (old)
           2000 = Version 2.000 (new)
         */
        int version;
        // Tag size in bytes including footer and all tag items
        // excluding the header to be as compatible as possible
        // with APE Tags 1.000
        int tagSize;
        int numItems; // Number of items in the Tag (n)
        // Global flags of all items (there are also private flags for every item)
        int tagsFlags;

    public:

        // Default ctor
        HeaderFooter();

        // Initialize to be a whole new header/footer.
        void init();

        enum StructType {
            HFST_HEADER,
            HFST_FOOTER,
        };

        // Write back to stream
        bool save(std::ostream &ostrm, StructType st);

        // Test preamble
        bool isValid(StructType st) const;

    private:
        int reserved1, reserved2;
    };

    //=========================================================

    bool m_hasHeader;
    HeaderFooter m_footer;

    typedef std::vector<Item *> ItemVec;
    ItemVec m_items;
};
}
