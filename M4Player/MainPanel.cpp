/***************************************************************
 * Name:      MainPanel.cpp
 * Purpose:   定义 MainPanel
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-2-27
 **************************************************************/
#include "StdAfx.h"
#include "MainPanel.h"

#include "VdkCtrlParserInfo.h"
#include "VdkLabel.h"
#include "OOPApp.h"
#include "OOPIcon.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

#ifdef __WXGTK__
#   include "DefaultIcon.xpm"
#endif

extern OOPlayerApp* g_app;

MainPanel::MainPanel()
	: OOPWindow( NULL, L"16D3DE86-170A-48EE-B9DE-509186AAC389", L"OOPlayer" )
{
	SetAddinStyle( VWS_FIRST_SHOWN_EVT );
	SetSkin();
}

void MainPanel::DoSetSkin()
{
	MapOfCtrlIdInfo ids( 25 );
	g_app->GenerateMainPanelCtrlIds( ids );
	FromXrc( L"player_window.xml", &ids );
	ClearPtrMap( ids );

	SetIcon( wxICON( DefaultIcon ) );
	BindCloseBtnEvent();

	//-----------------------------------------------------

	InitRegionButtons();
	VdkLabel* label = NULL;

	label = (VdkLabel *) FindCtrl( L"stereo" );
	if( label )
	{
		label->SetCaption( L"立体声", NULL );
	}

	OOPIcon* icon = FindCtrl< OOPIcon >( L"icon" );
	if( icon )
	{
		icon->SetFrame( this );
	}

	//-----------------------------------------------------

	g_app->SetTopWindow( this );
	g_app->SetMainPanel( this, NULL );
}
