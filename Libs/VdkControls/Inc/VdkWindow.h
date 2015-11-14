/***************************************************************
 * Name:      VdkWindow.h
 * Author:    Ning (vanxining@139.com)
 * Created:   2009-12-19
 * Copyright: Ning
 **************************************************************/
/*!\mainpage VDK(Virtual-controls Development Kit)
 *
 * \section intro_sec ���
 *
 * ��� DirectUI �з��ĵ����ơ�����д VDK �ĳ���ֻ�� Gtk ��֧�� 
 * borderless BitmapButton ������Ҫдһ�� DirectUI ����ͷ��
 * ��˴�����ȫû��һ�������Ӧ�е��Ͻ����ǡ������ݴ����Գ�����
 * �㲻�ϵ����ع�����д�����������ճ���һ���Լ��Ƚ��������Ʒ��
 *
 * \section install_sec ��װ
 *
 * �޷��ṩһ����̬�����DLL�������Լ������������������һ����롣\n
 * Ϊʲô�أ���Ϊ���������Ż�����Ϊ���е� VdkControl �����п��ܴ�һ��
 * XRC �ļ��ж�̬���������������ǵĴ����п��ܾ͸�����û���ù�����һ��
 * ��������к�������˱���������Ϊ����û�����õ�����࣬���Ծ�û�н�
 * ������Ŀ��������ӽ����յĿ�ִ���ļ��\n
 * Orz��ֻ�ܲ�ȡ��İ취�ˡ�\n
 * ���������������еġ��Ǿ�����һ��C++�ļ����������� VdkControl 
 * ��ͷ�ļ��������� IMPLEMENT_DYNMATIC_VOBJECT( VdkXXXX ) ��д���ļ��С�
 * �������᲻���������һ���׶�:���е� VdkControl �������ӽ�����ļ����ˣ�
 * �᲻�ᵼ�´������ͣ�
 *
 */
#pragma once
#include "VdkScalableBitmap.h"
#include "VdkUtil.h"
#include <wx/vector.h>

class VdkControl;
class VdkWindow;
class VdkMenu;

class VdkMouseEvent;
class VdkEventFilter;
class VdkVObjEvent;
class VdkDC;
class VdkDcPostListener;

class MouseHoldTimer;
class TrayIcon;
class RedrawCallback;
class TabOrderIterator;
class MapOfCtrlIdInfo;

VDK_DECLARE_WX_PTR_LIST( VdkEventFilter, VdkEventFilterList, EventFilterIter )

/// \brief VdkWindow �ĳ�ʼ��������ԣ�
/// 
/// ֻ�ڳ�ʼ�� VdkWindow ʱ��Ч�����벻ͬ�� VdkWindowStyle ���Ѷ���ֵ��
enum VdkWindowInitStyle
{
	/// \brief �����ڱ������϶����壬ֻ���ڴ��ڿհ״��϶�����
	VWIS_DRAG_BY_SPACE		= 1 << 6,
};

/// \brief VdkWindow �ķ�����ԣ�
enum VdkWindowStyle
{
	/// \brief ���������ʼ����λ
	VWS_USER				= 20,
	/// \brief �Ƿ���Ҫ�����пؼ�������һ������� wxPanel ��
	///
	/// ���Ǻܶ� GUI ���Ҫ��ģ������� wxGTK ����һ�� wxFrame
	/// �� wxDialog ���޷����յ������¼��ģ�������һ����ӿؼ���
	VWS_BASE_PANEL			= 1 << 0,
	VWS_RESIZEABLE			= 1 << 1, ///< �ܷ�ı��С
	VWS_DRAGGABLE			= 1 << 2, ///< �ܷ�ʹ������϶�
	VWS_MAXIMIZABLE			= 1 << 3, ///< �ܷ����
	VWS_ALWAYS_REFRESH		= 1 << 4, ///< OnPaint ��ʼ���ػ���������
	VWS_MENU_IMPL			= 1 << 5, ///< ���������˵�ʵ��ʵ�ִ���

