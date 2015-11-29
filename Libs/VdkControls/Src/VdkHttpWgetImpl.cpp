/***************************************************************
 * Name:      VdkHttpWgetImpl.cpp
 * Purpose:   使用 wget 来实现 VdkHTTP 的接口
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-03-17
 **************************************************************/
#include "StdAfx.h"
#include "VdkHttpWgetImpl.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkHttpWgetImpl::VdkHttpWgetImpl(wxFontEncoding defaultEncoding)
    : VdkHTTP(defaultEncoding), m_gzm(GZM_ENABLED) {

}

VdkHttpWgetImpl::~VdkHttpWgetImpl() {

}

bool VdkHttpWgetImpl::Get(const wxString &url, wxString &result) {
    wxString cmd(L"wget ");
    cmd += url;
    cmd += L" -O- -q";

    wxArrayString output;
    wxExecute(cmd, output);

    if (!output.empty()) {
        return true;
    }

    return false;
}

void VdkHttpWgetImpl::EnableGzip(GZipMode gzm) {
    m_gzm = gzm;
}

bool VdkHttpWgetImpl::IsGzipEnabled() const {
    return m_gzm == GZM_ENABLED;
}
