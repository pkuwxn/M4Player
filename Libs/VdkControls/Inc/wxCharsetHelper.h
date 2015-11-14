/***************************************************************
 * Name:      wxCharsetHelper.h
 * Purpose:   字符集便利函数
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-8-20
 **************************************************************/
#pragma once

/// 字符集便利静态工具类
class wxCharsetHelper
{
public:

	/// 要进行的 HTTP 操作（GET/POST）
	enum UrlAction {
		UA_GET,
		UA_POST,
	};

	/// @brief 转义将要 HTTP GET/POST 的数据
	/// @param str 要转义的字符串
	/// @parma conv 要转义成的字符集转换器
	static wxString Escape(const wxString& str, const wxMBConv& conv);

	/// @brief 将 URL 编码转换回成 GB2312 格式的字符串
	static wxString UnescapeFromAnsi(const char* str);

	/// Utf8 -> wx Unicode
	static wxString UnescapeFromUtf8(const char* src);

private:

	// 将 URL 编码格式的 %XX 转换成 char 的辅助函数
	static char StrToBin(const char* str);
	static char CharToInt(const char ch);
};

// 获取 GBK 的 wxCSConv 对象
#ifdef __WXMSW__
#   define GetGbkConv() wxMBConv& conv = wxConvLocal
#else
#   define GetGbkConv() \
	wxCSConv gb18030( L"gb18030" ); \
	wxMBConv& conv = gb18030
#endif