	/// \brief ����˫�����ͼ
	///
	/// �ѷ�����
	VWS_NO_DOUBLE_BUFFER	= 1 << 7, 
	VWS_FIRST_SHOWN_EVT		= 1 << 8, ///< ���͵�һ����ʾ��֪ͨ��Ϣ
	/// \brief �û����� ESC ʱ�����ش��ڣ�һ�����ڶԻ���
	VWS_DISMISS_BY_ESC		= 1 << 9,
	/// \brief �����ܵ����ڹر��¼�ʱ�����ٴ���
	/// 
	/// һ�����ڶԻ��򣬿��崰��(Frame window)�ر�ʱ��Ĭ�����١�
	VWS_CLOSE_TO_DESTORY	= 1 << 10,
};

/// \brief VdkWindow ����ʱ״̬��
enum VdkWindowState
{
	/// \brief ���������ʼ����λ
	VWST_USER						= 20,
	/// \brief ����ǲ���һֱ��ס����ס���ʱ�������Ͳ�����ͬ������¼�
	VWST_MOUSE_HOLD_ON				= 1 << 0,
	/// \brief ���ڵ�һ�ε��� OnPaint ʱ����Ҫ��Ҫ���� VdkButton �� m_Region ��\n
	/// ͬʱ�ڴ�֮ǰ����Ӧ��������¼�
	VWST_INITING					= 1 << 1,
	/// \brief �������һ������¼�ͬʱ�ڴ�֮����Ӧ��������¼�
	VWST_EXITING					= 1 << 2,
	/// \brief �Ƿ�ող� Reset ��
	VWST_RESET						= 1 << 3,
	/// \brief �Ƿ����ƶ� VdkWindow
	VWST_DRAG_AND_MOVING			= 1 << 4,
	/// \brief �Ƿ��ϴ���갴���¼��ѱ�����
	///
	/// �������ж��϶��¼��Ƿ�ר���������϶����ڡ�
	VWST_LEFT_DOWN_SKIPPED			= 1 << 5,
	/// \brief �Ƿ��ڸı� VdkWindow �Ĵ�С
	VWST_DRAG_AND_RESIZING			= 1 << 6,
	/// \brief �Ƿ����ڴ�������¼�
	VWST_MAXIMIZING					= 1 << 7,
	/// \brief ����ȫ���ػ�����
	VWST_REDRAW_ALL					= 1 << 8,
};

//////////////////////////////////////////////////////////////////////////

/// \brief Ϊ VdkWindow ����Ƥ��
class VdkWindowInitializer
{
public:

	typedef VdkWindowInitializer Window;

	/// \brief ���캯��
	VdkWindowInitializer();

	/// \brief ָ��һ��λͼ
	Window& bgBitmap(const wxBitmap& bm) { bmBkGnd = bm; return *this; }

	/// \brief ָ��������ɫ
	Window& bgColor(const wxColour& color) { BgColor = color; return *this; }

	/// \brief ָ��һ���ļ�������ʱ����ָ��һ����һ��������\a bmBkGnd ��
	Window& fileName(const wxString& s) { strFileName = s; return *this; }

	/// \brief ���ڵ������򡣵� VdkWindow Ϊ�Ӵ���ʱ��
	/// Rect.x �� Rect.y �����������Ͻǵ�����Ϊ������㣬\n
	/// ���������(Rect.x, Rect.y)����Զ��Ե�
	Window& rect(const wxRect& rc) { Rect = rc; return *this; }

	/// \param ƽ��/����
	Window& resizeable(bool b) { bResizeable = b; return *this; }

	/// \brief �� VDK ƽ��/���챳��λͼ������
	Window& tileType(VdkResizeableBitmapType type) { ResizeType = type; return *this; }

	/// \brief �� VDK ƽ��/���챳��λͼ������
	Window& tileArea(const wxRect& rc) { TileArea = rc; return *this; }

private:

