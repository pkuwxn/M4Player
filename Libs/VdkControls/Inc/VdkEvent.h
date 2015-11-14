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

/// \brief ������η���ͬ������¼��Ĵ�����
class MouseHoldTimer : public wxTimer
{
public:

	/// \brief �ڴ������ڲ���������һ��Ҫ�������͵��¼�
	virtual void Notify();

	/// \brief ����������
	void Start(VdkControl* pCtrl, int evtCode, const wxPoint& Point);

	/// \brief �õ���������Ŀ��ؼ�
	VdkControl* GetCtrl() const { return m_pCtrl; }

private:

	VdkControl*	m_pCtrl;		// Ҫ����� VdkControl
	int			m_evtCode;		// Ҫ�������͵��¼� ID
	wxPoint		m_Point;		// Ҫ�������ͳ�ȥ�����ָ��λ��
};

//////////////////////////////////////////////////////////////////////////

#if 1
/// \brief VDK ���̺�����¼��Ļ���
class VdkEvent
{
public:

	/// \brief ���캯��
	VdkEvent(int e, wxDC& d);

	/// \brief �������¼�
	///
	/// ��֪�¼�������ֹͣ�Ա��¼��ĺ�������
	void Skip(bool skip) { m_bSkipped = skip; }

	/// \brief �¼��Ƿ�ȡ��
	bool GetSkipped() const { return m_bSkipped; }

	//////////////////////////////////////////////////////////////////////////

	int evtCode; ///< ����¼�����
	wxDC& dc; ///< ���ӵĻ�ͼ������

	bool controlDown; ///< �����ϵ� Ctrl ���Ƿ����ڰ���
	bool shiftDown; ///< �����ϵ� Shift ���Ƿ����ڰ���

private:

	bool m_bSkipped;
};

/// \brief ����һ������¼�
class VdkMouseEvent : public VdkEvent
{
public:

	/// ���캯��
	VdkMouseEvent(int e, const wxPoint& m, wxDC& d);

	/// ���캯��
	VdkMouseEvent(wxMouseEvent& e, int code, wxDC& d);

	/// ��ȡ�� wxWidgets �Ķ�Ӧ�¼�����
	wxMouseEvent* GetNativeEventObj() const { return m_nativeEventObj; }

public:

	wxPoint mousePos; ///< ָ��λ��

private:

	wxMouseEvent* m_nativeEventObj; // wxWidgets �Ķ�Ӧ�¼�����
};

/// \brief ����һ�����̰����¼�
class VdkKeyEvent : public VdkEvent
{
public:

	/// \brief ���캯��
	VdkKeyEvent(VdkKeyEventType type, wxKeyEvent& e, wxDC& dcref);

	/// \brief ���캯��
	VdkKeyEvent(VdkKeyEventType type, int keyCode, wxDC& dcref);

	/// \brief ��ȡ wxWidgets ��Ӧ���¼�����
	wxKeyEvent* GetNativeObj() { return m_nativeEventObj; }

	/// \brief ��ȡ�¼������ļ����ַ�����
	int GetKeyCode() const { return m_keyCode; }

private:

	int m_keyCode; // ��������
	wxKeyEvent* m_nativeEventObj; // wxWidgets ��Ӧ���¼�����
};
#endif

/// \brief �ӱ�׼ wx ����¼��� VDK �Ķ�Ӧ����
VdkMouseEventType FromStd(const wxMouseEvent& e);

//////////////////////////////////////////////////////////////////////////
/// \brief ����һ�� Vdk �ַ���֪ͨ
class VdkNotify
{
public:

	/// \brief ���캯��
	VdkNotify(VdkCtrlNotifyCode notice);

	/// \brief ����֪ͨ��ʶ��
	void SetNotifyCode(VdkCtrlNotifyCode notice) {
		m_notice = notice;
	}

	/// \brief ��ȡ֪ͨ��ʶ��
	VdkCtrlNotifyCode GetNotifyCode() const { return m_notice; }

	/// \brief ���ø�������
	void SetInt(int data) { m_int = data; }

	/// \brief ��ȡ��������
	int GetInt() const { return m_int; }

	/// \brief ���ø�������
	void SetLong(long data) { m_long = data; }

	/// \brief ��ȡ��������
	long GetLong() const { return m_long; }

#   ifndef WPARAM
	typedef unsigned int WPARAM;
#   endif

#   ifndef LPARAM
	typedef unsigned int LPARAM;
#   endif

	/// \brief ���� WPARAM
	void SetWparam(WPARAM wParam) { m_wParam = wParam; }

	/// \brief ��ȡ WPARAM
	WPARAM GetWparam() const { return m_wParam; }

