/***************************************************************
 * Name:      OOPLyricParser.cpp
 * Purpose:   ���� LRC/QRC/... ����ļ�
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-05
 **************************************************************/
#include "StdAfx.h"
#include "OOPLyricParser.h"

#include "wxUtil.h" // for IsEncodedInUtf8()��ReadAll()

#include <wx/wfstream.h> // for wxFFile
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wx/tokenzr.h> // for wxStringTokenizer

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

LineInfo::LineInfo(const wxString& strLyric, int nStartTime)
		: m_strLyric( strLyric ),
		  m_nStartTime( nStartTime ),
		  m_nMilSeconds( 0 )
{

}

void LineInfo::CalcYourTime(const LineInfo* next)
{
	m_nMilSeconds = next->m_nStartTime - m_nStartTime;
}

int LineInfo::IsBefore(const LineInfo** p1, const LineInfo** p2)
{
	return (*p1)->m_nStartTime - (*p2)->m_nStartTime;
}

//////////////////////////////////////////////////////////////////////////

#include <wx/listimpl.cpp>
WX_DEFINE_LIST( _ListOfLineInfo );

//////////////////////////////////////////////////////////////////////////

OOPLyricParser::OOPLyricParser()
	: m_nTimeSum( 0 )
{

}

OOPLyricParser::~OOPLyricParser()
{
	Clear();
}

void OOPLyricParser::Clear()
{
	m_lines.DeleteContents( true );
	m_lines.Clear();

	m_nTimeSum = 0;
}

bool OOPLyricParser::LoadFile(const wxString& strLrcFile, int nTimeSum)
{
	wxASSERT( nTimeSum > 0 );

	wxString lyric( LoadFile( strLrcFile ) );
	if( !lyric.empty() )
	{
		return Load( lyric, nTimeSum );
	}

	return false;
}

/*static*/
wxString OOPLyricParser::LoadFile(const wxString& strLrcFile)
{
	if( wxFileExists( strLrcFile  ) )
	{
		wxFFile input( strLrcFile );
		if( !input.Error() )
		{
			wxString lyric;

			// ����Ƿ��� UTF-8 ��ʽ����
			char buf[3];
			input.Read( buf, sizeof buf );
			// �����ļ�ָ��
			input.Seek( 0 );

			if( IsEncodedInUtf8( buf ) )
			{
				input.ReadAll( &lyric );
			}
			else
			{
				input.ReadAll( &lyric, wxConvAuto( wxFONTENCODING_GB2312 ) );
			}

			return lyric;
		}
	}

	wxLogDebug( L"Cannot load LRC file.\n`%s`", strLrcFile );

	return wxEmptyString;
}

bool OOPLyricParser::Load(const wxString& lyric, int nTimeSum)
{
	wxASSERT( nTimeSum > 0 );

	Clear();

	if( !lyric.empty() )
	{
		wxStringTokenizer tokenizer( lyric, L"\r\n" );
		while( tokenizer.HasMoreTokens() )
		{
			wxString token = tokenizer.GetNextToken();
			ParseLine( token );
		}

		BuilidLines( nTimeSum );
	}
	else
	{
		wxLogError( L"����ַ���Ϊ��" );
	}

	return IsOk();
}

void OOPLyricParser::Load(wxInputStream& stream, int nTimeSum)
{
	wxASSERT( nTimeSum > 0 );
	wxASSERT( stream.IsOk() );

	Clear();

	//==================================================

	wxTextInputStream text( stream );
	while( !stream.Eof() )
	{
		ParseLine( text.ReadLine() );
	}

	BuilidLines( nTimeSum );
}

bool OOPLyricParser::IsOk() const
{
	return !m_lines.empty();
}

bool OOPLyricParser::BuilidLines(int nTimeSum)
{
	wxASSERT( nTimeSum > 0 );

	if( !IsOk() )
	{
		wxLogDebug( L"δʶ��ĸ���ַ���" );
		return false;
	}

	//-----------------------------------------------------

	m_lines.Sort( LineInfo::IsBefore );

	// ��һ�п�ʼǰ��ͣ�ټ�����ֶ�����һ������
	if( m_lines[0]->m_nStartTime > 0 )
	{
		LineInfo* beforeFirstLine = new LineInfo( wxEmptyString, 0 );
		m_lines.push_front( beforeFirstLine );
	}

	//-----------------------------------------------------
	// ��ȡ������ʾ��ʵ��ʱ��

	LineInfo* pInfo;
	int numLines = m_lines.size(), curr = 0;

	ListOfLineInfo::iterator i;
	for( i = m_lines.begin(); curr < numLines - 1; ++curr )
	{
		pInfo = *i;
		pInfo->CalcYourTime( *(++i) );
	}

	m_nTimeSum = nTimeSum;

	// �����һ�н������⴦��
	pInfo = *i;
	pInfo->m_nMilSeconds = m_nTimeSum - pInfo->m_nStartTime;
	if( pInfo->m_nMilSeconds < 0 )
	{
		// TODO: �᲻���ߵ����
		enum {
			// �����ʳ��ȱȸ�������Ҫ������ô���һ���ʳ�����ʱ��
			// Ĭ��ΪһС��ʱ��
			DEFAULT_LAST_LINE_LAST = 1000,
		};

		pInfo->m_nMilSeconds = DEFAULT_LAST_LINE_LAST;
	}

	return true;
}

