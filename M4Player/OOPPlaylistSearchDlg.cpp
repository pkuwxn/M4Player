/***************************************************************
 * Name:      OOPPlaylistSearchDlg.cpp
 * Purpose:   “快速定位文件”对话框
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-11
 **************************************************************/
#include "StdAfx.h"
#include "OOPPlaylistSearchDlg.h"

#include "VdkEdit.h"
#include "wxUtil.h" // for wxEasyCreatFont()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT( OOP_EVT_PLAYLIST_SEARCH, wxCommandEvent );

OOPPlaylistSearchDlg::OOPPlaylistSearchDlg(VdkWindow* parent)
	: VdkDialog( parent->GetHandle(), L"快速定位文件", 0, 
				 VWS_DRAGGABLE | VWS_BASE_PANEL | VWS_DISMISS_BY_ESC ),
	  m_keyword( NULL )
{
	FromXrc( L"../../App/playlist_search_dlg.xml", NULL );
	m_keyword = (VdkEdit *) FindCtrl( L"keyword" );

	VdkCtrlId id = VdkGetUniqueId();
	VdkControl* searchButton = FindCtrl( L"search" );
	searchButton->SetID( id );

	Bind( wxEVT_VOBJ, &OOPPlaylistSearchDlg::OnSearchButton, this, id );
	
	// 覆写默认关闭事件处理器
	Bind( wxEVT_CLOSE_WINDOW, &OOPPlaylistSearchDlg::OnClose, this );
	Bind( wxEVT_SHOW, &OOPPlaylistSearchDlg::OnShow, this );
}

OOPPlaylistSearchDlg::~OOPPlaylistSearchDlg()
{

}

bool OOPPlaylistSearchDlg::DoHandleKeyEvent(wxKeyEvent& e)
{
	switch( e.GetKeyCode() )
	{
	case WXK_RETURN:
		{
			PostSearchEvent();
			return true;
		}

	default:

		break;
	}

	return false;
}

void OOPPlaylistSearchDlg::OnSearchButton(VdkVObjEvent&)
{
	PostSearchEvent();
}

wxString OOPPlaylistSearchDlg::GetKeyword() const
{
	return m_keyword->GetValue();
}

void OOPPlaylistSearchDlg::OnClose(wxCloseEvent&)
{
	HideAndFocusParent();
}

void OOPPlaylistSearchDlg::OnShow(wxShowEvent&)
{
	FocusCtrl( m_keyword, NULL );

	VdkDC dc( this );
	m_keyword->SelectAll( &dc );
}

void OOPPlaylistSearchDlg::HideAndFocusParent()
{
	Hide();
	GetParent()->SetFocus();
}

void OOPPlaylistSearchDlg::PostSearchEvent()
{
	if( !m_keyword->IsEmpty() )
	{
		wxCommandEvent evt( OOP_EVT_PLAYLIST_SEARCH );
		wxPostEvent( GetParent(), evt );
	}

	HideAndFocusParent();
}