	/// \brief ���� LPARAM
	void SetLparam(LPARAM lParam) { m_lParam = lParam; }

	/// \brief ��ȡ LPARAM
	LPARAM GetLparam() const { return m_lParam; }

	/// \brief ��ȡ DC
	void SetVObjDC(wxDC* pDC) { m_pDC = pDC; }

	/// \brief ��ȡ DC
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

/// \brief �ⲿ�¼�������
class VdkEventFilter
{
public:

	/// \brief ���캯��
	VdkEventFilter(VdkWindow* win = NULL);

	/// \brief ��������
	~VdkEventFilter();

	/// \brief ��ͣ���˲���
	void Pause() { m_paused = true; }

	/// \brief �������˲���
	void Resume() { m_paused = false; }

	/// \brief �������Ƿ��ڻ״̬
	bool IsActive() const { return !m_paused; }

	/// \brief Ҫ���˵��¼�����ϸ��Ϣ
	class EventForFiltering	{
	public:

		/// \brief ���캯��
		EventForFiltering( VdkWindow* window, int evtCode )
			: m_window( window ), m_evtCode( evtCode ) {}

		/// \brief ��ȡ����
		VdkWindow* window() const { return m_window; }

		/// \brief ��ȡ�¼�����
		int evtCode() const { return m_evtCode; }

		/// \brief ��������¼�����
		EventForFiltering& mouseEvent(const wxMouseEvent& e) {
			m_mouseEvent = e; return *this;
		}

		/// \brief ��ȡ����¼�����
		const wxMouseEvent& mouseEvent() const { return m_mouseEvent;	}

	private:

		VdkWindow* m_window;
		int m_evtCode;
		wxMouseEvent m_mouseEvent;
	};

	/// \brief �ڴ˹�������Ϊ\a evtCode ���¼�
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

	/// \brief �û����õ��Զ�����Ϣ��ʼ ID
	VEMC_USER					= -19900905 * 2,
	
	/// \brief ֪ͨ��������ʾ�����˵�
	///
	/// ���������߳��е�����ʾ���������򸸴��ڻ�ʧȥ���㡣
	/// ��һ������ǵ������ڻ�ʹ�������ػ����Ӷ��ƻ� VdkDC ��ά�ֵ�״̬��
	/// �ʽ�ʵ����ʾ�˵��Ĵ����Ӻ�ִ�С�
	VEMC_SHOW_CONTEXT_MENU		= -19900905,
	/// \brief ֪ͨ�����ڲ˵������أ��������״̬
	VEMC_MENU_HID,
};

/// \brief �ؼ����˵��Ļص�������Ϣ
class VdkVObjEvent : public wxCommandEvent
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkVObjEvent(int id = wxID_ANY);

	/// \brief ���ƹ��캯��
	VdkVObjEvent(const VdkVObjEvent& other);

	/// \brief wxWidgets RTTI ����
	virtual wxEvent* Clone() const { return new VdkVObjEvent( *this ); }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡ DC
	void SetVObjDC(wxDC* pDC) { m_pDC = pDC; }

	/// \brief ��ȡ DC
	wxDC* GetVObjDC() const { return m_pDC; }

	/// \brief ���ò˵����
	void SetMenu(VdkMenu* menu) { m_menu = menu; }

	/// \brief ��Ϊ�˵��ص�ʱ��ȡ�˵����
	VdkMenu* GetMenu() const { return m_menu; }

	/// \brief ���ò˵�����
	void SetMenuItem(VdkMenuItem* item) { m_menuItem = item; }

	/// \brief ��Ϊ�˵��ص�ʱ��ȡ�˵�����
	VdkMenuItem* GetMenuItem() const { return m_menuItem; }

	/// \brief ����������Ŀؼ����
	void SetCtrl(VdkControl* pCtrl) { m_pCtrl = pCtrl; }

	/// \brief ��ȡ������Ŀؼ����
	VdkControl* GetCtrl() const { return m_pCtrl; }

	/// \brief ��ȡ������Ŀؼ����
	template< class T >
	T* GetCtrl() const { return (T*) m_pCtrl; }

	/// \brief ���ü��̰����¼����ӵİ�����ʶ��
	void SetKeyCode(int k) { m_keyCode = k; }

	/// \brief ���ü��̰����¼����ӵİ�����ʶ��
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

/// \brief VdkWindow ��һ����ʾʱ���͵���Ϣ��ʶ��
class VdkWindowFirstShownEvent : public wxCommandEvent
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkWindowFirstShownEvent(VdkWindow* win);

	/// \brief wxWidgets RTTI ����
	virtual wxEvent *Clone() const {
		return new VdkWindowFirstShownEvent( *this );
	}

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡ������ VdkWindow
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
