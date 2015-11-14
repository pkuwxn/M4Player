/***************************************************************
 * Name:      OOPCharsetConv.cpp
 * Purpose:   使用 wxCSConv 实现 MyTagLib::Charset 接口
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-30
 **************************************************************/
#include "StdAfx.h"
#include "OOPCharsetConv.h"
#include "wxCharsetHelper.h" // for GetGbkConv()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

OOPCharsetConv& OOPCharsetConv::CreateInstance()
{
	static OOPCharsetConv sConv;
	return sConv;
}

size_t CalcMultibyteLength(const char* mb, size_t maxLen)
{
	size_t len = 0;
	while( len < maxLen )
	{
		if( mb[len] == 0 )
			break;

		len++;
	}

	return len;
}

MyTagLib::String OOPCharsetConv::multibyteToWide(const char* mb, size_t maxLen)
{
    GetGbkConv();

	size_t len = CalcMultibyteLength( mb, maxLen );
	wxWCharBuffer buf( conv.cMB2WC( mb, len, NULL ) );

	return buf ? (const wchar_t *) buf : MyTagLib::EmptyString;
}

MyTagLib::String OOPCharsetConv::utf16ToWide(const char* utf16, size_t numBytes)
{
	wxWCharBuffer buf( wxMBConvUTF16().cMB2WC( utf16, numBytes, NULL ) );
	return buf ? (const wchar_t *) buf : MyTagLib::EmptyString;
}

MyTagLib::String OOPCharsetConv::utf8ToWide(const char* utf8, size_t numBytes)
{
	size_t len = numBytes;
	wxWCharBuffer buf( wxMBConvUTF8().cMB2WC( utf8, len, NULL ) );

	return buf ? (const wchar_t *) buf : MyTagLib::EmptyString;
}

char* OOPCharsetConv::wideToMultibyte(const MyTagLib::String& wide)
{
    GetGbkConv();
	return conv.cWC2MB( wide.c_str() ).release();
}

char* OOPCharsetConv::wideToUtf16(const MyTagLib::String& wide)
{
	return wxMBConvUTF16().cWC2MB( wide.c_str() ).release();
}

char* OOPCharsetConv::wideToUtf8(const MyTagLib::String& wide)
{
	return wxConvUTF8.cWC2MB( wide.c_str() ).release();
}
