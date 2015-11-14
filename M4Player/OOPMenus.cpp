/***************************************************************
 * Name:      OOPMenus.cpp
 * Purpose:   ������˵��йصĴ���
 * Author:    Ning (vanxining@139.com)
 * Created:   2010
 * Copyright: Wang Xiao Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPApp.h"

#include "MainPanel.h"
#include "PlayListPanel.h"
#include "LyricPanel.h"

#include "OOPList.h"
#include "Lyric/OOPLyric.h"
#include "OOPToolBar.h"
#include "OOPFileSystem.h"
#include "OOPDefs.h"

#include "VdkMenu.h"
#include "OnMenuSlider.h"
#include "OOPSkinSelector.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

extern OOPlayerApp* g_app;

enum {

	MMBID_PLAY,
	MMBID_PAUSE,
	MMBID_STOP,
	MMBID_NEXT,
	MMBID_PREV,
	MMBID_UP,
	MMBID_DOWN,
	MMBID_EXIT,
	MMBID_OPTIONS,
	MMBID_LINKS,

	MMBID_COUNT,
};

void OOPlayerApp::GenerateMainMenu()
{
	VdkMenu* menu = m_mainPanel->CreateMenu( NULL );
	menu->SetBestWidth( 150 );

	if( !m_mainMenuBitmaps.IsOk() )
	{
		wxString imgPath( OOPFileSystem::GetAppResDir() + L"mainMenu.png" );
		m_mainMenuBitmaps.Set( imgPath, MMBID_COUNT, 1 );
	}

	//===============================================

	menu->AppendItem( NewMenuItem()->id( OMM_PLAYER_OPTIONS ).
					  caption( L"ѡ��" ).
					  bmpArrayID( VdkBitmapArrayId( m_mainMenuBitmaps, 
													MMBID_OPTIONS, 0 ) ) );

	VdkMenu* menuLinks = m_mainPanel->CreateMenu( menu );

		menuLinks->AppendItem( NewMenuItem()->id( CID_ABOUT ).
							   caption( L"����" ) );
		menuLinks->AppendItem( NewMenuItem()->id( OMM_LINKS_WEBSITE ).
							   caption( L"��վ" ) );
		menuLinks->AppendItem( NewMenuItem()->id( OMM_LINKS_FORUM ).
							   caption( L"��̳" ) );

	menu->AppendItem( NewMenuItem()->caption( L"�������" ).
					  subMenu( menuLinks ).
					  bmpArrayID( VdkBitmapArrayId( m_mainMenuBitmaps, 
													MMBID_LINKS, 0 ) ) );

	menu->AppendSeperator();

	//===============================================

	VdkMenu* menuPlayControl = m_mainPanel->CreateMenu(menu);

		m_playPauseItem = NewMenuItem();
		menuPlayControl->AppendItem( m_playPauseItem->id( OOM_PLAY_PAUSE ).
									 caption( L"��ͣ" ) );

		m_stopItem = NewMenuItem();
		menuPlayControl->AppendItem( m_stopItem->id( OOM_STOP ).
									 caption( L"ֹͣ" ) );

		menuPlayControl->AppendSeperator();

		m_prevItem = NewMenuItem();
		menuPlayControl->AppendItem( m_prevItem->id( OOM_PREV ).
									 caption( L"��һ��" ) );

		m_nextItem = NewMenuItem();
		menuPlayControl->AppendItem( m_nextItem->id( OOM_NEXT ).
									 caption( L"��һ��" ) );

	menu->AppendItem( NewMenuItem()->caption( L"���ſ���" ).
					  subMenu( menuPlayControl ) );

	//===============================================

	VdkMenu* menuVolumeCtrl = m_mainPanel->CreateMenu( menu );

		menuVolumeCtrl->AppendItem( NewMenuItem()->id( OOM_VOLUME_UP ).
									caption( L"����" ) );

		menuVolumeCtrl->AppendItem( NewMenuItem()->id( OOM_VOLUME_DOWN ).
									caption( L"��С" ) );

		menuVolumeCtrl->AppendSeperator();

		m_muteItem = NewMenuItem();
		menuVolumeCtrl->AppendItem( m_muteItem->id( OOM_MUTE ).
									caption( L"����" ) );

	menu->AppendItem( NewMenuItem()->caption( L"��������" ).
					  subMenu( menuVolumeCtrl ) );

	//===============================================

	m_playModes1 = m_mainPanel->CreateMenu( menu );
	FillPlayModesMenu( m_playModes1 );
	menu->AppendItem( NewMenuItem()->caption( L"����ģʽ" ).
					  subMenu( m_playModes1 ) );
	menu->AppendSeperator();

	// Ƥ��
	VdkMenu* menuSkin = m_mainPanel->CreateMenu( menu );
	menuSkin->SetAddinStyle( VMS_NO_EXTRA_SPACE );
	menuSkin->RemoveStyle( VMS_BITMAP );
	menuSkin->AttachCtrl( new OOPSkinSelector );

	menu->AppendItem( NewMenuItem()->caption( L"Ƥ��" ).
					  subMenu( menuSkin ) );

	// ͸����
	VdkMenu* menuTransparent = m_mainPanel->CreateMenu( menu );
	menuTransparent->AttachCtrl( new OnMenuSlider( m_mainPanel ) );

	menu->AppendItem( NewMenuItem()->caption( L"����͸��" ).
					  subMenu( menuTransparent ) );
	menu->AppendSeperator();

	//===============================================

#if 0
	menu->AppendItem( NewMenuItem()->id( CID_FEEDBACK ).caption( L"��Ҫ����" ) );
	menu->AppendSeperator();
#endif

	menu->AppendItem( NewMenuItem()->id( CID_MINIMIZE ).caption( L"��С��" ) );
	menu->AppendItem( NewMenuItem()->id( CID_EXIT ).
					  caption( L"�˳�" ).
					  bmpArrayID( VdkBitmapArrayId( m_mainMenuBitmaps, 
					  MMBID_EXIT, 0 ) )
	);

	m_mainPanel->AttachMenu( menu );
	m_MainMenu = menu;
}

void OOPlayerApp::FillPlayModesMenu(VdkMenu* menu, bool ownedByPLP)
{
	menu->SetBestWidth( 150 );

	const wchar_t* captions[] = {

		L"��������",
		L"����ѭ��",
		L"˳�򲥷�",
		L"ѭ������",
		L"�������",
	};

	VdkMenuItem* item;
	for( int i = 0; i < 5 ; i++ )
	{
		item = NewMenuItem();
		item->id( PLAY_MODE_SINGLE + i ).caption( captions[i] );
		menu->AppendItem( item );
	}

	menu->GetItem( m_playMode - PLAY_MODE_SINGLE )->checked( true );

	//===============================================
	
	if( ownedByPLP )
	{
		m_playModes2 = menu;
	}
}

//////////////////////////////////////////////////////////////////////////

enum PlayListMenuBitmapIds {

	PLMBID_PLAY,
	PLMBID_FILE_INFO,
	PLMBID_DELETE,
	PLMBID_LOCATE_FILE,
	PLMBID_OPTIONS,
	
	PLMBID_COUNT
};

enum {

	TBMBID_ADD_FILE,
	TBMBID_ADD_FOLDER,
	TBMBID_OPEN_PLAYLIST,
	TBMBID_SAVE_PLAYLIST,
	TBMBID_QUICK_SEARCH,
	TBMBID_DO_SEARCH,

	TBMBID_COUNT
};

void PlayListPanel::GenerateMenu()
{
	VdkMenu* menu = CreateMenu( NULL, m_menuId );
	menu->SetAddinStyle( VMS_SEND_PRESHOW_MSG );

	if( !m_playListMenuBitmaps.IsOk() )
	{
		wxString imgPath( OOPFileSystem::GetAppResDir() + L"playList.png" );
		m_playListMenuBitmaps.Set( imgPath, PLMBID_COUNT, 1 );
	}

	//===============================================

	menu->AppendItem( NewMenuItem()->
					  id( OPLM_PLAY_SELECTED ).
					  caption( L"����" ).
					  bmpArrayID( VdkBitmapArrayId( m_playListMenuBitmaps, 
													PLMBID_PLAY, 0 ) ) );

	menu->AppendSeperator();
	menu->AppendItem( NewMenuItem()->
					  id( OPLM_FILE_PROPERTIES ).
					  caption( L"�ļ�����" ).
					  bmpArrayID( VdkBitmapArrayId( m_playListMenuBitmaps, 
												    PLMBID_FILE_INFO, 0 ) ) );

	menu->AppendSeperator();
	menu->AppendItem( NewMenuItem()->
					  id( OPLM_DELETE ).
					  caption( L"ɾ��" ).
					  bmpArrayID( VdkBitmapArrayId( m_playListMenuBitmaps, 
					  PLMBID_DELETE, 0 ) ) );

	menu->AppendItem( NewMenuItem()->id( OPLM_DELETE_FILE ).
					  caption( L"����ɾ��" ) );

	VdkMenu* renameFiles = CreateMenu(menu);

		renameFiles->AppendItem( NewMenuItem()->id( OPLM_RENAME_SONGNAME_ONLY ).
								 caption( L"������.��չ��" ) );
		renameFiles->AppendItem( NewMenuItem()->id( OPLM_RENAME_SINGER_FIRST ).
								 caption( L"���� - ������.��չ��" ) );
		renameFiles->AppendItem( NewMenuItem()->id( OPLM_RENAME_SONGNAME_FIRST ).
								 caption( L"������ - ����.��չ��" ) );

	menu->AppendItem( NewMenuItem()->id( OPLM_FILE_PROPERTIES ).
								 caption( L"�������ļ�" ).
								 subMenu( renameFiles ) );

	menu->AppendItem( NewMenuItem()->id( OPLM_LOCATE_FILE ).
					  caption( L"����ļ�" ).
					  bmpArrayID( VdkBitmapArrayId( m_playListMenuBitmaps, 
					  PLMBID_LOCATE_FILE, 0 ) ) );

	menu->AppendSeperator();
	menu->AppendItem( NewMenuItem()->id( OPLM_PLAYLIST_OPTIONS ).
					  caption( L"ѡ��" ).
					  bmpArrayID( VdkBitmapArrayId( m_playListMenuBitmaps, 
					  PLMBID_OPTIONS, 0 ) ) );

	m_playList->AttachMenu( menu );

	//===============================================

	OOPToolBar* toolbar = (OOPToolBar *) FindCtrl( L"toolbar" );

	if( toolbar )
	{
		MenuGenerator gen( wxID_ANY );

		gen.itemMap[OTBM_ADD_FILE] = L"�ļ�(&F)";
		gen.itemMap[OTBM_ADD_FOLDER_RECURSIVE] = L"�ļ���(������Ŀ¼)(&R)";
		gen.itemMap[OTBM_ADD_FOLDER] = L"�ļ���(&O)";

		toolbar->Append( wxEmptyString, ::GenerateMenu( this, gen ) );
		gen.Clear();

		gen.itemMap[OPLM_DELETE] = L"ѡ�е��ļ�(&S)";
		gen.itemMap[OTBM_DELETE_DUPLICATED] = L"�ظ����ļ�(&D)";
		gen.itemMap[OTBM_DELETE_WRONG_FILE] = L"������ļ�(&E)";
		gen.itemMap[OTBM_DELETE_SEP_1] = L"<SEP>";
		gen.itemMap[OTBM_DELETE_ALL] = L"ȫ��ɾ��(&A)";
		gen.itemMap[OTBM_DELETE_SEP_2] = L"<SEP>";
		gen.itemMap[OTBM_DELETE_FILE] = L"����ɾ��(&P)";

		toolbar->Append( wxEmptyString, ::GenerateMenu( this, gen ) );
		gen.Clear();

		gen.itemMap[OTBM_LIST_OPEN] = L"���б�(&O)";
		gen.itemMap[OTBM_LIST_SAVE] = L"�����б�(&S)";
		gen.itemMap[OTBM_LIST_NEW] = L"�½��б�(&N)";

		toolbar->Append( wxEmptyString, ::GenerateMenu( this, gen ) );
		gen.Clear();

		gen.itemMap[OTBM_SORT_BY_TITLE] = L"����ʾ����(&T)";
		gen.itemMap[OTBM_SORT_BY_PATH] = L"��·����(&P)";
		gen.itemMap[OTBM_SORT_BY_ALBUM] = L"��ר����(&A)";
		gen.itemMap[OTBM_SORT_BY_LENGTH] = L"����������(&L)";
		gen.itemMap[OTBM_SORT_BY_PLAY_COUNT] = L"�����Ŵ���(&C)";
		gen.itemMap[OTBM_SORT_BY_ADD_TIME] = L"�������б�ʱ��(&F)";

		toolbar->Append( wxEmptyString, ::GenerateMenu( this, gen ) );
		gen.Clear();

		gen.itemMap[OTBM_LOCATE_SHOW_DIALOG] = L"���ٶ�λ(&S)";
		gen.itemMap[OTBM_LOCATE_FIND_NEXT] = L"������һ��(&N)";

		toolbar->Append( wxEmptyString, ::GenerateMenu( this, gen ) );
		gen.Clear();

		gen.itemMap[OTBM_SELECT_ALL] = L"ȫ��ѡ��(&A)";
		gen.itemMap[OTBM_SELECT_NONE] = L"ȫ����ѡ(&U)";
		gen.itemMap[OTBM_SELECT_REVERSE] = L"����ѡ��(&I)";

		toolbar->Append( wxEmptyString, ::GenerateMenu( this, gen ) );
		gen.Clear();

		VdkMenu* menu = CreateMenu();
		g_app->FillPlayModesMenu( menu, true );
		toolbar->Append( wxEmptyString, menu );
	}
}

//////////////////////////////////////////////////////////////////////////

enum {

	LMBID_COPY_LRC,
	LMBID_REMOVE_LRC,
	LMBID_SEARCH_LRC,
	LMBID_RELOAD_LRC,
	LMBID_ASSOCIATE_LRC,
	LMBID_OPTIONS,

	LMBID_COUNT,
};

void LyricPanel::GenerateMenu()
{
	VdkMenu* menu = CreateMenu( NULL, m_menuId );
	menu->SetAddinStyle( VMS_SEND_PRESHOW_MSG );
	menu->SetBestWidth( 150 );

	if( !m_menuBitmaps.IsOk() )
	{
		wxString imgPath( OOPFileSystem::GetAppResDir() + L"lyricMenu.png" );
		m_menuBitmaps.Set( imgPath, LMBID_COUNT, 1 );
	}

	//===============================================

	menu->AppendItem( NewMenuItem()->id( OLRCM_SEARCH ).
					  caption( L"��������(&D)..." ).
					  bmpArrayID( VdkBitmapArrayId( m_menuBitmaps, 
													LMBID_SEARCH_LRC, 0 ) ) );

	menu->AppendItem( NewMenuItem()->id( OLRCM_ASSOCIATE_LOCAL_FILE ).
					  caption( L"�������(&L)..." ).
					  bmpArrayID( VdkBitmapArrayId( m_menuBitmaps, 
													LMBID_ASSOCIATE_LRC, 0 ) ) );

	menu->AppendSeperator();

	menu->AppendItem( NewMenuItem()->id( OLRCM_COPY_LRC ).
					  caption( L"���Ƹ��(&C)" ).
					  bmpArrayID( VdkBitmapArrayId( m_menuBitmaps, 
													LMBID_COPY_LRC, 0 ) ) );
	menu->AppendItem( NewMenuItem()->id( OLRCM_REMOVE_LRC ).
					  caption( L"�������(&U)" ).
					  bmpArrayID( VdkBitmapArrayId( m_menuBitmaps, 
													LMBID_REMOVE_LRC, 0 ) ) );
	menu->AppendItem( NewMenuItem()->id( OLRCM_RELOAD_LRC ).
					  caption( L"��������(&R)" ).
					  bmpArrayID( VdkBitmapArrayId( m_menuBitmaps, 
													LMBID_RELOAD_LRC, 0 ) ) );

	menu->AppendSeperator();
	menu->AppendItem( NewMenuItem()->id( OLRCM_LRC_OPTIONS ).
					  caption( L"ѡ��(&P)" ).
					  bmpArrayID( VdkBitmapArrayId( m_menuBitmaps, 
													LMBID_OPTIONS, 0 ) ) );

	m_lyric->AttachMenu( menu );
}
