#pragma once
#include "VdkDefs.h"
#include "VdkObject.h"
#include "VdkEvent.h"
#include "VdkWindow.h"
#include "VdkCtrlId.h"
#include "VdkUtil.h"

//////////////////////////////////////////////////////////////////////////

class VdkMenu;

/// \brief 派生控件类自画的返回值
///
/// 指示父类后续的绘制操作。
enum VdkCusdrawReturnFlag {

	VCCDRF_DODEFAULT,       ///< 按预定绘制序列进行
	VCCDRF_SKIPDEFAULT,     ///< 中断预定绘制序列，完成绘制
};

/// \brief VdkControl 的风格属性集
enum VdkCtrlStyle {

	/// \brief 派生类风格位定义的起始位序号
	VCS_USER				= 15,
	/// \brief 忽略所有事件（不处理事件）
	VCS_IGNORE_ALL_EVENTS	= 1 << 0,
	/// \brief 将事件导向父控件
	VCS_REDIRECT_TO_PARENT	= 1 << 1,
	/// \brief 绘制时是否需要重画背景（在父窗口的 AddVdkControl 中调用）
	VCS_ERASE_BG			= 1 << 2,
	/// \brief 是否容器控件（暂时只有 VdkSlider 和 VdkScrollBar ）
	/// \attention 注意与 VdkCtrlHandler 的区别！VdkCtrlHandler 不处理事件，
	/// 而 Container 处理事件，但优先级较普通的 VdkControl 要低。
	VCS_CTRL_CONTAINER		= 1 << 3,
	/// \brief VdkControl是否可被按住时不断发送相同的事件
	VCS_HONLD_ON			= 1 << 4,
	/// \brief 只发送一次高亮事件，忽略在作用域内持续晃动指针带来的持续事件
	VCS_ONESHOT_HOVERING	= 1 << 5,
	/// \brief 类型是否为 VdkCtrlHandler
	VCS_HANDLER				= 1 << 6,
	/// \brief 鼠标指针位于控件作用域内时是否将指针指定为超链接状
	VCS_HAND_CURSOR			= 1 << 7,
	/// \brief 请求处理按键事件
	VCS_KEY_EVENT			= 1 << 8,
	/// \brief Use this to indicate that the window wants to get all char/key 
	/// events for all keys - even for keys like TAB or ENTER which are usually 
	/// used for dialog navigation and which wouldn't be generated without this 
	/// style. If you need to use this style in order to get the arrows or etc., 
	/// but would still like to have normal keyboard navigation take place, 
	/// you should call Navigate in response to the key events for Tab and 
	/// Shift-Tab. 
	VCS_WANTS_ALL_CHARS		= 1 << 9,
	/// \brief 允许在窗口内部使用 TAB 进行焦点转移
	VCS_TAB_TRAVERSAL		= 1 << 10,
	/// \brief 请求处理翻译后的按键事件（获取实际表示的代码）
	VCS_CHAR_EVENT			= 1 << 11,
	/// \brief 无边框
	VCS_BORDER_NONE			= 1 << 12,
	/// \brief 简单的平面边框
	VCS_BORDER_SIMPLE		= 1 << 13,
	/// \brief 延时发送 VdkVObjEvent 到已注册的事件处理器
	///
	/// 一般用于不希望 VdkVObjEvent 附带控件父窗口的绘图上下文，或者需要在
	/// 事件处理器中打开一个模态对话框等特殊情况。
	VCS_DELAY_EVENT			= 1 << 14,
};

/// \brief VdkControl 的运行时状态属性集
enum VdkCtrlState {

	/// \brief 派生类风格位定义的起始位序号
	VCST_USER				= 10,

	/// \brief 控件是否正在执行克隆操作
	///
	/// 假如是，则将控件加入到 VdkWindow 控件列表中不需知晓
	/// VdkWindow 当前大小与初始大小之间的差值。
	VCST_CLONING			= 1 << 0,
};

