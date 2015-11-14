/***************************************************************
 * Name:      LyricHost.cpp
 * Purpose:   歌词服务器
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-25
 **************************************************************/
#pragma once
#include "wxCharsetHelper.h"
#include "VdkHttpThread.h"

#include <wx/vector.h>

namespace LyricGrabber
{
	/*! 在网络上查找歌词时的匹配结果 */
	struct SearchResult
	{
		wxString artist; /*! 匹配的艺术家 */
		wxString title; /*! 匹配的标题 */
		wxString url; /*! 歌词文件 URL */
	};

	typedef wxVector< SearchResult > ResultSet;
}
