/***************************************************************
 * Name:      OOPApp.cpp
 * Purpose:   OOPlayer �Ŀ�������
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2009-12-19
 **************************************************************/
#include "StdAfx.h"
#include "OOPApp.h"

#include "DelayModStack.h"

#include "MainPanel.h"
#include "PlayListPanel.h"
#include "LyricPanel.h"
#include "EqPanel.h"

#include "OOPConf.h"
#include "OOPFileSystem.h"
#include "OOPLed.h"
#include "OOPLabel.h"
#include "OOPList.h"
#include "OOPSong.h"
#include "OOPTrayIcon.h" // ����ͼ��
#include "OOPSingleLyricTask.h"
#include "OOPSingleHttpThread.h" // ��ƽ��ֹ�߳�

#include "Lyric/OOPLyric.h"
#include "Lyric/OOPDesktopLyric.h"
#include "LyricGrabber/LyricGrabber.h"

#include "VdkLabel.h"
#include "VdkSlider.h"
#include "VdkMenu.h" // for VdkMenuItem
#include "VdkToggleButton.h"
#include "VdkCtrlParserInfo.h"
#include "wxUtil.h"

#include <wx/taskbar.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/wfstream.h> // for wxFileInputStream, playlist serialization
#include <wx/aboutdlg.h> // for wxAboutBox()
#include <wx/dynlib.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_APP( OOPlayerApp );
OOPlayerApp* g_app = NULL;

const static wchar_t* gs_appVersion = L"1.0 Beta 2(20120531)";
const static wchar_t* gs_DefaultInteractiveOutput = L"OOPlayer for Linux";

// ������ǩ��Ϣ����ʱ���� OOPLabel ����ʾ
wxDECLARE_EVENT( OOP_EVT_SONG_INFO_UPDATED, wxCommandEvent );

// Ӧ�ó���״̬״̬λ��Ϣ
enum OOPState
{
    OOPST_MUTE					= 1 << 0, // ����
    // �Ƿ��ڿɲ��ŵ�״̬���粥���б�Ϊ��
    OOPST_PLAYABLE				= 1 << 1,
    // �����Ѿ��Ե�ǰ�����ĸ�ʽ��й�һ���Զ���������ô�����Ͳ�Ҫ�ٵ�
    // ���������ˣ���Ҫ�Ǳ����Ҳ���ƥ����ʱ��������������һ�Ρ�
    OOPST_LYRIC_SEARCHED		= 1 << 2,
};

void AssignToolTip(VdkControl* pCtrl, const wxString& toolTip)
{
    if( pCtrl )
    {
        pCtrl->SetAddinStyle( VCS_HAND_CURSOR );
        pCtrl->SetToolTip( toolTip );
    }
}

void Alert(const wxString& errMsg)
{
    enum {
        ALERT_STYLE = wxOK | wxCENTER | wxICON_ERROR,
    };

    wxMessageBox( errMsg, L"OOPlayer Fatal Error", ALERT_STYLE );
}

//////////////////////////////////////////////////////////////////////////

OOPlayerApp::OOPlayerApp()
    : m_checker( NULL ),
      m_conf( new OOPConf ),
      m_state( OOPST_PLAYABLE ),
	  m_endingDetector( m_music ),
	  m_stopWatch( m_music, m_endingDetector ),
	  m_desktopLyric( NULL ),
      m_mainPanel( NULL ),
      m_playListPanel( NULL ),
      m_lyricPanel( NULL ),
      m_eqPanel( NULL ),
      m_playPauseItem( NULL ),
      m_stopItem( NULL ),
      m_muteItem( NULL ),
      m_prevItem( NULL ),
      m_nextItem( NULL ),
      m_playModes1( NULL ),
      m_playModes2( NULL ),
      m_playMode( PLAY_MODE_BY_LIST ),
      m_trayIcon( NULL )
{
    g_app = this;
}

//////////////////////////////////////////////////////////////////////////

#include <Client.hpp>
#include <Service.hpp>

#ifdef __WXDEBUG__
#   include <fstream>
#   include <string>

#   ifdef __WXMSW__
#       define CFG_PLUGINS "E:/Desktop/Audio.Plugins"
#   else
#       define CFG_PLUGINS "/media/E/Desktop/Audio.Plugins.Linux"
#   endif
#endif

void LoadPlugin(wxDynamicLibrary& dll, const wxString& pluginPath)
{
	dll.Load( pluginPath );
	if( dll.IsLoaded() )
	{
		typedef sf::Client* (*fxGetInterface_t)(void);
		
		// Create a valid function pointer
		fxGetInterface_t fxGetInterface = 
		    (fxGetInterface_t) dll.GetSymbol( "getInterface", NULL );

		//check if the function is found
		if( fxGetInterface )
		{
			// Important: Use Detach(), otherwise the DLL will be unloaded 
			// once the wxDynamibLibrary object goes out of scope
			dll.Detach();

			sf::Client* componetClient = fxGetInterface();
			sf::FactoryBase::addToList( componetClient->getFactoryList() );
		}
	}
}

void InitPlugins()
{
	wxDynamicLibrary dll;

#ifdef __WXDEBUG__
	std::ifstream f( CFG_PLUGINS );
	std::string pluginPath;
	while( std::getline( f, pluginPath ) )
	{
	    while( !pluginPath.empty() && isspace( pluginPath[pluginPath.length() - 1] ) )
	        pluginPath.erase( pluginPath.length() - 1 );
	
		if( pluginPath.empty() )
			break;

        LoadPlugin( dll, pluginPath );
    }
    
    f.close();
#else // !__WXDEBUG__
    wxString dirPath( OOPFileSystem::GetRootPath() + L"Plugins" );
    dirPath += wxFileName::GetPathSeparator();
    
	wxDir puluginDir( dirPath );
	if( !puluginDir.IsOpened() )
		return;
    
	wxString pluginPath;
	bool hasMore = puluginDir.GetFirst( &pluginPath, wxEmptyString, wxDIR_FILES );
	while( hasMore )
	{
        LoadPlugin( dll, dirPath + pluginPath );
		hasMore = puluginDir.GetNext( &pluginPath );
	}
#endif // __WXDEBUG__

	sf::FactoryBase::__internal__sysInitialized = true;
}

//////////////////////////////////////////////////////////////////////////

bool OOPlayerApp::OnInit()
{
    m_checker = new wxSingleInstanceChecker;
    if (m_checker->IsAnotherRunning())
    {
        Alert(L"Another program instance is already running, aborting.");

        delete m_checker; // OnExit() won't be called if we return false
        m_checker = NULL;

        return false;
    }

    //-------------------------------------------

    // ��ʼ�����в��
    InitPlugins();

    wxString confPath( GetAppConfFilePath() );
    if( wxFileExists( confPath ) )
    {
        if( !m_conf->Load( confPath ) )
        {
            return false;
        }
    }

    if( !InitConf() )
        return false;

    //-------------------------------------------

    new MainPanel();

    // �Ƿ�ɹ�����
    return m_mainPanel != NULL;
}

int OOPlayerApp::OnExit()
{
    wxDELETE( m_checker );

    //===============================================
    // ɾ�� HTTP �������߳�

    VdkHttpThread& singHttpThread = OOPSingleHttpThread::Instance();

    wxSemaphore destoryWaiter;
    singHttpThread.SetDestoryWaiter( destoryWaiter );
    singHttpThread.StopAndExit();
    destoryWaiter.Wait();

    return wxApp::OnExit();
}

bool OOPlayerApp::InitConf()
{
    wxString strSkinFolder( OOPFileSystem::GetSkinRootDir() );
    strSkinFolder += m_conf->m_skinFolderName;
    strSkinFolder += wxFileName::GetPathSeparator();

    if( !wxDirExists( strSkinFolder ) )
    {
        Alert( L"Error opening skin dir `" + strSkinFolder + L"`." );
        return false;
    }

    OOPSkin::SkinInfo info( m_conf->m_skinFolderName, strSkinFolder );
    SelectSkin( info );

    return true;
}

inline wxString OOPlayerApp::GetAppConfFilePath() const
{
    wxString confPath( OOPFileSystem::GetRootPath() );
    return confPath += L"OOPConf.xml";
}

void OOPlayerApp::UpdateConf()
{
    m_conf->m_currSong = wxNOT_FOUND;
    LcCellIter locked( m_playList->GetLocked() );
    if( locked != m_playList->end() )
        m_conf->m_currSong = m_playList->IndexOf( *locked );

    m_conf->m_playMode = m_playMode;
    // ��ͣҲ�����ڲ���������
    m_conf->m_playing = IsPlaying() || IsPaused();
    m_conf->m_currPos = m_music.getPlayingOffset().asMilliseconds();

    m_conf->m_mute = IsMute();
    m_conf->m_volume = m_volume->GetProgress();

    m_mainPanel->UpdateStickyState( m_conf->mainPanelWndState );
    if( m_playListPanel )
        m_playListPanel->UpdateStickyState( m_conf->playlistPanelWndState );
    if( m_lyricPanel )
        m_lyricPanel->UpdateStickyState( m_conf->lyricPanelWndState );
    if( m_eqPanel )
        m_eqPanel->UpdateStickyState( m_conf->eqPanelWndState );
}

void OOPlayerApp::SavePanelVisibilities()
{
    m_conf->m_playListShown = m_togglePlayList->IsToggled();
    m_conf->m_lyricShown = m_toggleLyric->IsToggled();
    m_conf->m_eqShown = m_toggleEq->IsToggled();
}

void OOPlayerApp::SaveConf()
{
    UpdateConf();

    m_conf->Save( GetAppConfFilePath() );
}

void OOPlayerApp::LoadPlayList(const wxString& path)
{
    if( wxFileExists( path ) )
    {
        wxFileInputStream fin( path );
        m_playList->Serialize( fin );
    }
}

void OOPlayerApp::SavePlayList(const wxString& path)
{
    wxFileOutputStream fout( path );
    m_playList->Serialize( fout );
    fout.Close();
}

wxString OOPlayerApp::GetPlayListPath() const
{
	wxString playlistPath( OOPFileSystem::GetRootPath() );
	return playlistPath += L"Default.Playlist";
}

void OOPlayerApp::OnSelectSkin(VdkVObjEvent& e)
{
    m_mainPanel->HideMenu();

    // �����������Ϣ���ܵ��޸�
    SavePanelVisibilities();
    UpdateConf(); // ���µ�������

    int index = (int) e.GetClientData();
    SelectSkin( m_skinList.GetSkin( index ) );
}

