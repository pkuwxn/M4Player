/***************************************************************
 * Name:      OOPLyricMgr.h
 * Purpose:   歌词秀管理器（内嵌窗口、桌面）
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-05
 **************************************************************/
#include "StdAfx.h"
#include "OOPLyricMgr.h"

#include "ILyric.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

OOPLyricMgr::OOPLyricMgr()
{

}

OOPLyricMgr::~OOPLyricMgr()
{

}

void OOPLyricMgr::Attach(ILyric* lyric)
{
	wxASSERT( lyric );

	m_lyrics.push_back( lyric );
}

void OOPLyricMgr::Dettach(ILyric* lyric)
{
	LyricVec::iterator it( m_lyrics.begin() );
	for( ; it != m_lyrics.end(); ++it )
	{
		if( *it == lyric )
		{
			m_lyrics.erase( it );
			break;
		}
	}
}

OOPLyricMgr::operator bool() const
{
	return !m_lyrics.empty();
}

//////////////////////////////////////////////////////////////////////////

bool OOPLyricMgr::IsOk() const
{
	LyricVec::const_iterator it( m_lyrics.begin() );
	for( ; it != m_lyrics.end(); ++it )
	{
		if( !(*it)->IsOk() )
			return false;
	}

	return true;
}

#define INVOKE( Exp ) \
	LyricVec::const_iterator it( m_lyrics.begin() ); \
	for( ; it != m_lyrics.end(); ++it ) \
		(*it)->Exp

void OOPLyricMgr::SetStopWatch(OOPStopWatch& sw)
{
	INVOKE( SetStopWatch( sw ) );
}

void OOPLyricMgr::AttachParser(const OOPLyricParser& parser)
{
	INVOKE( AttachParser( parser ) );
}

void OOPLyricMgr::Start()
{
	INVOKE( Start() );
}

void OOPLyricMgr::Pause()
{
	INVOKE( Pause() );
}

void OOPLyricMgr::Stop(wxDC* pDC)
{
	INVOKE( Stop( pDC ) );
}

void OOPLyricMgr::ClearLyric(wxDC* pDC)
{
	INVOKE( ClearLyric( pDC ) );
}

void OOPLyricMgr::GoTo(double percentage, wxDC* pDC, bool bPaused)
{
	INVOKE( GoTo( percentage, pDC, bPaused ) );
}

void OOPLyricMgr::SetDefualtInteractiveOutput(const wxString& msg)
{
	INVOKE( SetDefualtInteractiveOutput( msg ) );
}

void OOPLyricMgr::SetInteractiveOutput(const wxString& msg, wxDC* pDC)
{
	INVOKE( SetInteractiveOutput( msg, pDC ) );
}

void OOPLyricMgr::ResetInteractiveOutput(wxDC* pDC)
{
	INVOKE( ResetInteractiveOutput( pDC ) );
}
