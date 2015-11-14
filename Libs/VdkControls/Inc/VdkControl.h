#pragma once
#include "VdkDefs.h"
#include "VdkObject.h"
#include "VdkEvent.h"
#include "VdkWindow.h"
#include "VdkCtrlId.h"
#include "VdkUtil.h"

//////////////////////////////////////////////////////////////////////////

class VdkMenu;

/// \brief �����ؼ����Ի��ķ���ֵ
///
/// ָʾ��������Ļ��Ʋ�����
enum VdkCusdrawReturnFlag {

	VCCDRF_DODEFAULT,       ///< ��Ԥ���������н���
	VCCDRF_SKIPDEFAULT,     ///< �ж�Ԥ���������У���ɻ���
};

/// \brief VdkControl �ķ�����Լ�
enum VdkCtrlStyle {

	/// \brief ��������λ�������ʼλ���
	VCS_USER				= 15,
	/// \brief ���������¼����������¼���
	VCS_IGNORE_ALL_EVENTS	= 1 << 0,
	/// \brief ���¼����򸸿ؼ�
	VCS_REDIRECT_TO_PARENT	= 1 << 1,
	/// \brief ����ʱ�Ƿ���Ҫ�ػ��������ڸ����ڵ� AddVdkControl �е��ã�
	VCS_ERASE_BG			= 1 << 2,
	/// \brief �Ƿ������ؼ�����ʱֻ�� VdkSlider �� VdkScrollBar ��
	/// \attention ע���� VdkCtrlHandler ������VdkCtrlHandler �������¼���
	/// �� Container �����¼��������ȼ�����ͨ�� VdkControl Ҫ�͡�
	VCS_CTRL_CONTAINER		= 1 << 3,
	/// \brief VdkControl�Ƿ�ɱ���סʱ���Ϸ�����ͬ���¼�
	VCS_HONLD_ON			= 1 << 4,
	/// \brief ֻ����һ�θ����¼����������������ڳ����ζ�ָ������ĳ����¼�
	VCS_ONESHOT_HOVERING	= 1 << 5,
	/// \brief �����Ƿ�Ϊ VdkCtrlHandler
	VCS_HANDLER				= 1 << 6,
	/// \brief ���ָ��λ�ڿؼ���������ʱ�Ƿ�ָ��ָ��Ϊ������״
	VCS_HAND_CURSOR			= 1 << 7,
	/// \brief ���������¼�
	VCS_KEY_EVENT			= 1 << 8,
	/// \brief Use this to indicate that the window wants to get all char/key 
	/// events for all keys - even for keys like TAB or ENTER which are usually 
	/// used for dialog navigation and which wouldn't be generated without this 
	/// style. If you need to use this style in order to get the arrows or etc., 
	/// but would still like to have normal keyboard navigation take place, 
	/// you should call Navigate in response to the key events for Tab and 
	/// Shift-Tab. 
	VCS_WANTS_ALL_CHARS		= 1 << 9,
	/// \brief �����ڴ����ڲ�ʹ�� TAB ���н���ת��
	VCS_TAB_TRAVERSAL		= 1 << 10,
	/// \brief ���������İ����¼�����ȡʵ�ʱ�ʾ�Ĵ��룩
	VCS_CHAR_EVENT			= 1 << 11,
	/// \brief �ޱ߿�
	VCS_BORDER_NONE			= 1 << 12,
	/// \brief �򵥵�ƽ��߿�
	VCS_BORDER_SIMPLE		= 1 << 13,
	/// \brief ��ʱ���� VdkVObjEvent ����ע����¼�������
	///
	/// һ�����ڲ�ϣ�� VdkVObjEvent �����ؼ������ڵĻ�ͼ�����ģ�������Ҫ��
	/// �¼��������д�һ��ģ̬�Ի�������������
	VCS_DELAY_EVENT			= 1 << 14,
};

/// \brief VdkControl ������ʱ״̬���Լ�
enum VdkCtrlState {

	/// \brief ��������λ�������ʼλ���
	VCST_USER				= 10,