void OOPlayerApp::SelectSkin(OOPSkin::SkinInfo& info)
{
    if( (&info == m_skinList.GetSelectedSkin()) ||
         (VdkControl::GetRootPath() == info.rootPath()) ||
        (!wxDirExists( info.rootPath() )) )
    {
        return;
    }

    // ����ո��������״�����Ƥ��
    bool isFirstRun = !m_mainPanel;

    //-------------------------------------------------------
    // Ƥ����ص�����
    
    // ��ղ�ƥ���Ƥ����ش���λ����Ϣ
    if( !isFirstRun )
        m_conf->ClearStickyWindowStates();

    // ���ý��̿ռ��� VdkControl ��Ƥ����Ϣ
    VdkControl::Initialize( info.maskColor(), info.rootPath() );

    if( isFirstRun )
        return;

    //------------------------------------------------------

    OOPList* playList = m_playList;
	OOPLyric* embeddedLyric = m_embeddedLyric;
    OOPPtrs::Clear();

    bool showStates[3]; // �����Ӵ��ڵĿ���״̬
    memset( showStates, 0, sizeof( showStates ) );

    showStates[0] = m_playListPanel->IsShown();
    if( m_lyricPanel )
    {
        showStates[1] = m_lyricPanel->IsShown();
    }

    if( m_eqPanel )
    {
        showStates[2] = m_eqPanel->IsShown();
    }

    m_mainPanel->ResetSkin();

    if( showStates[0] && m_togglePlayList )
        m_togglePlayList->Toggle( true, NULL, false );

    if( showStates[1] && m_toggleLyric )
        m_toggleLyric->Toggle( true, NULL, false );

    if( showStates[2] && m_toggleEq )
        m_toggleEq->Toggle( true, NULL, false );

    //------------------------------------------------------

    // ����ǰ��ע�Ᵽ��� m_currSong
    int index = playList->GetLockedIndex();
    // ���������־
    playList->SetLocked( playList->end(), NULL );
    playList = NULL;

    m_playListPanel->ResetSkin();

    if( index != wxNOT_FOUND )
    {
        m_playList->SetLocked( index, NULL );
        // ��ʼ����������Ϣ
        RollLabel( NULL, true );
    }
    else if( IsPlayable() ) // ���¸��ؼ��Ŀ��ܡ����á�״̬
    {
        DoStop( NULL, NULL, CL_LEAVE );
    }

    // ����ͼ��
    if( m_trayIcon )
    {
        UpdateTrayIcon();
    }

    //-------------------------------------------

    if( m_lyricPanel )
    {
		m_lyric.Dettach( embeddedLyric );

        m_lyricPanel->ResetSkin();
        InitLyric();
    }

    if( m_eqPanel )
    {
        m_eqPanel->ResetSkin();
    }

    SaveCurrentSkin( info );
}

void OOPlayerApp::SaveCurrentSkin(OOPSkin::SkinInfo& info)
{
    m_skinList.SetSelectedSkin( &info );
    m_conf->m_skinFolderName = info.folderName();
}

OOPStickyWindowState* OOPlayerApp::GetPanelStickyState(const wxString& uuid)
{
	const static wchar_t* sUUIDs[] = {
		L"16D3DE86-170A-48EE-B9DE-509186AAC389", // ������
		L"C5BC105E-ED0F-4578-9215-FA1EE71FC5A9",
		L"20772658-9A2B-4AE1-BDEC-5BE1823A223E",
		L"0F146699-FA80-4120-85CD-057E09FED8C8"
	};

    OOPStickyWindowState* states = &m_conf->mainPanelWndState;

	for( int i = 0; i < sizeof( sUUIDs ) / sizeof( sUUIDs[0] ); i++ )
	{
		if( uuid == sUUIDs[i] )
			return states[i].GetRect().IsEmpty() ? NULL : &states[i];
	}

	wxASSERT( false );
	return NULL;
}

void OOPlayerApp::HidePanel(OOPWindow* win)
{
    if( win == m_mainPanel )
    {
        VdkVObjEvent fakeEvent;
        Quit( fakeEvent );
    }
    else if( win == m_playListPanel )
    {
        DismissPlayListPanel();
    }
    else if( win == m_lyricPanel )
    {
        DismissLyricPanel();
    }
    else if( win == m_eqPanel )
    {
        DismissEqPanel();
    }
    else
    {
        wxLogError( L"Unexpected panel handle." );
    }
}

void OOPlayerApp::Quit(VdkVObjEvent&)
{
    SavePanelVisibilities();

    // ������ȫ�����أ�������֡�������
    if( m_playListPanel ) m_playListPanel->Hide();
    if( m_lyricPanel ) m_lyricPanel->Hide();
    if( m_eqPanel ) m_eqPanel->Hide();
    m_mainPanel->Hide();

    SaveConf();

    // ֹͣ����
    if( IsPlayable() && !IsStopped() )
    {
        DoStop( NULL, NULL, CL_LEAVE );
    }

	// ���浱ǰ�����б�
	SavePlayList( GetPlayListPath() );

	// ɾ������ OOPSong �����
	m_playList->RemoveAllSongPtrs();

	//-------------------------------------------------
	// �������д���

    if( m_trayIcon )
    {
        wxDELETE( m_trayIcon );
    }

    m_mainPanel->BeginExit();
    m_mainPanel->Destroy();
}

void OOPlayerApp::Minimize(VdkVObjEvent& e)
{
    if( ToggleAllOnScreenWindow() )
    {
        // ���硰���ص����̡������ɹ���������С����ť������״̬
        m_minimize->Update( VdkButton::NORMAL, e.GetVObjDC() );
    }
}

bool OOPlayerApp::DoPlay()
{
    if( IsStopped() )
    {
        bool ok = m_music.openFromFile( m_currSong->path() );
        if( !ok )
        {
            // ����ʧ��
            return false;
        }

		if( m_music.play() )
		{
			// �������������������
			m_endingDetector.Play();
		}
    }
    else
    {
        m_music.play();
    }

    return IsPlaying();
}

void OOPlayerApp::DoPause()
{
    m_music.pause();
}

void OOPlayerApp::DoStop()
{
	// ���ø��������������������ʹ֮���� Music::stop() �����ã���������
	// �᲻��ȷ����Ϊ����һ�θ�����������¼�
	m_endingDetector.Stop();

    m_music.stop();
}

void OOPlayerApp::DoMute(bool mute)
{
    if( IsMute() == mute )
        return;

    SetAddinState( OOPST_MUTE );
    if( !mute )
        RemoveState( OOPST_MUTE );

    if( !IsStopped() )
        m_music.setVolume( mute ? 0 : m_volume->GetProgress() );
}

void OOPlayerApp::OnPlayModes(VdkVObjEvent& e)
{
    SwitchPlayMode( (OOPlayerPlayMode) e.GetId() );
}

void OOPlayerApp::SwitchPlayMode(OOPlayerPlayMode mode)
{
    int old = m_playMode - PLAY_MODE_SINGLE;
    m_playModes1->GetItem( old )->checked( false );
    m_playModes2->GetItem( old )->checked( false );

    m_playMode = mode;
    int curr = m_playMode - PLAY_MODE_SINGLE;
    m_playModes1->GetItem( curr )->checked( true );
    m_playModes2->GetItem( curr )->checked( true );
}

void OOPlayerApp::Play(VdkVObjEvent& e)
{
    wxASSERT( e.GetMenu() == NULL ); // ������һ���˵��¼�
    wxASSERT( !m_playList->IsEmpty() );

    wxDC* pDC = e.GetVObjDC();
    if( !m_playList->HasLocked() )
    {
        // ֱ�Ӳ��Ų����б��еĵ�һ��
        VdkDC vdc( m_playListPanel );
        m_playList->SetLocked( m_playList->begin(), &vdc );
    }

	// �ȸ��µ�ǰ��� OOPSong ����DoPlay() �õ���
	UpdateCurrSong();

    if( !DoPlay() )
    {
        // ����ʧ�ܣ����ó�����ǰ������
        return;
    }

    //-------------------------------------------
	// ���� GUI

    ShowHidePlayPause( pDC );

    if( m_stop ) m_stop->Enable( true, pDC );
    if( m_led ) m_led->ResumeLed();
    if( m_progress ) m_progress->Enable( true, pDC );
    if( m_rollingLabel )
    {
        if( m_rollingLabel->IsOk() )
            m_rollingLabel->StartRolling();
        else
            RollLabel( pDC, true );
    }

    m_playPauseItem->caption( L"��ͣ" );
    m_stopItem->disabled( false );

    SetStatusLabel( wxEmptyString, pDC );

    //-------------------------------------------

    // ��ʼ���Ÿ��
    UpdateLyricState();
}

void OOPlayerApp::StopPlayer(wxDC* pDC)
{
    RemoveState( OOPST_PLAYABLE );

    if( m_play ) m_play->Enable( false, pDC );
    if( m_pause ) m_pause->Enable( false, pDC );
    if( m_stop ) m_stop->Enable( false, pDC );
    if( m_prev ) m_prev->Enable( false, pDC );
    if( m_btnNext ) m_btnNext->Enable( false, pDC );
    if( m_progress ) m_progress->Enable( false, pDC );

    if( m_rollingLabel )
    {
        m_rollingLabel->StopRolling( pDC );
    }

    SetStatusLabel( wxEmptyString, pDC );

    // ���á����ſ��ơ��˵���
    m_playPauseItem->disabled( true );
    m_stopItem->disabled( true );
    m_prevItem->disabled( true );
    m_nextItem->disabled( true );
}

void OOPlayerApp::EnablePlayer()
{
    if( !IsPlayable() )
    {
        ReverseState( OOPST_PLAYABLE );

        VdkDC vdc( m_mainPanel );
        if( m_play ) m_play->Enable( true, &vdc );
        if( m_pause ) m_pause->Enable( true, NULL );
        if( m_playList->GetItemCount() > 1 )
        {
            if( m_prev ) m_prev->Enable( true, &vdc );
            if( m_btnNext ) m_btnNext->Enable( true, &vdc );
        }

        m_playPauseItem->disabled( false );
        m_stopItem->disabled( false );
        m_prevItem->disabled( false );
        m_nextItem->disabled( false );
    }
}

void OOPlayerApp::Pause(VdkVObjEvent& e)
{
    wxASSERT( e.GetMenu() == NULL );

    DoPause();

    //-------------------------------------------

    VdkValidDC vdc( e.GetVObjDC(), m_mainPanel, NULL );
    wxDC* pDC = vdc.operator wxDC*();
    ShowHidePlayPause( pDC );

    if( m_lyric )
        m_lyric->Pause();

    if( m_led )
        m_led->PauseLed();

    m_playPauseItem->caption( L"����" );
    wxASSERT( !m_stopItem->disabled() );

    SetStatusLabel( wxEmptyString, pDC );
}

void OOPlayerApp::Stop(VdkVObjEvent& e)
{
    wxASSERT( e.GetMenu() == NULL );

    VdkValidDC vdc( e.GetVObjDC(), m_mainPanel, NULL );
    VdkDC playListDC( m_playListPanel );

    DoStop( vdc, &playListDC, CL_CLEAR );
}

