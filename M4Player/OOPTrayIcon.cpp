/***************************************************************
 * Name:      OOPTrayIcon.cpp
 * Purpose:   OOPlayer 的托盘图标
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


wxString MenuCaption(const wchar_t *caption)
{
#ifdef __WXMSW__
    return caption;
#else
    return wxString::Format( L"%s", caption );
#endif
}

// Overridables
wxMenu* OOPTrayIcon::CreatePopupMenu()
{
#ifdef USE_NATIVE_TRAY_MENU
    wxMenu* menu = new wxMenu;
    menu->Append( OMM_PLAYER_OPTIONS, MenuCaption( L"选项(&O)" ) );
    menu->AppendSeparator();

    wxMenu* playCtrl = new wxMenu;
    playCtrl->Append( OOM_PLAY_PAUSE, MenuCaption( L"暂停(&P)" ) );
    playCtrl->Append( OOM_STOP, MenuCaption( L"停止(&S)" ) );
    playCtrl->AppendSeparator();
    playCtrl->Append( OOM_PREV, MenuCaption( L"上一首(&R)" ) );
    playCtrl->Append( OOM_NEXT, MenuCaption( L"下一首(&N)" ) );
    menu->Append( wxID_ANY, MenuCaption( L"播放控制(&P)" ), playCtrl );
    
    wxMenu* volumeCtrl = new wxMenu;
    volumeCtrl->Append( OOM_VOLUME_UP, MenuCaption( L"增大(&I)" ) );
    volumeCtrl->Append( OOM_VOLUME_DOWN, MenuCaption( L"减小(&D)" ) );
    volumeCtrl->AppendSeparator();
    volumeCtrl->AppendCheckItem( OOM_MUTE, MenuCaption( L"静音(&M)" ) );
    menu->Append( wxID_ANY, MenuCaption( L"音量控制(&V)" ), volumeCtrl );
    
    wxMenu* playMode = new wxMenu;
    playMode->AppendCheckItem( PLAY_MODE_SINGLE, MenuCaption( L"单曲播放(&S)" ) );
    playMode->AppendCheckItem( PLAY_MODE_SINGLE_RECYCLE, MenuCaption( L"单曲循环(&C)" ) );
    playMode->AppendCheckItem( PLAY_MODE_BY_LIST, MenuCaption( L"顺序播放(&O)" ) );
    playMode->AppendCheckItem( PLAY_MODE_LIST_RECYCLE, MenuCaption( L"列表循环(&L)" ) );
    playMode->AppendCheckItem( PLAY_MODE_LIST_RANDOM, MenuCaption( L"随机播放(&R)" ) );
    menu->Append( wxID_ANY, MenuCaption( L"播放模式(&M)" ), playMode );
    
    menu->AppendSeparator();
	menu->Append( CID_MINIMIZE, MenuCaption( L"最小化(&M)" ) );
    menu->Append( CID_EXIT, MenuCaption( L"退出(&E)" ) );
    
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
