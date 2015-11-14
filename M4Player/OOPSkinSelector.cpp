/***************************************************************
 * Name:      OOPSkinSelector.cpp
 * Purpose:   使用 VdkListCtrl 来显示大批量的皮肤
 * Author:    Ning (vanxining@139.com)
 * Created:   2011.04.02
 * Copyright: Wang Xiao Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPSkinSelector.h"
#include "OOPSkinMenuSelector.h"
#include "VdkDC.h"
#include "OOPApp.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

DECLARE_APP( OOPlayerApp );
extern OOPlayerApp* g_app;

//////////////////////////////////////////////////////////////////////////

const static int gs_avatarCtrlX = 100;
const static int gs_avatarCtrlY = 100;

OOPSkinSelector::OOPSkinSelector()
	: VdkMenuXrcCtrlWrapper
		  ( L"<object class=\"VdkWindow\" name=\"stats\">"
			L"<rect>0,0,238,276</rect>"
			L"<object class=\"VdkTab\" name=\"tab\">"
				L"<rect>1,1,236,274</rect>"

				L"<object class=\"VdkToggleButton\" name=\"home\">"
				    L"皮肤"
				    L"<rect>4,0,80,24</rect>"
				    L"<expl>3</expl>"
				    L"<image>../../App/tabs.bmp</image>"
				L"</object>"
				L"<object class=\"VdkPanel\" name=\"home\">"
					L"<bg><bg-color>#FFFFFF</bg-color></bg>"
					L"<object class=\"OOPSkinMenuSelector\" name=\"skinSelector\">"
						L"<rect>0,0,236,250</rect>"
					L"</object>"
				L"</object>"

			L"</object>"
		    L"</object>" ),
	  m_selector( NULL ),
	  m_avatarCount( 0 ),
	  m_progress( 0 )
{

}

OOPSkinSelector::~OOPSkinSelector()
{

}

VdkControl* OOPSkinSelector::Implement(VdkWindow* menuWin, const wxPoint& pos)
{
	wxASSERT( !m_selector );
	VdkMenuXrcCtrlWrapper::Implement( menuWin, wxPoint( 0, 0 ) );

	//----------------------------------------

	m_selector = (OOPSkinMenuSelector *) menuWin->FindCtrl( L"skinSelector" );
	m_selector->SetBackgroundColour( *wxWHITE );
	m_selector->VdkControl::SetAddinStyle( VCS_BORDER_NONE );
#ifdef __WXDEBUG__
	m_selector->SetDebugCaption( L"OOPSkinSelector" );
#endif
	//----------------------------------------

	// TODO:
	OOPSkin& skins = wxGetApp().GetSkinList();
	skins.GetAllSkins();

	//----------------------------------------

	int j = 0;
	OOPSkin::SkinIter i;
	for( i = skins.begin(); i != skins.end(); ++i )
	{
		AddAvatar( **i );

		j++;
		if( j == 100 )
			break;
	}

	m_selector->CalcVirtualSize();
	m_avatarCount = 0;

	if( m_progress > 0 )
	{
		VdkDC vdc( m_selector, m_selector->Rect00(), NULL );
		m_selector->GoTo( m_progress, &vdc );
	}

	//----------------------------------------
	// 绑定事件处理函数

	VdkWindow::CtrlIter ci;
	for( ci = m_selector->VdkWindow::begin();
		 ci != m_selector->VdkWindow::end(); ++ci )
	{
		(*ci)->SetID( m_id );
	}

	m_selector->Bind( wxEVT_VOBJ, &OOPSkinSelector::OnAvatar, this, m_id );

	//----------------------------------------

	return m_pCtrl;
}

void OOPSkinSelector::AddAvatar(const OOPSkin::SkinInfo& info)
{
	const static int s_padding = 5;
	SkinAvatar* avatar = new SkinAvatar;

	int x = m_avatarCount % 2 == 0 ?
			s_padding : gs_avatarCtrlX + s_padding * 2;
	int y = gs_avatarCtrlY * ( m_avatarCount / 2 ) +
			( m_avatarCount / 2 + 1 ) * s_padding;

	avatar->Create( m_selector,
		wxRect( x, y, gs_avatarCtrlX, gs_avatarCtrlY ), info );

	m_selector->AddCtrl( avatar );
	m_avatarCount++;
}

void OOPSkinSelector::SaveState()
{
	if( m_selector )
	{
		m_progress = m_selector->GetProgress();
		m_selector = NULL;
	}
}

void OOPSkinSelector::OnAvatar(VdkVObjEvent& e)
{
	SkinAvatar* avatar = (SkinAvatar *) e.GetCtrl();
	wxPoint pos( avatar->GetPosition() );

	int x = pos.x / gs_avatarCtrlX;
	int y = pos.y / gs_avatarCtrlY;

	int index = y * 2 + x;

	e.SetClientData( (void *) index );

	//----------------------------------------

	OOPlayerApp& app = wxGetApp();
	app.OnSelectSkin( e );
}