void OOPlayerApp::DoStop(wxDC* pDC, wxDC* playListDC, ClearLyricOpt clearLyric)
{
    wxASSERT( IsPlayable() );

    DoStop();

    //----------------------------------------------

    // ��ղ����б�������
    m_playList->SetLocked( m_playList->end(), playListDC );

	// ����ֹͣ�����µ�ǰ��� OOPSong ����
	UpdateCurrSong();

    if( clearLyric == CL_CLEAR )
    {
        ClearLyric();
    }

    if( m_led )
    {
        m_led->StopLed( pDC );
    }

    if( m_rollingLabel )
    {
        m_rollingLabel->StopRolling( pDC );
    }

    if( m_stop )
    {
        m_stop->Enable( false, pDC );
    }

    if( m_progress )
    {
        m_progress->GoTo( 0, pDC, false );
        m_progress->Enable( false, pDC );
    }

    RollTaskbarTitle();
    ShowHidePlayPause( pDC );
    SetStatusLabel( wxEmptyString, pDC );
    UpdateTrayIcon();

    m_playPauseItem->caption( L"����" );
    m_stopItem->disabled( true );

	// �ύ���п��ܵı��ӻ��Ķ�ռ���ļ�����
	SingleDelayModStack::Instance().CommitAll();
}

void OOPlayerApp::OnMenuPlayCtrl(VdkVObjEvent& e)
{
    VdkDC dc( m_mainPanel );
    VdkVObjEvent vobjEvt;
    vobjEvt.SetVObjDC( &dc );

    switch( e.GetId() )
    {
    case OOM_PLAY_PAUSE:

        if( IsPlaying() )
        {
            vobjEvt.SetId( CID_PAUSE );
            vobjEvt.SetCtrl( m_pause );
            Pause( vobjEvt );
        }
        else
        {
            vobjEvt.SetId( CID_PLAY );
            vobjEvt.SetCtrl( m_play );
            Play( vobjEvt );
        }

        break;

    case OOM_STOP:

        if( !IsStopped() )
        {
            vobjEvt.SetId( CID_STOP );
            vobjEvt.SetCtrl( m_stop );
            Stop( vobjEvt );
        }

        break;

    default:

        break;
    }
}

void OOPlayerApp::OnMainPanelFirstShown(VdkWindowFirstShownEvent&)
{
    wxASSERT( m_mainPanel );

#ifdef __WXMSW__
    // ��֪��Ϊʲô���� wxMSW �������� Show �Ĵ��ڵĵ�һ��
    // �ػ��¼������ȵ��õĴ��ڵ�ǰ�����ȵõ�
    ShowAllPanelsOnReady();
#endif
}

void OOPlayerApp::ShowAllPanelsOnReady()
{
    if( m_conf->m_playListShown )
    {
        m_playListPanel->Show( true );
    }

    if( m_conf->m_lyricShown )
    {
        InitLyric();
        m_lyricPanel->Show( true );
    }

    if( m_conf->m_eqShown )
    {
        m_eqPanel->Show( true );
    }
}

void OOPlayerApp::SetMainPanel(MainPanel* panel, wxDC* pDC)
{
    if( !m_togglePlayList || !m_toggleLyric )
    {
        Alert( L"�л������Ӵ����Ƿ���ʾ�İ�ť������ XML �ļ��ж��塣" );
        return;
    }

    bool firstRun = !m_mainPanel;
    if( firstRun )
    {
        m_windowManager.SetMainWindow( panel );
        m_mainPanel = panel;

        // �����˵�
        GenerateMainMenu();

        // ��������ͼ��
        if( OOPTrayIcon::IsAvailable() )
        {
#ifdef __WXGTK__
            char* wm = getenv( "XDG_CURRENT_DESKTOP" );
            bool isUnity = wm && (strcmp( wm, "Unity" ) == 0);

            if( !isUnity || m_conf->m_showTaskBarIconUnderUnity )
#endif
            {
                m_mainPanel->Bind( wxEVT_IDLE,
                                   &OOPlayerApp::OnIdleCreateTrayIcon,
                                   this );
            }
        }
    }

    // �ض�λ�����ڣ���Ϊ������һ��û��ָ����rect���ڵ㣬OOPWindow �޷�����
    OOPStickyWindowState* sstate = GetPanelStickyState( m_mainPanel->GetUUID() );
    if( sstate )
    {
        wxRect mainRect( sstate->GetRect() );
        if( !mainRect.IsEmpty() )
        {
            m_mainPanel->Move( mainRect.x, mainRect.y );
        }
    }
    else
    {
        m_mainPanel->Move( 250, 250 );
    }

    //-------------------------------------------

    if( m_progress )
    {
        m_progress->SetAddinStyle( VSS_CLICK_TO_GO );
        m_progress->GoTo( GetProgress(), pDC );

        AssignToolTip( m_progress, L"���Ž���" );
    }

    if( m_volume )
    {
        m_volume->SetAddinStyle( VSS_CLICK_TO_GO );
        AssignToolTip( m_volume, L"����" );
    }

    if( m_pause && m_play )
    {
        m_play->Show( false, pDC );

        AssignToolTip( m_play, L"����" );
        AssignToolTip( m_pause, L"��ͣ" );
    }

    if( m_led )
    {
        m_led->SetStopWatch( &m_stopWatch );
    }

    if( m_openFile )
    {
        m_openFile->SetAddinStyle( VCS_DELAY_EVENT );
        AssignToolTip( m_openFile, L"���ļ�" );
    }

	if( m_exit )
	{
		// ��Ҫ�ȴ������߳��սᣬ�ڴ��ڼ���ܻ��������¼������ɴ���
		m_exit->SetAddinStyle( VCS_DELAY_EVENT );
	}

    //-------------------------------------------
    // ������ť�� ToolTip

    AssignToolTip( m_minimize, L"��С��" );
    AssignToolTip( m_minimode, L"����ģʽ" );
    AssignToolTip( m_exit, L"�˳�" );
    AssignToolTip( m_stop, L"ֹͣ" );
    AssignToolTip( m_mute, L"����" );
    AssignToolTip( m_prev, L"ǰһ��" );
    AssignToolTip( m_btnNext, L"��һ��" );
    AssignToolTip( m_toggleLyric, L"�����" );
    AssignToolTip( m_togglePlayList, L"�����б�" );
    AssignToolTip( m_toggleEq, L"������" );

    if( m_toggleBrowser )
    {
        AssignToolTip( m_toggleBrowser, L"�����" );
    }

    //-------------------------------------------

    // �ָ�����
    DoMute( m_conf->m_mute );
    ToggleMute( m_conf->m_mute, pDC );

    m_volume->GoTo( m_conf->m_volume, pDC, false );
    SetStatusLabel( wxEmptyString, pDC );
    ShowHidePlayPause( pDC );

    if( firstRun )
    {
        wxASSERT( !m_playListPanel );

        // �����Ӵ���ǰ�����ڱ���Ϊ����״̬�������Ӵ��ڻ�������������ʾ
        m_mainPanel->Show( true );
        m_playListPanel = new PlayListPanel( m_mainPanel, false );

        if( m_conf->m_playListShown )
        {
            m_togglePlayList->Toggle( true );
        }

        // ����Ĳ����б�����ǿյģ����ò�����
        if( m_playList->IsEmpty() )
        {
            StopPlayer( NULL );
        }
        else
        {
            // ����Ϊ�ϴ����ڲ��ŵĸ���
            if( m_conf->m_playing )
            {
                if( m_conf->m_currSong >= 0 &&
                    m_conf->m_currSong < m_playList->GetItemCount() )
                {
					BeginSong( m_conf->m_currSong, pDC, NULL );

					if( (m_conf->m_currPos > 0) && 
						(m_conf->m_currPos < int( GetCurrSongLength() )) )
					{
						SetPlayingOffset( sf::milliseconds( m_conf->m_currPos ) );
					}

					// TODO: ����һ��������
					if( !IsStopped() )
					{
						m_music.setVolume( IsMute() ? 0 : m_volume->GetProgress() );
					}
                }
                else
                {
                    m_conf->m_playing = false;
                    m_conf->m_currSong = wxNOT_FOUND;
                    m_conf->m_currPos = 0;
                }
            }
            else // ���¸��ؼ��Ŀ��ܡ����á�״̬
            {
                DoStop( pDC, NULL, CL_LEAVE );
            }
        }

        if( m_conf->m_lyricShown )
        {
            m_toggleLyric->Toggle( true );

            m_lyricPanel = new LyricPanel( m_mainPanel );
        }

        if( m_conf->m_eqShown )
        {
            m_toggleEq->Toggle( true );

            m_eqPanel = new EqPanel( m_mainPanel );
        }
    }
    // !firstRun
    else if( !IsStopped() )
    {
        // �ڲ��Ź������л�Ƥ������Ҫ���õ�ǰ���ڲ��ŵĸ���
        if( m_led )
            m_led->StartLed();

        if( m_progress )
            m_progress->GoTo( GetProgress(), pDC, false );
    }

    // �ָ�����ģʽ
    if( m_conf->m_playMode > PLAY_MODE_LOWER_BOUND &&
        m_conf->m_playMode < PLAY_MODE_UPPER_BOUND )
    {
        SwitchPlayMode( (OOPlayerPlayMode) m_conf->m_playMode );
    }

#ifndef __WXMSW__
    // ò���� GTK+ �����һ����ʾ�����ں�����ʾ�Ӵ����е�����
    if( firstRun )
    {
        ShowAllPanelsOnReady();
    }
#endif
}

void OOPlayerApp::BeginSong(LcCellIter songIter, wxDC* pDC, wxDC* playListDC)
{
    // �޷��� index ���ŵصõ� LcCellIter
    BeginSong( m_playList->IndexOf( *songIter ), pDC, playListDC );
}

void OOPlayerApp::BeginSong(int index, wxDC* pDC, wxDC* playListDC)
{
    // ����� playListDC ��Ӧ��Ϊ NULL ,
    // ����ԭ����Ч���Ǹ��������õ�����
    DoStop( pDC, playListDC, CL_LEAVE );

    int indexAtFirst = index;
    int numSongs = m_playList->GetItemCount();
	// ��ೢ�� 20 ��
	const static int maxTries = 20;
	int tries = 0;

    do
    {
        m_playList->SetLocked( index, playListDC );

        if( m_lyric )
        {
            // ���ø������״̬
            RemoveState( OOPST_LYRIC_SEARCHED );

            m_lyric->ClearLyric( NULL );
        }

        VdkVObjEvent fakeEvent( CID_PLAY );
        fakeEvent.SetVObjDC( pDC );
        Play( fakeEvent );

        // ���ڲ���ʧ�ܵĿ���
        if( m_music.getStatus() == sf::Music::Playing )
        {
            // ��Ӧ���� Play() �����Ӳ��Ŵ���( Play() �����ǻָ�����)
            m_currSong->playCountPlus();
            UpdateTrayIcon();

            // ���ųɹ����˳�ѭ��
            return;
        }
        else // ����ʧ��
        {
            wxLogDebug( L"����`%s`����ʧ�ܡ�", m_currSong->path() );

            //--------------------------------------------

            // ������һ��
            index++;

            // ��ͷ
            if( index == numSongs )
            {
                // ��������ģʽ�������˴�ͷ��ʼ��
                // ��Щ����ģʽ�ֱ�Ϊ�������š�����ѭ�����б�˳��
                if( (m_playMode == PLAY_MODE_LIST_RECYCLE) ||
                    (m_playMode == PLAY_MODE_LIST_RANDOM) )
                {
                    index = 0;
                }
            }
        }

		tries++;

	} while( (tries < maxTries) &&
			 (index < numSongs) &&
			 (index != indexAtFirst) );

    // ���������б����ɲ���
    DoStop( pDC, playListDC, CL_CLEAR );
}

