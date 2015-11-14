/***************************************************************
 * Name:      OOPStopWatch.cpp
 * Purpose:   播放引擎计时器
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-09
 **************************************************************/
#include "StdAfx.h"
#include "OOPStopWatch.h"
#include <Music.hpp>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

OOPStopWatch::OOPStopWatch
	(const sf::Music& music, MusicEndingDetector& endingDetector)
	: m_music( music ), m_endingDectector( endingDetector )
{

}

size_t OOPStopWatch::Time()
{
	m_endingDectector.Update();
	return m_music.getPlayingOffset().asMilliseconds();
}

bool OOPStopWatch::IsRunning() const
{
	return m_music.getStatus() == sf::Music::Playing;
}

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT( MM_EVT_SONG_FINISHED, wxCommandEvent );

MusicEndingDetector::MusicEndingDetector(const sf::Music& music)
	: m_music( music ), m_finishEventSent( true )
{

}

void MusicEndingDetector::Update()
{
	if( !m_finishEventSent && (m_music.getStatus() == sf::Music::Stopped) )
	{
		wxASSERT( wxTheApp );

		wxCommandEvent e( MM_EVT_SONG_FINISHED );
		wxPostEvent( wxTheApp, e );

		m_finishEventSent = true;
	}
}

void MusicEndingDetector::Play()
{
	m_finishEventSent = false;
}

void MusicEndingDetector::Stop()
{
	// 用户已经知晓播放已停止
	m_finishEventSent = true;
}
