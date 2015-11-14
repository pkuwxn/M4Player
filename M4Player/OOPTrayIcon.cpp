/***************************************************************
 * Name:      OOPTrayIcon.cpp
 * Purpose:   OOPlayer ������ͼ��
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-16
 **************************************************************/
#include "StdAfx.h"
#include "OOPTrayIcon.h"

#include "VdkEvent.h"
#include "OOPDefs.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

#if defined( __WXGTK__ ) || !defined( TEST_VDK_TRAY_MENU )
#   define USE_NATIVE_TRAY_MENU
#endif

//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( OOPTrayIcon, wxTaskBarIcon )
	EVT_MENU_RANGE( OMM_LOWER_BOUND, OMM_UPPER_BOUND, OOPTrayIcon::OnMenu )
    EVT_MENU_RANGE( CID_MINIMIZE, CID_EXIT, OOPTrayIcon::OnMenu )
#ifndef USE_NATIVE_TRAY_MENU
	EVT_TASKBAR_RIGHT_DOWN( OOPTrayIcon::OnRightDown )
#endif
END_EVENT_TABLE()

// Overridables
wxMenu* OOPTrayIcon::CreatePopupMenu()
{
#ifdef USE_NATIVE_TRAY_MENU
    wxMenu* menu = new wxMenu;
    menu->Append( OMM_PLAYER_OPTIONS, L"ѡ��(&O)" );
    menu->AppendSeparator();

    wxMenu* playCtrl = new wxMenu;
    playCtrl->Append( OOM_PLAY_PAUSE, L"��ͣ(&P)" );
    playCtrl->Append( OOM_STOP, L"ֹͣ(&S)" );
    playCtrl->AppendSeparator();
    playCtrl->Append( OOM_PREV, L"��һ��(&R)" );
    playCtrl->Append( OOM_NEXT, L"��һ��(&N)" );
    menu->Append( wxID_ANY, L"���ſ���(&P)", playCtrl );
    
    wxMenu* volumeCtrl = new wxMenu;
    volumeCtrl->Append( OOM_VOLUME_UP, L"����(&I)" );
    volumeCtrl->Append( OOM_VOLUME_DOWN, L"��С(&D)" );
    volumeCtrl->AppendSeparator();
    volumeCtrl->AppendCheckItem( OOM_MUTE, L"����(&M)" );
    menu->Append( wxID_ANY, L"��������(&V)", volumeCtrl );
    
    wxMenu* playMode = new wxMenu;
    playMode->AppendCheckItem( PLAY_MODE_SINGLE, L"��������(&S)" );
    playMode->AppendCheckItem( PLAY_MODE_SINGLE_RECYCLE, L"����ѭ��(&C)" );
    playMode->AppendCheckItem( PLAY_MODE_BY_LIST, L"˳�򲥷�(&O)" );
    playMode->AppendCheckItem( PLAY_MODE_LIST_RECYCLE, L"�б�ѭ��(&L)" );
    playMode->AppendCheckItem( PLAY_MODE_LIST_RANDOM, L"�������(&R)" );
    menu->Append( wxID_ANY, L"����ģʽ(&M)", playMode );
    
    menu->AppendSeparator();
	menu->Append( CID_MINIMIZE, L"��С��(&M)" );
    menu->Append( CID_EXIT, L"�˳�(&E)" );
    
    return menu;
#else
	return NULL;
#endif // USE_NATIVE_TRAY_MENU
}

void OOPTrayIcon::OnMenu(wxCommandEvent& e)
{
    VdkVObjEvent vobje( e.GetId() );
    wxPostEvent( wxTheApp, vobje );
}

#ifndef USE_NATIVE_TRAY_MENU
#include "MainPanel.h"
#include "VdkMenu.h"

void OOPTrayIcon::OnRightDown(wxTaskBarIconEvent&)
{
	MainPanel* mainPanel = (MainPanel *) wxTheApp->GetTopWindow();
	mainPanel->GetMenu()->ShowContext( wxGetMousePosition(), NULL );
}
#endif // ! USE_NATIVE_TRAY_MENU