//////////////////////////////////////////////////////////////////////////

/// \brief 为 VdkControl 设置初始化信息
template< typename T >
class VdkCtrlInitializer
{
public:

	typedef VdkCtrlInitializer Control;
	typedef long align_type;

	/// \brief 构造函数
	VdkCtrlInitializer() : 
		Id( wxID_ANY ), Win( NULL ), Parent( NULL ), 
		AddToWindow( false ), Align( 0 ), Style( 0 ), 
		Show( true ), Thaw( true )
	{

	}

	/// \brief 设置控件 ID
	T& id(int i) { Id = i; return static_cast< T& >( *this ); }

	/// \brief 设置所属的 VdkWindow
	T& window(VdkWindow* w) { Win = w; return static_cast< T& >( *this ); }

	/// \brief 设置父控件
	T& parent(VdkControl* p) { Parent = p; return static_cast< T& >( *this ); }

	/// \brief 控件初始化后是否直接添加到所属的 VdkWindow 的控件列表中
	T& addToWindow(bool b) { AddToWindow = b; return static_cast< T& >( *this ); }

	/// \brief 设置控件的名字
	T& name(const wxString& s) { Name = s; return static_cast< T& >( *this ); }

	/// \brief 设置控件的作用域。当 VdkWindow 为子窗口时，
	/// Rect.x 和 Rect.y 以主窗口左上角的坐标为计量起点，\n
	/// 即这个坐标(Rect.x, Rect.y)是相对而言的
	T& rect(const wxRect& rc) { Rect = rc; return static_cast< T& >( *this ); }

	/// \brief 设置控件的作用域
	T& rect(int x, int y, int w, int h) { 
		return rect( wxRect( x, y, w, h ) );
	}

	/// \brief 设置控件所用的字体
	T& font(const wxFont& f) { Font = f; return static_cast< T& >( *this ); }

	/// \brief 设置控件相对于父控件的对齐属性
	T& align(align_type a) { Align = a; return static_cast< T& >( *this ); }

	/// \brief 设置控件的<b>**额外**</b>风格
	T& style(long s) { Style = s; return static_cast< T& >( *this ); }

	/// \brief 设置控件初始化后是否立即显示
	T& show(bool b) { Show = b; return static_cast< T& >( *this ); }

	/// \brief 设置控件初始化后是否立即显示（不被冻结）
	T& thaw(bool b) { Thaw = b; return static_cast< T& >( *this ); }

