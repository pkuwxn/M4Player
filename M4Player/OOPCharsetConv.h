/***************************************************************
 * Name:      OOPCharsetConv.h
 * Purpose:   ʹ�� wxCSConv ʵ�� MyTagLib::Charset �ӿ�
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-30
 **************************************************************/
#pragma once
#include "Charset.h"

/// ʹ�� wxCSConv ʵ�� MyTagLib::Charset �ӿ�
class OOPCharsetConv : public MyTagLib::Charset
{
public:

	/// �ھ�̬ջ�ռ��ϴ����������
	static OOPCharsetConv& CreateInstance();

private:

	virtual MyTagLib::String multibyteToWide(const char* mb, size_t len);
	virtual MyTagLib::String utf16ToWide(const char* utf16, size_t numBytes);
	virtual MyTagLib::String utf8ToWide(const char* utf8, size_t numBytes);
	virtual char* wideToUtf16(const MyTagLib::String& wide);
	virtual char* wideToUtf8(const MyTagLib::String& wide);
	virtual char* wideToMultibyte(const MyTagLib::String& wide);
};
