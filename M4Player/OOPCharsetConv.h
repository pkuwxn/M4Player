/***************************************************************
 * Name:      OOPCharsetConv.h
 * Purpose:   使用 wxCSConv 实现 MyTagLib::Charset 接口
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-30
 **************************************************************/
#pragma once
#include "Charset.h"

/// 使用 wxCSConv 实现 MyTagLib::Charset 接口
class OOPCharsetConv : public MyTagLib::Charset
{
public:

	/// 在静态栈空间上创建单体对象
	static OOPCharsetConv& CreateInstance();

private:

	virtual MyTagLib::String multibyteToWide(const char* mb, size_t len);
	virtual MyTagLib::String utf16ToWide(const char* utf16, size_t numBytes);
	virtual MyTagLib::String utf8ToWide(const char* utf8, size_t numBytes);
	virtual char* wideToUtf16(const MyTagLib::String& wide);
	virtual char* wideToUtf8(const MyTagLib::String& wide);
	virtual char* wideToMultibyte(const MyTagLib::String& wide);
};
