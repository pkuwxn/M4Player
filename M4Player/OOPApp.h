/***************************************************************
 * Name:      OOPApp.h
 * Purpose:   Defines Application Class
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2009-12-19
 * Copyright: Wang Xiaoning
 **************************************************************/
#pragma once

class VdkLabel;
class VdkButton;
class VdkToggleButton;
class VdkScrollBar;
class VdkSlider;
class VdkListCtrl;
class VdkControl;
class VdkMenu;
class VdkMenuItem;

class MainPanel;
class PlayListPanel;
class LyricPanel;
class EqPanel;

class MapOfCtrlIdInfo;
class wxXmlSerializer;

class OOPLed;
class OOPLyric;
class OOPList;
class OOPLabel;
class OOPSong;
class OOPDesktopLyric;
class OOPWindow;
class OOPConf;
class OOPStickyWindowState;

class OOPTrayIcon;
class wxTaskBarIconEvent;

#include "VdkListCtrl.h" // for LcCellIter
#include "VdkBitmapArray.h"

#include "OOPDefs.h" // for enum OOPlayerPlayMode
#include "OOPSkin.h"
#include "StickyWindows.h"
#include "OOPScanner.h"
#include "OOPStopWatch.h"
#include "Lyric/OOPLyricParser.h"
#include "Lyric/OOPLyricMgr.h"
#include "OOPSongPtr.h"

#include <Music.hpp>
#include <wx/snglinst.h>

//////////////////////////////////////////////////////////////////////////

/*! 额外的消息 ID (避免重名) */
enum OOPWindowExtraMessageID {

    /// \brief 重设窗口皮肤
    OOPEMC_RESET_WINDOW         = VEMC_USER + 0,
};

/*! 程序中用到的所有面板的唯一标识枚举值 */
enum OOPPanel {
    PANEL_MAIN, /*! 主面板 */
    PANEL_PLAYLIST, /*! 播放列表窗口 */
    PANEL_LYRIC, /*! 歌词秀窗口 */
    PANEL_EQ, /*! 均衡器窗口 */

    NUM_PANELS, /*! 面板总数 */
};

//////////////////////////////////////////////////////////////////////////

/// \brief 保存所有需要用到的指针
class OOPPtrs {
public:

    /// \brief 构造函数
    OOPPtrs() {
        Clear();
    }

    /// \brief 初始化所有指针
    void Clear() {
        memset(this, 0, sizeof(OOPPtrs));
    }

protected:

    VdkButton *m_minimize;
    VdkButton *m_exit;
    VdkButton *m_minimode;

    VdkButton *m_play;
    VdkButton *m_pause;
    VdkButton *m_stop;
    VdkButton *m_prev;
    VdkButton *m_btnNext;

    VdkSlider *m_progress;
    VdkSlider *m_volume;
    VdkLabel *m_status;
    VdkLabel *m_stereo;
    VdkToggleButton *m_togglePlayList;
    VdkToggleButton *m_toggleLyric;
    VdkToggleButton *m_toggleEq;
    VdkToggleButton *m_toggleBrowser;

    VdkToggleButton *m_mute;
    VdkButton *m_openFile;

    OOPList *m_playList;
    OOPLyric *m_embeddedLyric;
    OOPLed *m_led;
    OOPLabel *m_rollingLabel;

    VdkMenu *m_MainMenu;
    VdkMenu *m_PlayListMenu;
    VdkMenu *m_LyricMenu;
};

/// \brief 应用程序类，同时也是整个程序的控制中心
class OOPlayerApp : public wxApp, public OOPPtrs {
public:

    /// \brief 构造函数
    OOPlayerApp();

    /// \brief 切换播放器至可播放歌曲的状态
    void EnablePlayer();

    /// \brief 从头播放一首歌曲
    void BeginSong(LcCellIter songIter, wxDC *pDC, wxDC *playListDC);

    /// \brief 从头播放一首歌曲
    void BeginSong(int index, wxDC *pDC, wxDC *playListDC);

