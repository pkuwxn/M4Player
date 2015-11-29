/***************************************************************
 * Name:      CharsetIconvImpl.cpp
 * Purpose:   Text charset converter using libiconv
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-03-22
 **************************************************************/
#include "StdAfx.h"
#include "CharsetIconvImpl.h"

#include <iconv.h>
#include <assert.h>
#include <memory.h>

int code_convert(const char *from_charset,
                 const char *to_charset,
                 size_t *pInBytesLeft, size_t *pOutBytesLeft,
                 const char *inbuf, char *outbuf) {
    iconv_t cd;
    cd = iconv_open(to_charset, from_charset);
    if (cd == 0) {
        return -1;
    }

    memset(outbuf, 0, *pOutBytesLeft);
    char *dummy = const_cast<char *>(inbuf); // TODO: why not const char** ?
    if (iconv(cd, &dummy, pInBytesLeft, &outbuf, pOutBytesLeft) == (size_t) -1) {
        return -1;
    }

    iconv_close(cd);
    return 0;
}

const wchar_t *testUtf32BOM(const wchar_t *utf16) {

}

MyTagLib::String CharsetIconvImpl::multibyteToWide(const char *szAnsi, size_t len) {
    assert(szAnsi);
    assert(len);

    size_t nOutBytes = (len + 2) * 4; // TODO：这里为什么需要 +2 ？
    wchar_t *outbuf = new wchar_t[nOutBytes];

    code_convert("gb2312", "utf-32", &len, &nOutBytes,
                  szAnsi, (char *) outbuf);

    MyTagLib::String ret(outbuf + 1);
    delete [] outbuf;

    return ret;
}

MyTagLib::String CharsetIconvImpl::utf16ToWide(const char *utf16, size_t numBytes) {
    size_t nOutBytes = (numBytes + 4) * 2; // TODO：这里为什么需要 +4 ？
    wchar_t *outbuf = new wchar_t[nOutBytes];

    code_convert("utf-16", "utf-32", &numBytes, &nOutBytes,
                  utf16, (char *) outbuf);

    // It's extreamly strange that iconv will add UTF-16 BOM for the output string.
    MyTagLib::String ret(outbuf + 1);
    delete [] outbuf;

    return ret;
}

MyTagLib::String CharsetIconvImpl::utf8ToWide(const char *utf8, size_t numBytes) {
    size_t nOutBytes = (numBytes + 2) * 4; // TODO：这里为什么需要 +2 ？
    wchar_t *outbuf = new wchar_t[nOutBytes];

    code_convert("utf-8", "utf-32", &numBytes, &nOutBytes,
                  utf8, (char *) outbuf);

    MyTagLib::String ret(outbuf + 1);
    delete [] outbuf;

    return ret;
}

char *CharsetIconvImpl::wideToUtf16(const MyTagLib::String &wide) {
    size_t nInBytes = wide.length() * 4;
    size_t nOutBytes = nInBytes + sizeof(wchar_t);
    char *outbuf = new char[nOutBytes];

    code_convert("utf-32", "utf-16", &nInBytes, &nOutBytes,
                 (const char *) wide.c_str(),
                  outbuf);

    char *outbuf2 = new char[nOutBytes - 2];
    memcpy(outbuf2, outbuf + 2, nOutBytes - 2);

    delete [] outbuf;
    return outbuf2;
}

char *CharsetIconvImpl::wideToUtf8(const MyTagLib::String &wide) {
    size_t nInBytes = wide.length() * 4;
    size_t nOutBytes = nInBytes + sizeof(wchar_t);
    char *outbuf = new char[nOutBytes];

    code_convert("utf-32", "utf-8", &nInBytes, &nOutBytes,
                 (const char *) wide.c_str(),
                  outbuf);

    return outbuf;
}

char *CharsetIconvImpl::wideToMultibyte(const MyTagLib::String &wide) {
    size_t nInBytes = wide.length() * 4;
    size_t nOutBytes = nInBytes / 2 + 1;
    char *outbuf = new char[nOutBytes];

    code_convert("utf-32", "gb2312", &nInBytes, &nOutBytes,
                 (const char *) wide.c_str(),
                  outbuf);

    return outbuf;
}
