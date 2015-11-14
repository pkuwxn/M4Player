/***************************************************************
 * Name:      Playlist.cpp
 * Purpose:   播放列表实体类
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-04-19
 **************************************************************/
#include "StdAfx.h"
#include "Playlist.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

/*static*/
Playlist& Playlist::GetInstance()
{
	static Playlist sPlaylist;
	return sPlaylist;
}