// ��ȡ��׼ LRC ����Ŀ¼
wxString GetStdLyricDir()
{
    wxString lrcDir( OOPFileSystem::GetRootPath() );
    lrcDir += L"Lyrics";
    lrcDir += wxFileName::GetPathSeparator();

    return lrcDir;
}

void OOPlayerApp::UpdateLyricState()
{
    if( m_lyric )
    {
        if( !m_lyric->IsOk() )
        {
            if( LoadCurrSongLyric() )
            {
                m_lyric->Start();
            }
        }
        else
        {
            m_lyric->Start();
        }
    }
}

void OOPlayerApp::ClearLyric()
{
    if( m_lyric && m_lyric->IsOk() )
    {
        m_lyric->ClearLyric( NULL );

        VdkDC lyricDC( m_lyricPanel );
        m_lyric->ResetInteractiveOutput( &lyricDC );
    }
}

bool OOPlayerApp::LoadCurrSongLyric()
{
    if( m_lyric )
    {
        wxASSERT( m_currSong );

        int nTimeSum = m_currSong->length() * 1000;

        // ����ʹ����Ƕ���
        wxString embedded( m_currSong->embeddedLyric() );
        if( !embedded.empty() )
        {
            if( m_lyricParser.Load( embedded, nTimeSum ) )
			{
				m_lyric->AttachParser( m_lyricParser );
                return true;
			}
        }

        wxString path( m_currSong->lrcPath() );
        if( wxFileExists( path ) )
        {
			if( m_lyricParser.LoadFile( path, nTimeSum ) )
			{
				m_lyric->AttachParser( m_lyricParser );
				return true;
			}

            // ����ʹ������;����������
        }

        // �����Ѿ����й�һ���Զ���������ô ClientData Ӧ����Ϊ��Ϊ�ա�
        // ��Ҫ�Ǳ����Ҳ���ƥ����ʱ��������������һ�Ρ�
        if( !TestState( OOPST_LYRIC_SEARCHED ) )
        {
            // ����������������
            NewLyricDownloadSession();

            //------------------------------------------------------

            m_lyric->ClearLyric( NULL ); // �������

            wxString interactiveOutput( L"���ڷ��������������..." );
            {
                VdkDC dc( m_lyricPanel );
                m_lyric->SetInteractiveOutput( interactiveOutput, &dc );
            }
        }
    }

    return false;
}

void OOPlayerApp::NewLyricDownloadSession()
{
	wxASSERT( m_currSong );

    wxString strArtist;
    wxString strTitle;

    // �����޷���ȡ��ȷ�������Һͱ��⣬ֱ��ʹ���ļ�����������
    if( !m_currSong->TryGetArtistAndTitle( strArtist, strTitle ) )
        strTitle = m_currSong->fileName();

    LyricGrabber::Task& singleTask = OOPSingleLyricTask::Instance();
    singleTask.SetHost( 0 ); // Ĭ��ѡ��һ����ʷ�����

    singleTask.SetArtist( strArtist );
    singleTask.SetTitle( strTitle );
    singleTask.SetPath( m_currSong->path() );

    singleTask.GrabbList( LyricGrabber::Task::ST_AUTO_SELECT_AND_DOWNLOAD );
}

void OOPlayerApp::RollLabel(wxDC* pDC, bool startAtOnce)
{
    if( m_rollingLabel )
    {
        wxASSERT( m_currSong );

        wxArrayString info;
        info.Add( m_currSong->BuildTitle( GetPlayListTitleFormat() ) );

		wxString strArtist( m_currSong->artist() );
		if( !strArtist.empty() )
		{
			info.Add( L"������: " + strArtist );
		}

		wxString strAlbum( m_currSong->album() );
        if( !strAlbum.empty() )
        {
            info.Add( L"ר��: " + strAlbum );
        }

		wxString strLength( OOPSong::GetStdTimeStr( m_currSong->length() ) );
        info.Add( wxString::Format( L"����: %s", strLength ) );

        //-------------------------------------------------------

        m_rollingLabel->SetItems( info, pDC );

        if( startAtOnce )
        {
            m_rollingLabel->StartRolling();
        }
    }
}

void OOPlayerApp::GeneratePlayList(OOPList* playList)
{
    m_playList = playList;
    m_playList->SetID( CID_PLAY_LIST );

	// ��������Ҫ���¶��벥���б�
	if( !m_playList->GetVdkWindow()->IsReseting() )
		LoadPlayList( GetPlayListPath() );
}

wxString OOPlayerApp::GetPlayListTitleFormat() const
{
    return L"%A - %T";
}

void OOPlayerApp::ShowPlayListPanel(VdkVObjEvent& e)
{
    bool show = m_togglePlayList->IsToggled();
    // �����ڸı䴰��λ��֮ǰ����
    m_playListPanel->NotifyShowHideAndStick( show );
    m_playListPanel->Show( show );

    if( show )
    {
        m_playListPanel->SetFocus();
    }
}

void OOPlayerApp::DismissPlayListPanel()
{
    m_playListPanel->NotifyShowHideAndStick( false );
    m_playListPanel->Hide();

    VdkDC dc( m_mainPanel );
    m_togglePlayList->Toggle( false, &dc, false );
}

void OOPlayerApp::ShowLyricPanel(VdkVObjEvent& e)
{
    if( !m_lyricPanel )
    {
        wxASSERT( m_toggleLyric->IsToggled() );

        //-------------------------------------

        m_lyricPanel = new LyricPanel( m_mainPanel );
        InitLyric();

        m_lyricPanel->SetTitle( m_mainPanel->GetTitle() );
    }

    bool show = m_toggleLyric->IsToggled();
    m_lyricPanel->NotifyShowHideAndStick( show );
    m_lyricPanel->Show( show );
}

void OOPlayerApp::DismissLyricPanel()
{
    m_lyricPanel->NotifyShowHideAndStick( false );
    m_lyricPanel->Hide();

    VdkDC dc( m_mainPanel );
    m_toggleLyric->Toggle( false, &dc, false );
}

void OOPlayerApp::InitLyric()
{
	//CreateDesktopLyric();

	//====================================================
	// ��Ƕ���

	m_embeddedLyric = m_lyricPanel->m_lyric;
    m_lyric.Attach( m_embeddedLyric );

    if( m_embeddedLyric )
    {
		m_embeddedLyric->SetID( CID_LYRIC );

		m_lyric->SetStopWatch( m_stopWatch );
        m_lyric->SetDefualtInteractiveOutput( gs_DefaultInteractiveOutput );

        if( !IsStopped() )
		{
			if( LoadCurrSongLyric() )
			{
				if( IsPlaying() )
				{
					m_lyric->Start();
				}
				else
				{
					// Start() ��������ȷ�Ľ��ȣ���˽�������ͣʱ
					// ���ñ�����
					m_lyric->GoTo( GetProgress(), NULL, true );
				}
			}
		}
		else // ���ƽ������ı�
		{
			VdkDC lyricDC( m_lyricPanel );
			m_embeddedLyric->Draw( lyricDC );
		}
    }
}

void OOPlayerApp::CreateDesktopLyric()
{
	wxASSERT( m_mainPanel );

	if( !m_desktopLyric)
	{
		m_desktopLyric = new OOPDesktopLyric( m_mainPanel );
		m_desktopLyric->Move( 20, 620 );

		m_desktopLyric->SetStopWatch( m_stopWatch );
		m_desktopLyric->AttachParser( m_lyricParser );

		m_lyric.Attach( m_desktopLyric );
	}

	m_desktopLyric->Show();
}

void OOPlayerApp::ShowEqPanel(VdkVObjEvent&)
{
    if( !m_eqPanel )
    {
        wxASSERT( m_toggleEq->IsToggled() );

        m_eqPanel = new EqPanel( m_mainPanel );
        m_eqPanel->SetTitle( m_mainPanel->GetTitle() );
    }

    bool show = m_toggleEq->IsToggled();
    m_eqPanel->NotifyShowHideAndStick( show );
    m_eqPanel->Show( show );
}

void OOPlayerApp::DismissEqPanel()
{
    m_eqPanel->NotifyShowHideAndStick( false );
    m_eqPanel->Hide();

    VdkDC dc( m_mainPanel );
    m_toggleEq->Toggle( false, &dc, false );
}

//////////////////////////////////////////////////////////////////////////

void OOPlayerApp::ShowHidePlayPause(wxDC* pDC)
{
    if( m_play && m_pause )
    {
        VdkButton* show = IsPlaying() ? m_pause : m_play;
        VdkButton* hide = (show == m_pause) ? m_play : m_pause;

        show->Show( true, NULL );

        // ���谴ť״̬������״̬����
        if( show->ContainsPointerGlobally() )
            show->Update( VdkButton::HOVERING, pDC );
        else
            show->Update( VdkButton::NORMAL, pDC );

        hide->Show( false, NULL );
    }
}

void OOPlayerApp::OnProgress(VdkVObjEvent& e)
{
	// �������¼���
	if( IsStopped() )
		return;

    unsigned int nTimeSum = GetCurrSongLength();
    int eventId = (int) e.GetClientData();
    wxDC* pDC = e.GetVObjDC();

    if( eventId == NORMAL )
    {
        double newProgress = m_progress->GetProgress();
        // �ϵ��������Ч
        if( newProgress == 1 )
        {
            m_progress->GoTo( GetProgress(), pDC );
            return;
        }
        else
        {
            // �����ȸı䵱ǰ���Ž��ȣ�����������ڵ�ǰֵ
            if( !SetPlayingOffset( sf::milliseconds( nTimeSum * newProgress ) ) )
                return; // ���������Զ��ָ����ʵ���λ��

            if( m_lyric && m_lyric->IsOk() )
            {
                VdkDC dc( m_lyricPanel );
                m_lyric->GoTo( newProgress, &dc, IsPaused() );
            }
        }

		if( m_led )
		{
			// ResumeLed() ���������ػ��ؼ�
			if( pDC )
			{
				m_led->Draw( *pDC );
			}
			
			m_led->ResumeLed();
		}
    }
    else if( eventId == DRAGGING )
    {
        if( m_led )
        {
            m_led->PauseLed();

            if( pDC )
            {
				long currTime = nTimeSum * m_progress->GetProgress();
                m_led->Combine( currTime, *pDC );
            }
        }
    }
}

