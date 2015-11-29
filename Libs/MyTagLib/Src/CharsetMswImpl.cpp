/***************************************************************
 * Name:      CharsetMswImpl.cpp
 * Purpose:   Text charset converter using Win32 SDK
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-3-19
 **************************************************************/
#include "StdAfx.h"
#include "CharsetMswImpl.h"

#include <Windows.h>

MyTagLib::String CharsetMswImpl::multibyteToWide(const char *szAnsi, size_t len) {
    assert(szAnsi);
    assert(len);

    int nWideLen = MultiByteToWideChar(CP_ACP, 0, szAnsi, len, 0, 0);
    wchar_t *szWideForm = new wchar_t[nWideLen + 1];

    MultiByteToWideChar(CP_ACP, 0, szAnsi, len, szWideForm, nWideLen);

    szWideForm[nWideLen] = 0;
    MyTagLib::String ret(szWideForm);
    delete [] szWideForm;

    return ret;
}

MyTagLib::String CharsetMswImpl::utf16ToWide(const char *utf16, size_t numBytes) {
    wchar_t *p = (wchar_t *) utf16;
    // TODO: Extra to a test case.
    return p[numBytes / 2 - 1]
           ? MyTagLib::String(p, p + numBytes / 2)
           : MyTagLib::String(p);
}

MyTagLib::String CharsetMswImpl::utf8ToWide(const char *utf8, size_t numBytes) {
    // TODO: Extra to a test case.
    if (utf8[numBytes - 1] == 0) {
        numBytes = strlen(utf8);
    }

    int nWideLen = MultiByteToWideChar(CP_UTF8, 0, utf8, numBytes, 0, 0);
    wchar_t *szWideForm = new wchar_t[nWideLen + 1];
    MultiByteToWideChar(CP_UTF8, 0, utf8, numBytes, szWideForm, nWideLen);

    szWideForm[nWideLen] = 0;
    MyTagLib::String ret(szWideForm);
    delete [] szWideForm;

    return ret;
}

char *CharsetMswImpl::wideToUtf8(const MyTagLib::String &wide) {
    int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0,
                                       wide.c_str(), wide.length(), 0,  0, NULL, NULL);

    char *szUtf8 = new char[nUtf8Len + 1];

    WideCharToMultiByte(CP_UTF8, 0,
                        wide.c_str(), wide.length(), szUtf8, nUtf8Len, NULL, NULL);
    szUtf8[nUtf8Len] = 0;

    return szUtf8;
}

char *CharsetMswImpl::wideToUtf16(const MyTagLib::String &wide) {
    size_t size = wide.length() * sizeof(wchar_t);
    char *szUtf16 = new char[size + 2];

    memcpy(szUtf16, wide.c_str(), size);
    szUtf16[size] = 0;
    szUtf16[size + 1] = 0;

    return szUtf16;
}

char *CharsetMswImpl::wideToMultibyte(const MyTagLib::String &wide) {
    int nAnsiLen = WideCharToMultiByte(CP_ACP, 0,
                                       wide.c_str(), wide.length(), 0,  0, NULL, NULL);
    char *szAnsi = new char[nAnsiLen + 1];

    WideCharToMultiByte(CP_ACP, 0,
                        wide.c_str(), wide.length(), szAnsi, nAnsiLen, NULL, NULL);
    szAnsi[nAnsiLen] = 0;

    return szAnsi;
}