	wxBitmap		bmBkGnd;
	wxString		strFileName;
	wxRect			Rect;
	wxColour		BgColor;

	bool			bResizeable;
	wxRect			TileArea;
	VdkResizeableBitmapType ResizeType;

	friend class VdkWindow;
};

/*!\brief һ���������� virtual-controls �Ĵ���
 *
 * ���� VdkControl ��������һ�� VdkWindow Ϊ�����ڡ�
**/
class VdkWindow : public VdkStyleAndStateOwner
{
public:

	/// \brief ���캯��
	VdkWindow(wxWindow* handle, long style = VWS_DRAGGABLE);

	/// \brief XRC ��̬����
	VdkWindow* Create(wxXmlNode* xmlNode, MapOfCtrlIdInfo* ids = NULL);

	/// \brief ֪ͨ VdkWindow ��Ҫ�ٴ����¼���
	void BeginExit();

	/// \brief ��������
	virtual ~VdkWindow();

	/// \brief Ϊ VdkWindow ����Ƥ��
	void Create(const VdkWindowInitializer& init_data);

	/// \brief �Ƿ����ڳ�ʼ�����в�������ͼ
	bool IsInitializing() const { return TestState( VWST_INITING ); }

	/// \brief �Ƿ����ύ���ٴ��������ʱ�����ӿؼ�Ӧֹͣ���У�ͬʱ׼��
	/// �����Լ���
	bool IsExiting() const { return TestState( VWST_EXITING ); }

	/// \brief �����ڴ����ѱ����ã��������ؽ��ؼ��б��״̬���򷵻���
	bool IsReseting() const { return TestState( VWST_RESET ); }

	/// \brief ��ʼ������ʹ�� wxRegion ��Ϊ�������־�� VdkButton
	void InitRegionButtons();

	/// \brief ��ȫ������һ�� VdkWindow ����
	template< class T >
	static void DestoryWindow(T& win)
	{
		if( win )
		{
			win->BeginExit();
			win->GetWindowHandle()->Destroy();
			win = NULL;
		}
	}

	/// \brief ��ȡ�����
	wxWindow* GetHandle() const { return m_panel; }

	/// \brief ��ȡ���������ڵľ��
	wxWindow* GetWindowHandle() const { return m_this; }

	/// \brief ��ȡ wxTopLevelWindow ���
	///
	/// \warning ���صľ������Ϊ�ա�
	wxTopLevelWindow* GetTopLevelWindowHandle() const;

#ifdef __WXDEBUG__
	/// \brief ���ô��ڱ�ʶ��
	void SetDebugCaption(const wxString& debugCaption) { 
		m_debugCaption = debugCaption;
	}

	/// \brief  ��ȡ���ڱ�ʶ��
	wxString GetDebugCaption() const { return m_debugCaption; }
#endif

	//////////////////////////////////////////////////////////////////////////
	// �봰��������ȽϽ��ܵ�һЩ GUI ����

	/// \brief �������ָ�����״
	void AssignCursor(const wxCursor& cursor);

	/// \brief �������ָ��ΪĬ����״
	void ResetCursor();

	/// \brief ��ȡ�� VdkWindow �����������ͼ��
	TrayIcon* GetTrayIcon() const { return m_trayIcon; }

	/// \brief ������ VdkWindow �����������ͼ��
	void SetTrayIcon(TrayIcon* trayIcon);

	/// \brief ����������Ϊ�Լ��� VdkMenu
	/// \note ÿ�� GUI ���嶼��Ҫһ�������ڡ�
	VdkMenu* CreateMenu(VdkMenu* parent = NULL, int mid = wxID_ANY);

	/// \brief ʹ VdkMenu ������ VdkWindow �ϣ�ʹ�ÿ���ʧȥ����ʱ VdkMenu ��ʧ
	void AttachMenu(VdkMenu* menu){ m_menu = menu; }

	/// \brief ��ȡ������ VdkWindow �� VdkMenu
	VdkMenu* GetMenu() const { return m_menu; }

