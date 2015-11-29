/***************************************************************
 * Name:      ID3v1.h
 * Purpose:   Implement the ID3 v1/v1.1 tagging format
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-3-19
 **************************************************************/
#pragma once
#include "Tag.h"

namespace MyTagLib {
/// Implement the ID3 v1/v1.1 tagging format.
class ID3v1 : public Tag {
public:

    /// Default constructor
    ID3v1();

    /// Returns true if the tag has been read successfully.
    virtual bool isOk() const;

private:

    OVERRIDE_ALL_ACCESSORS();

    virtual bool doLoad(std::istream &istrm);
    virtual WriteResult doSave(std::ostream &ostrm);
    virtual bool doInit();
    virtual void doClear();

    // Get the specified field data.
    String getString(int fieldOffset, size_t maxFiledSize) const;

    // Assign new value to the field.
    bool setString(int fieldOffset, int maxFiledSize, const String &val);

private:

    enum {
        TAG_DATA_SIZE = 128,
    };

    char m_data[TAG_DATA_SIZE];
};
}
