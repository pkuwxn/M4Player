/***************************************************************
 * Name:      VdkHTTP.h
 * Purpose:   wxHttpHelper 的实体类
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-19
 **************************************************************/
#include "StdAfx.h"
#include "VdkHTTP.h"

#include <wx/mstream.h>
#include <wx/zstream.h>

#ifndef _MSC_VER
#   define _stricmp strcasecmp
#endif

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkHTTP::VdkHTTP(wxFontEncoding defaultEncoding)
    : m_defaultConv(defaultEncoding) {

}

void VdkHTTP::SetDefaultEncoding(wxFontEncoding defaultEncoding) {
    m_defaultConv = defaultEncoding;
}

const wxMBConv &VdkHTTP::GetDefaultCSConv() const {
    return m_defaultConv;
}

/*static*/
wxString VdkHTTP::DetectCharset(const char *p) {
    wxASSERT(p);

    const char *first = strstr(p, "charset=");
    if (!first) {
        first = strstr(p, "encoding=\"");   // XML 格式
        if (first) {
            first += 10;
        }
    } else {
        first += 8;
    }

    //================================================
    // 截取字符集名称

    if (first) {
        const char *last = first;
        while (isalpha(*last) || isdigit(*last) || *last == '-') {
            last++;
        }

        const static size_t s_bufSize = 16;
        size_t lenRange = last - first;

        if (lenRange < s_bufSize) {
            char charset[s_bufSize];
            strncpy(charset, first, lenRange);
            charset[lenRange] = 0;

            for (char *p = charset; p < charset + lenRange; p++) {
                if (isalpha(*p) && !isupper(*p)) {
                    *p = _toupper(*p);
                }
            }

            return charset;
        }

    }

    return wxEmptyString;
}

bool VdkHTTP::Get(const wxString &url, wxString &result) {
    SetHeader(L"User-Agent", L"Mozilla/5.0 (Windows NT 10.0; WOW64; rv:41.0) Gecko/20100101 Firefox/42.0");
    SetHeader(L"Accept-Language", L"zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3");
    SetHeader(L"Cache-Control", L"no-cache");

    if (IsGzipEnabled()) {
        SetHeader(L"Accept-Encoding", L"gzip");
    }

    // 先清空，避免残留
    result.clear();

    return DoGet(url, result);
}

bool VdkHTTP::DecompressGZipData
(void *p, size_t size, wxMemoryOutputStream &decompressed) {
    bool gzipped = GetHeader(L"Content-Encoding").CmpNoCase(L"gzip") == 0;

    if (gzipped) {
        wxMemoryInputStream mis(p, size);
        wxZlibInputStream zis(mis, wxZLIB_GZIP);
        zis.Read(decompressed);

        return true;
    }

    return false;
}

wxString VdkHTTP::EncodeResult(const char *p, size_t len) const {
    wxString charset(DetectCharset(GetHeader(L"Content-Type")));
    if (charset.empty()) {
        charset.assign(DetectCharset(p));
    }

    if (charset.empty()) {
        wxLogDebug(L"页面没有指定任何字符集，默认使用本地多字节字符集");
        return wxString(p, GetDefaultCSConv(), len);
    } else {
        return wxString(p, wxCSConv(charset), len);
    }
}

wxString VdkHTTP::ParseReturnedData(wxMemoryOutputStream &out_stream) {
    wxFileOffset writeCount = out_stream.TellO();
    if (writeCount == 0) {
        wxLogDebug(L"[VdkHTTP::ParseReturnedData] No data input.");
        return wxEmptyString;
    }

    wxStreamBuffer *buf = out_stream.GetOutputStreamBuffer();
    const char *p = (const char *) buf->GetBufferStart();
    char zero = 0;

    // bad trick
    wxMemoryOutputStream decompressed;
    if (DecompressGZipData((void *) p, writeCount, decompressed)) {
        writeCount = decompressed.TellO();
        buf = decompressed.GetOutputStreamBuffer();
        p = (const char *) buf->GetBufferStart();
    }

    return p ? EncodeResult(p, static_cast<size_t>(writeCount)) : wxString();
}