    /// \brief 设置静音/不静音
    void ToggleMute(bool mute, wxDC *pDC);

    //////////////////////////////////////////////////////////////////////////
    // 获取播放器当前状态

    /// \brief 获取本首歌的进度
    double GetProgress() const;

    /// \brief 是否正在播放
    bool IsPlaying() const;

    /// \brief 是否已然暂停
    bool IsPaused() const;

    /// \brief 是否已然停止播放
    bool IsStopped() const;

    /// \brief 是否静音
    bool IsMute() const;

    /// \brief 播放器是否处于可播放的状态
    bool IsPlayable() const;

    /// \brief 获取正在播放的歌曲的长度[量纲:ms(毫秒)]
    unsigned int GetCurrSongLength() const;

    //////////////////////////////////////////////////////////////////////////

    /// \brief 设置主窗口
    ///
    /// 主窗口构造完毕后应调用本函数
    void SetMainPanel(MainPanel *panel, wxDC *pDC);

    /// \brief 生成主窗口的控件 ID
    void GenerateMainPanelCtrlIds(MapOfCtrlIdInfo &ids);

    /// \brief 只有在主窗口显示后才显示其它子窗口
    void OnMainPanelFirstShown(VdkWindowFirstShownEvent &);

    /// \brief 滚动显示当前播放歌曲信息
    void RollLabel(wxDC *pDC, bool startAtOnce);

    /// \brief 显示播放列表窗口
    void ShowPlayListPanel(VdkVObjEvent &);

    /// \brief 关闭/隐藏播放列表窗口
    void DismissPlayListPanel();

    /// \brief 在播放列表窗口的按键事件
    /// \return 是否终止后续事件处理
    bool OnPlayListKeys(wxKeyEvent &e);

    /// \brief 显示歌词秀窗口
    void ShowLyricPanel(VdkVObjEvent &);

    /// \brief 关闭/隐藏歌词窗口
    void DismissLyricPanel();

    /// \brief 初始化歌词秀
    void InitLyric();

    /// \brief 显示均衡器窗口
    void ShowEqPanel(VdkVObjEvent &);

    /// \brief 关闭/隐藏均衡器窗口
    void DismissEqPanel();

    /// \brief 用户隐藏了制定面板，更新各种状态
    void HidePanel(OOPWindow *win);

    //////////////////////////////////////////////////////////////////////////

    /// \brief 获取皮肤列表管理器
    OOPSkin &GetSkinList() {
        return m_skinList;
    }

    /// \brief 动态换肤
    void SelectSkin(OOPSkin::SkinInfo *info) {
        SelectSkin(*info);
    }

    /// \brief 动态换肤
    /// \param strSkinFolder 要切换到的皮肤的根目录
    void SelectSkin(OOPSkin::SkinInfo &info);

    /// \brief 响应换肤事件
    void OnSelectSkin(VdkVObjEvent &);

    //////////////////////////////////////////////////////////////////////////

    /// \brief 获取面板管理器
    StickyWindows &GetWindowManager() {
        return m_windowManager;
    }

    /// \brief 将面板托付给 StickyWindows 管理
    void AddToWindowManager(VdkWindow *win) {
        m_windowManager.AddWindow(win);
    }

    /// \brief 将面板托付给 StickyWindows 管理
    void RemoveFromWindowManager(VdkWindow *win) {
        m_windowManager.RemoveWindow(win);
    }

    /// \brief \a win 是否已然粘附到主窗口
    bool IsStickyToMain(VdkWindow *win) {
        return m_windowManager.IsStickyToMain(win);
    }

    /// \brief 根据已保存的皮肤相关配置文件调整子面板的作用域
    ///
    /// 返回值可能为空，这时候一般是第一次应用这个皮肤。
    OOPStickyWindowState *GetPanelStickyState(const wxString &uuid);

    //////////////////////////////////////////////////////////////////////////

    /// \brief 生成播放窗口主菜单
    void GenerateMainMenu();

