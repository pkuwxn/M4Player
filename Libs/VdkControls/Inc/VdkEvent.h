#pragma once
#include "VdkDefs.h"
#include "VdkDC.h"

//////////////////////////////////////////////////////////////////////////

#define vdkDECLARE_EVENT( name, type ) \
	wxDECLARE_EVENT( name, type ); \
	typedef void (wxEvtHandler::*Vdk ## type ## Function)(type&)

#define vdkDECLARE_EVENT_TABLE_ENTRY( id0, id1, fn, name, type ) \
	DECLARE_EVENT_TABLE_ENTRY( name, id0, id1, \
		(wxObjectEventFunction)(wxEventFunction)(Vdk ## type ## Function) \
			&fn, (wxObject*) NULL ),

#define vdkDEFINE_EVENT wxDEFINE_EVENT

//////////////////////////////////////////////////////////////////////////

class VdkControl;
class VdkWindow;
class VdkMenu;
class VdkMenuItem;

/// \brief 连续多次发送同样鼠标事件的触发器
class MouseHoldTimer : public wxTimer
{
public:

	/// \brief 在触发器内产生并发送一次要持续发送的事件
	virtual void Notify();

	/// \brief 启动触发器
	void Start(VdkControl* pCtrl, int evtCode, const wxPoint& Point);

	/// \brief 得到触发器的目标控件
	VdkControl* GetCtrl() const { return m_pCtrl; }

private:

	VdkControl*	m_pCtrl;		// 要处理的 VdkControl
	int			m_evtCode;		// 要持续发送的事件 ID
	wxPoint		m_Point;		// 要持续发送出去的鼠标指针位置
};

//////////////////////////////////////////////////////////////////////////

#if 1
/// \brief VDK 键盘和鼠标事件的基类
class VdkEvent
{
public:

	/// \brief 构造函数
	VdkEvent(int e, wxDC& d);

	/// \brief 跳过本事件
	///
	/// 告知事件发送者停止对本事件的后续处理。
	void Skip(bool skip) { m_bSkipped = skip; }

	/// \brief 事件是否被取消
	bool GetSkipped() const { return m_bSkipped; }

	//////////////////////////////////////////////////////////////////////////

	int evtCode; ///< 鼠标事件类型
	wxDC& dc; ///< 附加的绘图上下文

	bool controlDown; ///< 键盘上的 Ctrl 键是否正在按下
	bool shiftDown; ///< 键盘上的 Shift 键是否正在按下

private:

	bool m_bSkipped;
};

/// \brief 代表一个鼠标事件
class VdkMouseEvent : public VdkEvent
{
public:

	/// 构造函数
	VdkMouseEvent(int e, const wxPoint& m, wxDC& d);

	/// 构造函数
	VdkMouseEvent(wxMouseEvent& e, int code, wxDC& d);

	/// 获取与 wxWidgets 的对应事件对象
	wxMouseEvent* GetNativeEventObj() const { return m_nativeEventObj; }

public:

	wxPoint mousePos; ///< 指针位置

private:

	wxMouseEvent* m_nativeEventObj; // wxWidgets 的对应事件对象
};

/// \brief 代表一个键盘按键事件
class VdkKeyEvent : public VdkEvent
{
public:

	/// \brief 构造函数
	VdkKeyEvent(VdkKeyEventType type, wxKeyEvent& e, wxDC& dcref);

	/// \brief 构造函数
	VdkKeyEvent(VdkKeyEventType type, int keyCode, wxDC& dcref);

	/// \brief 获取 wxWidgets 对应的事件对象
	wxKeyEvent* GetNativeObj() { return m_nativeEventObj; }

	/// \brief 获取事件附带的键盘字符代码
	int GetKeyCode() const { return m_keyCode; }

private:

	int m_keyCode; // 按键代码
	wxKeyEvent* m_nativeEventObj; // wxWidgets 对应的事件对象
};
#endif

/// \brief 从标准 wx 鼠标事件到 VDK 的对应定义
VdkMouseEventType FromStd(const wxMouseEvent& e);

//////////////////////////////////////////////////////////////////////////
/// \brief 代表一个 Vdk 分发的通知
class VdkNotify
{
public:

	/// \brief 构造函数
	VdkNotify(VdkCtrlNotifyCode notice);

	/// \brief 设置通知标识符
	void SetNotifyCode(VdkCtrlNotifyCode notice) {
		m_notice = notice;
	}

	/// \brief 获取通知标识符
	VdkCtrlNotifyCode GetNotifyCode() const { return m_notice; }

	/// \brief 设置附加整数
	void SetInt(int data) { m_int = data; }

	/// \brief 获取附加整数
	int GetInt() const { return m_int; }

	/// \brief 设置附加整数
	void SetLong(long data) { m_long = data; }

	/// \brief 获取附加整数
	long GetLong() const { return m_long; }

#   ifndef WPARAM
	typedef unsigned int WPARAM;
#   endif

#   ifndef LPARAM
	typedef unsigned int LPARAM;
#   endif

	/// \brief 设置 WPARAM
	void SetWparam(WPARAM wParam) { m_wParam = wParam; }

	/// \brief 获取 WPARAM
	WPARAM GetWparam() const { return m_wParam; }

	/// \brief 设置 LPARAM
	void SetLparam(LPARAM lParam) { m_lParam = lParam; }

	/// \brief 获取 LPARAM
	LPARAM GetLparam() const { return m_lParam; }

	/// \brief 获取 DC
	void SetVObjDC(wxDC* pDC) { m_pDC = pDC; }

	/// \brief 获取 DC
	wxDC* GetVObjDC() const { return m_pDC; }

private:

	VdkCtrlNotifyCode m_notice;
	int m_int;
	long m_long;
	WPARAM m_wParam;
	LPARAM m_lParam;
	wxDC* m_pDC;
};

//////////////////////////////////////////////////////////////////////////

/// \brief 外部事件过滤器
class VdkEventFilter
{
public:

	/// \brief 构造函数
	VdkEventFilter(VdkWindow* win = NULL);

	/// \brief 析构函数
	~VdkEventFilter();

	/// \brief 暂停过滤操作
	void Pause() { m_paused = true; }

	/// \brief 继续过滤操作
	void Resume() { m_paused = false; }

	/// \brief 过滤器是否处于活动状态
	bool IsActive() const { return !m_paused; }

	/// \brief 要过滤的事件的详细信息
	class EventForFiltering	{
	public:

		/// \brief 构造函数
		EventForFiltering( VdkWindow* window, int evtCode )
			: m_window( window ), m_evtCode( evtCode ) {}

		/// \brief 获取窗口
		VdkWindow* window() const { return m_window; }

		/// \brief 获取事件类型
		int evtCode() const { return m_evtCode; }

		/// \brief 设置鼠标事件对象
		EventForFiltering& mouseEvent(const wxMouseEvent& e) {
			m_mouseEvent = e; return *this;
		}

		/// \brief 获取鼠标事件对象
		const wxMouseEvent& mouseEvent() const { return m_mouseEvent;	}

	private:

		VdkWindow* m_window;
		int m_evtCode;
		wxMouseEvent m_mouseEvent;
	};

	/// \brief 在此过滤类型为\a evtCode 的事件
	virtual bool FilterEvent(const EventForFiltering& e) = 0;

private:

	VdkWindow* m_win;
	bool m_paused;
};

//////////////////////////////////////////////////////////////////////////

#define IMPORT_EVENT( evtname ) \
	extern const wxEventType evtname;

#define NEW_EVENT( evtname ) \
	IMPORT_EVENT( evtname ) \
	const wxEventType evtname( wxNewEventType() );

//////////////////////////////////////////////////////////////////////////

enum VdkExtraMessageID {

	/// \brief 用户可用的自定义消息起始 ID
	VEMC_USER					= -19900905 * 2,
	
	/// \brief 通知主窗口显示弹出菜单
	///
	/// 必须在主线程中调用显示函数，否则父窗口会失去焦点。
	/// 另一种情况是弹出窗口会使父窗口重画，从而破坏 VdkDC 中维持的状态。
	/// 故将实际显示菜单的代码延后执行。
	VEMC_SHOW_CONTEXT_MENU		= -19900905,
	/// \brief 通知主窗口菜单已隐藏，更新相关状态
	VEMC_MENU_HID,
};

/// \brief 控件、菜单的回调函数信息
class VdkVObjEvent : public wxCommandEvent
{
public:

	/// \brief 默认构造函数
	VdkVObjEvent(int id = wxID_ANY);

	/// \brief 复制构造函数
	VdkVObjEvent(const VdkVObjEvent& other);

	/// \brief wxWidgets RTTI 必须
	virtual wxEvent* Clone() const { return new VdkVObjEvent( *this ); }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 获取 DC
	void SetVObjDC(wxDC* pDC) { m_pDC = pDC; }

	/// \brief 获取 DC
	wxDC* GetVObjDC() const { return m_pDC; }

	/// \brief 设置菜单句柄
	void SetMenu(VdkMenu* menu) { m_menu = menu; }

	/// \brief 作为菜单回调时获取菜单句柄
	VdkMenu* GetMenu() const { return m_menu; }

	/// \brief 设置菜单项句柄
	void SetMenuItem(VdkMenuItem* item) { m_menuItem = item; }

	/// \brief 作为菜单回调时获取菜单项句柄
	VdkMenuItem* GetMenuItem() const { return m_menuItem; }

	/// \brief 设置相关联的控件句柄
	void SetCtrl(VdkControl* pCtrl) { m_pCtrl = pCtrl; }

	/// \brief 获取相关联的控件句柄
	VdkControl* GetCtrl() const { return m_pCtrl; }

	/// \brief 获取相关联的控件句柄
	template< class T >
	T* GetCtrl() const { return (T*) m_pCtrl; }

	/// \brief 设置键盘按键事件附加的按键标识符
	void SetKeyCode(int k) { m_keyCode = k; }

	/// \brief 设置键盘按键事件附加的按键标识符
	int GetKeyCode() { return m_keyCode; }

private:

	VdkControl* m_pCtrl;
	VdkMenu* m_menu;
	VdkMenuItem* m_menuItem;
	wxDC* m_pDC;
	int m_keyCode;
};

vdkDECLARE_EVENT( wxEVT_VOBJ, VdkVObjEvent );
#define EVT_VOBJ_RANGE( id0, id1, fn )  \
	vdkDECLARE_EVENT_TABLE_ENTRY( id0, id1, fn, wxEVT_VOBJ, VdkVObjEvent )

#define EVT_VOBJ( id, fn ) EVT_VOBJ_RANGE( id, wxID_ANY, fn )

//////////////////////////////////////////////////////////////////////////

/// \brief VdkWindow 第一次显示时发送的消息标识符
class VdkWindowFirstShownEvent : public wxCommandEvent
{
public:

	/// \brief 默认构造函数
	VdkWindowFirstShownEvent(VdkWindow* win);

	/// \brief wxWidgets RTTI 必须
	virtual wxEvent *Clone() const {
		return new VdkWindowFirstShownEvent( *this );
	}

	//////////////////////////////////////////////////////////////////////////

	/// \brief 获取关联的 VdkWindow
	VdkWindow* GetWindow() const { return m_window; }

private:

	VdkWindow* m_window;
};

wxDECLARE_EVENT( wxEVT_VW_FIRST_SHOWN, VdkWindowFirstShownEvent );

typedef void ( wxEvtHandler::* VdkWindowFirstShownEventFunction )
	(VdkWindowFirstShownEvent&);
#define VdkWindowFirstShownEventHandler( func ) \
	wxEVENT_HANDLER_CAST( VdkWindowFirstShownEventFunction, func )
#define EVT_VW_FIRST_SHOWN( func ) \
	wx__DECLARE_EVT0( wxEVT_VW_FIRST_SHOWN, \
		VdkWindowFirstShownEventHandler( func ) )

//////////////////////////////////////////////////////////////////////////

wxDECLARE_EVENT( wxEVT_REDIRECT_CMD, wxCommandEvent );

#define EVT_RED_CMD( id, func ) \
	EVT_COMMAND( id, wxEVT_REDIRECT_CMD, func )

#define EVT_RED_CMD_RANGE( id1, id2, func ) \
	EVT_COMMAND_RANGE( id1, id2, wxEVT_REDIRECT_CMD, func )
