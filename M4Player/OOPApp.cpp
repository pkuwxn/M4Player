/***************************************************************
 * Name:      OOPApp.cpp
 * Purpose:   OOPlayer 的控制中心
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
#include "OOPTrayIcon.h" // 托盘图标
#include "OOPSingleLyricTask.h"
#include "OOPSingleHttpThread.h" // 和平终止线程
#include "OOPUtil.h"

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

IMPLEMENT_APP(OOPlayerApp);
OOPlayerApp *g_app = NULL;

const static wchar_t *gs_appVersion = L"1.0 Beta 3(20151122)";
const static wchar_t *gs_DefaultInteractiveOutput = L"M4Player for Linux";

// 歌曲标签信息更新时更新 OOPLabel 的显示
wxDECLARE_EVENT(OOP_EVT_SONG_INFO_UPDATED, wxCommandEvent);

// 应用程序状态状态位信息
enum OOPState {
    OOPST_MUTE                  = 1 << 0, // 静音
    // 是否处于可播放的状态，如播放列表不为空
    OOPST_PLAYABLE              = 1 << 1,
    // 假如已经对当前歌曲的歌词进行过一次自动搜索，那么后续就不要再到
    // 网络上找了，主要是避免找不到匹配歌词时换肤后又重新找一次。
    OOPST_LYRIC_SEARCHED        = 1 << 2,
};

void AssignToolTip(VdkControl *pCtrl, const wxString &toolTip) {
    if (pCtrl) {
        pCtrl->SetAddinStyle(VCS_HAND_CURSOR);
        pCtrl->SetToolTip(toolTip);
    }
}

void Alert(const wxString &errMsg) {
    enum {
        ALERT_STYLE = wxOK | wxCENTER | wxICON_ERROR,
    };

    wxMessageBox(errMsg, L"M4Player Fatal Error", ALERT_STYLE);
}

//////////////////////////////////////////////////////////////////////////

OOPlayerApp::OOPlayerApp()
    : m_checker(NULL),
      m_conf(new OOPConf),
      m_state(OOPST_PLAYABLE),
      m_endingDetector(m_music),
      m_stopWatch(m_music, m_endingDetector),
      m_desktopLyric(NULL),
      m_mainPanel(NULL),
      m_playListPanel(NULL),
      m_lyricPanel(NULL),
      m_eqPanel(NULL),
      m_playPauseItem(NULL),
      m_stopItem(NULL),
      m_muteItem(NULL),
      m_prevItem(NULL),
      m_nextItem(NULL),
      m_playModes1(NULL),
      m_playModes2(NULL),
      m_playMode(PLAY_MODE_BY_LIST),
      m_trayIcon(NULL) {
    g_app = this;
}

//////////////////////////////////////////////////////////////////////////

#include <Client.hpp>
#include <Service.hpp>

#ifdef __WXDEBUG__
#   include <fstream>
#   include <string>

#   ifdef __WXMSW__
#       define CFG_PLUGINS "E:/Desktop/OOPlayer/Audio.Plugins"
#   else
#       define CFG_PLUGINS "/media/E/Desktop/OOPlayer/Audio.Plugins.Linux"
#   endif
#endif

void LoadPlugin(wxDynamicLibrary &dll, const wxString &pluginPath) {
    dll.Load(pluginPath);
    if (dll.IsLoaded()) {
        typedef sf::Client *(*fxGetInterface_t)(void);

        // Create a valid function pointer
        fxGetInterface_t fxGetInterface =
            (fxGetInterface_t) dll.GetSymbol("getInterface", NULL);

        //check if the function is found
        if (fxGetInterface) {
            // Important: Use Detach(), otherwise the DLL will be unloaded
            // once the wxDynamibLibrary object goes out of scope
            dll.Detach();

            sf::Client *componetClient = fxGetInterface();
            sf::FactoryBase::addToList(componetClient->getFactoryList());
        }
    }
}

void InitPlugins() {
    wxDynamicLibrary dll;

#ifdef __WXDEBUG__
    std::ifstream f(CFG_PLUGINS);
    std::string pluginPath;
    while (std::getline(f, pluginPath)) {
        while (!pluginPath.empty() && isspace(pluginPath[pluginPath.length() - 1])) {
            pluginPath.erase(pluginPath.length() - 1);
        }

        if (pluginPath.empty()) {
            break;
        }

        LoadPlugin(dll, pluginPath);
    }

    f.close();
#else // !__WXDEBUG__
    wxString dirPath(OOPFileSystem::GetRootPath() + L"Plugins");
    dirPath += wxFileName::GetPathSeparator();

    wxDir puluginDir(dirPath);
    if (!puluginDir.IsOpened()) {
        return;
    }

    wxString pluginPath;
    bool hasMore = puluginDir.GetFirst(&pluginPath, wxEmptyString, wxDIR_FILES);
    while (hasMore) {
        LoadPlugin(dll, dirPath + pluginPath);
        hasMore = puluginDir.GetNext(&pluginPath);
    }
#endif // __WXDEBUG__

    sf::FactoryBase::__internal__sysInitialized = true;
}

//////////////////////////////////////////////////////////////////////////

bool OOPlayerApp::OnInit() {
    m_checker = new wxSingleInstanceChecker;
    if (m_checker->IsAnotherRunning()) {
        Alert(L"Another program instance is already running, aborting.");

        delete m_checker; // OnExit() won't be called if we return false
        m_checker = NULL;

        return false;
    }

    //-------------------------------------------

    // 初始化所有插件
    InitPlugins();

    wxString confPath(GetAppConfFilePath());
    if (wxFileExists(confPath)) {
        if (!m_conf->Load(confPath)) {
            return false;
        }
    }

    if (!InitConf()) {
        return false;
    }

    //-------------------------------------------

    new MainPanel();

    // 是否成功创建
    return m_mainPanel != NULL;
}

int OOPlayerApp::OnExit() {
    wxDELETE(m_checker);

    //===============================================
    // 删除 HTTP 工作者线程

    VdkHttpThread &singHttpThread = OOPSingleHttpThread::Instance();

    wxSemaphore destoryWaiter;
    singHttpThread.SetDestoryWaiter(destoryWaiter);
    singHttpThread.StopAndExit();
    destoryWaiter.Wait();

    return wxApp::OnExit();
}

bool OOPlayerApp::InitConf() {
    wxString strSkinFolder(OOPFileSystem::GetSkinRootDir());
    strSkinFolder += m_conf->m_skinFolderName;
    strSkinFolder += wxFileName::GetPathSeparator();

    if (!wxDirExists(strSkinFolder)) {
        Alert(L"Error opening skin dir `" + strSkinFolder + L"`.");
        return false;
    }

    OOPSkin::SkinInfo info(m_conf->m_skinFolderName, strSkinFolder);
    SelectSkin(info);

    return true;
}

inline wxString OOPlayerApp::GetAppConfFilePath() const {
    wxString confPath(OOPFileSystem::GetRootPath());
    return confPath += L"OOPConf.xml";
}

void OOPlayerApp::UpdateConf() {
    m_conf->m_currSong = wxNOT_FOUND;
    LcCellIter locked(m_playList->GetLocked());
    if (locked != m_playList->end()) {
        m_conf->m_currSong = m_playList->IndexOf(*locked);
    }

    m_conf->m_playMode = m_playMode;
    // 暂停也作正在播放来考虑
    m_conf->m_playing = IsPlaying() || IsPaused();
    m_conf->m_currPos = m_music.getPlayingOffset().asMilliseconds();

    m_conf->m_mute = IsMute();
    m_conf->m_volume = m_volume->GetProgress();

    m_mainPanel->UpdateStickyState(m_conf->mainPanelWndState);
    if (m_playListPanel) {
        m_playListPanel->UpdateStickyState(m_conf->playlistPanelWndState);
    }
    if (m_lyricPanel) {
        m_lyricPanel->UpdateStickyState(m_conf->lyricPanelWndState);
    }
    if (m_eqPanel) {
        m_eqPanel->UpdateStickyState(m_conf->eqPanelWndState);
    }
}

void OOPlayerApp::SavePanelVisibilities() {
    m_conf->m_playListShown = m_togglePlayList->IsToggled();
    m_conf->m_lyricShown = m_toggleLyric->IsToggled();
    m_conf->m_eqShown = m_toggleEq->IsToggled();
}

void OOPlayerApp::SaveConf() {
    UpdateConf();

    m_conf->Save(GetAppConfFilePath());
}

void OOPlayerApp::LoadPlayList(const wxString &path) {
    if (wxFileExists(path)) {
        wxFileInputStream fin(path);
        m_playList->Serialize(fin);
    }
}

void OOPlayerApp::SavePlayList(const wxString &path) {
    wxFileOutputStream fout(path);
    m_playList->Serialize(fout);
    fout.Close();
}

wxString OOPlayerApp::GetPlayListPath() const {
    wxString playlistPath(OOPFileSystem::GetRootPath());
    return playlistPath += L"Default.Playlist";
}

void OOPlayerApp::OnSelectSkin(VdkVObjEvent &e) {
    m_mainPanel->HideMenu();

    // 保存对配置信息可能的修改
    SavePanelVisibilities();
    UpdateConf(); // 更新但不保存

    int index = (int) e.GetClientData();
    SelectSkin(m_skinList.GetSkin(index));
}

void OOPlayerApp::SelectSkin(OOPSkin::SkinInfo &info) {
    if ((&info == m_skinList.GetSelectedSkin()) ||
            (VdkControl::GetRootPath() == info.rootPath()) ||
            (!wxDirExists(info.rootPath()))) {
        return;
    }

    // 程序刚刚启动，首次设置皮肤
    bool isFirstRun = !m_mainPanel;

    //-------------------------------------------------------
    // 皮肤相关的配置

    // 清空不匹配的皮肤相关窗口位置信息
    if (!isFirstRun) {
        m_conf->ClearStickyWindowStates();
    }

    // 设置进程空间内 VdkControl 的皮肤信息
    VdkControl::Initialize(info.maskColor(), info.rootPath());

    if (isFirstRun) {
        return;
    }

    //------------------------------------------------------

    OOPList *playList = m_playList;
    OOPLyric *embeddedLyric = m_embeddedLyric;
    OOPPtrs::Clear();

    bool showStates[3]; // 三个子窗口的可视状态
    memset(showStates, 0, sizeof(showStates));

    showStates[0] = m_playListPanel->IsShown();
    if (m_lyricPanel) {
        showStates[1] = m_lyricPanel->IsShown();
    }

    if (m_eqPanel) {
        showStates[2] = m_eqPanel->IsShown();
    }

    m_mainPanel->ResetSkin();

    if (showStates[0] && m_togglePlayList) {
        m_togglePlayList->Toggle(true, NULL, false);
    }

    if (showStates[1] && m_toggleLyric) {
        m_toggleLyric->Toggle(true, NULL, false);
    }

    if (showStates[2] && m_toggleEq) {
        m_toggleEq->Toggle(true, NULL, false);
    }

    //------------------------------------------------------

    // 换肤前后注意保存好 m_currSong
    int index = playList->GetLockedIndex();
    // 清除锁定标志
    playList->SetLocked(playList->end(), NULL);
    playList = NULL;

    m_playListPanel->ResetSkin();

    if (index != wxNOT_FOUND) {
        m_playList->SetLocked(index, NULL);
        // 开始滚动歌曲信息
        RollLabel(NULL, true);
    } else if (IsPlayable()) { // 更新各控件的可能“禁用”状态
        DoStop(NULL, NULL, CL_LEAVE);
    }

    // 托盘图标
    if (m_trayIcon) {
        UpdateTrayIcon();
    }

    //-------------------------------------------

    if (m_lyricPanel) {
        m_lyric.Dettach(embeddedLyric);

        m_lyricPanel->ResetSkin();
        InitLyric();
    }

    if (m_eqPanel) {
        m_eqPanel->ResetSkin();
    }

    SaveCurrentSkin(info);
}

void OOPlayerApp::SaveCurrentSkin(OOPSkin::SkinInfo &info) {
    m_skinList.SetSelectedSkin(&info);
    m_conf->m_skinFolderName = info.folderName();
}

OOPStickyWindowState *OOPlayerApp::GetPanelStickyState(const wxString &uuid) {
    const static wchar_t *sUUIDs[] = {
        L"16D3DE86-170A-48EE-B9DE-509186AAC389", // 主窗口
        L"C5BC105E-ED0F-4578-9215-FA1EE71FC5A9",
        L"20772658-9A2B-4AE1-BDEC-5BE1823A223E",
        L"0F146699-FA80-4120-85CD-057E09FED8C8"
    };

    OOPStickyWindowState *states = &m_conf->mainPanelWndState;

    for (int i = 0; i < sizeof(sUUIDs) / sizeof(sUUIDs[0]); i++) {
        if (uuid == sUUIDs[i]) {
            return states[i].GetRect().IsEmpty() ? NULL : &states[i];
        }
    }

    wxASSERT(false);
    return NULL;
}

void OOPlayerApp::HidePanel(OOPWindow *win) {
    if (win == m_mainPanel) {
        VdkVObjEvent fakeEvent;
        Quit(fakeEvent);
    } else if (win == m_playListPanel) {
        DismissPlayListPanel();
    } else if (win == m_lyricPanel) {
        DismissLyricPanel();
    } else if (win == m_eqPanel) {
        DismissEqPanel();
    } else {
        wxLogError(L"Unexpected panel handle.");
    }
}

void OOPlayerApp::Quit(VdkVObjEvent &) {
    SavePanelVisibilities();

    // 将窗口全部隐藏，避免出现“假死”
    if (m_playListPanel) {
        m_playListPanel->Hide();
    }
    if (m_lyricPanel) {
        m_lyricPanel->Hide();
    }
    if (m_eqPanel) {
        m_eqPanel->Hide();
    }
    m_mainPanel->Hide();

    SaveConf();

    // 停止播放
    if (IsPlayable() && !IsStopped()) {
        DoStop(NULL, NULL, CL_LEAVE);
    }

    // 保存当前播放列表
    SavePlayList(GetPlayListPath());

    // 删除所有 OOPSong 类对象
    m_playList->RemoveAllSongPtrs();

    //-------------------------------------------------
    // 销毁所有窗口

    if (m_trayIcon) {
        wxDELETE(m_trayIcon);
    }

    m_mainPanel->BeginExit();
    m_mainPanel->Destroy();
}

void OOPlayerApp::Minimize(VdkVObjEvent &e) {
    if (ToggleAllOnScreenWindow()) {
        // 假如“隐藏到托盘”操作成功，更新最小化按钮的正常状态
        m_minimize->Update(VdkButton::NORMAL, e.GetVObjDC());
    }
}

bool OOPlayerApp::DoPlay() {
    if (IsStopped()) {
        if (!m_music.openFromFile(NarrowedPath(m_currSong->path()))) {
            // 播放失败
            return false;
        }

        if (m_music.play()) {
            // 启动歌曲播放完结检测器
            m_endingDetector.Play();
        }
    } else {
        m_music.play();
    }

    return IsPlaying();
}

void OOPlayerApp::DoPause() {
    m_music.pause();
}

void OOPlayerApp::DoStop() {
    // 重置歌曲播放完结检测器，必须使之先于 Music::stop() 被调用，否则检测器
    // 会不正确地认为这是一次歌曲正常完结事件
    m_endingDetector.Stop();

    m_music.stop();
}

void OOPlayerApp::DoMute(bool mute) {
    if (IsMute() == mute) {
        return;
    }

    SetAddinState(OOPST_MUTE);
    if (!mute) {
        RemoveState(OOPST_MUTE);
    }

    if (!IsStopped()) {
        m_music.setVolume(mute ? 0 : m_volume->GetProgress());
    }
}

void OOPlayerApp::OnPlayModes(VdkVObjEvent &e) {
    SwitchPlayMode((OOPlayerPlayMode) e.GetId());
}

void OOPlayerApp::SwitchPlayMode(OOPlayerPlayMode mode) {
    int old = m_playMode - PLAY_MODE_SINGLE;
    m_playModes1->GetItem(old)->checked(false);
    m_playModes2->GetItem(old)->checked(false);

    m_playMode = mode;
    int curr = m_playMode - PLAY_MODE_SINGLE;
    m_playModes1->GetItem(curr)->checked(true);
    m_playModes2->GetItem(curr)->checked(true);
}

void OOPlayerApp::Play(VdkVObjEvent &e) {
    wxASSERT(e.GetMenu() == NULL);   // 不能是一个菜单事件
    wxASSERT(!m_playList->IsEmpty());

    wxDC *pDC = e.GetVObjDC();
    if (!m_playList->HasLocked()) {
        // 直接播放播放列表中的第一个
        VdkDC vdc(m_playListPanel);
        m_playList->SetLocked(m_playList->begin(), &vdc);
    }

    // 先更新当前项的 OOPSong 对象（DoPlay() 用到）
    UpdateCurrSong();

    if (!DoPlay()) {
        // 播放失败，不用撤销当前歌曲？
        return;
    }

    //-------------------------------------------
    // 更新 GUI

    ShowHidePlayPause(pDC);

    if (m_stop) {
        m_stop->Enable(true, pDC);
    }
    if (m_led) {
        m_led->ResumeLed();
    }
    if (m_progress) {
        m_progress->Enable(true, pDC);
    }
    if (m_rollingLabel) {
        if (m_rollingLabel->IsOk()) {
            m_rollingLabel->StartRolling();
        } else {
            RollLabel(pDC, true);
        }
    }

    m_playPauseItem->caption(L"暂停");
    m_stopItem->disabled(false);

    SetStatusLabel(wxEmptyString, pDC);

    //-------------------------------------------

    // 开始播放歌词
    UpdateLyricState();
}

void OOPlayerApp::StopPlayer(wxDC *pDC) {
    RemoveState(OOPST_PLAYABLE);

    if (m_play) {
        m_play->Enable(false, pDC);
    }
    if (m_pause) {
        m_pause->Enable(false, pDC);
    }
    if (m_stop) {
        m_stop->Enable(false, pDC);
    }
    if (m_prev) {
        m_prev->Enable(false, pDC);
    }
    if (m_btnNext) {
        m_btnNext->Enable(false, pDC);
    }
    if (m_progress) {
        m_progress->Enable(false, pDC);
    }

    if (m_rollingLabel) {
        m_rollingLabel->StopRolling(pDC);
    }

    SetStatusLabel(wxEmptyString, pDC);

    // 禁用“播放控制”菜单项
    m_playPauseItem->disabled(true);
    m_stopItem->disabled(true);
    m_prevItem->disabled(true);
    m_nextItem->disabled(true);
}

void OOPlayerApp::EnablePlayer() {
    if (!IsPlayable()) {
        ReverseState(OOPST_PLAYABLE);

        VdkDC vdc(m_mainPanel);
        if (m_play) {
            m_play->Enable(true, &vdc);
        }
        if (m_pause) {
            m_pause->Enable(true, NULL);
        }
        if (m_playList->GetItemCount() > 1) {
            if (m_prev) {
                m_prev->Enable(true, &vdc);
            }
            if (m_btnNext) {
                m_btnNext->Enable(true, &vdc);
            }
        }

        m_playPauseItem->disabled(false);
        m_stopItem->disabled(false);
        m_prevItem->disabled(false);
        m_nextItem->disabled(false);
    }
}

void OOPlayerApp::Pause(VdkVObjEvent &e) {
    wxASSERT(e.GetMenu() == NULL);

    DoPause();

    //-------------------------------------------

    VdkValidDC vdc(e.GetVObjDC(), m_mainPanel, NULL);
    wxDC *pDC = vdc.operator wxDC*();
    ShowHidePlayPause(pDC);

    if (m_lyric) {
        m_lyric->Pause();
    }

    if (m_led) {
        m_led->PauseLed();
    }

    m_playPauseItem->caption(L"播放");
    wxASSERT(!m_stopItem->disabled());

    SetStatusLabel(wxEmptyString, pDC);
}

void OOPlayerApp::Stop(VdkVObjEvent &e) {
    wxASSERT(e.GetMenu() == NULL);

    VdkValidDC vdc(e.GetVObjDC(), m_mainPanel, NULL);
    VdkDC playListDC(m_playListPanel);

    DoStop(vdc, &playListDC, CL_CLEAR);
}

void OOPlayerApp::DoStop(wxDC *pDC, wxDC *playListDC, ClearLyricOpt clearLyric) {
    wxASSERT(IsPlayable());

    DoStop();

    //----------------------------------------------

    // 清空播放列表锁定项
    m_playList->SetLocked(m_playList->end(), playListDC);

    // 播放停止，更新当前项的 OOPSong 对象
    UpdateCurrSong();

    if (clearLyric == CL_CLEAR) {
        ClearLyric();
    }

    if (m_led) {
        m_led->StopLed(pDC);
    }

    if (m_rollingLabel) {
        m_rollingLabel->StopRolling(pDC);
    }

    if (m_stop) {
        m_stop->Enable(false, pDC);
    }

    if (m_progress) {
        m_progress->GoTo(0, pDC, false);
        m_progress->Enable(false, pDC);
    }

    RollTaskbarTitle();
    ShowHidePlayPause(pDC);
    SetStatusLabel(wxEmptyString, pDC);
    UpdateTrayIcon();

    m_playPauseItem->caption(L"播放");
    m_stopItem->disabled(true);

    // 提交所有可能的被延缓的独占性文件操作
    SingleDelayModStack::Instance().CommitAll();
}

void OOPlayerApp::OnMenuPlayCtrl(VdkVObjEvent &e) {
    VdkDC dc(m_mainPanel);
    VdkVObjEvent vobjEvt;
    vobjEvt.SetVObjDC(&dc);

    switch (e.GetId()) {
    case OOM_PLAY_PAUSE:

        if (IsPlaying()) {
            vobjEvt.SetId(CID_PAUSE);
            vobjEvt.SetCtrl(m_pause);
            Pause(vobjEvt);
        } else {
            vobjEvt.SetId(CID_PLAY);
            vobjEvt.SetCtrl(m_play);
            Play(vobjEvt);
        }

        break;

    case OOM_STOP:

        if (!IsStopped()) {
            vobjEvt.SetId(CID_STOP);
            vobjEvt.SetCtrl(m_stop);
            Stop(vobjEvt);
        }

        break;

    default:

        break;
    }
}

void OOPlayerApp::OnMainPanelFirstShown(VdkWindowFirstShownEvent &) {
    wxASSERT(m_mainPanel);

#ifdef __WXMSW__
    // 不知道为什么，在 wxMSW 下面后调用 Show 的窗口的第一次
    // 重画事件会在先调用的窗口的前面首先得到
    ShowAllPanelsOnReady();
#endif
}

void OOPlayerApp::ShowAllPanelsOnReady() {
    if (m_conf->m_playListShown) {
        m_playListPanel->Show(true);
    }

    if (m_conf->m_lyricShown) {
        InitLyric();
        m_lyricPanel->Show(true);
    }

    if (m_conf->m_eqShown) {
        m_eqPanel->Show(true);
    }
}

void OOPlayerApp::SetMainPanel(MainPanel *panel, wxDC *pDC) {
    if (!m_togglePlayList || !m_toggleLyric) {
        Alert(L"切换各个子窗口是否显示的按钮必须在 XML 文件中定义。");
        return;
    }

    bool firstRun = !m_mainPanel;
    if (firstRun) {
        m_windowManager.SetMainWindow(panel);
        m_mainPanel = panel;

        // 创建菜单
        GenerateMainMenu();

        // 创建托盘图标
        if (OOPTrayIcon::IsAvailable()) {
#ifdef __WXGTK__
            char *wm = getenv("XDG_CURRENT_DESKTOP");
            bool isUnity = wm && (strcmp(wm, "Unity") == 0);

            if (!isUnity || m_conf->m_showTaskBarIconUnderUnity)
#endif
            {
                m_mainPanel->Bind(wxEVT_IDLE,
                                  &OOPlayerApp::OnIdleCreateTrayIcon,
                                  this);
            }
        }
    }

    // 重定位主窗口，因为主窗口一般没有指定“rect”节点，OOPWindow 无法处理
    OOPStickyWindowState *sstate = GetPanelStickyState(m_mainPanel->GetUUID());
    if (sstate) {
        wxRect mainRect(sstate->GetRect());
        if (!mainRect.IsEmpty()) {
            m_mainPanel->Move(mainRect.x, mainRect.y);
        }
    } else {
        m_mainPanel->Move(250, 250);
    }

    //-------------------------------------------

    if (m_progress) {
        m_progress->SetAddinStyle(VSS_CLICK_TO_GO);
        m_progress->GoTo(GetProgress(), pDC);

        AssignToolTip(m_progress, L"播放进度");
    }

    if (m_volume) {
        m_volume->SetAddinStyle(VSS_CLICK_TO_GO);
        AssignToolTip(m_volume, L"音量");
    }

    if (m_pause && m_play) {
        m_play->Show(false, pDC);

        AssignToolTip(m_play, L"播放");
        AssignToolTip(m_pause, L"暂停");
    }

    if (m_led) {
        m_led->SetStopWatch(&m_stopWatch);
    }

    if (m_openFile) {
        m_openFile->SetAddinStyle(VCS_DELAY_EVENT);
        AssignToolTip(m_openFile, L"打开文件");
    }

    if (m_exit) {
        // 需要等待播放线程终结，在此期间可能会有其它事件被分派处理
        m_exit->SetAddinStyle(VCS_DELAY_EVENT);
    }

    //-------------------------------------------
    // 其他按钮的 ToolTip

    AssignToolTip(m_minimize, L"最小化");
    AssignToolTip(m_minimode, L"迷你模式");
    AssignToolTip(m_exit, L"退出");
    AssignToolTip(m_stop, L"停止");
    AssignToolTip(m_mute, L"静音");
    AssignToolTip(m_prev, L"前一首");
    AssignToolTip(m_btnNext, L"后一首");
    AssignToolTip(m_toggleLyric, L"歌词秀");
    AssignToolTip(m_togglePlayList, L"播放列表");
    AssignToolTip(m_toggleEq, L"均衡器");

    if (m_toggleBrowser) {
        AssignToolTip(m_toggleBrowser, L"浏览器");
    }

    //-------------------------------------------

    // 恢复音量
    DoMute(m_conf->m_mute);
    ToggleMute(m_conf->m_mute, pDC);

    m_volume->GoTo(m_conf->m_volume, pDC, false);
    SetStatusLabel(wxEmptyString, pDC);
    ShowHidePlayPause(pDC);

    if (firstRun) {
        wxASSERT(!m_playListPanel);

        // 创建子窗口前主窗口必须为可视状态，否则子窗口会在任务栏上显示
        m_mainPanel->Show(true);
        m_playListPanel = new PlayListPanel(m_mainPanel, false);

        if (m_conf->m_playListShown) {
            m_togglePlayList->Toggle(true);
        }

        // 载入的播放列表可能是空的，禁用播放器
        if (m_playList->IsEmpty()) {
            StopPlayer(NULL);
        } else {
            // 设置为上次正在播放的歌曲
            if (m_conf->m_playing) {
                if (m_conf->m_currSong >= 0 &&
                    m_conf->m_currSong < m_playList->GetItemCount()) {
                    BeginSong(m_conf->m_currSong, pDC, NULL);

                    if ((m_conf->m_currPos > 0) &&
                        (m_conf->m_currPos < int(GetCurrSongLength()))) {
                        SetPlayingOffset(sf::milliseconds(m_conf->m_currPos));
                    }

                    // TODO: 将这一句分离出来
                    if (!IsStopped()) {
                        m_music.setVolume(IsMute() ? 0 : m_volume->GetProgress());
                    }
                } else {
                    m_conf->m_playing = false;
                    m_conf->m_currSong = wxNOT_FOUND;
                    m_conf->m_currPos = 0;
                }
            } else { // 更新各控件的可能“禁用”状态
                DoStop(pDC, NULL, CL_LEAVE);
            }
        }

        if (m_conf->m_lyricShown) {
            m_toggleLyric->Toggle(true);

            m_lyricPanel = new LyricPanel(m_mainPanel);
        }

        if (m_conf->m_eqShown) {
            m_toggleEq->Toggle(true);

            m_eqPanel = new EqPanel(m_mainPanel);
        }
    }
    // !firstRun
    else if (!IsStopped()) {
        // 在播放过程中切换皮肤，不要重置当前正在播放的歌曲
        if (m_led) {
            m_led->StartLed();
        }

        if (m_progress) {
            m_progress->GoTo(GetProgress(), pDC, false);
        }
    }

    // 恢复播放模式
    if (m_conf->m_playMode > PLAY_MODE_LOWER_BOUND &&
        m_conf->m_playMode < PLAY_MODE_UPPER_BOUND) {
        SwitchPlayMode((OOPlayerPlayMode) m_conf->m_playMode);
    }

#ifndef __WXMSW__
    // 貌似在 GTK+ 下面第一次显示主窗口后再显示子窗口有点晚了
    if (firstRun) {
        ShowAllPanelsOnReady();
    }
#endif
}

void OOPlayerApp::BeginSong(LcCellIter songIter, wxDC *pDC, wxDC *playListDC) {
    // 无法从 index 优雅地得到 LcCellIter
    BeginSong(m_playList->IndexOf(*songIter), pDC, playListDC);
}

void OOPlayerApp::BeginSong(int index, wxDC *pDC, wxDC *playListDC) {
    // 这里的 playListDC 不应设为 NULL ,
    // 否则原来无效的那个锁定项不会得到更新
    DoStop(pDC, playListDC, CL_LEAVE);

    int indexAtFirst = index;
    int numSongs = m_playList->GetItemCount();
    // 最多尝试 20 次
    const static int maxTries = 20;
    int tries = 0;

    do {
        m_playList->SetLocked(index, playListDC);

        if (m_lyric) {
            // 重置歌词搜索状态
            RemoveState(OOPST_LYRIC_SEARCHED);

            m_lyric->ClearLyric(NULL);
        }

        VdkVObjEvent fakeEvent(CID_PLAY);
        fakeEvent.SetVObjDC(pDC);
        Play(fakeEvent);

        // 存在播放失败的可能
        if (m_music.getStatus() == sf::Music::Playing) {
            // 不应该在 Play() 中增加播放次数( Play() 可能是恢复播放)
            m_currSong->playCountPlus();
            UpdateTrayIcon();

            // 播放成功，退出循环
            return;
        } else { // 播放失败
            wxLogDebug(L"歌曲`%s`播放失败。", m_currSong->path());

            //--------------------------------------------

            // 跳到下一首
            index++;

            // 回头
            if (index == numSongs) {
                // 其它迭代模式都不适宜从头开始找
                // 这些迭代模式分别为单曲播放、单曲循环、列表顺序
                if ((m_playMode == PLAY_MODE_LIST_RECYCLE) ||
                        (m_playMode == PLAY_MODE_LIST_RANDOM)) {
                    index = 0;
                }
            }
        }

        tries++;

    } while ((tries < maxTries) &&
             (index < numSongs) &&
             (index != indexAtFirst));

    // 整个播放列表都不可播放
    DoStop(pDC, playListDC, CL_CLEAR);
}

// 获取标准 LRC 保存目录
wxString GetStdLyricDir() {
    wxString lrcDir(OOPFileSystem::GetRootPath());
    lrcDir += L"Lyrics";
    lrcDir += wxFileName::GetPathSeparator();

    return lrcDir;
}

void OOPlayerApp::UpdateLyricState() {
    if (m_lyric) {
        if (!m_lyric->IsOk()) {
            if (LoadCurrSongLyric()) {
                m_lyric->Start();
            }
        } else {
            m_lyric->Start();
        }
    }
}

void OOPlayerApp::ClearLyric() {
    if (m_lyric && m_lyric->IsOk()) {
        m_lyric->ClearLyric(NULL);

        VdkDC lyricDC(m_lyricPanel);
        m_lyric->ResetInteractiveOutput(&lyricDC);
    }
}

bool OOPlayerApp::LoadCurrSongLyric() {
    if (m_lyric) {
        wxASSERT(m_currSong);

        int nTimeSum = m_currSong->length() * 1000;

        // 优先使用内嵌歌词
        wxString embedded(m_currSong->embeddedLyric());
        if (!embedded.empty()) {
            if (m_lyricParser.Load(embedded, nTimeSum)) {
                m_lyric->AttachParser(m_lyricParser);
                return true;
            }
        }

        wxString path(m_currSong->lrcPath());
        if (wxFileExists(path)) {
            if (m_lyricParser.LoadFile(path, nTimeSum)) {
                m_lyric->AttachParser(m_lyricParser);
                return true;
            }

            // 继续使用其它途径进行搜索
        }

        // 假如已经进行过一次自动搜索，那么 ClientData 应该设为不为空。
        // 主要是避免找不到匹配歌词时换肤后又重新找一次。
        if (!TestState(OOPST_LYRIC_SEARCHED)) {
            // 到服务器上面搜索
            NewLyricDownloadSession();

            //------------------------------------------------------

            m_lyric->ClearLyric(NULL);   // 清除背景

            VdkDC dc(m_lyricPanel);
            m_lyric->SetInteractiveOutput(L"正在服务器上搜索歌词...", &dc);
        }
    }

    return false;
}

void OOPlayerApp::NewLyricDownloadSession() {
    wxASSERT(m_currSong);

    wxString strArtist;
    wxString strTitle;

    // 假如无法提取正确的艺术家和标题，直接使用文件名进行搜索
    if (!m_currSong->TryGetArtistAndTitle(strArtist, strTitle)) {
        strTitle = m_currSong->fileName();
    }

    LyricGrabber::Task &singleTask = OOPSingleLyricTask::Instance();
    singleTask.SetHost(0);   // 默认选第一个歌词服务器

    singleTask.SetArtist(strArtist);
    singleTask.SetTitle(strTitle);
    singleTask.SetPath(m_currSong->path());

    singleTask.GrabbList(LyricGrabber::Task::ST_AUTO_SELECT_AND_DOWNLOAD);
}

void OOPlayerApp::RollLabel(wxDC *pDC, bool startAtOnce) {
    if (m_rollingLabel) {
        wxASSERT(m_currSong);

        wxArrayString info;
        info.Add(m_currSong->BuildTitle(GetPlayListTitleFormat()));

        wxString strArtist(m_currSong->artist());
        if (!strArtist.empty()) {
            info.Add(L"艺术家: " + strArtist);
        }

        wxString strAlbum(m_currSong->album());
        if (!strAlbum.empty()) {
            info.Add(L"专辑: " + strAlbum);
        }

        wxString strLength(OOPSong::GetStdTimeStr(m_currSong->length()));
        info.Add(wxString::Format(L"长度: %s", strLength));

        //-------------------------------------------------------

        m_rollingLabel->SetItems(info, pDC);

        if (startAtOnce) {
            m_rollingLabel->StartRolling();
        }
    }
}

void OOPlayerApp::GeneratePlayList(OOPList *playList) {
    m_playList = playList;
    m_playList->SetID(CID_PLAY_LIST);

    // 换肤后不需要重新读入播放列表
    if (!m_playList->GetVdkWindow()->IsReseting()) {
        LoadPlayList(GetPlayListPath());
    }
}

wxString OOPlayerApp::GetPlayListTitleFormat() const {
    return L"%A - %T";
}

void OOPlayerApp::ShowPlayListPanel(VdkVObjEvent &e) {
    bool show = m_togglePlayList->IsToggled();
    // 必须在改变窗口位置之前调用
    m_playListPanel->NotifyShowHideAndStick(show);
    m_playListPanel->Show(show);

    if (show) {
        m_playListPanel->SetFocus();
    }
}

void OOPlayerApp::DismissPlayListPanel() {
    m_playListPanel->NotifyShowHideAndStick(false);
    m_playListPanel->Hide();

    VdkDC dc(m_mainPanel);
    m_togglePlayList->Toggle(false, &dc, false);
}

void OOPlayerApp::ShowLyricPanel(VdkVObjEvent &e) {
    if (!m_lyricPanel) {
        wxASSERT(m_toggleLyric->IsToggled());

        //-------------------------------------

        m_lyricPanel = new LyricPanel(m_mainPanel);
        InitLyric();

        m_lyricPanel->SetTitle(m_mainPanel->GetTitle());
    }

    bool show = m_toggleLyric->IsToggled();
    m_lyricPanel->NotifyShowHideAndStick(show);
    m_lyricPanel->Show(show);
}

void OOPlayerApp::DismissLyricPanel() {
    m_lyricPanel->NotifyShowHideAndStick(false);
    m_lyricPanel->Hide();

    VdkDC dc(m_mainPanel);
    m_toggleLyric->Toggle(false, &dc, false);
}

void OOPlayerApp::InitLyric() {
    //CreateDesktopLyric();

    //====================================================
    // 内嵌歌词

    m_embeddedLyric = m_lyricPanel->m_lyric;
    m_lyric.Attach(m_embeddedLyric);

    if (m_embeddedLyric) {
        m_embeddedLyric->SetID(CID_LYRIC);

        m_lyric->SetStopWatch(m_stopWatch);
        m_lyric->SetDefualtInteractiveOutput(gs_DefaultInteractiveOutput);

        if (!IsStopped()) {
            if (LoadCurrSongLyric()) {
                if (IsPlaying()) {
                    m_lyric->Start();
                } else {
                    // Start() 会设置正确的进度，因此仅需在暂停时
                    // 调用本函数
                    m_lyric->GoTo(GetProgress(), NULL, true);
                }
            }
        } else { // 绘制交互性文本
            VdkDC lyricDC(m_lyricPanel);
            m_embeddedLyric->Draw(lyricDC);
        }
    }
}

void OOPlayerApp::CreateDesktopLyric() {
    wxASSERT(m_mainPanel);

    if (!m_desktopLyric) {
        m_desktopLyric = new OOPDesktopLyric(m_mainPanel);
        m_desktopLyric->Move(20, 620);

        m_desktopLyric->SetStopWatch(m_stopWatch);
        m_desktopLyric->AttachParser(m_lyricParser);

        m_lyric.Attach(m_desktopLyric);
    }

    m_desktopLyric->Show();
}

void OOPlayerApp::ShowEqPanel(VdkVObjEvent &) {
    if (!m_eqPanel) {
        wxASSERT(m_toggleEq->IsToggled());

        m_eqPanel = new EqPanel(m_mainPanel);
        m_eqPanel->SetTitle(m_mainPanel->GetTitle());
    }

    bool show = m_toggleEq->IsToggled();
    m_eqPanel->NotifyShowHideAndStick(show);
    m_eqPanel->Show(show);
}

void OOPlayerApp::DismissEqPanel() {
    m_eqPanel->NotifyShowHideAndStick(false);
    m_eqPanel->Hide();

    VdkDC dc(m_mainPanel);
    m_toggleEq->Toggle(false, &dc, false);
}

//////////////////////////////////////////////////////////////////////////

void OOPlayerApp::ShowHidePlayPause(wxDC *pDC) {
    if (m_play && m_pause) {
        VdkButton *show = IsPlaying() ? m_pause : m_play;
        VdkButton *hide = (show == m_pause) ? m_play : m_pause;

        show->Show(true, NULL);

        // 重设按钮状态，避免状态残留
        if (show->ContainsPointerGlobally()) {
            show->Update(VdkButton::HOVERING, pDC);
        } else {
            show->Update(VdkButton::NORMAL, pDC);
        }

        hide->Show(false, NULL);
    }
}

void OOPlayerApp::OnProgress(VdkVObjEvent &e) {
    // 迟来的事件？
    if (IsStopped()) {
        return;
    }

    unsigned int nTimeSum = GetCurrSongLength();
    int eventId = (int) e.GetClientData();
    wxDC *pDC = e.GetVObjDC();

    if (eventId == NORMAL) {
        double newProgress = m_progress->GetProgress();
        // 拖到最后当作无效
        if (newProgress == 1) {
            m_progress->GoTo(GetProgress(), pDC);
            return;
        } else {
            // 必须先改变当前播放进度，歌词秀依赖于当前值
            if (!SetPlayingOffset(sf::milliseconds(nTimeSum * newProgress))) {
                return; // 进度条会自动恢复到适当的位置
            }

            if (m_lyric && m_lyric->IsOk()) {
                VdkDC dc(m_lyricPanel);
                m_lyric->GoTo(newProgress, &dc, IsPaused());
            }
        }

        if (m_led) {
            // ResumeLed() 不会立刻重画控件
            if (pDC) {
                m_led->Draw(*pDC);
            }

            m_led->ResumeLed();
        }
    } else if (eventId == DRAGGING) {
        if (m_led) {
            m_led->PauseLed();

            if (pDC) {
                long currTime = nTimeSum * m_progress->GetProgress();
                m_led->Combine(currTime, *pDC);
            }
        }
    }
}

void OOPlayerApp::OnLyricDragg(VdkVObjEvent &e) {
    wxASSERT(!IsStopped());

    unsigned int timeToGo = (unsigned int) e.GetClientData();
    unsigned int nTimeSum = GetCurrSongLength();

    if (!SetPlayingOffset(sf::milliseconds(timeToGo))) {
        // 寻道失败
        m_lyric->GoTo(GetProgress(), e.GetVObjDC(), IsPaused());
        return;
    }

    // 桌面歌词无法知悉内嵌歌词的拖动事件
    if (m_desktopLyric) {
        m_desktopLyric->GoTo(GetProgress(), NULL, IsPaused());
    }

    if (m_progress || m_led) {
        VdkDC dc(m_mainPanel);

        if (m_progress) {
            m_progress->GoTo(double(timeToGo) / nTimeSum, &dc);
        }

        if (m_led) {
            m_led->Draw(dc);
        }
    }
}

bool OOPlayerApp::SetPlayingOffset(sf::Time offset) {
    m_endingDetector.Stop();
    bool ok = m_music.setPlayingOffset(offset);
    // 重启探测器
    m_endingDetector.Play();

    return ok;
}

void OOPlayerApp::OnPlaySel(VdkVObjEvent &e) {
    const ArrayOfSortedInts &selItems = m_playList->GetSelectedItems();
    wxASSERT(!selItems.empty());

    if (m_playList->GetLockedIndex() != selItems[0]) {
        VdkDC dc(m_mainPanel);
        VdkDC playListDC(m_playListPanel);

        BeginSong(selItems[0], &dc, &playListDC);
    }
}

void OOPlayerApp::HandleDelete(VdkVObjEvent &e) {
    // 物理删除歌曲文件
    bool delPhysically =
        (e.GetId() == OPLM_DELETE_FILE) ||
        (e.GetId() == OTBM_DELETE_FILE);

    // TODO:
    if (delPhysically) {
        if (wxMessageBox(L"是否确认从磁盘上删除这些歌曲文件？\n"
                         L"注意：一旦删除就不可恢复！",
                         L"物理删除歌曲文件",
                         wxYES_NO | wxICON_ASTERISK,
                         m_playListPanel)
                == wxNO) {
            return;
        }
    }

    VdkDC dc(m_mainPanel);
    // 这里不能将更新区域只设为播放列表控件的作用域，因为
    // 这个作用域没有将滚动条包含进来
    VdkDC playListDC(m_playListPanel);

    DelAction da = delPhysically ? DA_DEL_PHYSICALLY : DA_DEL_FROM_LIST_ONLY;
    DoHandleDelete(da, dc, playListDC);
}

void OOPlayerApp::DoHandleDelete(DelAction da, wxDC &dc, wxDC &playListDC) {
    if (!m_playList->HasSelected()) {
        return;
    }

    const ArrayOfSortedInts &currSel = m_playList->GetSelectedItems();
    wxVector<int> selItems;

    size_t numRows = m_playList->GetItemCount();
    size_t numSel = currSel.size();

    if (m_playList->TestState(VLCST_SELECT_ALL)) {
        wxASSERT(selItems.empty());

        //---------------------------------------

        selItems.reserve(numRows);

        for (size_t i = 0; i < numRows; i++) {
            selItems.push_back(i);
        }

        numSel = numRows;
    } else {
        selItems.reserve(numSel);

        for (size_t i = 0; i < numSel; i++) {
            selItems.push_back(currSel[i]);
        }
    }

    int currIndex = m_playList->GetLockedIndex();

    //-------------------------------------------

    // 保存第一个被选中的 ID
    size_t firstSel = selItems[0];

    for (int i = numSel - 1; i >= 0 ; i--) {
        // 用户想删除正在播放的歌曲，为简化模型，直接停止播放
        if (currIndex == selItems[i]) {
            DoStop(&dc, &playListDC, CL_CLEAR);

            // 更新滚动文本
            if (m_rollingLabel) {
                m_rollingLabel->StopRolling(&dc);
            }

            // 清除歌词显示
            ClearLyric();
        }

        // 物理删除歌曲文件
        if (da == DA_DEL_PHYSICALLY) {
            OOPSongPtr song = m_playList->GetSongPtr(selItems[i]);
            wxRemoveFile(song->path());
        }

        m_playList->RemoveSong(selItems[i]);
    }

    //==================================================

    numRows -= numSel;

    // 重新选中下一个
    if (firstSel < numRows) {
        m_playList->Select(firstSel, NULL);
    }

    m_playList->UpdateSize(&playListDC);
}

void OOPlayerApp::OnDelAll(VdkVObjEvent &e) {
    VdkDC mainPanelDC(m_mainPanel);
    VdkDC playListDC(m_playListPanel);

    DeleteAllSongs(&mainPanelDC, &playListDC);
}

void OOPlayerApp::RemoveDuplicatedItems(VdkVObjEvent &) {
    wxASSERT(m_playList);

    bool changed = false;
    LcCellIter end(m_playList->end());
    LcCellIter iter(m_playList->begin()), iterNext,
               following, followingNext;

    for (; iter != end; iter = iterNext) {
        iterNext = iter;
        ++iterNext;

        OOPListEntry *currSong = m_playList->GetListEntry(iter);

        for (following = iterNext; following != end;
                following = followingNext) {
            followingNext = following;
            ++followingNext;

            OOPListEntry *followingSong = m_playList->GetListEntry(following);
            if (currSong->path() == followingSong->path()) {
                LcCellIter songToRemove(following);
                // 删除不在播放的那一首，即基准项
                if (songToRemove == m_playList->GetLocked()) {
                    songToRemove = iter;

                    // 更新当前歌曲的指针
                    currSong = followingSong;
                }

                // 要删除的歌曲刚好紧跟着用来比较的基准项，即基准项下面就是
                // 与它自己同属于一首歌的一个重复项
                if (iterNext == songToRemove) {
                    iterNext = songToRemove;
                    ++iterNext;
                }

                // 不会出现 followingNext == songToRemove 的情况，因为
                // followingNext 出现的位置肯定比基准项 iter 和 following
                // 要后。

                m_playList->RemoveSong(songToRemove);
                changed = true;
            }
        }

    }

    if (changed) {
        VdkDC dc(m_playListPanel);
        m_playList->UpdateSize(&dc);
    }
}

void OOPlayerApp::RemoveWrongItems(VdkVObjEvent &) {
    wxASSERT(m_playList);

    bool changed = false;
    LcCellIter iter(m_playList->begin()), end(m_playList->end()), curr;
    for (; iter != end;) {
        curr = iter++;

        OOPSongPtr song = m_playList->GetSongPtr(curr);
        if ((song->length() == 0) || !wxFileExists(song->path())) {
            m_playList->RemoveSong(curr);
            changed = true;
        }
    }

    if (changed) {
        VdkDC dc(m_playListPanel);
        m_playList->UpdateSize(&dc);
    }
}

void OOPlayerApp::DeleteAllSongs(wxDC *pDC, wxDC *playListDC) {
    if (m_playList->IsEmpty()) {
        return;
    }

    DoStop(pDC, playListDC, CL_CLEAR);

    DelayModStack &dms = SingleDelayModStack::Instance();
    dms.CommitAll();

    m_playList->Clear(playListDC);
    ClearLyric();

    StopPlayer(pDC);
}

void OOPlayerApp::SelectAllNoneReverse(VdkVObjEvent &e) {
    VdkDC vdc(m_playListPanel);
    wxDC *pDC = &vdc;

    switch (e.GetId()) {
    case OTBM_SELECT_ALL:

        m_playList->SelectAll(pDC);
        break;

    case OTBM_SELECT_NONE:

        m_playList->SelectNone(pDC);
        break;

    case OTBM_SELECT_REVERSE:

        m_playList->SelectReverse(pDC);
        break;

    default:
        break;
    }

    // 用户可能对由编程实现的选中的列表项做某些操作，激活播放列表。
    // 同时，必须考虑到搜索结果列表框是否正在显示
    if (m_playList->IsShown()) {
        m_playListPanel->FocusCtrl(m_playList, pDC);
    }
}

void OOPlayerApp::AddPlayList(VdkVObjEvent &e) {
    e.SetId(OTBM_DELETE_ALL);
    OnDelAll(e);
}

void OOPlayerApp::SaveListByUser(VdkVObjEvent &) {
    wxString path(AskPathToList(L"保存播放列表", wxFD_SAVE));
    if (!IsPlayListPathOk(path)) {
        return;
    }

    SavePlayList(path);
}

void OOPlayerApp::OpenListByUser(VdkVObjEvent &) {
    wxString path(AskPathToList(L"打开播放列表", wxFD_OPEN));
    if (!IsPlayListPathOk(path)) {
        return;
    }

    DeleteAllSongs(NULL, NULL);
    LoadPlayList(path);
    EnablePlayer();

    m_playListPanel->QueueRedrawEvent();
    m_mainPanel->QueueRedrawEvent();
}

wxString OOPlayerApp::AskPathToList(const wxString &alert, long dlgStyle) {
    wxFileDialog fdlg(m_playListPanel,
                      alert,
                      wxEmptyString,
                     "Default",
                      L"OOPlayer 播放列表(*.Playlist)|*.Playlist|"
                      L"所有文件(*.*)|*.*",
                      dlgStyle);

    if (fdlg.ShowModal() == wxID_CANCEL) {
        return wxEmptyString;
    }

    return fdlg.GetPath();
}

bool OOPlayerApp::IsPlayListPathOk(const wxString &path) {
    if (path.IsEmpty()) {
        return false;
    }

    return (path != GetPlayListPath());
}

void OOPlayerApp::SortPlayList(VdkVObjEvent &e) {
    m_playList->SortList(e.GetId());
}

double OOPlayerApp::GetProgress() const {
    OOPStopWatch &sw = const_cast<OOPStopWatch &>(m_stopWatch);
    double nTimeSum = GetCurrSongLength();

    return (nTimeSum > 0) ? (sw.Time() / nTimeSum) : 0;
}

//////////////////////////////////////////////////////////////////////////

bool OOPlayerApp::IsPlayable() const {
    return TestState(OOPST_PLAYABLE);
}

bool OOPlayerApp::IsPlaying() const {
    return !IsPaused() && !IsStopped();
}

bool OOPlayerApp::IsPaused() const {
    return (m_music.getStatus() == sf::Music::Paused);
}

bool OOPlayerApp::IsStopped() const {
    return (m_music.getStatus() == sf::Music::Stopped) || !IsPlayable();
}

bool OOPlayerApp::IsMute() const {
    return TestState(OOPST_MUTE);
}

void OOPlayerApp::UpdateCurrSong() {
    if (m_playList) {
        m_currSong = m_playList->GetLockedSong();
        if (m_currSong) {
            m_currSong->ParseTag(false);
        }
    } else {
        m_currSong = OOPSongPtr(NULL);
    }
}

unsigned int OOPlayerApp::GetCurrSongLength() const {
    if (IsStopped()) { // 假如已经停止播放，使用缓存值
        return m_currSong ? (m_currSong->length() * 1000) : 0;
    } else {
        return m_music.getDuration().asMilliseconds();
    }
}

void OOPlayerApp::OnVolume(VdkVObjEvent &e) {
    wxDC *pDC = e.GetVObjDC();
    int evtID = (int) e.GetClientData();

    if (evtID == NORMAL) {
        SetStatusLabel(wxEmptyString, pDC);
    } else if ((evtID == DRAGGING) || (evtID == LEFT_DOWN)) {
        float volume = m_volume->GetProgress();

        if (IsPlaying()) {
            m_music.setVolume(volume);
        }

        int volumeInt = static_cast<int>(volume * 100);
        SetStatusLabel(wxString::Format(L"音量: %d%%", volumeInt), pDC);
    }

    if (IsMute()) {
        ToggleMute(false, pDC);
    }
}

void OOPlayerApp::OnMute(VdkVObjEvent &e) {
    VdkToggleButton *mute = e.GetCtrl<VdkToggleButton>();
    ToggleMute(mute->IsToggled(), e.GetVObjDC());
}

void OOPlayerApp::OnMenuVolumeCtrl(VdkVObjEvent &e) {
    double newVolume = m_volume->GetProgress();
    VdkDC dc(m_mainPanel);

    switch (e.GetId()) {
    case OOM_VOLUME_UP:

        if (newVolume == 1) {
            return;
        }

        newVolume += 0.1;

        break;

    case OOM_VOLUME_DOWN:

        if (newVolume == 0) {
            return;
        }

        newVolume -= 0.1;

        break;

    case OOM_MUTE:

        ToggleMute(!IsMute(), &dc);

        return;
    }

    if (newVolume > 1) {
        newVolume = 1;
    } else if (newVolume < 0) {
        newVolume = 0;
    }

    m_volume->GoTo(newVolume, &dc, false);

    if (IsPlaying()) {
        m_music.setVolume(newVolume);
    }

    new VdkLabelGhost(m_status, m_status->GetCaption(), 1500);
    int volumeInt = static_cast<int>(newVolume * 100);
    SetStatusLabel(wxString::Format(L"音量: %d%%", volumeInt), &dc);

    if (newVolume > 0) {
        if (IsMute()) {
            ToggleMute(false, &dc);
        }
    } else {
        if (!IsMute()) {
            ToggleMute(true, &dc);
        }
    }
}

void OOPlayerApp::ToggleMute(bool mute, wxDC *pDC) {
    DoMute(mute);

    if (m_mute) {
        m_mute->Toggle(mute, pDC, false);
        m_muteItem->checked(mute);
    }

    if (m_stereo) {
        m_stereo->SetCaption(mute ? L"静音" : L"立体声", pDC);
    }
}

void OOPlayerApp::StepProgress(VdkVObjEvent &e) {
    if (m_progress) {
        if (m_progress->GetThumb()->GetLastState() != VdkButton::PUSHED) {
            m_progress->GoTo(GetProgress(), e.GetVObjDC(), false);
        }
    }

    RollTaskbarTitle();
}

void OOPlayerApp::RollTaskbarTitle() {
    wxString title;

    if (m_currSong) {
        title.assign(m_currSong->BuildTitle(GetPlayListTitleFormat()));
        title += L" - M4Player  ";

        int mod = (m_stopWatch.Time() / 1000) % title.length();
        if (mod) {
            wxString left(title.substr(0, mod));
            title.erase(0, mod);
            title += left;
        }
    } else {
        title.assign(L"M4Player");
    }

    //====================================================

    OOPWindow **windows = (OOPWindow **) &m_mainPanel;

    for (int i = 0; i < NUM_PANELS; i++) {
        if (windows[i]) {
            windows[i]->SetTitle(title);
        }
    }
}

wxIcon OOPLoadPngIcon(int size) {
    wxIcon icon;

    if (!wxImage::FindHandler(wxBITMAP_TYPE_PNG)) {
        wxImage::AddHandler(new wxPNGHandler);
    }

    wxString path(OOPFileSystem::GetAppResDir() + L"OOPlayer.png");
    wxImage imgIcon(path, wxBITMAP_TYPE_PNG);

    if (imgIcon.IsOk()) {
        imgIcon.Rescale(size, size, wxIMAGE_QUALITY_HIGH);
        icon.CopyFromBitmap(wxBitmap(imgIcon));
    }

    return icon;
}

void OOPlayerApp::OnIdleCreateTrayIcon(wxIdleEvent &) {
    wxASSERT(!m_trayIcon);

    m_mainPanel->Unbind(wxEVT_IDLE, &OOPlayerApp::OnIdleCreateTrayIcon, this);

    CreateTrayIcon();
    UpdateTrayIcon();
}

void OOPlayerApp::CreateTrayIcon() {
    wxASSERT(!m_trayIcon);

    m_trayIcon = new OOPTrayIcon;
    m_trayIcon->Bind(wxEVT_UPDATE_UI, &OOPlayerApp::OnUpdateTrayMenuUI,
                     this, CID_MINIMIZE);
    m_trayIcon->Bind(wxEVT_UPDATE_UI, &OOPlayerApp::OnUpdateTrayMenuUI,
                     this, OOM_PLAY_PAUSE, OOM_MUTE);
    m_trayIcon->Bind(wxEVT_UPDATE_UI, &OOPlayerApp::OnUpdateTrayMenuUI,
                     this, PLAY_MODE_SINGLE, PLAY_MODE_LIST_RANDOM);

#if 0
    m_trayIcon->Bind(wxEVT_TASKBAR_RIGHT_DOWN,
                    &OOPlayerApp::OnTrayIconRightDown,
                     this);
#endif
}

void OOPlayerApp::UpdateTrayIcon() {
    if (!m_trayIcon) {
        return;
    }

    wxString tooltip(L"M4Player");
    if (m_currSong) {
        tooltip = m_currSong->BuildTitle(GetPlayListTitleFormat());
        tooltip += L" - M4Player";
    }

    wxIcon icon
#ifdef __WXMSW__
        (m_mainPanel->GetIcon());
#else
        (OOPLoadPngIcon(16));
#endif

    m_trayIcon->SetIcon(icon, tooltip);
}

void OOPlayerApp::OnUpdateTrayMenuUI(wxUpdateUIEvent &e) {
    switch (e.GetId()) {
    case CID_MINIMIZE:

        e.SetText(m_mainPanel->IsShown() ? L"最小化(&M)" : L"还原(&R)");
        break;

    case OOM_PLAY_PAUSE:

        e.SetText(IsPlaying() ? L"暂停(&P)" : L"播放(&P)");
        goto MAINMENU_PLAY_CONTROL;

    case OOM_STOP:

        e.Enable(!IsStopped());
        break;

MAINMENU_PLAY_CONTROL:
    case OOM_PREV:
    case OOM_NEXT:

        e.Enable(IsPlayable());
        break;

    case OOM_MUTE:

        e.Check(IsMute());
        break;

    case PLAY_MODE_SINGLE:
    case PLAY_MODE_SINGLE_RECYCLE:
    case PLAY_MODE_BY_LIST:
    case PLAY_MODE_LIST_RECYCLE:
    case PLAY_MODE_LIST_RANDOM:

        e.Check(m_playMode == e.GetId());
        break;

    default:

        break;
    }
}

void OOPlayerApp::OnTrayIconRightDown(wxTaskBarIconEvent &) {
    wxCoord x, y;
    wxGetMousePosition(&x, &y);
    m_mainPanel->ScreenToClient(&x, &y);

    m_mainPanel->SetFocus();
    m_mainPanel->ShowContextMenu(NULL, x, y, true);
}

void OOPlayerApp::OnTrayIconLeftClick(wxTaskBarIconEvent &) {
    // 在 MSW 下面，使用任务栏状态条切换窗口显示状态时，隐藏操作是通过将
    // 窗口移动到某一个不可见的位置来实现的，但此时窗口仍然是可见的
    // (IsShown() == true)，所以这里要分开判断
    if (m_mainPanel->IsIconized()) {
        m_mainPanel->Restore();
        m_mainPanel->Raise();
    } else {
        ToggleAllOnScreenWindow();
    }
}

bool OOPlayerApp::ToggleAllOnScreenWindow() {
    if (!m_trayIcon) {
        wxLogDebug(L"无法最小化到托盘: 当前窗口管理器不支持标准托盘图标协议。");
        return false;
    }

#ifdef __WXMSW__
    bool toShow = !m_mainPanel->IsShown();
    m_mainPanel->Show(toShow);
    m_mainPanel->Raise();
    ::ShowOwnedPopups(GetHwndOf(m_mainPanel), toShow);
#else
    wxASSERT(m_togglePlayList && m_toggleLyric && m_toggleEq);

    bool toShow = !m_mainPanel->IsShown();
    m_mainPanel->Show(toShow);
    if (toShow) {
        m_mainPanel->Raise();
    }

    if (m_togglePlayList->IsToggled()) {
        m_playListPanel->Show(toShow);
    }

    if (m_toggleLyric->IsToggled()) {
        m_lyricPanel->Show(toShow);
    }

    if (m_toggleEq->IsToggled()) {
        m_eqPanel->Show(toShow);
    }
#endif

    return true;
}

void OOPlayerApp::OnPlayListDClick(VdkVObjEvent &e) {
    int index = (int) e.GetClientData();

    if (index == -1) {
        e.Skip();
        return;
    }

    VdkDC vdc(m_mainPanel);
    BeginSong(index, &vdc, e.GetVObjDC());
}

bool OOPlayerApp::OnPlayListKeys(wxKeyEvent &e) {
    if (e.GetEventType() != wxEVT_KEY_DOWN) {
        return false;
    }

    bool keyForPanel = false;
    VdkControl *focus = m_playListPanel->GetCtrlOnFocus();
    if (focus != m_playList) {
        keyForPanel = true;
    }

    int k = e.GetKeyCode();
    if ((focus == m_playList) && (k == WXK_DELETE)) {
        // HandleSel 会自己申请 DC，故必须小心处理
        VdkVObjEvent fakeEvent;
        fakeEvent.SetId(OPLM_DELETE);
        HandleDelete(fakeEvent);

        return true;
    }

    if (keyForPanel) {
        return false;
    }

    switch (k) {
    case WXK_RETURN: {
        const ArrayOfSortedInts &selItems(m_playList->GetSelectedItems());

        if (!selItems.empty()) {
            VdkDC dc(m_playListPanel);
            // 默认播放第一首
            int sel = selItems[0];
            m_playList->SetLocked(sel, &dc);

            VdkVObjEvent fakeEvent;
            fakeEvent.SetClientData((void *) sel);
            OnPlayListDClick(fakeEvent);
        }

        break;
    }

    default:

        return false;
    }

    return true;
}

void OOPlayerApp::OnMenuPrevNext(VdkVObjEvent &e) {
    VdkDC vdc(m_mainPanel);

    VdkVObjEvent fakeEvent((e.GetId() == OOM_PREV) ? CID_PREV : CID_NEXT);
    fakeEvent.SetVObjDC(&vdc);

    OnPrevNext(fakeEvent);
}

void OOPlayerApp::OnPrevNext(VdkVObjEvent &e) {
    wxASSERT(m_playList);
    wxASSERT(!m_playList->IsEmpty());

    wxDC *mainPanelDC = e.GetVObjDC();
    VdkDC playListDC(m_playListPanel);

    // 原本列表为空，用户第一次点击“上一首、下一首”而不是“播放”
    if (!m_playList->HasLocked()) {
        LcCellIter songToPlay(m_playList->begin());

        m_playList->SetLocked(songToPlay, &playListDC);
        BeginSong(songToPlay, mainPanelDC, &playListDC);

        return;
    }

    PrevNextAction pna = (e.GetId() == CID_PREV) ? PNA_PREV : PNA_NEXT;
    HandlePrevNext(pna, mainPanelDC, &playListDC);
}

void OOPlayerApp::HandlePrevNext
(PrevNextAction pna, wxDC *mainPanelDC, wxDC *playListDC) {
    wxASSERT(m_playList);
    wxASSERT(!m_playList->IsEmpty());

    // TODO: return false or wxASSERT?
    if (!m_playList->HasLocked()) {
        return;
    }

    switch (m_playMode) {
    case PLAY_MODE_BY_LIST:
    case PLAY_MODE_LIST_RECYCLE:
    case PLAY_MODE_SINGLE:
    case PLAY_MODE_SINGLE_RECYCLE: {
        LcCellIter songToPlay(m_playList->GetLocked());
        LcCellIter lastSong(m_playList->end());
        --lastSong;

        if (pna == PNA_PREV) {
            // 到头了
            if (songToPlay == m_playList->begin()) {
                if (m_playMode == PLAY_MODE_LIST_RECYCLE) {
                    // 跳到最后一首
                    songToPlay =lastSong;
                } else { // 其它迭代模式直接忽略此事件
                    break;
                }
            } else {
                --songToPlay;
            }
        } // END if( e.GetId() == CID_PREV )
        else {
            // 到头了
            if (songToPlay == lastSong) {
                if (m_playMode == PLAY_MODE_LIST_RECYCLE) {
                    songToPlay = m_playList->begin();
                } else { // 其它迭代模式直接忽略此事件
                    // 必须显式地停止播放
                    DoStop(mainPanelDC, playListDC, CL_CLEAR);

                    break;
                }
            } else {
                ++songToPlay;
            }
        }

        m_playList->SetLocked(songToPlay, playListDC);
        BeginSong(songToPlay, mainPanelDC, playListDC);

        break;
    }

    case PLAY_MODE_LIST_RANDOM: {
        int randNum = Rand(m_playList->GetItemCount());
        BeginSong(randNum, mainPanelDC, playListDC);

        break;
    }

    default:

        Alert(L"未知播放列表迭代方式。");
        break;
    }
}

void OOPlayerApp::OnSongFinished(wxCommandEvent &) {
    wxASSERT(m_playList->HasLocked());

    VdkDC mainPanelDC(m_mainPanel);
    VdkDC playListDC(m_playListPanel);

    switch (m_playMode) {
    case PLAY_MODE_LIST_RECYCLE:
    case PLAY_MODE_BY_LIST:
    case PLAY_MODE_LIST_RANDOM: {
        HandlePrevNext(PNA_NEXT, &mainPanelDC, &playListDC);

        break;
    }

    case PLAY_MODE_SINGLE: {
        DoStop(&mainPanelDC, &playListDC, CL_CLEAR);

        break;
    }

    case PLAY_MODE_SINGLE_RECYCLE: {
        BeginSong(m_playList->GetLocked(), &mainPanelDC, &playListDC);

        break;
    }

    default:

        wxLogError(L"未知播放列表迭代方式。");
        break;
    }
}

void OOPlayerApp::OnSearchPlaylist(VdkVObjEvent &e) {
    wxKeyEvent ke(wxEVT_KEY_DOWN);
    if (e.GetId() == OTBM_LOCATE_FIND_NEXT) {
        ke.m_keyCode = WXK_F3;
    } else {
        ke.m_keyCode = 'F';
        ke.m_controlDown = true;
    }

    wxControl *hidden = m_playListPanel->GetHiddenCtrl();
    wxEvtHandler *sinker = hidden ? hidden :
                           m_playListPanel->VdkWindow::GetHandle();

    wxPostEvent(sinker, ke);
}

void OOPlayerApp::OnSongInfoUpdated(wxCommandEvent &e) {
    if (m_currSong) {
        if (e.GetString() == m_currSong->path()) {
            // 重读元信息
            m_currSong->ParseTag(true);

            VdkDC dc(m_mainPanel);
            RollLabel(&dc, !IsStopped());
        }
    }
}

#ifdef __WXMSW__
#   include <Shellapi.h>
#endif

void OOPlayerApp::OnLocateFile(VdkVObjEvent &) {
    int index = m_playList->GetLastSelIndex();
    wxASSERT(index != wxNOT_FOUND);

    OOPSongPtr song = m_playList->GetSongPtr(index);

#ifdef __WXMSW__
    wxString param(L"/select,");
    param += song->path();

    ::ShellExecute(0, L"open", L"explorer.exe", param, NULL, SW_SHOWNORMAL);
#elif defined( __WXGTK__ )
    // TODO:“打开文件路径”功能会导致很奇怪的行为：Nautilus 窗口被打开后，播放
    // 列表窗口的 Z 轴坐标不会被降低，而程序的其他窗口都被降低了，就剩它孤零零的
    // 在屏幕上显示着
    wxExecute(wxString::Format(L"nautilus \"%s\"", song->path()));
#endif
}

void OOPlayerApp::OnSearchLyric(VdkVObjEvent &) {
    wxASSERT(m_currSong);
    m_lyricPanel->PopupSchDlg(m_currSong);
}

void OOPlayerApp::OnLyricLoaded(wxCommandEvent &e) {
    wxASSERT(m_lyric);

    if (!m_currSong || (m_currSong->path() != e.GetString())) {
        // 当前正在播放的歌曲不是歌词刚被下载成功的那一首
        return;
    }

    // 表明这首歌的歌词已经在网络上查找过了
    SetAddinState(OOPST_LYRIC_SEARCHED);

    VdkHttpThread::FinishEvent &finishEvt = (VdkHttpThread::FinishEvent &) e;
    wxString lyric(finishEvt.GetResult());

    if (lyric.empty()) {
        VdkDC dc(m_lyricPanel);
        m_lyric->SetInteractiveOutput(L"未能在服务器上找到歌词", &dc);

        return;
    }

    if (m_lyricParser.Load(lyric, m_currSong->length() * 1000)) {
        m_lyric->AttachParser(m_lyricParser);
        m_lyric->Start();
    }

    //--------------------------------------
    // 保存到磁盘文件中

    LyricGrabber::SaveLrcFile(lyric, m_currSong->lrcPath());

    //--------------------------------------
    // 嵌入到歌曲文件中

    if (m_currSong->IsTaggingSupported()) {
        m_currSong->embeddedLyric(lyric);

        if (!m_currSong->Save()) {
            // 添加到延迟修改栈
            SingleDelayModStack::Instance().Add(m_currSong);
        }
    }
}

void OOPlayerApp::OnCopyLyric(VdkVObjEvent &) {
#ifdef __WXDEBUG__
    wxASSERT(m_currSong);

    wxString lyric(m_currSong->embeddedLyric());
    if (lyric.empty()) {
        lyric.assign(OOPLyricParser::LoadFile(m_currSong->lrcPath()));
        wxASSERT(!lyric.empty());
    }

    CopyText(lyric);
#else
    CopyText(m_lyric->GetLyric());
#endif
}

void OOPlayerApp::OnAssociateLyric(VdkVObjEvent &e) {
    wxASSERT(m_lyric);
    wxASSERT(m_currSong);

    wxFileDialog fdlg(m_lyricPanel,
                      L"指定歌词文件",
                      wxEmptyString,
                      wxEmptyString,
                      L"歌词文件(*.lrc)|*.lrc",
                      wxFD_OPEN |
                      wxFD_FILE_MUST_EXIST);

    if (fdlg.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxString src(fdlg.GetPath());
    wxString dst(m_currSong->lrcPath());

    // 优先显示内嵌歌词时，用户又从歌词存储文件夹选中默认关联的 LRC 文件
    if (src != dst) {
        wxCopyFile(src, dst, true);
    }

    // 不要调用 LoadCurrSongLyric() ，因为该函数优先读取内嵌歌词
    if (m_lyricParser.LoadFile(dst, m_currSong->length() * 1000)) {
        m_lyric->AttachParser(m_lyricParser);
        UpdateLyricState();
    }
}

void OOPlayerApp::OnReloadLyric(VdkVObjEvent &) {
    // 强制重读歌词文件
    LoadCurrSongLyric();

    // 更新歌词秀状态
    UpdateLyricState();
}

void OOPlayerApp::OnRemoveLyric(VdkVObjEvent &) {
    wxASSERT(m_lyric);
    wxASSERT(m_currSong);

    // 清空即为撤销关联
    m_currSong->lrcPath(wxEmptyString);

    ClearLyric();
}

void OOPlayerApp::SetStatusLabel(wxString text, wxDC *pDC) {
    if (!m_status) {
        return;
    }

    if (text.IsEmpty()) {
        if (IsStopped()) {
            text = L"状态: 停止";
        } else if (IsPaused()) {
            text = L"状态: 暂停";
        } else {
            text = L"状态: 播放";
        }
    }

    m_status->SetCaption(text, pDC);
}

void OOPlayerApp::OnWebsiteLinks(VdkVObjEvent &e) {
    wxString url;

    switch (e.GetId()) {
    case OMM_LINKS_FORUM:

        url.assign("http://www.ooplayer.org/bbs");
        break;

    case OMM_LINKS_WEBSITE:

        url.assign(L"http://www.ooplayer.org/");
        break;

    default:

        return;
    }

    wxLaunchDefaultBrowser(url);
}

void OOPlayerApp::OnAbout(VdkVObjEvent &) {
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName(gs_DefaultInteractiveOutput);
    aboutInfo.SetVersion(gs_appVersion);
    aboutInfo.SetDescription(L"好用的跨平台音乐播放器");
    aboutInfo.SetCopyright(L"(C) 2009-2015 wxn");
    aboutInfo.SetWebSite("http://www.OOPlayer.org/");
    aboutInfo.AddDeveloper("wxn (wxn@ooplayer.org)");

    aboutInfo.SetLicense(L"Thanks for \n"
                         L"wxWidgets\n
                         L"Loki C++ Library\n"
                         L"千千静听(TTPlayer)\n"
                         L"MyTagLib\n");

#ifdef __WXGTK__
    wxIcon icon(OOPLoadPngIcon(64));
    if (icon.IsOk()) {
        aboutInfo.SetIcon(icon);
    }
#endif

    wxAboutBox(aboutInfo);
}

void OOPlayerApp::OnFeedback(VdkVObjEvent &) {

}

//////////////////////////////////////////////////////////////////////////

/*static*/
wxString OOPlayerApp::GetSupportedFileTypes() {
    return sf::Music::getSupportedFileTypes();
}