	/// \brief ���Ե�ǰ�������Ͻ�Ϊ��������(\a x, \a y )����ʾ�����˵�\a menu
	///
	/// һ��������������Ϊ\link VdkMenu::ShowContext \endlink 
	/// Ҫ������Ļ���Ͻ�ΪΪ����������ʾ���ꡣ
	/// \param delay �Ƿ��ӳ���ʾ
	void ShowContextMenu(VdkControl* pCtrl, int x, int y, bool delay = false);

	/// \brief ���Ե�ǰ�������Ͻ�Ϊ��������(\a Point.x, \a Point.y )
	/// ����ʾ�����˵�\a menu
	void ShowContextMenu(VdkControl* pCtrl, const wxPoint& Point, 
						 bool delay = false)
	{
		ShowContextMenu( pCtrl, Point.x, Point.y, delay );
	}

	/// \brief ���ص�ǰ������ʾ�� VdkMenu
	static void HideMenu();

	/////////////////////////////////////////////////////////////////////////////////

	/// \brief ѹ��һ���¼�������
	void PushEventFilter(VdkEventFilter* filter) {
		m_EventFilters.Append(filter);
	}

	/// \brief �������Ϊ\a filter ���¼�������
	void PopEventFilter(VdkEventFilter* filter, bool del);

	/// \brief ��������¼�
	bool HandleMouseEvent(wxMouseEvent& e, VdkMouseEventType evtCode);

	/// \brief ��ȡ�ϴδ��������������¼�
	VdkMouseEventType GetLastMouseEvent() const { return m_nLastMouseEvent; }

	/// \brief ��ȡ��ǰ�������ָ��֮�µĿؼ�����ÿ�
	static VdkControl* GetCtrlOnHover() { return ms_hovering; }

	/// \brief ��ȡ��ǰ�������뽹��Ŀؼ�
	VdkControl* GetCtrlOnFocus() const { return m_focus; }

	/// \brief �������ؿؼ���׼�����ռ�������
	void FocusHiddenCtrl();

