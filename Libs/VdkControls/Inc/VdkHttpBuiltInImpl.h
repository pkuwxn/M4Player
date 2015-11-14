/***************************************************************
 * Name:      VdkHttpBuiltInImpl.h
 * Purpose:   ʹ���ڽ��� wxHttp ��ʵ�� VdkHTTP �Ľӿ�
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-18
 **************************************************************/
#pragma once
#include "VdkHTTP.h"
#include <wx/protocol/http.h>

/// ʹ���ڽ��� wxHttp ��ʵ�� VdkHTTP �Ľӿ�
class VdkHttpBuiltInImpl : public VdkHTTP
{
public:

	/// ���캯��
	VdkHttpBuiltInImpl(const wxCSConv& conv);

	/// ����������
	virtual ~VdkHttpBuiltInImpl();

	//====================================================

	/// ����/���� GZip
	virtual void EnableGzip(GZipMode gzm);

	/// GZip �Ƿ���Ȼ����
	virtual bool IsGzipEnabled() const;

	/// Set the default http timeout in seconds.
	virtual void SetTimeout(long seconds);

	/// Get the default http timeout in seconds.
	virtual long GetTimeout() const;

	/// Sets data of a field to be sent during the next request 
	/// to the HTTP server.
	virtual void SetHeader(const wxString& header, const wxString& h_data);

	/// Returns the data attached with a field whose name is 
	/// specified by @a header. 
	virtual wxString GetHeader(const wxString& header) const;

private:

	// HTTP Get
	virtual bool DoGet(const wxString& url, wxString& result);

private:

	wxHTTP m_httpConn;
	GZipMode m_gzm;
};