/*static*/
bool OOPlayerApp::TestFileType(const wxString &extName) {
    if (extName.empty() || (extName == L'.')) {
        return false;
    }

    wxString ext(extName);
    ext.MakeLower();

    wxString wildTypes(GetSupportedFileTypes());
    wxString::size_type pos = 0;

    while ((pos = wildTypes.find(ext, pos)) != wxString::npos) {
        wxChar following = wildTypes[pos + ext.length()];

        if (!isalpha(following) && !isdigit(following)) {
            return true;
        }

        pos += ext.length();
    }

    return false;
}

void OOPlayerApp::GenerateMainPanelCtrlIds(MapOfCtrlIdInfo &ids) {
    BindCtrl(L"play", CID_PLAY, m_play);
    BindCtrl(L"pause", CID_PAUSE, m_pause);
    BindCtrl(L"stop", CID_STOP, m_stop);
    BindCtrl(L"prev", CID_PREV, m_prev);
    BindCtrl(L"next", CID_NEXT, m_btnNext);
    BindCtrl(L"mute", CID_MUTE, m_mute);

    BindCtrl(L"lyric", CID_TOGGLE_LYRIC, m_toggleLyric);
    BindCtrl(L"playlist", CID_TOGGLE_PLAYLIST, m_togglePlayList);
    BindCtrl(L"equalizer", CID_TOGGLE_EQ, m_toggleEq);   // TODO: Memory leak?

    BindCtrl(L"progress", CID_PROGRESS, m_progress);
    BindCtrl(L"volume", CID_VOLUME, m_volume);

    BindCtrl(L"info", CID_INFO, m_rollingLabel);
    BindCtrl(L"led", CID_LED, m_led);
    BindCtrl(L"stereo", CID_STEREO, m_stereo);
    BindCtrl(L"status", CID_STATUS, m_status);

    BindCtrl(L"minimize", CID_MINIMIZE, m_minimize);
    BindCtrl(L"minimode", CID_MINIMODE, m_minimode);
    BindCtrl(L"exit", CID_EXIT, m_exit);

    BindCtrlID(L"visual", CID_VISUAL);
    BindCtrlID(L"icon", CID_ICON);
    BindCtrl(L"equalizer", CID_TOGGLE_EQ, m_toggleEq);
    BindCtrl(L"open", CID_OPEN_FILE, m_openFile);
    BindCtrl(L"browser", CID_TOGGLE_BROWSER, m_toggleBrowser);
}