    /// \brief 填充播放列表窗口的播放迭代模式切换菜单
    /// \param ownedByPLP 是否为播放列表窗口所拥有
    void FillPlayModesMenu(VdkMenu *menu, bool ownedByPLP = false);

    /// \brief 填充播放列表
    void GeneratePlayList(OOPList *playList);

    /// \brief 生成歌词窗口菜单
    void GenerateLyricMenu();

    /// \brief 获取播放列表项目的标题格式
    wxString GetPlayListTitleFormat() const;

    //////////////////////////////////////////////////////////////////////////

    /// \brief 得到所有支持的文件类型
    ///
    /// 可用于“打开”对话框进行打开文件。
    static wxString GetSupportedFileTypes();

    /// \brief 测试文件名是否符合播放器支持的文件类型
    static bool TestFileType(const wxString &strFileExtName);

private:

    // 临时测试用
    void CreateDesktopLyric();

    // 初始化应用程序
    virtual bool OnInit();

    // 析构已分配的对象
    virtual int OnExit();

    // 初始化配置文件
    bool InitConf();

    // 保存应用程序相关配置信息
    void SaveConf();

    // 单独保存当前所有面板窗口的可见性(防止退出程序时假死)
    void SavePanelVisibilities();

    // 更新所有配置信息到配置信息类(但不保存)
    void UpdateConf();

    // 保存播放列表
    void SavePlayList(const wxString &path);
    // 载入播放列表
    void LoadPlayList(const wxString &path);

    // 保存当前选择的皮肤
    void SaveCurrentSkin(OOPSkin::SkinInfo &info);

    // 获取应用程序配置文件的路径
    wxString GetAppConfFilePath() const;

    // 获取默认播放列表的文件路径
    wxString GetPlayListPath() const;

    // 创建所有子窗口后一次性全部显示出来
    void ShowAllPanelsOnReady();

    //////////////////////////////////////////////////////////////////////////

    // 播放指定路径的歌曲
    // @return 是否成功开始播放，因为文件可能是一个“假”的歌曲文件
    bool DoPlay();
    void DoPause(); // 暂停
    void DoStop(); // 停止播放
    void DoMute(bool mute); // 静音

    //////////////////////////////////////////////////////////////////////////

    // 决定“播放”、“暂停”两按钮的实现与否
    // @param hilight 是否高亮要显示的按钮
    void ShowHidePlayPause(wxDC *pDC);

    // 设置状态文本条的文本
    //
    // 拖动音量滚动条时必须持续改变其值，但不能大量使用 VdkLabelGhost。
    void SetStatusLabel(wxString text = wxEmptyString, wxDC *pDC = NULL);

    //////////////////////////////////////////////////////////////////////////

    void Quit(VdkVObjEvent &);
    void Minimize(VdkVObjEvent &);

    // 切换所有显示在屏幕上的窗口的显示与否
    bool ToggleAllOnScreenWindow();

    void Pause(VdkVObjEvent &);
    void Play(VdkVObjEvent &);

    void Stop(VdkVObjEvent &);
    void OnMenuPlayCtrl(VdkVObjEvent &); // 菜单控制播放、暂停、停止

    void OnProgress(VdkVObjEvent &); // 拖动主进度条
    void StepProgress(VdkVObjEvent &); // OOPLed 回调
    void RollTaskbarTitle(); // 在任务栏滚动歌曲标题

    //////////////////////////////////////////////////////////////////////////
    // 托盘图标

    // 不知道为什么在 MSW 下面假如加了创建托盘图标的代码，窗口显示会变得很慢
    // 所以准备在窗口空闲时间里面创建
    void OnIdleCreateTrayIcon(wxIdleEvent &);

    // 创建托盘图标
    void CreateTrayIcon();

    void UpdateTrayIcon(); // 设置托盘图标的提示文本
    // 托盘图标事件
    void OnTrayIconLeftClick(wxTaskBarIconEvent &);
    void OnTrayIconRightDown(wxTaskBarIconEvent &);

    // 按照当前配置更新托盘图标菜单的状态(选中、文本等等)
    void OnUpdateTrayMenuUI(wxUpdateUIEvent &e);