	/// \brief ��ȡ���ؿؼ����
	wxControl* GetHiddenCtrl() const { return m_hiddenCtrl; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief �ػ洰��
	void Draw(wxDC& pdc);

	/// \brief ���ȫ���ػ洰�ڵ��¼������ڵ���Ϣ������
	void QueueRedrawEvent();

	/// \brief ��С������
	void Minimize() { DoMinimize(); }

	/// \brief �õ����ڳ�ʼ��С
	///
	/// ��ʼ��ĳЩ VdkControl ʱ�����õ���
	wxSize GetMinSize() const;

	/// \brief �õ����ڳ�ʼ��С
	void GetMinSize(int* w, int* h) const;

	/// \brief �õ����ڵĴ�С
	wxRect Rect00() const
	{
		int w, h;
		m_this->GetSize( &w, &h );

		return wxRect( 0, 0, w, h );
	}

private:

	// �¼��ṩ����ѯ����ע����¼�������
	// ����ֵ���Ƿ�ϣ��������ֹ��������
	bool FilterEvent(int evtCode, const wxMouseEvent& e);

	/// �¼��ṩ��ѯ�� VdkWindow ���������Ƿ���Ҫ���˴��¼�
	/// \return �Ƿ�ϣ��������ֹ��������
	virtual bool FilterEventBefore(wxMouseEvent& evt, int evtCode) {
		return false;
	}

	/// �¼��ṩ��ѯ�� VdkWindow ���������Ƿ���Ҫ���˴��¼�
	///
	/// �������ܶԷ����ڴ���հ״����¼����С�����ڡ�ʽ����
	/// ������û�пؼ��ӹܵ��¼�
	/// \return �Ƿ�ϣ��������ֹ��������
	/// \TODO VdkMouseEvent ==> VdkEvent
	virtual bool FilterEventAfter(wxMouseEvent& evt, int evtCode) {
		return false;
	}

	// ִ��ʵ�ʻ��ƹ���
	void DoDraw(wxDC& dc);

	/// ��������Զ��ػ��¼�������Ӧ
	virtual void DoPaint(wxDC& dc){}

	/// ��������ԶԷ����� VdkWindow �ϵļ����¼�������Ӧ
	/// \return �Ƿ���ֹ VdkWindow �ĺ�������
	virtual bool DoHandleKeyEvent(wxKeyEvent& e) { return false; }

protected:

	/// \brief ���¼�������
	void BindHandlers();

	wxWindow* m_this; ///< VdkWindow ��ԭ�����ھ��
	wxWindow* m_panel; ///< �����ӿؼ���ֱ�Ӹ����ڣ�wxPanel �ľ����

private:

	// ȫ�־�̬��������ǰ�������ָ��֮�µĿؼ�
	static VdkControl* ms_hovering;

	// ���صĿؼ������ڽ��ռ������루ֱ���ڴ���һ���ػ�̫û�����ˣ�
	wxControl* m_hiddenCtrl;
	// ��ǰ���ھ������뽹��Ŀؼ�
	VdkControl* m_focus;

#ifdef __WXDEBUG__
	// ���ڱ�ʶ��
	wxString m_debugCaption;
#endif

	// ����������������Ĳ˵�
	VdkMenu* m_menu;

	// ��˴��������������ͼ��
	TrayIcon* m_trayIcon;

	// ���ڵ�ǰ�����ָ����״
	// ��Ҫ������ VDK ���ܻ��ڲ��Եظı����ָ�����״�Ա���
	// �û��Զ������״���Իָ���
	wxCursor m_cursor;

	// �ⲿ�¼�����������
	VdkEventFilterList m_EventFilters;

	// �ϴ� VdkControl ������״̬
	int m_nLastCtrlState;

	// ��������¼������ڱ�����
	// 
	// �����϶��¼��������¼���Щ���ܻ���������������������¼�Ӧ�÷��͵���ǰ
	// �������뽹��Ĵ��ڶ��������ָ����λ�����ϵ��Ǹ����ڡ�
	bool m_mouseEventNotForMe;

	// �ϴ������������¼�
	VdkMouseEventType m_nLastMouseEvent;

	// �ڰ�ס���ʱ�������Ͳ�����ͬ������¼�
	MouseHoldTimer* m_pMouseHoldTimer;

	// ���ڳ�ʼʱ�̵Ŀ��
	int m_nMinWidth;

	// ���ڳ�ʼʱ�̵ĸ߶�
	int m_nMinHeight;

private:

	void OnMouseMove(wxMouseEvent& e);
	void OnMouseWheel(wxMouseEvent& e);
	void OnMouseLost(wxMouseCaptureLostEvent&);
	void OnLeftDown(wxMouseEvent& e);
	void OnLeftUp(wxMouseEvent& e);
	void OnDLeftDown(wxMouseEvent& e);
	void OnRightUp(wxMouseEvent& e);
	void OnEnterWindow(wxMouseEvent&);
	void OnLeaveWindow(wxMouseEvent&);

	void OnKeyDown(wxKeyEvent& e);
	void OnKeyUp(wxKeyEvent& e);
	void OnChars(wxKeyEvent& e);

	// wxWidgets ��Ĭ�Ͻ���Windows context menu�������¼�ת������ͨ���Ҽ������¼�
	void OnWindowMenuKeyUp(wxMouseEvent&);

	// ͳһ�������ֲ�ͬ����İ����¼�
	// �ֱ�Ϊ�������¡��ͷţ��Լ��ѷ���İ����ַ���
	void HandleKeys(VdkKeyEventType type, wxKeyEvent &e);

	void OnIconize(wxIconizeEvent&);
	void OnMaximize(wxMaximizeEvent&);
	void OnSize(wxSizeEvent&);
	void OnShow(wxShowEvent& e);

	void OnPaint(wxPaintEvent& e);
	void OnWindowPaintDummy(wxPaintEvent&);
	void OnEraseBackground(wxEraseEvent&){}

	void OnWindowFocus(wxActivateEvent& e);
	void OnClose(wxCloseEvent& e);

	// �ӳ���ʾ�Ҽ��˵�
	void OnDelayShowContextMenu(VdkVObjEvent& e);

	// ��Ӧ VdkMenu ��������Ϣ
	void OnMenuHid(VdkVObjEvent& e);

	// ��ǰ�����Ƿ�Ϊһ���˵�ʵ�崰��
	bool IsMenuImpl() { return TestStyle( VWS_MENU_IMPL ); }

	// �¼��Ƿ���Լ��ǰ�ؼ�
	//
	// �絥�����һ��ȡ�
	bool IsActivatableEvent(int evtCode);

	// ���ϴμ���Ŀؼ����ڵ�ǰ�����ڣ�����øô��ڵ���һ������
	void RecoverCtrl(VdkControl* pCtrl);
	void HandleMouseHoldOn(int evtCode, const wxPoint& Point, VdkControl* pCtrl);

	// ���õ�ǰ�������ָ��֮�µĿؼ�
	static void SetCtrlOnHover(VdkControl* pCtrl);

	// ����ǰ�������ָ��֮�µĿؼ�����ÿ�
	static void ResetCtrlOnHover();

	// �����ϴδ��������������¼�
	void SetLastMouseEvent(VdkMouseEventType evtCode);

	// ���õ�ǰ�������뽹��Ŀؼ�
	void SetCtrlOnFocus(VdkControl* pCtrl);

	// ����ǰ�������뽹��Ŀؼ�����ÿ�
	void ResetCtrlOnFocus();

	// ��ȡ��ǰ����������ʾ�Ĳ˵�
	static VdkMenu*& GetMenuOnShow();

	// ���õ�ǰ����������ʾ�Ĳ˵�
	static void SetMenuOnShow(VdkMenu* menu);

	//////////////////////////////////////////////////////////////////////////

	// �ڴ����ϰ���������ʱ����ĳ�ʼλ��
	// ����ģ���϶����ڣ����濪ʼ�϶�ʱ���ָ����Դ������Ͻǵ�λ�á�
	wxPoint m_mouseOn;

	// ����Ĵ��������������϶��ı䴰�ڸı��С�Լ������ڴ�С�ı��¼�
	wxRect m_rectCached;

public:

	/// \brief ��ȡ�ڴ����ϰ���������ʱ����ĳ�ʼλ��
	wxPoint GetMouseOnForm() const { return m_mouseOn; }

#if 1
	/// \brief ���ô��ڴ�С
	bool Resize(int w, int h);

	/// \brief ���ô��ڴ�С
	bool Resize(const wxRect& rc) {
		return Resize( rc.x, rc.y, rc.width, rc.height );
	}

	/// \brief ���ô��ڴ�С
	bool Resize(const wxSize& size) { return Resize( size.x, size.y ); }

	/// \brief ���ô��ڴ�С
	/// \attention ����ֱ�ӵ��� wxWindow::SetSize ����Ϊ���ڿ���Ϊ���δ��ڡ�
	bool Resize(int x, int y, int w, int h, int sizeFlags = wxSIZE_AUTO);
#endif

	/// \brief ������С�Ŀ�߶�
	///
	/// �����С��߶���ʵ��������С��߶Ȳ�ͬ��������Ƥ���ṩ��λͼ�ļ�������\n
	/// �����ֵ���ɳ���Ա���ָ�������û��ı䴰�ڴ�Сʱ���ܴﵽ����С��߶ȡ�\n
	/// \note ���Խ�ʡƤ��λͼ�ļ��Ĵ�С���������ƴ��ڵ���С��С��
	void SetMinSize(int w, int h);

protected:

	/// \brief ֪ͨ�������� VdkControl ���� VdkWindow �ı��С�¼�
	void LayoutWidgets(int dX, int dY);

private:

	// �ػ�ǰ���뱣֤˫����λͼ����ȷ���õ�
	bool IsBufferedBitmapOk();

	// ���Ĵ�С
	virtual void DoResize(int x, int y, int width, int height, 
						  int sizeFlags = wxSIZE_AUTO) = 0;

	// ��С��
	virtual void DoMinimize() = 0;

private:

	// ����ı䴰�ڴ�Сʱ�����ָ��仯
	void HandleRzCursor(const wxPoint& Point);

public:

	/// \brief ����һ������Ļ���Ͻ�Ϊ�������꣬�����ڷ������õ㴦��
	void EmulateDragAndResize(const wxPoint& Point);

	/// \brief ģ���϶�����
	void EmulateDragAndMove(const wxPoint& Point);

	/// \brief �����ҷ�ı䴰�ڴ�Сʱָ���ڴ��ڱ߽��ϵ�λ��
	typedef int MousePtrPos;

	/// \brief ��ȡ��ǰ����϶����촰�ڵ�����
	MousePtrPos GetDragAndResizeType() const
	{
		return m_dragAndResizeType;
	}

	/// \brief ����(ȡ��)�ı䴰�ڴ�Сʱ������״̬���ָ��
	void ResetRzCursor();

	/// \brief �Ƿ�����ͨ���϶����ڿհ״�ʵ���϶���������
	bool CanDragBySpace() const { return m_dragBySpace; }

protected:

	VdkScalableBitmap m_bkCanvas; ///< ���ڵı���λͼ

private:

	bool m_dragBySpace; // �Ƿ�����ͨ���϶����ڿհ״�ʵ���϶���������

	// �������ȥ�����ı䴰�ڴ�С
	MousePtrPos m_dragAndResizeType;

	// �ϴη��� Resize �¼���ʱ�����Resize �¼�ÿ�� 40ms ����һ��
	wxMilliClock_t m_nLastTimeStamp;

public:

	/////////////////////////////////////////////////////////////////////////////////
	// VdkControl ���

	/// \brief ���ô��ڣ���������Ƥ��
	void Reset();

	/// \brief ��\a pCtrl ��ӵ��ؼ��б���
	///
	/// \attention Ӧ���ɴ����ߵ��ã������Ǳ������Ŀؼ�����
	void AddCtrl(VdkControl* pCtrl);

	/// \brief �ӿؼ��б����Ƴ�ָ���ؼ�
	///
	/// �����������Ƴ�\a pCtrl ���ӿؼ�����ʵ������\a pCtrl �ĸ��ӿؼ�
	/// һ����֪��
	/// \return �Ƿ�ɹ��Ƴ��˸ÿؼ���
	bool RemoveCtrl(VdkControl* pCtrl);

	/// \brief ������Ϊ\a strName �� VdkControl
	VdkControl* FindCtrl(const wxString& strName);

	/// \brief ������Ϊ\a strName �� VdkControl
	template< class T >
	T* FindCtrl(const wxString& strName) {
		return static_cast< T* >( FindCtrl( strName ) );
	}

	/// \brief ������Ϊ\a strName �� VdkControl
	template< class T >
	T* operator [] (const wxString& strName) {
		return static_cast< T* >( FindCtrl( strName ) );
	}

	/// \brief ȷ�Ͼ��Ϊ\a pCtrl �� VdkControl �Ƿ���λ�ڿؼ��б�֮��
	bool FindCtrl(VdkControl* pCtrl);

	typedef VdkCtrlList::iterator CtrlIter;

	/// \brief ��ȡ���пؼ��ĵ��������
	CtrlIter begin() { return m_Ctrls.begin(); }

	/// \brief ��ȡ���пؼ��ĵ��������
	CtrlIter end() { return m_Ctrls.end(); }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ֪ͨ\a pCtrl ��ǰ���ڴ�С�����Сֵ�ĸı���
	void LayoutCtrl(VdkControl* pCtrl);

	/// \brief ���ÿؼ��Ŀɼ���
	void ShowCtrl(const wxString& strName, bool bShow, wxDC* pDC);

	/// \brief ����ָ���ؼ���ģ������������ϣ�
	void HilightCtrl(VdkControl* pCtrl, wxDC& dc);

	/// \brief ���þ������뽹��Ŀؼ�
	/// \param pCtrl Ҫ����Ŀؼ�������Ϊ��
	void FocusCtrl(VdkControl* pCtrl, wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���û���˫�����ͼ������
	void SetCachedDC(VdkDC& vdc);

	/// \brief �����ѻ����˫�����ͼ�����ĵ�ĳЩ��ʼ����
	virtual void PrepareCachedDC();

	/// \brief �����ѻ����˫�����ͼ������
	/// \param pdc �ǻ����ͼ������
	void ResetCachedDC(wxDC& pdc);

	/// \brief ��ȡ�ѻ����˫�����ͼ������
	VdkDC* GetCachedDC() const { return m_cachedDC; }

	/// \brief ��ӻ�ͼ�ύ������
	void AddPostDrawListener(VdkDcPostListener* lsner);

	/// \brief ɾ����ͼ�ύ������
	void RemovePostDrawListener(VdkDcPostListener* lsner);

	/// \brief �õ�˫�����ͼ����λͼ
	///
	/// ���Լ���ÿ��ʹ��˫�����ͼʱ������ʱλͼ�Ŀ�����
	wxBitmap& GetBufferedBitmap() { return m_bmBuffered; }

	/// \brief �Ƿ�����˫����
	bool IsDoubleBuffering() const;

	/// \brief ����\a rc ָ���ľ��Σ��ָ�����
	/// \param rc �������Ϊ���ؼ������Ͻ�
	virtual void EraseBackground(wxDC& dc, const wxRect& rc);

private:

	// ��ʼ������״̬
	void Init();

	// ���þ������뽹��Ŀؼ�
	// @param pCtrl Ҫ����Ŀؼ�������Ϊ��
	// @param x ���ָ�뵱ǰ����봰�ڿͻ����� X ����
	// @param y ���ָ�뵱ǰ����봰�ڿͻ����� Y ����
	void FocusCtrl(VdkControl* pCtrl, wxDC* pDC, int x, int y);

private:

	VdkCtrlList m_Ctrls;
	
	// ����ʹ�� TAB �ڴ����������е���
	TabOrderIterator* m_tabOrderIter;

	wxBitmap m_bmBuffered;	// ˫����λͼ
	VdkDC* m_cachedDC; // �ѻ����˫�����ͼ������
	wxVector< VdkDcPostListener* >* m_postListeners;

	// ��ʾ��ǰ���ڵ��豸�������豸��ʼ����
	int m_xDcOrigin, m_yDcOrigin;

public:

	//////////////////////////////////////////////////////////////////////////
	// ׾�ӵ���ƣ���������ר��

	/// \brief ��ȡ��ǰ���ڵ��豸�������豸��ʼ����
	void GetDcOrigin(int* x, int* y) const;

	/// \brief ��ȡ��ǰ���ڵ��豸�������豸��ʼ����
	void SetDcOrigin(int x, int y);

	/// \brief ���õ�ǰ���ڵ��豸�������豸��ʼ����
	void ResetDcOrigin(wxDC& dc) const;

private:

	DECLARE_NO_COPY_CLASS( VdkWindow ) // ���ɸ���
};

//////////////////////////////////////////////////////////////////////////

/// \brief VdkWindow �ػ��¼��Ļص�����
/// \attention VdkWindow �������಻Ӧ�ü̳��Ա��ӿڣ�
/// �������� this ָ���δ��󣬽�����÷����� VdkControl ʵ�֡�
/// \note ������Ҫʵ�����ƹ��ܿɸ�д\link VdkWindow::DoPaint \endlink ��
class RedrawCallback
{
public:

	/// \brief ִ�л�ͼ����
	virtual void DoRedraw(wxDC& dc) const = 0;
};