void OOPlayerApp::OnLyricDragg(VdkVObjEvent& e)
{
    wxASSERT( !IsStopped() );

    unsigned int timeToGo = (unsigned int) e.GetClientData();
    unsigned int nTimeSum = GetCurrSongLength();
    
    if( !SetPlayingOffset( sf::milliseconds( timeToGo ) ) )
    {
        // Ѱ��ʧ��
        m_lyric->GoTo( GetProgress(), e.GetVObjDC(), IsPaused() );
        return;
    }
	
	// �������޷�֪Ϥ��Ƕ��ʵ��϶��¼�
	if( m_desktopLyric )
	{
		m_desktopLyric->GoTo( GetProgress(), NULL, IsPaused() );
	}

    if( m_progress || m_led )
    {
        VdkDC dc( m_mainPanel );

		if( m_progress )
			m_progress->GoTo( double( timeToGo ) / nTimeSum, &dc );

		if( m_led ) m_led->Draw( dc );
    }
}

bool OOPlayerApp::SetPlayingOffset(sf::Time offset)
{
	m_endingDetector.Stop();
	bool ok = m_music.setPlayingOffset( offset );
	// ����̽����
	m_endingDetector.Play();
	
	return ok;
}

void OOPlayerApp::OnPlaySel(VdkVObjEvent& e)
{
    const ArrayOfSortedInts& selItems = m_playList->GetSelectedItems();
    wxASSERT( !selItems.empty() );

    if( m_playList->GetLockedIndex() != selItems[0] )
    {
        VdkDC dc( m_mainPanel );
        VdkDC playListDC( m_playListPanel );

        BeginSong( selItems[0], &dc, &playListDC );
    }
}

void OOPlayerApp::HandleDelete(VdkVObjEvent& e)
{
    // ����ɾ�������ļ�
    bool delPhysically =
        (e.GetId() == OPLM_DELETE_FILE) ||
        (e.GetId() == OTBM_DELETE_FILE);

    // TODO:
    if( delPhysically )
    {
        if( wxMessageBox( L"�Ƿ�ȷ�ϴӴ�����ɾ����Щ�����ļ���\n"
                            L"ע�⣺һ��ɾ���Ͳ��ɻָ���",
                            L"����ɾ�������ļ�",
                            wxYES_NO | wxICON_ASTERISK,
                            m_playListPanel )
            == wxNO )
        {
            return;
        }
    }

    VdkDC dc( m_mainPanel );
    // ���ﲻ�ܽ���������ֻ��Ϊ�����б�ؼ�����������Ϊ
    // ���������û�н���������������
    VdkDC playListDC( m_playListPanel );

    DelAction da = delPhysically ? DA_DEL_PHYSICALLY : DA_DEL_FROM_LIST_ONLY;
    DoHandleDelete( da, dc, playListDC );
}

void OOPlayerApp::DoHandleDelete(DelAction da, wxDC& dc, wxDC& playListDC)
{
    if( !m_playList->HasSelected() )
        return;

    const ArrayOfSortedInts& currSel = m_playList->GetSelectedItems();
    wxVector< int > selItems;

    size_t numRows = m_playList->GetItemCount();
    size_t numSel = currSel.size();

    if( m_playList->TestState( VLCST_SELECT_ALL ) )
    {
        wxASSERT( selItems.empty() );

        //---------------------------------------

        selItems.reserve( numRows );

        for( size_t i = 0; i < numRows; i++ )
        {
            selItems.push_back( i );
        }

        numSel = numRows;
    }
    else
    {
        selItems.reserve( numSel );

        for( size_t i = 0; i < numSel; i++ )
        {
            selItems.push_back( currSel[i] );
        }
    }

    int currIndex = m_playList->GetLockedIndex();

    //-------------------------------------------

    // �����һ����ѡ�е� ID
    size_t firstSel = selItems[0];

    for( int i = numSel - 1; i >= 0 ; i-- )
    {
        // �û���ɾ�����ڲ��ŵĸ�����Ϊ��ģ�ͣ�ֱ��ֹͣ����
        if( currIndex == selItems[i] )
        {
            DoStop( &dc, &playListDC, CL_CLEAR );

            // ���¹����ı�
            if( m_rollingLabel )
            {
                m_rollingLabel->StopRolling( &dc );
            }

            // ��������ʾ
            ClearLyric();
        }

        // ����ɾ�������ļ�
        if( da == DA_DEL_PHYSICALLY )
        {
            OOPSongPtr song = m_playList->GetSongPtr( selItems[i] );
            wxRemoveFile( song->path() );
        }

        m_playList->RemoveSong( selItems[i] );
    }

    //==================================================

    numRows -= numSel;

    // ����ѡ����һ��
    if( firstSel < numRows )
    {
        m_playList->Select( firstSel, NULL );
    }

    m_playList->UpdateSize( &playListDC );
}

void OOPlayerApp::OnDelAll(VdkVObjEvent& e)
{
    VdkDC mainPanelDC( m_mainPanel );
    VdkDC playListDC( m_playListPanel );

    DeleteAllSongs( &mainPanelDC, &playListDC );
}

void OOPlayerApp::RemoveDuplicatedItems(VdkVObjEvent&)
{
    wxASSERT( m_playList );

    bool changed = false;
    LcCellIter end( m_playList->end() );
    LcCellIter iter( m_playList->begin() ), iterNext,
               following, followingNext;

    for( ; iter != end; iter = iterNext )
    {
        iterNext = iter;
        ++iterNext;

        OOPListEntry* currSong = m_playList->GetListEntry( iter );

        for( following = iterNext; following != end;
             following = followingNext )
        {
            followingNext = following;
            ++followingNext;

            OOPListEntry* followingSong = m_playList->GetListEntry( following );
            if( currSong->path() == followingSong->path() )
            {
                LcCellIter songToRemove( following );
                // ɾ�����ڲ��ŵ���һ�ף�����׼��
                if( songToRemove == m_playList->GetLocked() )
                {
                    songToRemove = iter;

                    // ���µ�ǰ������ָ��
                    currSong = followingSong;
                }

                // Ҫɾ���ĸ����պý����������ȽϵĻ�׼�����׼���������
                // �����Լ�ͬ����һ�׸��һ���ظ���
                if( iterNext == songToRemove )
                {
                    iterNext = songToRemove;
                    ++iterNext;
                }

                // ������� followingNext == songToRemove ���������Ϊ
                // followingNext ���ֵ�λ�ÿ϶��Ȼ�׼�� iter �� following
                // Ҫ��

                m_playList->RemoveSong( songToRemove );
                changed = true;
            }
        }

    }

    if( changed )
    {
        VdkDC dc( m_playListPanel );
        m_playList->UpdateSize( &dc );
    }
}

void OOPlayerApp::RemoveWrongItems(VdkVObjEvent&)
{
    wxASSERT( m_playList );

    bool changed = false;
    LcCellIter iter( m_playList->begin() ), end( m_playList->end() ), curr;
    for( ; iter != end; )
    {
        curr = iter++;

        OOPSongPtr song = m_playList->GetSongPtr( curr );
        if( (song->length() == 0) || !wxFileExists( song->path() ) )
        {
            m_playList->RemoveSong( curr );
            changed = true;
        }
    }

    if( changed )
    {
        VdkDC dc( m_playListPanel );
        m_playList->UpdateSize( &dc );
    }
}

void OOPlayerApp::DeleteAllSongs(wxDC* pDC, wxDC* playListDC)
{
    if( m_playList->IsEmpty() )
        return;

    DoStop( pDC, playListDC, CL_CLEAR );

    DelayModStack& dms = SingleDelayModStack::Instance();
    dms.CommitAll();

    m_playList->Clear( playListDC );
    ClearLyric();

    StopPlayer( pDC );
}

void OOPlayerApp::SelectAllNoneReverse(VdkVObjEvent& e)
{
    VdkDC vdc( m_playListPanel );
    wxDC* pDC = &vdc;

    switch( e.GetId() )
    {
    case OTBM_SELECT_ALL:

        m_playList->SelectAll( pDC );
        break;

    case OTBM_SELECT_NONE:

        m_playList->SelectNone( pDC );
        break;

    case OTBM_SELECT_REVERSE:

        m_playList->SelectReverse( pDC );
        break;

    default:
        break;
    }

    // �û����ܶ��ɱ��ʵ�ֵ�ѡ�е��б�����ĳЩ������������б�
    // ͬʱ�����뿼�ǵ���������б���Ƿ�������ʾ
    if( m_playList->IsShown() )
        m_playListPanel->FocusCtrl( m_playList, pDC );
}

void OOPlayerApp::AddPlayList(VdkVObjEvent& e)
{
    e.SetId( OTBM_DELETE_ALL );
    OnDelAll( e );
}

void OOPlayerApp::SaveListByUser(VdkVObjEvent&)
{
    wxString path( AskPathToList( L"���沥���б�", wxFD_SAVE ) );
    if( !IsPlayListPathOk( path ) )
        return;

    SavePlayList( path );
}

void OOPlayerApp::OpenListByUser(VdkVObjEvent&)
{
    wxString path( AskPathToList( L"�򿪲����б�", wxFD_OPEN ) );
    if( !IsPlayListPathOk( path ) )
        return;

    DeleteAllSongs( NULL, NULL );
    LoadPlayList( path );
    EnablePlayer();

    m_playListPanel->QueueRedrawEvent();
    m_mainPanel->QueueRedrawEvent();
}

wxString OOPlayerApp::AskPathToList(const wxString& alert, long dlgStyle)
{
    wxFileDialog fdlg( m_playListPanel,
                       alert,
                       wxEmptyString,
                       "Default",
                       L"OOPlayer �����б�(*.Playlist)|*.Playlist|"
                       L"�����ļ�(*.*)|*.*",
                       dlgStyle );

    if( fdlg.ShowModal() == wxID_CANCEL )
        return wxEmptyString;

    return fdlg.GetPath();
}

bool OOPlayerApp::IsPlayListPathOk(const wxString& path)
{
    if( path.IsEmpty() )
        return false;

    return (path != GetPlayListPath());
}

void OOPlayerApp::SortPlayList(VdkVObjEvent& e)
{
    m_playList->SortList( e.GetId() );
}

double OOPlayerApp::GetProgress() const
{
	OOPStopWatch& sw = const_cast< OOPStopWatch& >( m_stopWatch );
    double nTimeSum = GetCurrSongLength();

    return (nTimeSum > 0) ? (sw.Time() / nTimeSum) : 0;
}

//////////////////////////////////////////////////////////////////////////

bool OOPlayerApp::IsPlayable() const
{
	return TestState( OOPST_PLAYABLE );
}

bool OOPlayerApp::IsPlaying() const
{
    return !IsPaused() && !IsStopped();
}

bool OOPlayerApp::IsPaused() const
{
	return (m_music.getStatus() == sf::Music::Paused);
}

bool OOPlayerApp::IsStopped() const
{
    return (m_music.getStatus() == sf::Music::Stopped) || !IsPlayable();
}