	/// \brief 强制转型为派生类
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

/*!\brief 所有 virtual-controls 的基类
 *
 * 整个 VDK 的根基之一。（另一根基是 VdkWindow ）
**/
class VdkControl : public VdkObject,
				   public VdkUtil,
				   public VdkClientDataOwner,
				   public VdkStyleAndStateOwner
{
public:

	/// \brief 初始化 VdkControl 的创建环境
	/// \param maskColor 位图的掩码色
	/// \param strRootPath 皮肤的根目录路径，
	/// 所有 VdkControl 构造函数中指定的文件名都是相对这个 \a strRootPath 而言的
	static void Initialize( const wxColour& maskColor, const wxString& strRootPath );

	/// \brief 从 XML 文件创建 VdkControl 体系
	/// \return 返回动态生成的第一个 VdkControl 句柄。
	static VdkControl* ParseObjects( const VdkCtrlParserInfo& info );

	/// \brief 获取当前进程空间的控件皮肤位图的透明掩码色
	static const wxColour& GetMaskColour() { return ms_maskColor; }

	/// \brief 设置当前进程空间的控件皮肤位图的透明掩码色
	static void SetMaskColour(const wxColour& color) { ms_maskColor = color; }

	/// \brief 获取当前进程空间的控件皮肤文件根目录
	static const wxString& GetRootPath() { return ms_rootPath; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 克隆前测试提供的原本控件句柄是否类型正确
	///
	/// 仅在调试版本中执行测试。
	template< class DT >
	static bool CheckSourceCtrl(VdkControl* pCtrl) {
		return dynamic_cast< DT* >( pCtrl ) != NULL;
	}

	/// \brief 从\a o 克隆
	/// \param o 用以克隆的实体
	/// \param parent 指定的新父控件
	virtual void Clone(VdkControl* o, VdkControl* parent);

	/// \brief 从\a o 克隆
	///
	/// 与 Clone 的不同主要是自动指定与\a o 相同的父控件，
	/// 以及克隆成功后自动将自己加入到父窗口的控件池
	/// \param o 用以克隆的实体
	void EasyClone(VdkControl* o)
	{
		Clone( o, o->GetParent() );
		m_Window->AddCtrl( this );
	}

	/// \brief 析构函数
	virtual ~VdkControl();

public:

	/// \brief 响应所属 VdkWindow 的改变大小事件
	void HandleResize(int dX, int dY);

	/// \brief 处理得到的鼠标事件
	///
	/// 派生控件类具体要重载的虚函数是\link DoHandleMouseEvent \endlink 。
	void HandleMouseEvent(VdkMouseEvent& e);

	/// \brief 处理得到的键盘事件
	///
	/// 派生控件类具体要重载的虚函数是\link DoHandleKeyEvent \endlink 。
	void HandleKeyEvent(VdkKeyEvent& e);

	/// \brief 接收、处理通知信息
	///
	/// 派生控件类具体要重载的虚函数是\link DoHandleNotify \endlink 。
	void HandleNotify(const VdkNotify& notice);

	/// \brief 鼠标指针是否位于控件作用域内
	bool ContainsPointerGlobally() const;

	/// \brief 判断鼠标指针的位置\a mousePos 是否位于自己的作用域里
	virtual bool HitTest(const wxPoint& mousePos) const;

	/// \brief 将指定的\a rc 转换成以 VdkWindow 为计量起点的坐标
	///
	/// 本函数应该由子控件请求父控件。\a rc 的计量起点是父控件左上角。
	/// \note 主要用于嵌套控件，同时 VdkScrolledWindow 也会受益匪浅。\n
	/// \link GetAbsoluteRect() \endlink 对本函数进行了简单的封装。
	/// \see GetAbsoluteRect
	wxRect TranslateRect(const wxRect& rc) const;

	/// \brief 将指定的\a rc 转换成以父控件左上角为计量起点的坐标
	///
	/// \a rc 的计量起点是 VdkWindow 左上角。
	/// 用于当前子控件请求父控件将一个指定的\a rc 转换成与自己同级的坐标。
	wxRect DeTranslateRect(const wxRect& rc) const;

	/// \brief 将以父控件坐标为计量的点\a p 转换成以 VdkWindow 左上角为计量起点的点
	/// \see DeTranslatePoint
	wxPoint TranslatePoint(const wxPoint& p) const;

	/// \brief 将以 VdkWindow 左上角为计量起点的点\a p 转换成以父控件坐标为计量的点
	/// \see TranslatePoint
	wxPoint DeTranslatePoint(const wxPoint& p) const;

	/// \brief 适配子控件的绘图 DC
	///
	/// 一般由父窗口（ VdkWindow 或者 VdkPanel 调用）。可以继承父辈的各种绘图属性，
	/// 例如字体、字体颜色、坐标转换等。
	/// \return 假如修改了\a dc 则返回 true ， 否则 false 。
	void TranslateDC(wxDC& dc) const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief 移动控件到新的位置
	void Move(int x, int y, wxDC* pDC = NULL) {
		SetRect( x, y, m_Rect.width, m_Rect.height, pDC );
	}

	/// \brief 重载函数，仅提供便利
	void Move(const wxPoint& pos, wxDC* pDC = NULL) {
		Move( pos.x, pos.y, pDC );
	}

	/// \brief 为控件设置新的大小
	void SetSize(int w, int h, wxDC* pDC = NULL) {
		SetRect( m_Rect.x, m_Rect.y, w, h, pDC );
	}

	/// \brief 重载函数，仅提供便利
	void SetSize(const wxSize& size, wxDC* pDC = NULL) {
		SetSize( size.x, size.y, pDC );
	}

    /// \brief 设置控件的作用域
    ///
    /// 作用域是相对于其父控件而言的。对于没有父控件的 VdkControl ，
	/// 则是相对于 VdkWindow 。
	/// \param 传入一个有效的\a pDC 用以清除背景、重画控件。
	void SetRect(int x, int y, int w, int h, wxDC* pDC = NULL);

	/// \brief 重载函数，仅提供便利
	void SetRect(const wxRect& rc, wxDC* pDC = NULL) {
		SetRect( rc.x, rc.y, rc.width, rc.height, pDC );
	}

    /// \brief 获取控件相对于所属父控件的作用域
    /// \see GetAbsoluteRect
	virtual wxRect GetRect() const { return m_Rect; }

	/// \brief 得到控件相对于父控件的位置
	void GetPosition(int* x, int* y) const;

	/// \brief 得到控件相对于父控件的位置
	wxPoint GetPosition() const { return wxPoint( m_Rect.x, m_Rect.y ); }

	/// \brief 得到控件的宽度/高度
	void GetSize(int* w, int* h) const;

	/// \brief 获取控件相对于所属 VdkWindow 的作用域
	/// \see GetRect
	wxRect GetAbsoluteRect() const { return TranslateRect( m_Rect ); }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 调用本函数进行绘制
	///
	/// 本函数绘制自己和所有属下子控件。\n
	/// 派生控件类具体要重载的虚函数是\link DoDraw \endlink 。
	/// \see DoDraw
	void Draw(wxDC& dc);

	/// \brief 擦除\a rc 指定的矩形
	/// \param rc 坐标起点为父控件的左上角，与当前作用域同起点
	void EraseBackground(wxDC& dc, const wxRect& rc);

	/// \brief 设置控件绘制文本时所用字体
	void SetFont(const wxFont& font, wxDC* pDC);

	/// \brief 获取控件绘制文本时所用字体
	const wxFont& GetFont() const { return m_Font; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 获取控件相对于父控件/所属 VdkWindow 的对齐属性
	///
	/// 当没有父控件时，对其属性是相对于所属 VdkWindow 而言的。
	align_type GetAlign() const { return m_align; }

	/// \brief 设置控件相对于父控件/所属 VdkWindow 的对齐属性
	///
	/// 当没有父控件时，对其属性是相对于所属 VdkWindow 而言的。
	void SetAlign(const align_type& align) { m_align = align; }

	/// \brief 处理控件相对父控件的对齐属性
	///
	/// 例如相对父控件/父窗口居中、右对齐等。
	void HandleRelativeAlign(align_type align);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 设置控件 ID
	void SetID(VdkCtrlId id);

	/// \brief 获取控件 ID
	VdkCtrlId GetID() const { return m_id; }

	/// \brief 设置控件的名字
	void SetName(const wxString& strName) { m_strName = strName; }

	/// \brief 获取控件的名字
	wxString GetName() const { return m_strName; }

	/// \brief 得到父窗口的句柄
	VdkWindow* GetVdkWindow() const { return m_Window; }

	/// \brief 设置父窗口的句柄
	/// \attentiion 只应该由 VDK 内部调用
	void SetVdkWindow(VdkWindow* win);

	/// \brief 重新设置父窗口
	bool ReAttachToWindow(VdkWindow* win);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 设置父控件
	/// \param parent 可以为 NULL
	void SetParent(VdkControl* parent);

	/// \brief 获取父控件
	VdkControl* GetParent() const { return m_parent; }

	/// \brief 获取根父控件
	VdkControl* GetRootParent() const;

	/// \brief 获取子控件链表的前一个结点
	VdkControl* GetPrev() const { return m_prev; }

	/// \brief 设置子控件链表的前一个结点
	void SetPrev(VdkControl* prev) { m_prev = prev; }

	/// \brief 获取子控件链表的后一个结点
	VdkControl* GetNext() const { return m_next; }

	/// \brief 设置子控件链表的后一个结点
	void SetNext(VdkControl* next) { m_next = next; }

	/// \brief 获取子控件链表的第一个结点
	VdkControl* GetFirstChild() const { return m_firstChild; }

	/// \brief 在子控件链表中移除指定子控件
	///
	/// 删除一个控件时应该向父控件调用。不能直接 delete 。
	/// 本函数不会接管\a pCtrl 。
	void RemoveChild(VdkControl* pCtrl);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 撤销相关联的 ToolTip
	void ResetToolTip();

	/// \brief 设置相关联的 ToolTip
	/// \param updateOnce 是否立即设置主窗口的 Tooltip ，
	/// 否则将在控件的作用域内显示。
	void SetToolTip(const wxString& strToolTip, bool updateOnce = false);

	/// \brief 获取相关联的 ToolTip
	wxString GetToolTip() const { return m_strToolTip; }

	/////////////////////////////////////////////////////////////////

	/// \brief 将一个 VdkMenu 关联到控件上
	///
	/// 当控件有依附到其上的 VdkMenu 时，按右键会在相应位置弹出来。
	void AttachMenu(VdkMenu* menu);

	/// \brief 得到与控件相关联的 VdkMenu
	VdkMenu* GetMenu() const { return m_menu; }

	/// \brief 设置当前 VdkMenu 的可见性
	///
	/// 外部控制。菜单可能消失或者显示了，借此通知控件更新内部状态。
	void SetMenuState(bool bMenuOnShow, wxDC* pDC = NULL);

	/// \brief 相关联的 VdkMenu 是否在显示
	bool IsMenuOnShow() const;

	/////////////////////////////////////////////////////////////////

	/// \brief 获取控件的可见性
	///
	/// 注意与\link IsShown \endlink 的不同：本函数完全由用户控制控件状态，
	/// 而\link IsShown \endlink 可能会由 VDK 改变控件的可见状态。
	/// \see IsShown
	bool IsOnShow() const;

	/// \brief 获取控件的可见性
	/// \see IsOnShow IsFreezed IsShownOnScreen
	bool IsShown() const;

	/// \brief 控件是否显示在屏幕上
	/// \see IsOnShow IsShown IsFreezed
	bool IsShownOnScreen() const;

	/// \brief 设置控件的可见性
	/// \see IsShown IsOnShow IsShownOnScreen
	void Show(bool bShow, wxDC* pDC);

	/// \brief 隐藏控件
	/// \param pDC 擦除背景
	void Hide(wxDC* pDC) { Show( false, pDC ); }

	/// \brief 冻结一个 VdkControl 的显示状态，使之间接达到\n
	/// \link Show() \endlink 的效果
	///
	/// 考虑再三，决定添加这两个函数。按照经验，假如要隐藏一个按钮，\n
	/// 我们会直接应用\link Show() \endlink 函数。但\link Show() \endlink \n
	/// 有一个很大的弊端，就是无法区分是谁显示/隐藏自己（指 VdkControl 自身），\n
	/// 假如一个按钮，位于一个 VdkPanel 上，程序员在某些代码中隐藏了它。\n
	/// 接着用户切换到另一个 VdkPanel ，那么 VdkPanel 也会隐藏它。恩，没问题。\n
	/// 那么，当用户切换回这个 VdkPanel 时，问题来了。\n
	/// VdkPanel 自作主张地认为这个隐藏的按钮是它自己隐藏的，于是调用\n
	/// \link Show() \endlink 显示它……其实，这个按钮是程序员自己隐藏的，\n
	/// 而且这个按钮应该由程序员自己决定要不要\link Show() \endlink 它。\n
	/// 于是，\link Freeze() \endlink 和\link Thaw() \endlink\n
	/// 就是为了解决这种局面而提出来的。\n
	/// VDK 内部对未知的 VdkControl 不应该调用\link Show() \endlink ,\n
	/// 而是在要隐藏时调用\link Freeze() \endlink，重新显示时调用\n
	/// \link Thaw() \endlink。
	/// \see Thaw
	/// \see Show
	void Freeze();

	/// \brief 将 VdkControl 从冻结的状态中解放出来
	void Thaw(wxDC* pDC);

	/// \brief 当前是否被冻结
	bool IsFreezed() const { return m_freezed; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 更新控件当前的输入焦点状态
	/// \attention 设置当前具有输入焦点的控件应该调用
	/// \link VdkWindow::FocusCtrl \endlink 。
	void UpdateFocus(bool focus, wxDC* pDC = NULL);

	/// \brief 获取控件是否具有输入焦点
	bool IsFocused() const { return m_focused; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 是否启用
	bool IsEnabled() const { return m_enabled; }

	/// \brief 禁用/启用 VdkControl
	void Enable(bool bEnabled, wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 是否允许处理事件
	bool CanHandleEvent() const {
		return IsShown() && IsEnabled();
	}

	/// \brief 是否为 VdkControlHandler
	bool IsCtrlHandler() const { return TestStyle( VCS_HANDLER ); }

	/// \brief 是否适合激发回调函数
	bool IsReadyForEvent() const { return m_id != wxID_ANY; }

	/// \brief 激发控件的回调函数
	void FireEvent(wxDC* pDC, void* extraData);

	//////////////////////////////////////////////////////////////////////////
	// 子控件迭代器

	class ChildIterator;

	/// \brief 获取第一个子控件的迭代器
	ChildIterator begin()
	{
		ChildIterator i( this );
		return i;
	}

	/// \brief 获取最后一个子控件之外的迭代器
	ChildIterator end()
	{
		ChildIterator i( NULL );
		return i;
	}

protected:

	/// \brief XRC 动态创建
	///
	/// 一个 VdkControl 的派生类假如要实现动态创建，则应该实现这个函数
	virtual void Create(wxXmlNode* node) {}

	/// \brief 默认构造函数
	/// \attention 构造一个 VdkControl 前必须确保掩码色已经部署到位！
	/// \see Initialize
	VdkControl();

	/// \brief XRC 动态创建
	void DoXrcCreate(wxXmlNode* node);

	/// \brief XRC 动态创建的共同逻辑处理
	template< class D >
	void Create(const VdkCtrlInitializer<D>& init_data);

	/// \brief 处理鼠标事件
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief 处理键盘按键事件
	virtual void DoHandleKeyEvent(VdkKeyEvent& e) {}

	/// \brief 接收、处理通知信息
	virtual void DoHandleNotify(const VdkNotify& notice) {}

	/// \brief SetRect 时更新子控件的位置有用
	void HandleChildrenResize(int dX, int dY);

private:

	/// \brief 擦除\a rc 指定的矩形
	/// \param rc 坐标起点为父控件的左上角
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc) {}

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc) {}

	/// \brief 重新设置父窗口
	bool DoReAttachToWindow() { return true; }

	/// \brief 添加一个孩子
	/// \attention 谨慎调用，应该使用\link SetParent() \endlink。
	void AddChild(VdkControl* chd);

	/// \brief 发送控件大小改变的通知信息
	/// \param fromSetRect 消息是否来自\link SetRect \endlink 。
	void FireSizeNotify(int dX, int dY, bool fromSetRect = false, wxDC* pDC = NULL);

	/// \brief 发送控件位置改变的通知信息
	void FireMoveNotify(int xPosChanged, int yPosChanged);

protected:

    /// \brief 当前进程空间的控件皮肤位图的透明掩码色
	static wxColour			ms_maskColor;

	/// \brief 所有 VdkControl 共有的皮肤文件根目录
	static wxString			ms_rootPath;

	/// \brief 正在显示 ToolTip 的 VdkControl
	static VdkControl*		ms_pToolTipOnShow;

	/////////////////////////////////////////////////////////////////

	VdkCtrlId				m_id; ///< 控件 ID ，回调函数时做标识用

	/// \brief 父亲
	VdkControl*				m_parent;
	/// \brief 第一个孩子，相当于链表头
	VdkControl*				m_firstChild;
	/// \brief 哥哥
	VdkControl*				m_prev;
	/// \brief 弟弟
	VdkControl*				m_next;

	/// \brief 控件名字
	wxString				m_strName;

	/// \biref 相对于 VdkWindow 的对齐
	align_type				m_align;

	/// \brief ToolTip的显示文本
	wxString				m_strToolTip;

	/// \brief 控件占据的区域
	wxRect					m_Rect;

	/// \brief 包含控件的顶层窗口
	VdkWindow*				m_Window;

	/// \brief 父窗口句柄
	wxWindow*				m_WindowImpl;

	/// \brief 与此控件相关联的菜单
	VdkMenu*				m_menu;

	/// \brief 控件要使用的字体
	wxFont					m_Font;

private:

	// 当前是否是否显示
	bool					m_shown;
	// 当前是否可用
	bool					m_enabled;
	// 是否具有键盘输入焦点
	bool					m_focused;
	// 是否被处于被冻结的状态
	bool					m_freezed;

	DECLARE_NO_COPY_CLASS( VdkControl )   // VdkControl 不允许复制

public:

	/// \brief 获取一个新的控件
	/// \note 用于控件克隆。
	virtual VdkControl* GetImitation() const {
		return new VdkControl;
	}

public:

    /// \brief 子控件迭代器
	class ChildIterator
	{
	public:

		/// \brief 默认构造函数
		ChildIterator() { Init(); }
		/// \brief 构造函数
		ChildIterator(VdkControl* pCtrl);

		/// \brief 前自增
		ChildIterator& operator ++(); // prefix ++

		/// \brief 后自增
		ChildIterator operator ++(int); // postfix ++

		/// \brief 前自减
		ChildIterator& operator --(); // prefix ++

		/// \brief 后自减
		ChildIterator operator --(int); // postfix ++

		/// \brief 取值
		VdkControl& operator* ();

		/// \brief 取址
		VdkControl* operator-> ();

		/// \brief 明确方式的取址操作
		VdkControl* get() const { return m_curr; }

	private:

		// 初始化迭代器
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

/// \brief 不需调用\link VdkWindow::AddCtrl() \endlink 的幕后控件
///
/// VdkControlHandler与 VdkControl 有什么区别呢？最大的区别就是，\n
/// VdkControlHandler 一般拥有子控件（或许可以称之为复合控件），\n
/// 但它又不需要处理事件（可能需要重画背景）。
/// 即可以覆写\link VdkControl::DoDraw() \endlink ，
/// 但不能覆写\link VdkControl::DoHandleEvents() \endlink 。
class VdkCtrlHandler : public VdkControl
{
public:

	/// \brief 构造函数
	VdkCtrlHandler();
};

//////////////////////////////////////////////////////////////////////////

/// \brief 控件皮肤位图文件不存在时引发的异常
class VdkExpectionImageNotFound
{
public:

	/// \brief 构造函数
	explicit VdkExpectionImageNotFound(const wxString& strError)
		: m_strError( strError )
	{

	}

	/// \brief 异常的详细信息
	wxString GetMessage() const { return m_strError; }

private:

	wxString			m_strError;
};

//////////////////////////////////////////////////////////////////////////

/// \brief \link VdkControl::ParseObjects \endlink 的回调对象
class VdkCtrlParserCallback
{
public:

	/// \brief 回调通知函数
	/// \param win \a pCtrl 的父窗口
	/// \param clName \a pCtrl 的类名
	/// \param pCtrl 已被构建出来的控件对象
	virtual void Notify(VdkWindow* win, 
						const wxString& clName, 
						VdkControl* pCtrl) = 0;
};