    //////////////////////////////////////////////////////////////////////////

    void OnLyricDragg(VdkVObjEvent &); // 拖动歌词

    // 设置当前播放进度
    bool SetPlayingOffset(sf::Time offset);

    // 更新正在播放的歌曲的 OOPSong 对象
    //
    // 若歌曲标签信息尚未读入，则会将其读入。
    void UpdateCurrSong();

    // 处理播放列表删除项目命令
    void HandleDelete(VdkVObjEvent &);

    // 是否物理删除文件
    enum DelAction {
        DA_DEL_FROM_LIST_ONLY,
        DA_DEL_PHYSICALLY,
    };

    // 处理播放列表删除项目命令
    // 添加一个子函数防止出现对播放列表窗口分配两个 VdkDC 的情况
    void DoHandleDelete(DelAction da, wxDC &dc, wxDC &playListDC);

    // 响应播放列表窗口快捷菜单的“播放”项
    void OnPlaySel(VdkVObjEvent &e);

    // 响应“删除所有文件”菜单项
    void OnDelAll(VdkVObjEvent &e);

    // 删除重复的播放列表项目
    void RemoveDuplicatedItems(VdkVObjEvent &);
    // 删除错误的播放列表项目(判断标准：歌曲长度为0)
    void RemoveWrongItems(VdkVObjEvent &);

    // 删除列表中所有歌曲
    void DeleteAllSongs(wxDC *pDC, wxDC *playListDC);

    // 是否清空当前歌词的显示
    enum ClearLyricOpt {
        CL_CLEAR, // 清空
        CL_LEAVE, // 保留
    };

    // 停止播放
    // @param clearLyric 是否清空当前歌词的显示
    void DoStop(wxDC *pDC, wxDC *playListDC, ClearLyricOpt clearLyric);

    // 切换播放器至不可播放歌曲的状态
    //
    // 若播放列表中没有歌曲，则不能播放。
    void StopPlayer(wxDC *pDC);

    void DeleteSel(VdkVObjEvent &); // 删除选定项目
    // 处理播放列表的选择事件
    void SelectAllNoneReverse(VdkVObjEvent &);

    // 异步处理来自播放引擎的歌曲播放已顺利完结的通知事件
    void OnSongFinished(wxCommandEvent &);

    void OnVolume(VdkVObjEvent &); // 音量滚动条
    void DoSetVolume(); // 将音量控制实际应用到播放引擎中

    void OnMute(VdkVObjEvent &); // 在主窗口上单击“静音”按钮
    void OnMenuVolumeCtrl(VdkVObjEvent &); // 菜单控制音量

    // 菜单控制播放上一首、下一首
    void OnMenuPrevNext(VdkVObjEvent &);
    void OnPrevNext(VdkVObjEvent &); // 上一首、下一首

    enum PrevNextAction {
        PNA_PREV,
        PNA_NEXT,
    };

    // 统一处理切换上一首、下一首行为
    void HandlePrevNext(PrevNextAction pna, wxDC *pDC, wxDC *playListDC);

    void OnPlayListDClick(VdkVObjEvent &); // 双击列表项目
    void OnPlayModes(VdkVObjEvent &); // 切换播放列表迭代方式
    void SwitchPlayMode(OOPlayerPlayMode mode);

    // 询问用户并保存播放列表
    void SaveListByUser(VdkVObjEvent &);
    // 询问用户并打开一个新的播放列表
    void OpenListByUser(VdkVObjEvent &);
    // 询问用户要操作的播放列表路径(新建/打开)
    // 返回用户指定的路径
    wxString AskPathToList(const wxString &alert, long dlgStyle);
    // 参数 path 是否与当前的默认播放列表文件的路径相同
    bool IsPlayListPathOk(const wxString &path);

