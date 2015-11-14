/***************************************************************
 * Name:      wxCharsetHelper.h
 * Purpose:   �ַ�����������
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-8-20
 **************************************************************/
#pragma once

/// �ַ���������̬������
class wxCharsetHelper
{
public:

	/// Ҫ���е� HTTP ������GET/POST��
	enum UrlAction {
		UA_GET,
		UA_POST,
	};

	/// @brief ת�彫Ҫ HTTP GET/POST ������
	/// @param str Ҫת����ַ���
	/// @parma conv Ҫת��ɵ��ַ���ת����
	static wxString Escape(const wxString& str, const wxMBConv& conv);

	/// @brief �� URL ����ת���س� GB2312 ��ʽ���ַ���
	static wxString UnescapeFromAnsi(const char* str);

	/// Utf8 -> wx Unicode
	static wxString UnescapeFromUtf8(const char* src);

private:

	// �� URL �����ʽ�� %XX ת���� char �ĸ�������
	static char StrToBin(const char* str);
	static char CharToInt(const char ch);
};

// ��ȡ GBK �� wxCSConv ����
#ifdef __WXMSW__
#   define GetGbkConv() wxMBConv& conv = wxConvLocal
#else
#   define GetGbkConv() \
	wxCSConv gb18030( L"gb18030" ); \
	wxMBConv& conv = gb18030
#endif