bool OOPlayerApp::IsMute() const
{
	return TestState( OOPST_MUTE );
}

void OOPlayerApp::UpdateCurrSong()
{
	if( m_playList )
	{
		m_currSong = m_playList->GetLockedSong();
		if( m_currSong )
		{
			m_currSong->ParseTag( false );
		}
	}
	else
	{
		m_currSong = OOPSongPtr( NULL );
	}
}

unsigned int OOPlayerApp::GetCurrSongLength() const
{
    if( IsStopped() ) // �����Ѿ�ֹͣ���ţ�ʹ�û���ֵ
    {
        return m_currSong ? (m_currSong->length() * 1000) : 0;
    }
    else
    {
		return m_music.getDuration().asMilliseconds();
    }
}

void OOPlayerApp::OnVolume(VdkVObjEvent& e)
{
    wxDC* pDC = e.GetVObjDC();
    int evtID = (int) e.GetClientData();

    if( evtID == NORMAL )
    {
        SetStatusLabel( wxEmptyString, pDC );
    }
    else if( (evtID == DRAGGING) || (evtID == LEFT_DOWN) )
    {
        float volume = m_volume->GetProgress();

        if( IsPlaying() )
            m_music.setVolume( volume );

        int volumeInt = static_cast<int>( volume * 100 );
        SetStatusLabel( wxString::Format( L"����: %d%%", volumeInt ), pDC );
    }

    if( IsMute() )
    {
        ToggleMute( false, pDC );
    }
}

void OOPlayerApp::OnMute(VdkVObjEvent& e)
{
    VdkToggleButton* mute = e.GetCtrl< VdkToggleButton >();
    ToggleMute( mute->IsToggled(), e.GetVObjDC() );
}

void OOPlayerApp::OnMenuVolumeCtrl(VdkVObjEvent& e)
{
    double newVolume = m_volume->GetProgress();
    VdkDC dc( m_mainPanel );

    switch( e.GetId() )
    {
    case OOM_VOLUME_UP:

        if( newVolume == 1 )
            return;

        newVolume += 0.1;

        break;

    case OOM_VOLUME_DOWN:

        if( newVolume == 0 )
            return;

        newVolume -= 0.1;

        break;

    case OOM_MUTE:

        ToggleMute( !IsMute(), &dc );

        return;
    }

    if( newVolume > 1 )
        newVolume = 1;
    else if( newVolume < 0 )
        newVolume = 0;

    m_volume->GoTo( newVolume, &dc, false );

	if( IsPlaying() )
		m_music.setVolume( newVolume );

    new VdkLabelGhost( m_status, m_status->GetCaption(), 1500 );
    int volumeInt = static_cast<int>( newVolume * 100 );
    SetStatusLabel( wxString::Format( L"����: %d%%", volumeInt ), &dc );

    if( newVolume > 0 )
    {
        if( IsMute() )
            ToggleMute( false, &dc );
    }
    else
    {
        if( !IsMute() )
            ToggleMute( true, &dc );
    }
}

void OOPlayerApp::ToggleMute(bool mute, wxDC* pDC)
{
    DoMute( mute );

    if( m_mute )
    {
        m_mute->Toggle( mute, pDC, false );
        m_muteItem->checked( mute );
    }

    if( m_stereo )
    {
        m_stereo->SetCaption( mute ? L"����" : L"������", pDC );
    }
}

void OOPlayerApp::StepProgress(VdkVObjEvent& e)
{
    if( m_progress )
    {
        if( m_progress->GetThumb()->GetLastState() != VdkButton::PUSHED )
        {
            m_progress->GoTo( GetProgress(), e.GetVObjDC(), false );
        }
    }

    RollTaskbarTitle();
}

void OOPlayerApp::RollTaskbarTitle()
{
    wxString title;

    if( m_currSong )
    {
        title.assign( m_currSong->BuildTitle( GetPlayListTitleFormat() ) );
        title += L" - OOPlayer  ";

        int mod = (m_stopWatch.Time() / 1000) % title.length();
        if( mod )
        {
            wxString left( title.substr( 0, mod ) );
            title.erase( 0, mod );
            title += left;
        }
    }
    else
    {
        title.assign( L"OOPlayer" );
    }

    //====================================================

    OOPWindow** windows = (OOPWindow**) &m_mainPanel;

    for( int i = 0; i < NUM_PANELS; i++ )
    {
        if( windows[i] )
        {
            windows[i]->SetTitle( title );
        }
    }
}

void OOPlayerApp::OnIdleCreateTrayIcon(wxIdleEvent&)
{
    wxASSERT( !m_trayIcon );

    m_mainPanel->Unbind( wxEVT_IDLE, &OOPlayerApp::OnIdleCreateTrayIcon, this );

    CreateTrayIcon();
    UpdateTrayIcon();
}

void OOPlayerApp::CreateTrayIcon()
{
    wxASSERT( !m_trayIcon );

    m_trayIcon = new OOPTrayIcon;
    m_trayIcon->Bind( wxEVT_UPDATE_UI, &OOPlayerApp::OnUpdateTrayMenuUI,
                      this, CID_MINIMIZE );
    m_trayIcon->Bind( wxEVT_UPDATE_UI, &OOPlayerApp::OnUpdateTrayMenuUI,
                      this, OOM_PLAY_PAUSE, OOM_MUTE );
    m_trayIcon->Bind( wxEVT_UPDATE_UI, &OOPlayerApp::OnUpdateTrayMenuUI,
                      this, PLAY_MODE_SINGLE, PLAY_MODE_LIST_RANDOM );

#if 0
    m_trayIcon->Bind( wxEVT_TASKBAR_RIGHT_DOWN,
                      &OOPlayerApp::OnTrayIconRightDown,
                      this );
#endif
}

void OOPlayerApp::UpdateTrayIcon()
{
    if( !m_trayIcon )
        return;

    wxString tooltip( L"OOPlayer" );
    if( m_currSong )
    {
        tooltip = m_currSong->BuildTitle( GetPlayListTitleFormat() );
        tooltip += L" - OOPlayer";
    }

    m_trayIcon->SetIcon( m_mainPanel->GetIcon(), tooltip );
}

void OOPlayerApp::OnUpdateTrayMenuUI(wxUpdateUIEvent& e)
{
    switch( e.GetId() )
    {
    case CID_MINIMIZE:

        e.SetText( m_mainPanel->IsShown() ? L"��С��(&M)" : L"��ԭ(&R)" );
        break;

    case OOM_PLAY_PAUSE:

        e.SetText( IsPlaying() ? L"��ͣ(&P)" : L"����(&P)" );
        goto MAINMENU_PLAY_CONTROL;

    case OOM_STOP:

        e.Enable( !IsStopped() );
        break;

MAINMENU_PLAY_CONTROL:
    case OOM_PREV:
    case OOM_NEXT:

        e.Enable( IsPlayable() );
        break;

    case OOM_MUTE:

        e.Check( IsMute() );
        break;

    case PLAY_MODE_SINGLE:
    case PLAY_MODE_SINGLE_RECYCLE:
    case PLAY_MODE_BY_LIST:
    case PLAY_MODE_LIST_RECYCLE:
    case PLAY_MODE_LIST_RANDOM:

        e.Check( m_playMode == e.GetId() );
        break;

    default:

        break;
    }
}

void OOPlayerApp::OnTrayIconRightDown(wxTaskBarIconEvent&)
{
    wxCoord x, y;
    wxGetMousePosition( &x, &y );
    m_mainPanel->ScreenToClient( &x, &y );

    m_mainPanel->SetFocus();
    m_mainPanel->ShowContextMenu( NULL, x, y, true );
}

void OOPlayerApp::OnTrayIconLeftClick(wxTaskBarIconEvent&)
{
    // �� MSW ���棬ʹ��������״̬���л�������ʾ״̬ʱ�����ز�����ͨ����
    // �����ƶ���ĳһ�����ɼ���λ����ʵ�ֵģ�����ʱ������Ȼ�ǿɼ���
    // (IsShown() == true)����������Ҫ�ֿ��ж�
    if( m_mainPanel->IsIconized() )
    {
        m_mainPanel->Restore();
        m_mainPanel->Raise();
    }
    else
    {
        ToggleAllOnScreenWindow();
    }
}

bool OOPlayerApp::ToggleAllOnScreenWindow()
{
    if( !m_trayIcon )
    {
        wxLogDebug( L"�޷���С��������: ��ǰ���ڹ�������֧�ֱ�׼����ͼ��Э�顣" );
        return false;
    }

#ifdef __WXMSW__
    bool toShow = !m_mainPanel->IsShown();
    m_mainPanel->Show( toShow );
    m_mainPanel->Raise();
    ::ShowOwnedPopups( GetHwndOf( m_mainPanel ), toShow );
#else
    wxASSERT( m_togglePlayList && m_toggleLyric && m_toggleEq );

    bool toShow = !m_mainPanel->IsShown();
    m_mainPanel->Show( toShow );
    if( toShow )
    {
        m_mainPanel->Raise();
    }

    if( m_togglePlayList->IsToggled() )
        m_playListPanel->Show( toShow );

    if( m_toggleLyric->IsToggled() )
        m_lyricPanel->Show( toShow );

    if( m_toggleEq->IsToggled() )
        m_eqPanel->Show( toShow );
#endif

    return true;
}

void OOPlayerApp::OnPlayListDClick(VdkVObjEvent& e)
{
    int index = (int) e.GetClientData();

    VdkDC vdc( m_mainPanel );
    BeginSong( index, &vdc, e.GetVObjDC() );
}

bool OOPlayerApp::OnPlayListKeys(wxKeyEvent& e)
{
    if( e.GetEventType() != wxEVT_KEY_DOWN )
        return false;

    bool keyForPanel = false;
    VdkControl* focus = m_playListPanel->GetCtrlOnFocus();
    if( focus != m_playList )
        keyForPanel = true;

    int k = e.GetKeyCode();
    if( (focus == m_playList) && (k == WXK_DELETE) )
    {
        // HandleSel ���Լ����� DC���ʱ���С�Ĵ���
        VdkVObjEvent fakeEvent;
        fakeEvent.SetId( OPLM_DELETE );
        HandleDelete( fakeEvent );

        return true;
    }

    if( keyForPanel )
        return false;

    switch( k )
    {
    case WXK_RETURN:
        {
            const ArrayOfSortedInts& selItems( m_playList->GetSelectedItems() );

            if( !selItems.empty() )
            {
                VdkDC dc( m_playListPanel );
                // Ĭ�ϲ��ŵ�һ��
                int sel = selItems[0];
                m_playList->SetLocked( sel, &dc );

                VdkVObjEvent fakeEvent;
                fakeEvent.SetClientData( (void *) sel );
                OnPlayListDClick( fakeEvent );
            }

            break;
        }

    default:

        return false;
    }

    return true;
}

