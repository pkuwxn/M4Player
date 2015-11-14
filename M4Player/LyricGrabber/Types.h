/***************************************************************
 * Name:      LyricHost.cpp
 * Purpose:   ��ʷ�����
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-25
 **************************************************************/
#pragma once
#include "wxCharsetHelper.h"
#include "VdkHttpThread.h"

#include <wx/vector.h>

namespace LyricGrabber
{
	/*! �������ϲ��Ҹ��ʱ��ƥ���� */
	struct SearchResult
	{
		wxString artist; /*! ƥ��������� */
		wxString title; /*! ƥ��ı��� */
		wxString url; /*! ����ļ� URL */
	};

	typedef wxVector< SearchResult > ResultSet;
}
