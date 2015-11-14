/***************************************************************
 * Name:      VdkHttpWgetImpl.cpp
 * Purpose:   ʹ�� wget ��ʵ�� VdkHTTP �Ľӿ�
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-17
 **************************************************************/
#include "StdAfx.h"
#include "VdkHttpWgetImpl.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkHttpWgetImpl::VdkHttpWgetImpl(const wxCSConv& conv)
	: VdkHTTP( conv ), m_gzm( GZM_ENABLED )
{

}

VdkHttpWgetImpl::~VdkHttpWgetImpl()
{

}

bool VdkHttpWgetImpl::Get(const wxString& url, wxString& result)
{
	wxString cmd( L"wget " );
	cmd += url;
	cmd += L" -O- -q";

	wxArrayString output;
	wxExecute( cmd, output );

	if( !output.empty() )
	{
		return true;
	}

	return false;
}

void VdkHttpWgetImpl::EnableGzip(GZipMode gzm)
{
	m_gzm = gzm;
}

bool VdkHttpWgetImpl::IsGzipEnabled() const
{
	return m_gzm == GZM_ENABLED;
}