    // 添加一个新的播放列表
    void AddPlayList(VdkVObjEvent &);
    // 排序播放列表
    void SortPlayList(VdkVObjEvent &);
    // 根据用户用户输入查找播放列表
    void OnSearchPlaylist(VdkVObjEvent &);
    //  歌曲标签信息已更新
    void OnSongInfoUpdated(wxCommandEvent &e);
    // 打开文件路径
    void OnLocateFile(VdkVObjEvent &);

    // 添加文件的菜单回调函数
    // 假如将回调函数放到窗口类中会导致 this 指针不正确
    void OnAddFileFolders(VdkVObjEvent &);

    //////////////////////////////////////////////////////////////////////////
    // 歌词

    // 开始新的歌词下载会话
    void NewLyricDownloadSession();

    // 为正在播放的歌曲加载歌词
    //
    // 即使该歌曲已经加载了歌词，本函数仍然会重新加载。
    // @attention 不要直接调用这个函数，而应该调用
    // \link UpdateLyricState \endlink 。
    bool LoadCurrSongLyric();

    // 根据播放器当前状态为歌词秀控件指定正确的动作(暂停、滚动等等)
    void UpdateLyricState();

    // 清空歌词，并显示预定义交互性输出文本
    void ClearLyric();

    // 在线搜索歌词
    void OnSearchLyric(VdkVObjEvent &);
    // 用户选择了一个搜索得来的歌词
    void OnLyricLoaded(wxCommandEvent &e);

    /// 复制歌词到剪贴板
    void OnCopyLyric(VdkVObjEvent &);

    /// 关联歌词
    void OnAssociateLyric(VdkVObjEvent &);

    /// 重新加载歌词
    void OnReloadLyric(VdkVObjEvent &);

    /// 撤销歌词
    void OnRemoveLyric(VdkVObjEvent &);

    //////////////////////////////////////////////////////////////////////////
    // 处理应用程序当前状态

    // 增设一个状态标志位
    void SetAddinState(int state) {
        m_state |= state;
    }

    // 移除已设定的指定状态标志位
    void RemoveState(int state) {
        m_state &= ~state;
    }

    // 取反指定状态标志位
    void ReverseState(int state) {
        m_state ^= state;
    }

    // 测试指定状态标志位
    bool TestState(int state) const {
        return (m_state & state) != 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // 杂项

    // 打开网站链接
    void OnWebsiteLinks(VdkVObjEvent &);

    // 打开“关于”对话框
    void OnAbout(VdkVObjEvent &);

    // 打开“我要反馈”对话框
    void OnFeedback(VdkVObjEvent &);

private:

    wxSingleInstanceChecker *m_checker;

    OOPConf *m_conf; // 应用程序配置

    long m_state; // 当前应用程序状态

    sf::Music m_music;
    MusicEndingDetector m_endingDetector;
    OOPStopWatch m_stopWatch;
    OOPSongPtr m_currSong;

    OOPLyricParser m_lyricParser;
    OOPDesktopLyric *m_desktopLyric;
    OOPLyricMgr m_lyric;

    // 窗口和菜单的句柄在整个应用程序生命周期内是不变的
    MainPanel *m_mainPanel;
    PlayListPanel *m_playListPanel;
    LyricPanel *m_lyricPanel;
    EqPanel *m_eqPanel;

    VdkBitmapArray m_mainMenuBitmaps;

    VdkMenuItem *m_playPauseItem;
    VdkMenuItem *m_stopItem;
    VdkMenuItem *m_muteItem;
    VdkMenuItem *m_prevItem;
    VdkMenuItem *m_nextItem;
    VdkMenu *m_playModes1; // 播放模式菜单
    VdkMenu *m_playModes2; // 播放模式菜单(播放列表窗口)

    // 当前播放列表的迭代方式
    OOPlayerPlayMode m_playMode;

    //////////////////////////////////////////////////////////////////////////

    // 管理 OOPlayer 的皮肤列表
    OOPSkin m_skinList;

    // WinAMP 形式的粘附窗口
    StickyWindows m_windowManager;

    // 托盘图标
    OOPTrayIcon *m_trayIcon;

    DECLARE_EVENT_TABLE()
};
