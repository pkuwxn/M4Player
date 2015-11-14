/***************************************************************
 * Name:      LyricPanel.cpp
 * Purpose:   定义 LyricPanel
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-2-27
 **************************************************************/
#include "StdAfx.h"
#include "LyricPanel.h"

#include "OOPApp.h"
#include "OOPLyricSchDlg.h"
#include "Lyric/OOPLyric.h"

#include "VdkMenu.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

extern void AssignToolTip(VdkControl* pCtrl, const wxString& strToolTip);
extern OOPlayerApp* g_app;

LyricPanel::LyricPanel(wxWindow* parent)
	: OOPWindow( parent, L"20772658-9A2B-4AE1-BDEC-5BE1823A223E", L"LyricPanel" ),
	  m_schDlg( NULL ), m_lyric( NULL ), m_menuId( VdkGetUniqueId() )
{
	SetSkin();

	// 绑定菜单预显示事件
	Bind( wxEVT_VOBJ, &LyricPanel::OnPreShowMenu, this, m_menuId );
}

void LyricPanel::DoSetSkin()
{
	FromXrc( L"lyric_window.xml" );
	BindCloseBtnEvent();

	AssignToolTip( FindCtrl( L"desklrc" ), L"桌面歌词" );
	AssignToolTip( FindCtrl( L"close" ), L"关闭" );
	AssignToolTip( FindCtrl( L"ontop" ), L"总在最前" );

	m_lyric = (OOPLyric *) FindCtrl( L"OOPLyric" );
	if( m_lyric )
	{
		GenerateMenu();
	}
}

void LyricPanel::PopupSchDlg(OOPSongPtr song)
{
	if( !m_schDlg )
	{
		m_schDlg = new OOPLyricSchDlg( this );
	}

	m_schDlg->Popup( song );
}

void LyricPanel::OnPreShowMenu(VdkVObjEvent& e)
{
	VdkMenu* menu = e.GetMenu();
	VdkCtrlId operationsOnLyric[] = {
		OLRCM_COPY_LRC, OLRCM_REMOVE_LRC
	};

	int count = sizeof( operationsOnLyric ) / sizeof( operationsOnLyric[0] );
	for( int i = 0; i < count; i++ )
	{
		int id = operationsOnLyric[i] - OLRCM_LOWER_BOUND;
		menu->GetItem( id )->disabled( !m_lyric->IsOk() );
	}

	//=================================================
	// 根据当前播放状态控制菜单项的显示

	bool stopped = g_app->IsStopped();

	VdkCtrlId operationsOnLyric2[] = {
		OLRCM_SEARCH, OLRCM_ASSOCIATE_LOCAL_FILE, OLRCM_RELOAD_LRC
	};

	count = sizeof( operationsOnLyric2 ) / sizeof( operationsOnLyric2[0] );
	for( int i = 0; i < count; i++ )
	{
		int id = operationsOnLyric2[i] - OLRCM_LOWER_BOUND;
		menu->GetItem( id )->disabled( stopped );
	}
}
