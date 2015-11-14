/***************************************************************
 * Name:      VdkHttpWgetImpl.h
 * Purpose:   ʹ�� wget ��ʵ�� VdkHTTP �Ľӿ�
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-17
 **************************************************************/
#pragma once
#include "VdkHTTP.h"

/// ʹ�� wget ��ʵ�� VdkHTTP �Ľӿ�
class VdkHttpWgetImpl : public VdkHTTP
{
public:

	/// ���캯��
	VdkHttpWgetImpl(const wxCSConv& conv);

	/// ����������
	virtual ~VdkHttpWgetImpl();

	/// HTTP Get
	virtual bool Get(const wxString& url, wxString& result);

	/// ����/���� GZip
	virtual void EnableGzip(GZipMode gzm);

	/// GZip �Ƿ���Ȼ����
	virtual bool IsGzipEnabled() const;

private:

	GZipMode m_gzm;
};
