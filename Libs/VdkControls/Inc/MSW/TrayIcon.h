#pragma once
#include <wx/taskbar.h>

class VdkWindow;

/// \brief 必要菜单项所用的 ID
enum EssentialMenuItems {
	TI_MENU_EXIT = wxID_HIGHEST + 1000,
	TI_FLASH_TIMER
};

/// \brief 当前托盘图标所处的状态
enum TrayIconState {
	TI_STATE_NOMAL, ///< 正常
	TI_STATE_HID, ///< 隐藏
	TI_STATE_FLASHING, ///< 闪烁
};

/// \brief 原有 wxTrayIcon 的扩展类，添加了图标闪烁等特性
class TrayIcon : public wxTaskBarIcon
{
public:

	/// \brief 构造函数
	TrayIcon( VdkWindow* frame, 
			  const wxIcon& icon,
			  wxString strTrunkTooltip );

	/// \brief 设置相关联的窗体
	void SetMainFrame(VdkWindow* frame);

	/// \brief 生成弹出菜单
	virtual wxMenu* CreatePopupMenu();

	/// \brief 获取图标提示信息
	wxString GetToolTip() { return m_strTooltip; }
	/// \brief 设置图标提示信息
	void SetToolTip(wxString strTooltip);

	/// \brief 闪烁任务栏图标
	void Flash(bool bEnabled = true);
	
	/// \brief 得到当前托盘图标所处的状态（Flashing,etc.）
	TrayIconState GetState() const { return m_state; }

	/// \brief 隐藏/显示图标
	void Hide(bool bToHide = true);

private:

	void OnLeftDoubleClick(wxTaskBarIconEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnTimer(wxTimerEvent&);
	void OnTimerUser(wxCommandEvent& e);

	//////////////////////////////////////////////////////////////////////////

	VdkWindow*					m_mainFrame;

	NOTIFYICONDATA				m_nid;

	bool						m_bNull; ///<是否正在显示空白图标（用于闪烁后还原）
	TrayIconState				m_state; ///<当前托盘图标所处的状态（Flashing,etc.）

	wxTimer						m_timer; ///<各种可能需要用到 Timer 的显示功能所用 Timer
	wxString					m_strTrunkTooltip; ///<用于第一行的显示，区分多个程序同时运行的情况

	DECLARE_EVENT_TABLE()
};
