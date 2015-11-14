/***************************************************************
 * Name:      OOPConf.h
 * Purpose:   OOPlayer 的所有配置信息
 * Author:    Wang Xiao Ning (vanxining@139.com)
 * Created:   2011-05-07
 **************************************************************/
#pragma once

 /// \brief 粘附窗口状态持久化
class OOPStickyWindowState
{
public:

    /// \brief 构造函数
    OOPStickyWindowState();

    /// \brief 是否已粘附到主窗口
    bool IsStickyToMain() const { return m_stickyToMain; }

    /// \brief 获取保存的窗口作用域
    wxRect GetRect() const { return m_rect; }

    /// \brief 清空当前值，用于换肤时的状态置为默认值
    void Reset();

private:

    // 是否已经粘附到主窗口
    bool m_stickyToMain;

    // 窗口作用域，可能是相对于主窗口左上角而言，也有可能是屏幕坐标
    wxRect m_rect;

    friend class OOPWindow;
    friend bool FromString(const wxString &raw, OOPStickyWindowState &out);
};

/// \brief OOPlayer 的应用程序相关配置信息
class OOPConf
{
public:

	/// \brief 构造函数
	OOPConf();

    /// \brief 载入配置
    bool Load(const wxString& path);

    /// \brief 保存配置
    bool Save(const wxString& path);

    /// \brief 清空所有粘附窗口状态
    void ClearStickyWindowStates();

public:

	int m_currSong; /*! 当前正在播放的歌曲 */
	int m_playMode; /*! 当前播放列表迭代模式 */
	wxString m_skinFolderName; /*! 当前皮肤目录名 */
	bool m_playing; /*! 关闭时是否正在播放 */
	int m_currPos; /*! 当前播放进度 */

	/*! 即使在 Ubuntu Unity shell 下面也显示托盘图标 */
	bool m_showTaskBarIconUnderUnity;
	bool m_playListShown; /*! 播放列表窗口是否可见 */
	bool m_lyricShown; /*! 歌词秀窗口是否可见 */
	bool m_eqShown; /*! EQ 窗口是否可见 */

	double m_volume; /*! 当前音量 */
	bool m_mute; /*! 是否静音 */

    /*! 各窗口的位置及粘附状态 */
    OOPStickyWindowState mainPanelWndState;
    OOPStickyWindowState playlistPanelWndState;
    OOPStickyWindowState lyricPanelWndState;
    OOPStickyWindowState eqPanelWndState;

private:

    enum {
        NUM_PANELS = 4,
    };

private:

    bool Player(const wxXmlNode *node);
    wxXmlNode *Player();
};