	/// \brief �ؼ��Ƿ�����ִ�п�¡����
	///
	/// �����ǣ��򽫿ؼ����뵽 VdkWindow �ؼ��б��в���֪��
	/// VdkWindow ��ǰ��С���ʼ��С֮��Ĳ�ֵ��
	VCST_CLONING			= 1 << 0,
};

//////////////////////////////////////////////////////////////////////////

/// \brief Ϊ VdkControl ���ó�ʼ����Ϣ
template< typename T >
class VdkCtrlInitializer
{
public:

	typedef VdkCtrlInitializer Control;
	typedef long align_type;

	/// \brief ���캯��
	VdkCtrlInitializer() : 
		Id( wxID_ANY ), Win( NULL ), Parent( NULL ), 
		AddToWindow( false ), Align( 0 ), Style( 0 ), 
		Show( true ), Thaw( true )
	{

	}

	/// \brief ���ÿؼ� ID
	T& id(int i) { Id = i; return static_cast< T& >( *this ); }

	/// \brief ���������� VdkWindow
	T& window(VdkWindow* w) { Win = w; return static_cast< T& >( *this ); }

	/// \brief ���ø��ؼ�
	T& parent(VdkControl* p) { Parent = p; return static_cast< T& >( *this ); }

	/// \brief �ؼ���ʼ�����Ƿ�ֱ����ӵ������� VdkWindow �Ŀؼ��б���
	T& addToWindow(bool b) { AddToWindow = b; return static_cast< T& >( *this ); }

	/// \brief ���ÿؼ�������
	T& name(const wxString& s) { Name = s; return static_cast< T& >( *this ); }

	/// \brief ���ÿؼ��������򡣵� VdkWindow Ϊ�Ӵ���ʱ��
	/// Rect.x �� Rect.y �����������Ͻǵ�����Ϊ������㣬\n
	/// ���������(Rect.x, Rect.y)����Զ��Ե�
	T& rect(const wxRect& rc) { Rect = rc; return static_cast< T& >( *this ); }

	/// \brief ���ÿؼ���������
	T& rect(int x, int y, int w, int h) { 
		return rect( wxRect( x, y, w, h ) );
	}

	/// \brief ���ÿؼ����õ�����
	T& font(const wxFont& f) { Font = f; return static_cast< T& >( *this ); }

	/// \brief ���ÿؼ�����ڸ��ؼ��Ķ�������
	T& align(align_type a) { Align = a; return static_cast< T& >( *this ); }

	/// \brief ���ÿؼ���<b>**����**</b>���
	T& style(long s) { Style = s; return static_cast< T& >( *this ); }

	/// \brief ���ÿؼ���ʼ�����Ƿ�������ʾ
	T& show(bool b) { Show = b; return static_cast< T& >( *this ); }

	/// \brief ���ÿؼ���ʼ�����Ƿ�������ʾ���������ᣩ
	T& thaw(bool b) { Thaw = b; return static_cast< T& >( *this ); }

	/// \brief ǿ��ת��Ϊ������
	template< class DD >
	DD& castTo() { return static_cast< DD& >( *this ); }

protected:

	int Id;
	VdkWindow* Win;
	VdkControl* Parent;
	bool AddToWindow;
	wxString Name;
	wxRect Rect;
	wxFont Font;
	align_type Align;
	long Style;
	bool Show;
	bool Thaw;

	friend class VdkControl;
};

//////////////////////////////////////////////////////////////////////////

class VdkCtrlParserCallback;
class VdkCtrlParserInfo;

