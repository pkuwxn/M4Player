/***************************************************************
 * Name:      wxCharsetHelper.cpp
 * Purpose:   字符集便利函数
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-8-20
 **************************************************************/
#include "StdAfx.h"
#include "wxCharsetHelper.h"

#include <string> // Used in UnescapeFromAnsi()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

int isurlable(int ch)
{
	return ( isalnum( ch ) ||
			 ch == ':' ||
			 ch == '/' ||
			 ch == '.' );
}

wxString DoEscape(const char* p, size_t len)
{
    if( !p )
    {
        wxLogDebug( L"[%s:%d]DoEscape():Incorrect input: "
                    L"Null string pointer.",
                    __FILE__, __LINE__ );
                    
        return wxEmptyString;
    }

	if( len == wxNO_LEN )
	{
		len = strlen( p );
	}

	//-------------------------------------------------

	typedef unsigned char byte;

	char tempbuff[4];
	wxString ret;

	for( size_t i = 0; i < len; i++ )
	{
		if( isurlable( (byte) p[i] ) )
		{
			ret += (wxChar) p[i];
		}
		else
		{
			sprintf( tempbuff, "%%%X%X", ((byte *) p)[i] >> 4, 
										 ((byte *) p)[i]  % 16 );

			ret += tempbuff;
		}

	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////

wxString wxCharsetHelper::Escape(const wxString& str, const wxMBConv& conv)
{
	if( str.empty() )
		return str;

	return ::DoEscape( conv.cWC2MB( str ), wxNO_LEN );
}

wxString wxCharsetHelper::UnescapeFromAnsi(const char* str)
{
	// TODO:
	std::string output;
	char tmp[2];

	size_t i = 0;
	size_t len = strlen( str );

	while( i < len )
	{
		if( str[i] == '%' )
		{
			tmp[0] = str[i + 1];
			tmp[1] = str[i + 2];
			output += StrToBin( tmp );
			i = i + 3;
		}
		else if( str[i] == '+' )
		{
			output += ' ';
			i++;
		}
		else
		{
			output += str[i];
			i++;
		}
	}

	return output;
}

char wxCharsetHelper::StrToBin(const char* str)
{
	char tempWord[2];
	char chn;

	tempWord[0] = CharToInt( str[0 ]);           // make the B to 11 -- 00001011
	tempWord[1] = CharToInt( str[1] );           // make the 0 to 0  -- 00000000

	chn = (tempWord[0] << 4) | tempWord[1];    // to change the BO to 10110000

	return chn;
}

char wxCharsetHelper::CharToInt(const char ch)
{
	if( ch>='0' && ch<='9' ) return (char)( ch - '0' );
	if( ch>='a' && ch<='f' ) return (char)( ch - 'a' + 10 );
	if( ch>='A' && ch<='F' ) return (char)( ch - 'A' + 10 );

	// TODO:
	return -1;
}

int Hex2Dec(char ch)
{
	ch = toupper( ch );
	if( ch >= L'0' && ch <= L'9' )
	{
		return ch - L'0';
	}
	else
	{
		return ch - L'A' + 10;
	}
}

wxString wxCharsetHelper::UnescapeFromUtf8(const char* src)
{
	if( !src || !strchr( src, '%' ) )
		return src;

    size_t len = strlen( src );
    char* buf = new char[len + 1];

    const char* psrc = src;
    const char* plast = src + len;
    unsigned char* pbuf = (unsigned char*) buf;

	int dec;

    //*************************

    while( psrc != plast )
    {
        while( psrc != plast && *psrc != '%' )
            *pbuf++ = *psrc++;

		if( psrc == plast )
			break;

		dec = Hex2Dec( *(psrc + 2) );
		dec |= Hex2Dec( *(psrc + 1) ) << 4;
        *pbuf++ = dec;
        psrc += 3;
    }

    *pbuf = 0;
	wxString ret( buf, wxConvUTF8 );
    delete [] buf;

    return ret;
}
