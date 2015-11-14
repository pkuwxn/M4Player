/***************************************************************
 * Name:      VdkHTTP.h
 * Purpose:   wxHttpHelper ��ʵ����
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-17
 **************************************************************/
#pragma once
#include "wxCharsetHelper.h"

class wxMemoryOutputStream;

/// wxHttpHelper ��ʵ����
class VdkHTTP
{
public:

	/// Ĭ�Ϲ��캯��
	VdkHTTP(const wxCSConv& conv);

	/// ����������
	virtual ~VdkHTTP() {}

	/// @brief ������ṩ���ַ����������ڡ�charset=XXX���ִ���������ַ���
	///
	/// @a p ����Դ������ HTTP Header �еġ�Content-Type����Ҳ������
	/// HTML Դ���еġ�meta����ǩ��XML �����еġ�encoding�����ԡ�
	static wxString DetectCharset(const char* p);

	/// ����ϵͳ����Ĭ�϶��ֽ��ַ���ת����
	void SetAnsiLocalConv(const wxCSConv& conv);

	/// ��ȡ����󶨵�ϵͳ����Ĭ�϶��ֽ��ַ���ת����
	const wxMBConv& GetAnsiLocalConv() const;

	//================================================

	/// HTTP Get
	bool Get(const wxString& url, wxString& result);

	//================================================

	/// ���� GZip ���������
	enum GZipMode {
		GZM_ENABLED, ///< ���� GZip
		GZM_DISABLED, ///< ���� GZip
	};

	/// ����/���� GZip
	virtual void EnableGzip(GZipMode gzm) = 0;

	/// GZip �Ƿ���Ȼ����
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
