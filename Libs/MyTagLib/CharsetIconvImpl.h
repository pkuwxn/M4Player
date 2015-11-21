/***************************************************************
 * Name:      CharsetIconvImpl.cpp
 * Purpose:   Text charset converter using libiconv
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-22
 **************************************************************/
#pragma once
#include "Charset.h"

/// Text charset converter
class CharsetIconvImpl : public MyTagLib::Charset {
private:

    virtual MyTagLib::String multibyteToWide(const char *mb, size_t len);
    virtual MyTagLib::String utf16ToWide(const char *utf16, size_t numBytes);
    virtual MyTagLib::String utf8ToWide(const char *utf8, size_t numBytes);
    virtual char *wideToUtf16(const MyTagLib::String &wide);
    virtual char *wideToUtf8(const MyTagLib::String &wide);
    virtual char *wideToMultibyte(const MyTagLib::String &wide);
};
