/***************************************************************
 * Name:      VdkHTTP.h
 * Purpose:   wxHttpHelper 的实体类
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-17
 **************************************************************/
#pragma once
#include "wxCharsetHelper.h"

class wxMemoryOutputStream;

/// wxHttpHelper 的实体类
class VdkHTTP
{
public:

	/// 默认构造函数
	VdkHTTP(const wxCSConv& conv);

	/// 虚析构函数
	virtual ~VdkHTTP() {}

	/// @brief 检测所提供的字符串中类似于“charset=XXX”字串所代表的字符集
	///
	/// @a p 的来源可能是 HTTP Header 中的“Content-Type”，也可以是
	/// HTML 源码中的“meta”标签、XML 代码中的“encoding”属性。
	static wxString DetectCharset(const char* p);

	/// 设置系统本地默认多字节字符集转换器
	void SetAnsiLocalConv(const wxCSConv& conv);

	/// 获取对象绑定的系统本地默认多字节字符集转换器
	const wxMBConv& GetAnsiLocalConv() const;

	//================================================

	/// HTTP Get
	bool Get(const wxString& url, wxString& result);

	//================================================

	/// 控制 GZip 的启用与否
	enum GZipMode {
		GZM_ENABLED, ///< 启用 GZip
		GZM_DISABLED, ///< 禁用 GZip
	};

	/// 启用/禁用 GZip
	virtual void EnableGzip(GZipMode gzm) = 0;

	/// GZip 是否已然启用
	virtual bool IsGzipEnabled() const = 0;

	/// Set the default http timeout in seconds.
	virtual void SetTimeout(long seconds) = 0;

	/// Get the default http timeout in seconds.
	virtual long GetTimeout() const = 0;

	/// Sets data of a field to be sent during the next request 
	/// to the HTTP server.
	virtual void SetHeader(const wxString& header, const wxString& h_data) = 0;

	/// Returns the data attached with a field whose name is 
	/// specified by @a header. 
	virtual wxString GetHeader(const wxString& header) const = 0;

protected:

	/// Process the reponse from server
	wxString ParseReturnedData(wxMemoryOutputStream& out_stream);

private:

	// Decompress the returned data compressed by gzip.
	//
	// @return true if decompressed successfully.
	bool DecompressGZipData
		(void* p, size_t size, wxMemoryOutputStream& decompressed);

	// Convert the returned data to wxString properly.
	wxString EncodeResult(const char* p) const;

private:

	// HTTP Get
	virtual bool DoGet(const wxString& url, wxString& result) = 0;

private:

	wxCSConv m_ansiLocal;
};