/*!\brief ���� virtual-controls �Ļ���
 *
 * ���� VDK �ĸ���֮һ������һ������ VdkWindow ��
**/
class VdkControl : public VdkObject,
				   public VdkUtil,
				   public VdkClientDataOwner,
				   public VdkStyleAndStateOwner
{
public:

	/// \brief ��ʼ�� VdkControl �Ĵ�������
	/// \param maskColor λͼ������ɫ
	/// \param strRootPath Ƥ���ĸ�Ŀ¼·����
	/// ���� VdkControl ���캯����ָ�����ļ������������� \a strRootPath ���Ե�
	static void Initialize( const wxColour& maskColor, const wxString& strRootPath );

	/// \brief �� XML �ļ����� VdkControl ��ϵ
	/// \return ���ض�̬���ɵĵ�һ�� VdkControl �����
	static VdkControl* ParseObjects( const VdkCtrlParserInfo& info );

	/// \brief ��ȡ��ǰ���̿ռ�Ŀؼ�Ƥ��λͼ��͸������ɫ
	static const wxColour& GetMaskColour() { return ms_maskColor; }

	/// \brief ���õ�ǰ���̿ռ�Ŀؼ�Ƥ��λͼ��͸������ɫ
	static void SetMaskColour(const wxColour& color) { ms_maskColor = color; }

	/// \brief ��ȡ��ǰ���̿ռ�Ŀؼ�Ƥ���ļ���Ŀ¼
	static const wxString& GetRootPath() { return ms_rootPath; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��¡ǰ�����ṩ��ԭ���ؼ�����Ƿ�������ȷ
	///
	/// ���ڵ��԰汾��ִ�в��ԡ�
	template< class DT >
	static bool CheckSourceCtrl(VdkControl* pCtrl) {
		return dynamic_cast< DT* >( pCtrl ) != NULL;
	}

	/// \brief ��\a o ��¡
	/// \param o ���Կ�¡��ʵ��
	/// \param parent ָ�����¸��ؼ�
	virtual void Clone(VdkControl* o, VdkControl* parent);

	/// \brief ��\a o ��¡
	///
	/// �� Clone �Ĳ�ͬ��Ҫ���Զ�ָ����\a o ��ͬ�ĸ��ؼ���
	/// �Լ���¡�ɹ����Զ����Լ����뵽�����ڵĿؼ���
	/// \param o ���Կ�¡��ʵ��
	void EasyClone(VdkControl* o)
	{
		Clone( o, o->GetParent() );
		m_Window->AddCtrl( this );
	}

	/// \brief ��������
	virtual ~VdkControl();

public:

	/// \brief ��Ӧ���� VdkWindow �ĸı��С�¼�
	void HandleResize(int dX, int dY);

	/// \brief ����õ�������¼�
	///
	/// �����ؼ������Ҫ���ص��麯����\link DoHandleMouseEvent \endlink ��
	void HandleMouseEvent(VdkMouseEvent& e);

	/// \brief ����õ��ļ����¼�
	///
	/// �����ؼ������Ҫ���ص��麯����\link DoHandleKeyEvent \endlink ��
	void HandleKeyEvent(VdkKeyEvent& e);

	/// \brief ���ա�����֪ͨ��Ϣ
	///
	/// �����ؼ������Ҫ���ص��麯����\link DoHandleNotify \endlink ��
	void HandleNotify(const VdkNotify& notice);

	/// \brief ���ָ���Ƿ�λ�ڿؼ���������
	bool ContainsPointerGlobally() const;

	/// \brief �ж����ָ���λ��\a mousePos �Ƿ�λ���Լ�����������
	virtual bool HitTest(const wxPoint& mousePos) const;

	/// \brief ��ָ����\a rc ת������ VdkWindow Ϊ������������
	///
	/// ������Ӧ�����ӿؼ����󸸿ؼ���\a rc �ļ�������Ǹ��ؼ����Ͻǡ�
	/// \note ��Ҫ����Ƕ�׿ؼ���ͬʱ VdkScrolledWindow Ҳ�������ǳ��\n
	/// \link GetAbsoluteRect() \endlink �Ա����������˼򵥵ķ�װ��
	/// \see GetAbsoluteRect
	wxRect TranslateRect(const wxRect& rc) const;

	/// \brief ��ָ����\a rc ת�����Ը��ؼ����Ͻ�Ϊ������������
	///
	/// \a rc �ļ�������� VdkWindow ���Ͻǡ�
	/// ���ڵ�ǰ�ӿؼ����󸸿ؼ���һ��ָ����\a rc ת�������Լ�ͬ�������ꡣ
	wxRect DeTranslateRect(const wxRect& rc) const;

	/// \brief ���Ը��ؼ�����Ϊ�����ĵ�\a p ת������ VdkWindow ���Ͻ�Ϊ�������ĵ�
	/// \see DeTranslatePoint
	wxPoint TranslatePoint(const wxPoint& p) const;

	/// \brief ���� VdkWindow ���Ͻ�Ϊ�������ĵ�\a p ת�����Ը��ؼ�����Ϊ�����ĵ�
	/// \see TranslatePoint
	wxPoint DeTranslatePoint(const wxPoint& p) const;

	/// \brief �����ӿؼ��Ļ�ͼ DC
	///
	/// һ���ɸ����ڣ� VdkWindow ���� VdkPanel ���ã������Լ̳и����ĸ��ֻ�ͼ���ԣ�
	/// �������塢������ɫ������ת���ȡ�
	/// \return �����޸���\a dc �򷵻� true �� ���� false ��
	void TranslateDC(wxDC& dc) const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief �ƶ��ؼ����µ�λ��
	void Move(int x, int y, wxDC* pDC = NULL) {
		SetRect( x, y, m_Rect.width, m_Rect.height, pDC );
	}

	/// \brief ���غ��������ṩ����
	void Move(const wxPoint& pos, wxDC* pDC = NULL) {
		Move( pos.x, pos.y, pDC );
	}

	/// \brief Ϊ�ؼ������µĴ�С
	void SetSize(int w, int h, wxDC* pDC = NULL) {
		SetRect( m_Rect.x, m_Rect.y, w, h, pDC );
	}

	/// \brief ���غ��������ṩ����
	void SetSize(const wxSize& size, wxDC* pDC = NULL) {
		SetSize( size.x, size.y, pDC );
	}

    /// \brief ���ÿؼ���������
    ///
    /// ��������������丸�ؼ����Եġ�����û�и��ؼ��� VdkControl ��
	/// ��������� VdkWindow ��
	/// \param ����һ����Ч��\a pDC ��������������ػ��ؼ���
	void SetRect(int x, int y, int w, int h, wxDC* pDC = NULL);

	/// \brief ���غ��������ṩ����
	void SetRect(const wxRect& rc, wxDC* pDC = NULL) {
		SetRect( rc.x, rc.y, rc.width, rc.height, pDC );
	}

    /// \brief ��ȡ�ؼ�������������ؼ���������
    /// \see GetAbsoluteRect
	virtual wxRect GetRect() const { return m_Rect; }

	/// \brief �õ��ؼ�����ڸ��ؼ���λ��
	void GetPosition(int* x, int* y) const;

	/// \brief �õ��ؼ�����ڸ��ؼ���λ��
	wxPoint GetPosition() const { return wxPoint( m_Rect.x, m_Rect.y ); }

	/// \brief �õ��ؼ��Ŀ��/�߶�
	void GetSize(int* w, int* h) const;

	/// \brief ��ȡ�ؼ���������� VdkWindow ��������
	/// \see GetRect
	wxRect GetAbsoluteRect() const { return TranslateRect( m_Rect ); }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���ñ��������л���
	///
	/// �����������Լ������������ӿؼ���\n
	/// �����ؼ������Ҫ���ص��麯����\link DoDraw \endlink ��
	/// \see DoDraw
	void Draw(wxDC& dc);

	/// \brief ����\a rc ָ���ľ���
	/// \param rc �������Ϊ���ؼ������Ͻǣ��뵱ǰ������ͬ���
	void EraseBackground(wxDC& dc, const wxRect& rc);

	/// \brief ���ÿؼ������ı�ʱ��������
	void SetFont(const wxFont& font, wxDC* pDC);

	/// \brief ��ȡ�ؼ������ı�ʱ��������
	const wxFont& GetFont() const { return m_Font; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡ�ؼ�����ڸ��ؼ�/���� VdkWindow �Ķ�������
	///
	/// ��û�и��ؼ�ʱ��������������������� VdkWindow ���Եġ�
	align_type GetAlign() const { return m_align; }

	/// \brief ���ÿؼ�����ڸ��ؼ�/���� VdkWindow �Ķ�������
	///
	/// ��û�и��ؼ�ʱ��������������������� VdkWindow ���Եġ�
	void SetAlign(const align_type& align) { m_align = align; }

	/// \brief ����ؼ���Ը��ؼ��Ķ�������
	///
	/// ������Ը��ؼ�/�����ھ��С��Ҷ���ȡ�
	void HandleRelativeAlign(align_type align);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���ÿؼ� ID
	void SetID(VdkCtrlId id);

	/// \brief ��ȡ�ؼ� ID
	VdkCtrlId GetID() const { return m_id; }

	/// \brief ���ÿؼ�������
	void SetName(const wxString& strName) { m_strName = strName; }

	/// \brief ��ȡ�ؼ�������
	wxString GetName() const { return m_strName; }

	/// \brief �õ������ڵľ��
	VdkWindow* GetVdkWindow() const { return m_Window; }

	/// \brief ���ø����ڵľ��
	/// \attentiion ֻӦ���� VDK �ڲ�����
	void SetVdkWindow(VdkWindow* win);

	/// \brief �������ø�����
	bool ReAttachToWindow(VdkWindow* win);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���ø��ؼ�
	/// \param parent ����Ϊ NULL
	void SetParent(VdkControl* parent);

	/// \brief ��ȡ���ؼ�
	VdkControl* GetParent() const { return m_parent; }

	/// \brief ��ȡ�����ؼ�
	VdkControl* GetRootParent() const;

	/// \brief ��ȡ�ӿؼ������ǰһ�����
	VdkControl* GetPrev() const { return m_prev; }

	/// \brief �����ӿؼ������ǰһ�����
	void SetPrev(VdkControl* prev) { m_prev = prev; }

	/// \brief ��ȡ�ӿؼ�����ĺ�һ�����
	VdkControl* GetNext() const { return m_next; }

	/// \brief �����ӿؼ�����ĺ�һ�����
	void SetNext(VdkControl* next) { m_next = next; }

	/// \brief ��ȡ�ӿؼ�����ĵ�һ�����
	VdkControl* GetFirstChild() const { return m_firstChild; }

	/// \brief ���ӿؼ��������Ƴ�ָ���ӿؼ�
	///
	/// ɾ��һ���ؼ�ʱӦ���򸸿ؼ����á�����ֱ�� delete ��
	/// ����������ӹ�\a pCtrl ��
	void RemoveChild(VdkControl* pCtrl);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ����������� ToolTip
	void ResetToolTip();

	/// \brief ����������� ToolTip
	/// \param updateOnce �Ƿ��������������ڵ� Tooltip ��
	/// �����ڿؼ�������������ʾ��
	void SetToolTip(const wxString& strToolTip, bool updateOnce = false);

	/// \brief ��ȡ������� ToolTip
	wxString GetToolTip() const { return m_strToolTip; }

	/////////////////////////////////////////////////////////////////

	/// \brief ��һ�� VdkMenu �������ؼ���
	///
	/// ���ؼ������������ϵ� VdkMenu ʱ�����Ҽ�������Ӧλ�õ�������
	void AttachMenu(VdkMenu* menu);

	/// \brief �õ���ؼ�������� VdkMenu
	VdkMenu* GetMenu() const { return m_menu; }

	/// \brief ���õ�ǰ VdkMenu �Ŀɼ���
	///
	/// �ⲿ���ơ��˵�������ʧ������ʾ�ˣ����֪ͨ�ؼ������ڲ�״̬��
	void SetMenuState(bool bMenuOnShow, wxDC* pDC = NULL);

	/// \brief ������� VdkMenu �Ƿ�����ʾ
	bool IsMenuOnShow() const;

	/////////////////////////////////////////////////////////////////

	/// \brief ��ȡ�ؼ��Ŀɼ���
	///
	/// ע����\link IsShown \endlink �Ĳ�ͬ����������ȫ���û����ƿؼ�״̬��
	/// ��\link IsShown \endlink ���ܻ��� VDK �ı�ؼ��Ŀɼ�״̬��
	/// \see IsShown
	bool IsOnShow() const;

	/// \brief ��ȡ�ؼ��Ŀɼ���
	/// \see IsOnShow IsFreezed IsShownOnScreen
	bool IsShown() const;

	/// \brief �ؼ��Ƿ���ʾ����Ļ��
	/// \see IsOnShow IsShown IsFreezed
	bool IsShownOnScreen() const;

	/// \brief ���ÿؼ��Ŀɼ���
	/// \see IsShown IsOnShow IsShownOnScreen
	void Show(bool bShow, wxDC* pDC);

	/// \brief ���ؿؼ�
	/// \param pDC ��������
	void Hide(wxDC* pDC) { Show( false, pDC ); }

	/// \brief ����һ�� VdkControl ����ʾ״̬��ʹ֮��Ӵﵽ\n
	/// \link Show() \endlink ��Ч��
	///
	/// ��������������������������������վ��飬����Ҫ����һ����ť��\n
	/// ���ǻ�ֱ��Ӧ��\link Show() \endlink ��������\link Show() \endlink \n
	/// ��һ���ܴ�ı׶ˣ������޷�������˭��ʾ/�����Լ���ָ VdkControl ������\n
	/// ����һ����ť��λ��һ�� VdkPanel �ϣ�����Ա��ĳЩ����������������\n
	/// �����û��л�����һ�� VdkPanel ����ô VdkPanel Ҳ��������������û���⡣\n
	/// ��ô�����û��л������ VdkPanel ʱ���������ˡ�\n
	/// VdkPanel �������ŵ���Ϊ������صİ�ť�����Լ����صģ����ǵ���\n
	/// \link Show() \endlink ��ʾ��������ʵ�������ť�ǳ���Ա�Լ����صģ�\n
	/// ���������ťӦ���ɳ���Ա�Լ�����Ҫ��Ҫ\link Show() \endlink ����\n
	/// ���ǣ�\link Freeze() \endlink ��\link Thaw() \endlink\n
	/// ����Ϊ�˽�����־����������ġ�\n
	/// VDK �ڲ���δ֪�� VdkControl ��Ӧ�õ���\link Show() \endlink ,\n
	/// ������Ҫ����ʱ����\link Freeze() \endlink��������ʾʱ����\n
	/// \link Thaw() \endlink��
	/// \see Thaw
	/// \see Show
	void Freeze();

	/// \brief �� VdkControl �Ӷ����״̬�н�ų���
	void Thaw(wxDC* pDC);

	/// \brief ��ǰ�Ƿ񱻶���
	bool IsFreezed() const { return m_freezed; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���¿ؼ���ǰ�����뽹��״̬
	/// \attention ���õ�ǰ�������뽹��Ŀؼ�Ӧ�õ���
	/// \link VdkWindow::FocusCtrl \endlink ��
	void UpdateFocus(bool focus, wxDC* pDC = NULL);

	/// \brief ��ȡ�ؼ��Ƿ�������뽹��
	bool IsFocused() const { return m_focused; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief �Ƿ�����
	bool IsEnabled() const { return m_enabled; }

	/// \brief ����/���� VdkControl
	void Enable(bool bEnabled, wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief �Ƿ��������¼�
	bool CanHandleEvent() const {
		return IsShown() && IsEnabled();
	}

	/// \brief �Ƿ�Ϊ VdkControlHandler
	bool IsCtrlHandler() const { return TestStyle( VCS_HANDLER ); }

	/// \brief �Ƿ��ʺϼ����ص�����
	bool IsReadyForEvent() const { return m_id != wxID_ANY; }

	/// \brief �����ؼ��Ļص�����
	void FireEvent(wxDC* pDC, void* extraData);

	//////////////////////////////////////////////////////////////////////////
	// �ӿؼ�������

	class ChildIterator;

	/// \brief ��ȡ��һ���ӿؼ��ĵ�����
	ChildIterator begin()
	{
		ChildIterator i( this );
		return i;
	}

	/// \brief ��ȡ���һ���ӿؼ�֮��ĵ�����
	ChildIterator end()
	{
		ChildIterator i( NULL );
		return i;
	}

protected:

	/// \brief XRC ��̬����
	///
	/// һ�� VdkControl �����������Ҫʵ�ֶ�̬��������Ӧ��ʵ���������
	virtual void Create(wxXmlNode* node) {}

	/// \brief Ĭ�Ϲ��캯��
	/// \attention ����һ�� VdkControl ǰ����ȷ������ɫ�Ѿ�����λ��
	/// \see Initialize
	VdkControl();

	/// \brief XRC ��̬����
	void DoXrcCreate(wxXmlNode* node);

	/// \brief XRC ��̬�����Ĺ�ͬ�߼�����
	template< class D >
	void Create(const VdkCtrlInitializer<D>& init_data);

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief ������̰����¼�
	virtual void DoHandleKeyEvent(VdkKeyEvent& e) {}

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice) {}

	/// \brief SetRect ʱ�����ӿؼ���λ������
	void HandleChildrenResize(int dX, int dY);

private:

	/// \brief ����\a rc ָ���ľ���
	/// \param rc �������Ϊ���ؼ������Ͻ�
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc) {}

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc) {}

	/// \brief �������ø�����
	bool DoReAttachToWindow() { return true; }

	/// \brief ���һ������
	/// \attention �������ã�Ӧ��ʹ��\link SetParent() \endlink��
	void AddChild(VdkControl* chd);

	/// \brief ���Ϳؼ���С�ı��֪ͨ��Ϣ
	/// \param fromSetRect ��Ϣ�Ƿ�����\link SetRect \endlink ��
	void FireSizeNotify(int dX, int dY, bool fromSetRect = false, wxDC* pDC = NULL);

	/// \brief ���Ϳؼ�λ�øı��֪ͨ��Ϣ
	void FireMoveNotify(int xPosChanged, int yPosChanged);

protected:

    /// \brief ��ǰ���̿ռ�Ŀؼ�Ƥ��λͼ��͸������ɫ
	static wxColour			ms_maskColor;

	/// \brief ���� VdkControl ���е�Ƥ���ļ���Ŀ¼
	static wxString			ms_rootPath;

	/// \brief ������ʾ ToolTip �� VdkControl
	static VdkControl*		ms_pToolTipOnShow;

	/////////////////////////////////////////////////////////////////

	VdkCtrlId				m_id; ///< �ؼ� ID ���ص�����ʱ����ʶ��

	/// \brief ����
	VdkControl*				m_parent;
	/// \brief ��һ�����ӣ��൱������ͷ
	VdkControl*				m_firstChild;
	/// \brief ���
	VdkControl*				m_prev;
	/// \brief �ܵ�
	VdkControl*				m_next;

	/// \brief �ؼ�����
	wxString				m_strName;

	/// \biref ����� VdkWindow �Ķ���
	align_type				m_align;

	/// \brief ToolTip����ʾ�ı�
	wxString				m_strToolTip;

	/// \brief �ؼ�ռ�ݵ�����
	wxRect					m_Rect;

	/// \brief �����ؼ��Ķ��㴰��
	VdkWindow*				m_Window;

	/// \brief �����ھ��
	wxWindow*				m_WindowImpl;

	/// \brief ��˿ؼ�������Ĳ˵�
	VdkMenu*				m_menu;

	/// \brief �ؼ�Ҫʹ�õ�����
	wxFont					m_Font;

private:

	// ��ǰ�Ƿ��Ƿ���ʾ
	bool					m_shown;
	// ��ǰ�Ƿ����
	bool					m_enabled;
	// �Ƿ���м������뽹��
	bool					m_focused;
	// �Ƿ񱻴��ڱ������״̬
	bool					m_freezed;

	DECLARE_NO_COPY_CLASS( VdkControl )   // VdkControl ��������

public:

	/// \brief ��ȡһ���µĿؼ�
	/// \note ���ڿؼ���¡��
	virtual VdkControl* GetImitation() const {
		return new VdkControl;
	}

public:

    /// \brief �ӿؼ�������
	class ChildIterator
	{
	public:

		/// \brief Ĭ�Ϲ��캯��
		ChildIterator() { Init(); }
		/// \brief ���캯��
		ChildIterator(VdkControl* pCtrl);

		/// \brief ǰ����
		ChildIterator& operator ++(); // prefix ++

		/// \brief ������
		ChildIterator operator ++(int); // postfix ++

		/// \brief ǰ�Լ�
		ChildIterator& operator --(); // prefix ++

		/// \brief ���Լ�
		ChildIterator operator --(int); // postfix ++

		/// \brief ȡֵ
		VdkControl& operator* ();

		/// \brief ȡַ
		VdkControl* operator-> ();

		/// \brief ��ȷ��ʽ��ȡַ����
		VdkControl* get() const { return m_curr; }

	private:

		// ��ʼ��������
		void Init();

	private:

		VdkControl *m_depot, *m_curr;

		friend bool operator == (const ChildIterator& lhs, const ChildIterator& rhs);
		friend bool operator != (const ChildIterator& lhs, const ChildIterator& rhs);
	};
};

template< class D >
void VdkControl::Create(const VdkCtrlInitializer< D >& init_data)
{
	wxASSERT( m_Window || init_data.Win );
	
	//-----------------------------------------------

	m_id = init_data.Id;

	if( init_data.Win )
	{
		SetVdkWindow( init_data.Win );
	}

	if( init_data.Parent )
	{
		SetParent( init_data.Parent );
	}

	if( !init_data.Rect.IsEmpty() )
	{
		m_Rect = init_data.Rect;
	}

	if( !init_data.Name.empty() )
	{
		m_strName = init_data.Name;
	}

	HandleRelativeAlign( init_data.Align );
	SetAddinStyle( init_data.Style );

	if( init_data.AddToWindow )
	{
		m_Window->AddCtrl( this );
	}

	if( !init_data.Show || 
	  ( m_parent && !m_parent->IsOnShow() ) )
	{
		m_shown = false;
	}

	if( !init_data.Thaw || 
	  ( m_parent && m_parent->IsFreezed() ) )
	{
		m_freezed = true;
	}

	if( init_data.Font.IsOk() )
	{
		m_Font = init_data.Font;
	}
}

//////////////////////////////////////////////////////////////////////////

/// \brief �������\link VdkWindow::AddCtrl() \endlink ��Ļ��ؼ�
///
/// VdkControlHandler�� VdkControl ��ʲô�����أ�����������ǣ�\n
/// VdkControlHandler һ��ӵ���ӿؼ���������Գ�֮Ϊ���Ͽؼ�����\n
/// �����ֲ���Ҫ�����¼���������Ҫ�ػ���������
/// �����Ը�д\link VdkControl::DoDraw() \endlink ��
/// �����ܸ�д\link VdkControl::DoHandleEvents() \endlink ��
class VdkCtrlHandler : public VdkControl
{
public:

	/// \brief ���캯��
	VdkCtrlHandler();
};

//////////////////////////////////////////////////////////////////////////

/// \brief �ؼ�Ƥ��λͼ�ļ�������ʱ�������쳣
class VdkExpectionImageNotFound
{
public:

	/// \brief ���캯��
	explicit VdkExpectionImageNotFound(const wxString& strError)
		: m_strError( strError )
	{

	}

	/// \brief �쳣����ϸ��Ϣ
	wxString GetMessage() const { return m_strError; }

private:

	wxString			m_strError;
};

//////////////////////////////////////////////////////////////////////////

/// \brief \link VdkControl::ParseObjects \endlink �Ļص�����
class VdkCtrlParserCallback
{
public:

	/// \brief �ص�֪ͨ����
	/// \param win \a pCtrl �ĸ�����
	/// \param clName \a pCtrl ������
	/// \param pCtrl �ѱ����������Ŀؼ�����
	virtual void Notify(VdkWindow* win, 
						const wxString& clName, 
						VdkControl* pCtrl) = 0;
};