void OOPlayerApp::OnMenuPrevNext(VdkVObjEvent& e)
{
    VdkDC vdc( m_mainPanel );

    VdkVObjEvent fakeEvent( (e.GetId() == OOM_PREV) ? CID_PREV : CID_NEXT );
    fakeEvent.SetVObjDC( &vdc );

    OnPrevNext( fakeEvent );
}

void OOPlayerApp::OnPrevNext(VdkVObjEvent& e)
{
    wxASSERT( m_playList );
    wxASSERT( !m_playList->IsEmpty() );

    wxDC* mainPanelDC = e.GetVObjDC();
    VdkDC playListDC( m_playListPanel );

    // ԭ���б�Ϊ�գ��û���һ�ε������һ�ס���һ�ס������ǡ����š�
    if( !m_playList->HasLocked() )
    {
        LcCellIter songToPlay( m_playList->begin() );

        m_playList->SetLocked( songToPlay, &playListDC );
        BeginSong( songToPlay, mainPanelDC, &playListDC );

        return;
    }

    PrevNextAction pna = (e.GetId() == CID_PREV) ? PNA_PREV : PNA_NEXT;
    HandlePrevNext( pna, mainPanelDC, &playListDC );
}

void OOPlayerApp::HandlePrevNext
    (PrevNextAction pna, wxDC* mainPanelDC, wxDC* playListDC)
{
    wxASSERT( m_playList );
    wxASSERT( !m_playList->IsEmpty() );

    // TODO: return false or wxASSERT?
    if( !m_playList->HasLocked() )
        return;

    switch( m_playMode )
    {
    case PLAY_MODE_BY_LIST:
    case PLAY_MODE_LIST_RECYCLE:
    case PLAY_MODE_SINGLE:
    case PLAY_MODE_SINGLE_RECYCLE:
        {
            LcCellIter songToPlay( m_playList->GetLocked() );
            LcCellIter lastSong( m_playList->end() );
            --lastSong;

            if( pna == PNA_PREV )
            {
                // ��ͷ��
                if( songToPlay == m_playList->begin() )
                {
                    if( m_playMode == PLAY_MODE_LIST_RECYCLE )
                    {
                        // �������һ��
                        songToPlay =lastSong;
                    }
                    else // ��������ģʽֱ�Ӻ��Դ��¼�
                    {
                        break;
                    }
                }

                --songToPlay;

            } // END if( e.GetId() == CID_PREV )
            else
            {
                // ��ͷ��
                if( songToPlay == lastSong )
                {
                    if( m_playMode == PLAY_MODE_LIST_RECYCLE )
                    {
                        songToPlay = m_playList->begin();
                    }
                    else // ��������ģʽֱ�Ӻ��Դ��¼�
                    {
                        // ������ʽ��ֹͣ����
                        DoStop( mainPanelDC, playListDC, CL_CLEAR );

                        break;
                    }
                }

                ++songToPlay;
            }

            m_playList->SetLocked( songToPlay, playListDC );
            BeginSong( songToPlay, mainPanelDC, playListDC );

            break;
        }

    case PLAY_MODE_LIST_RANDOM:
        {
            int randNum = Rand( m_playList->GetItemCount() );
            BeginSong( randNum, mainPanelDC, playListDC );

            break;
        }

    default:

        Alert( L"δ֪�����б������ʽ��" );
        break;
    }
}

void OOPlayerApp::OnSongFinished(wxCommandEvent&)
{
    wxASSERT( m_playList->HasLocked() );

    VdkDC mainPanelDC( m_mainPanel );
    VdkDC playListDC( m_playListPanel );

    switch( m_playMode )
    {
    case PLAY_MODE_LIST_RECYCLE:
    case PLAY_MODE_BY_LIST:
    case PLAY_MODE_LIST_RANDOM:
        {
            HandlePrevNext( PNA_NEXT, &mainPanelDC, &playListDC );

            break;
        }

    case PLAY_MODE_SINGLE:
        {
            DoStop( &mainPanelDC, &playListDC, CL_CLEAR );

            break;
        }

    case PLAY_MODE_SINGLE_RECYCLE:
        {
            BeginSong( m_playList->GetLocked(), &mainPanelDC, &playListDC );

            break;
        }

    default:

        wxLogError( L"δ֪�����б������ʽ��" );
        break;
    }
}

void OOPlayerApp::OnSearchPlaylist(VdkVObjEvent& e)
{
    wxKeyEvent ke( wxEVT_KEY_DOWN );
    if( e.GetId() == OTBM_LOCATE_FIND_NEXT )
    {
        ke.m_keyCode = WXK_F3;
    }
    else
    {
        ke.m_keyCode = 'F';
        ke.m_controlDown = true;
    }

    wxControl* hidden = m_playListPanel->GetHiddenCtrl();
    wxEvtHandler* sinker = hidden ? hidden :
        m_playListPanel->VdkWindow::GetHandle();

    wxPostEvent( sinker, ke );
}

void OOPlayerApp::OnSongInfoUpdated(wxCommandEvent& e)
{
	if( m_currSong )
	{
		if( e.GetString() == m_currSong->path() )
		{
			// �ض�Ԫ��Ϣ
			m_currSong->ParseTag( true );

			VdkDC dc( m_mainPanel );
			RollLabel( &dc, !IsStopped() );
		}
	}
}

#ifdef __WXMSW__
#	include <Shellapi.h>
#endif

void OOPlayerApp::OnLocateFile(VdkVObjEvent&)
{
	int index = m_playList->GetLastSelIndex();
	wxASSERT( index != wxNOT_FOUND );

	OOPSongPtr song = m_playList->GetSongPtr( index );

#ifdef __WXMSW__
	wxString param( L"/select," );
	param += song->path();

	::ShellExecute( 0, L"open", L"explorer.exe", param, NULL, SW_SHOWNORMAL );
#elif defined( __WXGTK__ )
    // TODO:�����ļ�·�������ܻᵼ�º���ֵ���Ϊ��Nautilus ���ڱ��򿪺󣬲���
    // �б��ڵ� Z �����겻�ᱻ���ͣ���������������ڶ��������ˣ���ʣ���������
    // ����Ļ����ʾ��
	wxExecute( wxString::Format( L"nautilus \"%s\"", song->path() ) );
#endif
}

void OOPlayerApp::OnSearchLyric(VdkVObjEvent&)
{
    wxASSERT( m_currSong );
    m_lyricPanel->PopupSchDlg( m_currSong );
}

void OOPlayerApp::OnLyricLoaded(wxCommandEvent& e)
{
    wxASSERT( m_lyric );

    if( !m_currSong || (m_currSong->path() != e.GetString()) )
    {
        // ��ǰ���ڲ��ŵĸ������Ǹ�ʸձ����سɹ�����һ��
        return;
    }

    // �������׸�ĸ���Ѿ��������ϲ��ҹ���
    SetAddinState( OOPST_LYRIC_SEARCHED );

    VdkHttpThread::FinishEvent& finishEvt = (VdkHttpThread::FinishEvent &) e;
    wxString lyric( finishEvt.GetResult() );

    if( lyric.empty() )
    {
        VdkDC dc( m_lyricPanel );
        m_lyric->SetInteractiveOutput( L"δ���ڷ��������ҵ����", &dc );

        return;
    }

    if( m_lyricParser.Load( lyric, m_currSong->length() * 1000 ) )
	{
		m_lyric->AttachParser( m_lyricParser );
		m_lyric->Start();
	}

    //--------------------------------------
    // ���浽�����ļ���

    LyricGrabber::SaveLrcFile( lyric, m_currSong->lrcPath() );

    //--------------------------------------
    // Ƕ�뵽�����ļ���

    if( m_currSong->IsTaggingSupported() )
    {
        m_currSong->embeddedLyric( lyric );

		if( !m_currSong->Save() )
		{
			// ��ӵ��ӳ��޸�ջ
			SingleDelayModStack::Instance().Add( m_currSong );
		}
    }
}

void OOPlayerApp::OnCopyLyric(VdkVObjEvent&)
{
#ifdef __WXDEBUG__
    wxASSERT( m_currSong );

    wxString lyric( m_currSong->embeddedLyric() );
    if( lyric.empty() )
    {
        lyric.assign( OOPLyricParser::LoadFile( m_currSong->lrcPath() ) );
        wxASSERT( !lyric.empty() );
    }

	CopyText( lyric );
#else
	CopyText( m_lyric->GetLyric() );
#endif
}

void OOPlayerApp::OnAssociateLyric(VdkVObjEvent& e)
{
    wxASSERT( m_lyric );
    wxASSERT( m_currSong );

    wxFileDialog fdlg( m_lyricPanel,
                       L"ָ������ļ�",
                       wxEmptyString,
                       wxEmptyString,
                       L"����ļ�(*.lrc)|*.lrc",
                       wxFD_OPEN |
                       wxFD_FILE_MUST_EXIST );

    if( fdlg.ShowModal() == wxID_CANCEL )
        return;

    wxString src( fdlg.GetPath() );
    wxString dst( m_currSong->lrcPath() );

    // ������ʾ��Ƕ���ʱ���û��ִӸ�ʴ洢�ļ���ѡ��Ĭ�Ϲ����� LRC �ļ�
    if( src != dst )
    {
        wxCopyFile( src, dst, true );
    }

    // ��Ҫ���� LoadCurrSongLyric() ����Ϊ�ú������ȶ�ȡ��Ƕ���
    if( m_lyricParser.LoadFile( dst, m_currSong->length() * 1000 ) )
	{
		m_lyric->AttachParser( m_lyricParser );
        UpdateLyricState();
	}
}

void OOPlayerApp::OnReloadLyric(VdkVObjEvent&)
{
    // ǿ���ض�����ļ�
    LoadCurrSongLyric();

    // ���¸����״̬
    UpdateLyricState();
}

void OOPlayerApp::OnRemoveLyric(VdkVObjEvent&)
{
    wxASSERT( m_lyric );
    wxASSERT( m_currSong );

    // ��ռ�Ϊ��������
    m_currSong->lrcPath( wxEmptyString );

    ClearLyric();
}

void OOPlayerApp::SetStatusLabel(wxString text, wxDC* pDC)
{
    if( !m_status )
        return;

    if( text.IsEmpty() )
    {
        if( IsStopped() )
            text = L"״̬: ֹͣ";
        else if( IsPaused() )
            text = L"״̬: ��ͣ";
        else
            text = L"״̬: ����";
    }

    m_status->SetCaption( text, pDC );
}

void OOPlayerApp::OnWebsiteLinks(VdkVObjEvent& e)
{
    wxString url;

    switch( e.GetId() )
    {
    case OMM_LINKS_FORUM:

        url.assign( "http://www.ooplayer.org/bbs" );
        break;

    case OMM_LINKS_WEBSITE:

        url.assign( L"http://www.ooplayer.org/" );
        break;

    default:

        return;
    }

    wxLaunchDefaultBrowser( url );
}