void OOPLyricParser::ParseLine(const wxString& line)
{
	wxString strLine( line );
	if( !strLine.Trim( true ).Trim( false ).empty() )
	{
		strLine.Replace( L"��", L":" );
	}
	else
	{
		return;
	}

	wxString strTime, strNum, strLyric;
	wxString::size_type startPos = 0, endPos = -1;

	endPos = strLine.Find( L']', true );
	// ��ʹ�������Ϊ��Ҳ��������Ϊ���ܽ������һ���������Էָ�
	if( endPos != wxString::npos )
	{
		strLyric = strLine.Mid( endPos + 1 );
	}
	else // ��Ч��(û��ʱ���ǩ�������á�[����]������������)
	{
		return;
	}

	endPos = -1;
	while( true )
	{
		startPos = strLine.find( L'[', startPos );

		// ʱ���ǩ������������
		if( (startPos == wxString::npos) || (startPos != endPos + 1) )
		{
			break;
		}

		endPos = strLine.find( L']', startPos );
		if( endPos != wxString::npos )
		{
			startPos++;
			strTime = strLine.Mid( startPos, endPos - startPos );
		}
		else // TODO: ��ʽ����
		{
			break;
		}

		// ��ʽ����
		if( (strTime.length() != 8) && (strTime.length() != 5) )
			break;

		// ���ʱ���ǩ�ĺϷ���
		const static wchar_t* sLegalChars = L"0123456789:.";

		bool ok = true;
		wxString::size_type len = strTime.length();
		for( wxString::size_type i = 0; i < len; i++ )
		{
			if( !wcschr( sLegalChars, strTime[i] ) )
			{
				ok = false;
				break;
			}
		}

		if( !ok )
			break;

		//-----------------------------------------------------
		// 00:32.04

		long nStartTime = 0, nNum;

		strNum = strTime.Mid( 0, 2 );
		strNum.ToLong( &nNum );
		nStartTime += nNum * 60 * 1000;

		strNum = strTime.Mid( 3, 2 );
		strNum.ToLong( &nNum );
		nStartTime += nNum * 1000;

		// #(00:32.04) == 8
		if( strTime.length() == 8 )
		{
			strNum = strTime.Mid( 6, 2 );
			strNum.ToLong( &nNum );
			nStartTime += nNum * 10;
		}

		LineInfo* info = new LineInfo( strLyric, nStartTime );
		m_lines.Append( info );

		startPos = endPos;

	} // END while( true )
}

wxString OOPLyricParser::GetLyric() const
{
	wxString lyricAll;

	LineIter it( begin() ), e( end() );
	for( ; it != e; ++it )
		lyricAll += (*it)->m_strLyric + L"\r\n"; // TODO: ʹ��ϵͳԭ�����з�

	return lyricAll;
}

size_t OOPLyricParser::GetMaxTextWidth(const wxFont& font) const
{
	size_t m_nMaxTextWidth = 0;

	wxMemoryDC mdc;
	mdc.SetFont( font );

	LineIter it( begin() ), e( end() );
	for( ; it != e; ++it )
	{
		wxCoord w;
		mdc.GetTextExtent( (*it)->m_strLyric, &w, NULL );

		if( size_t( w ) > m_nMaxTextWidth )
			m_nMaxTextWidth = w;
	}

	return m_nMaxTextWidth;
}

ListOfLineInfo::size_type OOPLyricParser::IndexOf(LineIter it) const
{
	return m_lines.IndexOf( *it );
}

LineIter OOPLyricParser::GetLine(ListOfLineInfo::size_type index) const
{
	wxASSERT( index < GetLinesCount() );

	LineIter it( m_lines.begin() );
	for( size_t i = 0; i < index; i++ )
		++it;

	return it;
}

LineIter OOPLyricParser::WhichLine(size_t timeOffset) const
{
	if( IsOk() )
	{
		LineIter i( begin() ), j( i );
		for( ++j; j != end(); ++i, ++j )
		{
			if( ((*i)->GetStartTime() <= timeOffset) && 
				((*j)->GetStartTime() > timeOffset) )
			{
				return i;
			}
		}

		// �����һ�������⴦��
		if( (*i)->GetStartTime() <= timeOffset ) 
			return i;
	}

	return end();
}