void OOPlayerApp::OnAddFileFolders(VdkVObjEvent &e) {
    if (e.GetMenu()) {
        VdkCtrlId id = e.GetMenuItem()->id();

        if (id == OTBM_ADD_FOLDER) {
            m_playListPanel->AddFolder(PlayListPanel::RM_ONLY_TOP_LEVEL_FILES);
        } else if (id == OTBM_ADD_FOLDER_RECURSIVE) {
            m_playListPanel->AddFolder(PlayListPanel::RM_RECURSIVE);
        } else {
            m_playListPanel->AddFiles(m_playListPanel);
        }
    } else {
        if (e.GetId() == CID_OPEN_FILE) {
            {
                VdkDC dc(m_mainPanel);
                m_openFile->Update(VdkButton::NORMAL, &dc);
            }

            m_playListPanel->AddFiles(m_mainPanel);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

#ifdef __WXMSW__
#   define EVT_TASKBAR_LEFT_CLICK EVT_TASKBAR_LEFT_UP
#else
#   define EVT_TASKBAR_LEFT_CLICK EVT_TASKBAR_LEFT_DOWN
#endif

BEGIN_EVENT_TABLE(OOPlayerApp, wxApp)

    EVT_VW_FIRST_SHOWN(OOPlayerApp::OnMainPanelFirstShown)

    EVT_VOBJ(CID_EXIT, OOPlayerApp::Quit)
    EVT_VOBJ(CID_MINIMIZE, OOPlayerApp::Minimize)
    EVT_VOBJ(CID_TOGGLE_LYRIC, OOPlayerApp::ShowLyricPanel)
    EVT_VOBJ(CID_TOGGLE_PLAYLIST, OOPlayerApp::ShowPlayListPanel)
    EVT_VOBJ(CID_TOGGLE_EQ, OOPlayerApp::ShowEqPanel)
    EVT_VOBJ(CID_PROGRESS, OOPlayerApp::OnProgress)
    EVT_VOBJ(CID_PLAY, OOPlayerApp::Play)
    EVT_COMMAND(wxID_ANY, MM_EVT_SONG_FINISHED, OOPlayerApp::OnSongFinished)
    EVT_VOBJ(CID_PAUSE, OOPlayerApp::Pause)
    EVT_VOBJ(CID_STOP, OOPlayerApp::Stop)
    EVT_VOBJ_RANGE(CID_PREV, CID_NEXT, OOPlayerApp::OnPrevNext)
    EVT_VOBJ(CID_LED, OOPlayerApp::StepProgress)
    EVT_VOBJ(CID_VOLUME, OOPlayerApp::OnVolume)
    EVT_VOBJ(CID_MUTE, OOPlayerApp::OnMute)

    EVT_VOBJ(CID_LYRIC, OOPlayerApp::OnLyricDragg)
    EVT_VOBJ(CID_PLAY_LIST, OOPlayerApp::OnPlayListDClick)
    EVT_VOBJ_RANGE(OOM_PREV, OOM_NEXT, OOPlayerApp::OnMenuPrevNext)

    EVT_VOBJ(CID_ABOUT, OOPlayerApp::OnAbout)
    EVT_VOBJ_RANGE(OMM_LINKS_WEBSITE, OMM_LINKS_FORUM, OOPlayerApp::OnWebsiteLinks)
    EVT_VOBJ(CID_FEEDBACK, OOPlayerApp::OnFeedback)

    EVT_TASKBAR_LEFT_CLICK(OOPlayerApp::OnTrayIconLeftClick)

    EVT_VOBJ(OPLM_PLAY_SELECTED, OOPlayerApp::OnPlaySel)
    EVT_VOBJ(OTBM_DELETE_DUPLICATED, OOPlayerApp::RemoveDuplicatedItems)
    EVT_VOBJ(OTBM_DELETE_WRONG_FILE, OOPlayerApp::RemoveWrongItems)
    EVT_VOBJ(OTBM_DELETE_FILE, OOPlayerApp::HandleDelete)
    EVT_VOBJ(OTBM_DELETE_ALL, OOPlayerApp::OnDelAll)
    EVT_VOBJ_RANGE(OPLM_DELETE, OPLM_DELETE_FILE, OOPlayerApp::HandleDelete)

    EVT_VOBJ_RANGE(OTBM_SELECT_ALL, OTBM_SELECT_REVERSE, OOPlayerApp::SelectAllNoneReverse)

    EVT_VOBJ(CID_OPEN_FILE, OOPlayerApp::OnAddFileFolders)
    EVT_VOBJ_RANGE(OTBM_ADD_FILE, OTBM_ADD_FOLDER, OOPlayerApp::OnAddFileFolders)

    EVT_VOBJ(OTBM_LIST_NEW, OOPlayerApp::AddPlayList)
    EVT_VOBJ(OTBM_LIST_OPEN, OOPlayerApp::OpenListByUser)
    EVT_VOBJ(OTBM_LIST_SAVE, OOPlayerApp::SaveListByUser)
    EVT_VOBJ_RANGE(OTBM_SORT_BY_TITLE, OTBM_SORT_BY_ADD_TIME, OOPlayerApp::SortPlayList)
    EVT_VOBJ_RANGE(OTBM_LOCATE_SHOW_DIALOG, OTBM_LOCATE_FIND_NEXT, OOPlayerApp::OnSearchPlaylist)
    EVT_COMMAND(wxID_ANY, OOP_EVT_SONG_INFO_UPDATED, OOPlayerApp::OnSongInfoUpdated)
    EVT_VOBJ(OPLM_LOCATE_FILE, OOPlayerApp::OnLocateFile)

    EVT_VOBJ(OLRCM_SEARCH, OOPlayerApp::OnSearchLyric)
    EVT_COMMAND(wxID_ANY, LyricGrabber::OOP_EVT_LYRIC_LOADED, OOPlayerApp::OnLyricLoaded)
    EVT_VOBJ(OLRCM_COPY_LRC, OOPlayerApp::OnCopyLyric)
    EVT_VOBJ(OLRCM_ASSOCIATE_LOCAL_FILE, OOPlayerApp::OnAssociateLyric)
    EVT_VOBJ(OLRCM_RELOAD_LRC, OOPlayerApp::OnReloadLyric)
    EVT_VOBJ(OLRCM_REMOVE_LRC, OOPlayerApp::OnRemoveLyric)

    EVT_VOBJ_RANGE(OOM_PLAY_PAUSE, OOM_STOP, OOPlayerApp::OnMenuPlayCtrl)
    EVT_VOBJ_RANGE(OOM_VOLUME_UP, OOM_MUTE, OOPlayerApp::OnMenuVolumeCtrl)
    EVT_VOBJ_RANGE(PLAY_MODE_SINGLE, PLAY_MODE_LIST_RANDOM, OOPlayerApp::OnPlayModes)

END_EVENT_TABLE();