void OOPlayerApp::OnAbout(VdkVObjEvent&)
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName( gs_DefaultInteractiveOutput );
    aboutInfo.SetVersion( gs_appVersion );
    aboutInfo.SetDescription( L"���õĿ�ƽ̨���ֲ�����" );
    aboutInfo.SetCopyright( L"(C) 2009-2012 wxn");
    aboutInfo.SetWebSite( "http://www.OOPlayer.org/" );
    aboutInfo.AddDeveloper( "wxn (wxn@ooplayer.org)" );

    aboutInfo.SetLicense( L"��л���¸�����ʵ���Ŭ����\n"
                          L"wxWidgets\n"
                          L"wxXmlSerializer\n"
                          L"FMOD Ex\n"
                          L"Loki\n"
                          L"libcurl && c-ares\n"
                          L"ǧǧ����(TTPlayer)\n"
                          L"MyTagLib\n" );
	
#ifdef __WXGTK__
    if( !wxImage::FindHandler( wxBITMAP_TYPE_PNG ) )
        wxImage::AddHandler( new wxPNGHandler );

    wxImage imgIcon( OOPFileSystem::GetAppResDir() + L"OOPlayer.png",
                     wxBITMAP_TYPE_PNG );
                     
    if( imgIcon.IsOk() )
    {
        imgIcon.Rescale( 64, 64, wxIMAGE_QUALITY_HIGH );
    
        wxIcon icon;
        icon.CopyFromBitmap( wxBitmap( imgIcon ) );
        aboutInfo.SetIcon( icon );
    }
#endif

    wxAboutBox( aboutInfo );
}

void OOPlayerApp::OnFeedback(VdkVObjEvent&)
{

}

//////////////////////////////////////////////////////////////////////////

/*static*/
wxString OOPlayerApp::GetSupportedFileTypes()
{
	return sf::Music::getSupportedFileTypes();
}

/*static*/
bool OOPlayerApp::TestFileType(const wxString& extName)
{
    if( extName.empty() || (extName == L'.') )
        return false;

	wxString ext( extName );
	ext.MakeLower();

	wxString wildTypes( GetSupportedFileTypes() );
	wxString::size_type pos = 0;

	while( (pos = wildTypes.find( ext, pos )) != wxString::npos )
    {
        wxChar following = wildTypes[pos + ext.length()];

        if( !isalpha( following ) && !isdigit( following ) )
            return true;

		pos += ext.length();
    }

    return false;
}

void OOPlayerApp::GenerateMainPanelCtrlIds(MapOfCtrlIdInfo& ids)
{
    BindCtrl( L"play", CID_PLAY, m_play );
    BindCtrl( L"pause", CID_PAUSE, m_pause );
    BindCtrl( L"stop", CID_STOP, m_stop );
    BindCtrl( L"prev", CID_PREV, m_prev );
    BindCtrl( L"next", CID_NEXT, m_btnNext );
    BindCtrl( L"mute", CID_MUTE, m_mute );

    BindCtrl( L"lyric", CID_TOGGLE_LYRIC, m_toggleLyric );
    BindCtrl( L"playlist", CID_TOGGLE_PLAYLIST, m_togglePlayList );
    BindCtrl( L"equalizer", CID_TOGGLE_EQ, m_toggleEq ); // TODO: Memory leak?

    BindCtrl( L"progress", CID_PROGRESS, m_progress );
    BindCtrl( L"volume", CID_VOLUME, m_volume );

    BindCtrl( L"info", CID_INFO, m_rollingLabel );
    BindCtrl( L"led", CID_LED, m_led );
    BindCtrl( L"stereo", CID_STEREO, m_stereo );
    BindCtrl( L"status", CID_STATUS, m_status );

    BindCtrl( L"minimize", CID_MINIMIZE, m_minimize );
    BindCtrl( L"minimode", CID_MINIMODE, m_minimode );
    BindCtrl( L"exit", CID_EXIT, m_exit );

    BindCtrlID( L"visual", CID_VISUAL );
    BindCtrlID( L"icon", CID_ICON );
    BindCtrl( L"equalizer", CID_TOGGLE_EQ, m_toggleEq );
    BindCtrl( L"open", CID_OPEN_FILE, m_openFile );
    BindCtrl( L"browser", CID_TOGGLE_BROWSER, m_toggleBrowser );
}

void OOPlayerApp::OnAddFileFolders(VdkVObjEvent& e)
{
    if( e.GetMenu() )
    {
        VdkCtrlId id = e.GetMenuItem()->id();

        if( id == OTBM_ADD_FOLDER )
        {
            m_playListPanel->AddFolder( PlayListPanel::RM_ONLY_TOP_LEVEL_FILES );
        }
        else if( id == OTBM_ADD_FOLDER_RECURSIVE )
        {
            m_playListPanel->AddFolder( PlayListPanel::RM_RECURSIVE );
        }
        else
        {
            m_playListPanel->AddFiles( m_playListPanel );
        }
    }
    else
    {
        if( e.GetId() == CID_OPEN_FILE )
        {
            {
                VdkDC dc( m_mainPanel );
                m_openFile->Update( VdkButton::NORMAL, &dc );
            }

            m_playListPanel->AddFiles( m_mainPanel );
        }
    }
}

//////////////////////////////////////////////////////////////////////////

#ifdef __WXMSW__
#   define EVT_TASKBAR_LEFT_CLICK EVT_TASKBAR_LEFT_UP
#else
#   define EVT_TASKBAR_LEFT_CLICK EVT_TASKBAR_LEFT_DOWN
#endif

BEGIN_EVENT_TABLE( OOPlayerApp, wxApp )

    EVT_VW_FIRST_SHOWN( OOPlayerApp::OnMainPanelFirstShown )

    EVT_VOBJ( CID_EXIT, OOPlayerApp::Quit )
    EVT_VOBJ( CID_MINIMIZE, OOPlayerApp::Minimize )
    EVT_VOBJ( CID_TOGGLE_LYRIC, OOPlayerApp::ShowLyricPanel )
    EVT_VOBJ( CID_TOGGLE_PLAYLIST, OOPlayerApp::ShowPlayListPanel )
    EVT_VOBJ( CID_TOGGLE_EQ, OOPlayerApp::ShowEqPanel )
    EVT_VOBJ( CID_PROGRESS, OOPlayerApp::OnProgress )
    EVT_VOBJ( CID_PLAY, OOPlayerApp::Play )
    EVT_COMMAND( wxID_ANY, MM_EVT_SONG_FINISHED, OOPlayerApp::OnSongFinished )
    EVT_VOBJ( CID_PAUSE, OOPlayerApp::Pause )
    EVT_VOBJ( CID_STOP, OOPlayerApp::Stop )
    EVT_VOBJ_RANGE( CID_PREV, CID_NEXT, OOPlayerApp::OnPrevNext )
    EVT_VOBJ( CID_LED, OOPlayerApp::StepProgress )
    EVT_VOBJ( CID_VOLUME, OOPlayerApp::OnVolume )
    EVT_VOBJ( CID_MUTE, OOPlayerApp::OnMute )

    EVT_VOBJ( CID_LYRIC, OOPlayerApp::OnLyricDragg )
    EVT_VOBJ( CID_PLAY_LIST, OOPlayerApp::OnPlayListDClick )
    EVT_VOBJ_RANGE( OOM_PREV, OOM_NEXT, OOPlayerApp::OnMenuPrevNext )

    EVT_VOBJ( CID_ABOUT, OOPlayerApp::OnAbout )
    EVT_VOBJ_RANGE( OMM_LINKS_WEBSITE, OMM_LINKS_FORUM, OOPlayerApp::OnWebsiteLinks )
    EVT_VOBJ( CID_FEEDBACK, OOPlayerApp::OnFeedback )

    EVT_TASKBAR_LEFT_CLICK( OOPlayerApp::OnTrayIconLeftClick )

    EVT_VOBJ( OPLM_PLAY_SELECTED, OOPlayerApp::OnPlaySel )
    EVT_VOBJ( OTBM_DELETE_DUPLICATED, OOPlayerApp::RemoveDuplicatedItems )
    EVT_VOBJ( OTBM_DELETE_WRONG_FILE, OOPlayerApp::RemoveWrongItems )
    EVT_VOBJ( OTBM_DELETE_FILE, OOPlayerApp::HandleDelete )
    EVT_VOBJ( OTBM_DELETE_ALL, OOPlayerApp::OnDelAll )
    EVT_VOBJ_RANGE( OPLM_DELETE, OPLM_DELETE_FILE, OOPlayerApp::HandleDelete )

    EVT_VOBJ_RANGE( OTBM_SELECT_ALL, OTBM_SELECT_REVERSE, OOPlayerApp::SelectAllNoneReverse )

    EVT_VOBJ( CID_OPEN_FILE, OOPlayerApp::OnAddFileFolders )
    EVT_VOBJ_RANGE( OTBM_ADD_FILE, OTBM_ADD_FOLDER, OOPlayerApp::OnAddFileFolders )

    EVT_VOBJ( OTBM_LIST_NEW, OOPlayerApp::AddPlayList )
    EVT_VOBJ( OTBM_LIST_OPEN, OOPlayerApp::OpenListByUser )
    EVT_VOBJ( OTBM_LIST_SAVE, OOPlayerApp::SaveListByUser )
    EVT_VOBJ_RANGE( OTBM_SORT_BY_TITLE, OTBM_SORT_BY_ADD_TIME, OOPlayerApp::SortPlayList )
    EVT_VOBJ_RANGE( OTBM_LOCATE_SHOW_DIALOG, OTBM_LOCATE_FIND_NEXT, OOPlayerApp::OnSearchPlaylist )
    EVT_COMMAND( wxID_ANY, OOP_EVT_SONG_INFO_UPDATED, OOPlayerApp::OnSongInfoUpdated )
    EVT_VOBJ( OPLM_LOCATE_FILE, OOPlayerApp::OnLocateFile )

    EVT_VOBJ( OLRCM_SEARCH, OOPlayerApp::OnSearchLyric )
    EVT_COMMAND( wxID_ANY, LyricGrabber::OOP_EVT_LYRIC_LOADED, OOPlayerApp::OnLyricLoaded )
    EVT_VOBJ( OLRCM_COPY_LRC, OOPlayerApp::OnCopyLyric )
    EVT_VOBJ( OLRCM_ASSOCIATE_LOCAL_FILE, OOPlayerApp::OnAssociateLyric )
    EVT_VOBJ( OLRCM_RELOAD_LRC, OOPlayerApp::OnReloadLyric )
    EVT_VOBJ( OLRCM_REMOVE_LRC, OOPlayerApp::OnRemoveLyric )

    EVT_VOBJ_RANGE( OOM_PLAY_PAUSE, OOM_STOP, OOPlayerApp::OnMenuPlayCtrl )
    EVT_VOBJ_RANGE( OOM_VOLUME_UP, OOM_MUTE, OOPlayerApp::OnMenuVolumeCtrl )
    EVT_VOBJ_RANGE( PLAY_MODE_SINGLE, PLAY_MODE_LIST_RANDOM, OOPlayerApp::OnPlayModes )

END_EVENT_TABLE();

