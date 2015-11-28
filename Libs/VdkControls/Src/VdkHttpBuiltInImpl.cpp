/***************************************************************
 * Name:      VdkHttpBuiltInImpl.cpp
 * Purpose:   使用内建的 wxHTTP 来实现 VdkHTTP 的接口
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-18
 **************************************************************/
#include "StdAfx.h"
#include "VdkHttpBuiltInImpl.h"

#include <wx/mstream.h>
#include <wx/url.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

// 格式化 wxHTTP 错误信息
static const wxChar *FormatProtocolError(wxProtocolError err);

VdkHttpBuiltInImpl::VdkHttpBuiltInImpl(wxFontEncoding defaultEncoding)
    : VdkHTTP(defaultEncoding), m_gzm(GZM_ENABLED) {

}

VdkHttpBuiltInImpl::~VdkHttpBuiltInImpl() {
    m_httpConn.Close();
}

bool VdkHttpBuiltInImpl::IsGzipEnabled() const {
    return m_gzm == GZM_ENABLED;
}

void VdkHttpBuiltInImpl::EnableGzip(GZipMode gzm) {
    m_gzm = GZM_ENABLED;
}

bool VdkHttpBuiltInImpl::DoGet(const wxString &strUrl, wxString &result) {
    enum {
        SECONDS_TO_SLEEP_BETWEEN_CONN = 1,
        MAX_RETRIES = 2,
    };

    wxURL url(strUrl);
    unsigned int port = 80;
    if (url.HasPort()) {
        port = wxAtoi(url.GetPort());
    }

    int retries = 0;

    // This will wait until the user connects to the internet.
    // It is important in case of dialup (or ADSL) connections.
    // only the server, no pages here yet...
    while (!m_httpConn.Connect(url.GetServer(), port)) {
        wxLogDebug(L"[%s:%d] wxHttp::Connect [Error: %s]", __FILE__, __LINE__,
                   FormatProtocolError(m_httpConn.GetError()));

        retries++;
        if (retries == MAX_RETRIES) {
            return false;
        }

        wxSleep(SECONDS_TO_SLEEP_BETWEEN_CONN);
    }

    //----------------------------------------------

    wxApp::IsMainLoopRunning(); // should return true

    // use "/" for index.html, index.php, default.asp, etc.
    wxString fullPath(url.GetPath());
    wxString query(url.GetQuery());
    if (!query.empty()) {
        fullPath += L'?' + query;
    }

    //****************************************

    wxInputStream *httpStream = m_httpConn.GetInputStream(fullPath);
    bool ret = true;

    if (m_httpConn.GetError() == wxPROTO_NOERR) {
        wxMemoryOutputStream out_stream;
        httpStream->Read(out_stream);

        result = ParseReturnedData(out_stream);
    } else {
        wxLogDebug(L"[%s:%d] Unable to connect! [Error: %s]",
                   __FILE__, __LINE__,
                   FormatProtocolError(m_httpConn.GetError()));

        ret = false;
    }

    wxDELETE(httpStream);
    return ret;
}

void VdkHttpBuiltInImpl::SetTimeout(long seconds) {
    m_httpConn.SetTimeout(seconds);
}

long VdkHttpBuiltInImpl::GetTimeout() const {
    return m_httpConn.GetTimeout();
}

void VdkHttpBuiltInImpl::SetHeader(const wxString &header, const wxString &h_data) {
    m_httpConn.SetHeader(header, h_data);
}

wxString VdkHttpBuiltInImpl::GetHeader(const wxString &header) const {
    return m_httpConn.GetHeader(header);
}

/*static*/
const wxChar *FormatProtocolError(wxProtocolError err) {
    switch (err) {
    case wxPROTO_NETERR:
        return L"A generic network error occurred.";
        break;

    case wxPROTO_PROTERR:
        return L"Protocol error";
        break;

    case wxPROTO_CONNERR:
        return L"Connection error";
        break;

    case wxPROTO_INVVAL:
        return L"Invalid value";
        break;

    case wxPROTO_NOHNDLR:
        return L"No handler";
        break;

    case wxPROTO_NOFILE:
        return L"No file";
        break;

    case wxPROTO_ABRT:
        return L"Last action aborted";
        break;

    case wxPROTO_RCNCT:
        return L"An error occurred during reconnection";
        break;

    case wxPROTO_NOERR:
        return L"No error";
        break;

    default:
        return L"Unknown error";
        break;
    }
}
