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

VdkHTTP::VdkHTTP(const wxCSConv& conv)
	: m_ansiLocal( conv )
{

}

void VdkHTTP::SetAnsiLocalConv(const wxCSConv& conv)
{
	m_ansiLocal = conv;
}

const wxMBConv& VdkHTTP::GetAnsiLocalConv() const
{
	return m_ansiLocal;
}

/*static*/
wxString VdkHTTP::DetectCharset(const char* p)
{
	wxASSERT( p );

	const char* first = strstr( p, "charset=" );
	if( !first )
	{
		first = strstr( p, "encoding=\"" ); // XML 格式
		if( first )
		{
			first += 10;
		}
	}
	else
	{
		first += 8;
	}

	//================================================
	// 截取字符集名称

	if( first )
	{
		const char* last = first;
		while( isalpha( *last ) || isdigit( *last ) || *last == '-' )
			last++;

		const static size_t s_bufSize = 16;
		size_t lenRange = last - first;

		if( lenRange < s_bufSize )
		{
			char charset[s_bufSize];
			strncpy( charset, first, lenRange );
			charset[lenRange] = 0;

			for( char* p = charset; p < charset + lenRange; p++ )
			{
				if( isalpha( *p ) && !isupper( *p ) )
				{
					*p = _toupper( *p );
				}
			}

			return charset;
		}

	}

	return wxEmptyString;
}

bool VdkHTTP::Get(const wxString& url, wxString& result)
{
	SetHeader( L"User-Agent", L"Mozilla/4.0" );

	if( IsGzipEnabled() )
	{
		SetHeader( L"Accept-Encoding", L"gzip, deflate" );
	}

    // 先清空，避免残留
    result.clear();
	return DoGet( url, result );
}

bool VdkHTTP::DecompressGZipData
	(void* p, size_t size, wxMemoryOutputStream& decompressed)
{
	bool gzipped = GetHeader( L"Content-Encoding" ).CmpNoCase( L"gzip" ) == 0;
	
	if( gzipped )
	{
		wxMemoryInputStream mis( p, size );
		wxZlibInputStream zis( mis, wxZLIB_GZIP );
		zis.Read( decompressed );

		return true;
	}

	return false;
}

wxString VdkHTTP::EncodeResult(const char* p) const
{
	wxString charset( DetectCharset( GetHeader( L"Content-Type" ) ) );
	if( charset.empty() )
		charset.assign( DetectCharset( p ) );

	if( charset.empty() )
	{
		wxLogDebug( L"页面没有指定任何字符集，默认使用本地多字节字符集" );
		return wxString( p, GetAnsiLocalConv() );
	}
	else
	{
		return wxString( p, wxCSConv( charset ) );
	}
}

wxString VdkHTTP::ParseReturnedData(wxMemoryOutputStream& out_stream)
{
	wxFileOffset writeCount = out_stream.TellO();
	if( writeCount == 0 )
	{
		wxLogDebug( L"[VdkHTTP::ParseReturnedData]No data input." );
		return wxEmptyString;
	}

	wxStreamBuffer* buf = out_stream.GetOutputStreamBuffer();
	const char* p = (const char *) buf->GetBufferStart();
	char zero = 0;

	// bad trick
	wxMemoryOutputStream decompressed;
	if( DecompressGZipData( (void *) p, writeCount, decompressed ) )
	{
		decompressed.Write( &zero, sizeof( zero ) );

		buf = decompressed.GetOutputStreamBuffer();
		p = (const char *) buf->GetBufferStart();
	}
	else
	{
		out_stream.Write( &zero, sizeof( zero ) );
	}

	return p ? EncodeResult( p ) : wxString();
}
