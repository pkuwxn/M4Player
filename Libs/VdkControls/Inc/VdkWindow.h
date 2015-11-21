/***************************************************************
 * Name:      VdkWindow.h
 * Author:    Ning (vanxining@139.com)
 * Created:   2009-12-19
 * Copyright: Ning
 **************************************************************/
/*!\mainpage VDK(Virtual-controls Development Kit)
 *
 * \section intro_sec 简介
 *
 * 最近 DirectUI 有泛滥的趋势。但我写 VDK 的初衷只是 Gtk 不支持
 * borderless BitmapButton ，并无要写一个 DirectUI 的念头，
 * 因此代码完全没有一个代码库应有的严谨考虑。故整份代码自出生起
 * 便不断地在重构、重写，到现在最终成了一份自己比较满意的作品。
 *
 * \section install_sec 安装
 *
 * 无法提供一个静态库或者DLL。必须自己将代码包含进工程中一起编译。\n
 * 为什么呢？因为编译器的优化。因为所有的 VdkControl 都是有可能从一个
 * XRC 文件中动态创建而来，而我们的代码中可能就根本就没调用过哪怕一个
 * 该类的特有函数，因此编译器就认为我们没有引用到这个类，所以就没有将
 * 这个类的目标代码链接进最终的可执行文件里。\n
 * Orz，只能采取最笨的办法了。\n
 * 我想解决方法还是有的。那就是在一个C++文件中引用所有 VdkControl
 * 的头文件，将所有 IMPLEMENT_DYNMATIC_VOBJECT( VdkXXXX ) 宏写在文件中。
 * 但这样会不会带来另外一个弊端:所有的 VdkControl 都被链接进输出文件里了，
 * 会不会导致代码膨胀？
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

VDK_DECLARE_WX_PTR_LIST(VdkEventFilter, VdkEventFilterList, EventFilterIter)

/// \brief VdkWindow 的初始化风格（属性）
///
/// 只在初始化 VdkWindow 时生效。必须不同于 VdkWindowStyle 的已定义值。
enum VdkWindowInitStyle {
    /// \brief 不能在标题栏拖动窗体，只能在窗口空白处拖动窗口
    VWIS_DRAG_BY_SPACE      = 1 << 6,
};

/// \brief VdkWindow 的风格（属性）
enum VdkWindowStyle {
    /// \brief 派生类的起始属性位
    VWS_USER                = 20,
    /// \brief 是否需要将所有控件放置于一个额外的 wxPanel 上
    ///
    /// 这是很多 GUI 框架要求的，例如在 wxGTK 下面一个 wxFrame
    /// 或 wxDialog 是无法接收到键盘事件的，必须有一个活动子控件。
    VWS_BASE_PANEL          = 1 << 0,
    VWS_RESIZEABLE          = 1 << 1, ///< 能否改变大小
    VWS_DRAGGABLE           = 1 << 2, ///< 能否使用鼠标拖动
    VWS_MAXIMIZABLE         = 1 << 3, ///< 能否最大化
    VWS_ALWAYS_REFRESH      = 1 << 4, ///< OnPaint 中始终重画整个界面
    VWS_MENU_IMPL           = 1 << 5, ///< 窗体用作菜单实体实现窗体

    /// \brief 禁用双缓冲绘图
    ///
    /// 已废弃。
    VWS_NO_DOUBLE_BUFFER    = 1 << 7,
    VWS_FIRST_SHOWN_EVT     = 1 << 8, ///< 发送第一次显示的通知信息
    /// \brief 用户按下 ESC 时，隐藏窗口，一般用于对话框
    VWS_DISMISS_BY_ESC      = 1 << 9,
    /// \brief 当接受到窗口关闭事件时，销毁窗口
    ///
    /// 一般用于对话框，框体窗口(Frame window)关闭时即默认销毁。
    VWS_CLOSE_TO_DESTORY    = 1 << 10,
};

/// \brief VdkWindow 运行时状态集
enum VdkWindowState {
    /// \brief 派生类的起始属性位
    VWST_USER                       = 20,
    /// \brief 鼠标是不是一直按住？按住鼠标时持续发送产生相同的鼠标事件
    VWST_MOUSE_HOLD_ON              = 1 << 0,
    /// \brief 当在第一次调用 OnPaint 时决定要不要创建 VdkButton 的 m_Region 。\n
    /// 同时在此之前不响应所有鼠标事件
    VWST_INITING                    = 1 << 1,
    /// \brief 这是最后一次鼠标事件同时在此之后不响应所有鼠标事件
    VWST_EXITING                    = 1 << 2,
    /// \brief 是否刚刚才 Reset 过
    VWST_RESET                      = 1 << 3,
    /// \brief 是否在移动 VdkWindow
    VWST_DRAG_AND_MOVING            = 1 << 4,
    /// \brief 是否上次鼠标按下事件已被跳过
    ///
    /// 可用于判断拖动事件是否专属于用于拖动窗口。
    VWST_LEFT_DOWN_SKIPPED          = 1 << 5,
    /// \brief 是否在改变 VdkWindow 的大小
    VWST_DRAG_AND_RESIZING          = 1 << 6,
    /// \brief 是否正在处理最大化事件
    VWST_MAXIMIZING                 = 1 << 7,
    /// \brief 本次全部重画窗口
    VWST_REDRAW_ALL                 = 1 << 8,
};

//////////////////////////////////////////////////////////////////////////

/// \brief 为 VdkWindow 设置皮肤
class VdkWindowInitializer {
public:

    typedef VdkWindowInitializer Window;

    /// \brief 构造函数
    VdkWindowInitializer();

    /// \brief 指定一个位图
    Window &bgBitmap(const wxBitmap &bm) {
        bmBkGnd = bm;
        return *this;
    }

    /// \brief 指定背景颜色
    Window &bgColor(const wxColour &color) {
        BgColor = color;
        return *this;
    }

    /// \brief 指定一个文件名，此时无需指定一个第一个参数（\a bmBkGnd ）
    Window &fileName(const wxString &s) {
        strFileName = s;
        return *this;
    }

    /// \brief 窗口的作用域。当 VdkWindow 为子窗口时，
    /// Rect.x 和 Rect.y 以主窗口左上角的坐标为计量起点，\n
    /// 即这个坐标(Rect.x, Rect.y)是相对而言的
    Window &rect(const wxRect &rc) {
        Rect = rc;
        return *this;
    }

    /// \param 平铺/拉伸
    Window &resizeable(bool b) {
        bResizeable = b;
        return *this;
    }

    /// \brief 供 VDK 平铺/拉伸背景位图的区域
    Window &tileType(VdkResizeableBitmapType type) {
        ResizeType = type;
        return *this;
    }

    /// \brief 供 VDK 平铺/拉伸背景位图的区域
    Window &tileArea(const wxRect &rc) {
        TileArea = rc;
        return *this;
    }

private:

    wxBitmap        bmBkGnd;
    wxString        strFileName;
    wxRect          Rect;
    wxColour        BgColor;

    bool            bResizeable;
    wxRect          TileArea;
    VdkResizeableBitmapType ResizeType;

    friend class VdkWindow;
};

/*!\brief 一个可以容纳 virtual-controls 的窗口
 *
 * 所有 VdkControl 都必须以一个 VdkWindow 为父窗口。
**/
class VdkWindow : public VdkStyleAndStateOwner {
public:

    /// \brief 构造函数
    VdkWindow(wxWindow *handle, long style = VWS_DRAGGABLE);

    /// \brief XRC 动态创建
    VdkWindow *Create(wxXmlNode *xmlNode, MapOfCtrlIdInfo *ids = NULL);

    /// \brief 通知 VdkWindow 不要再处理事件了
    void BeginExit();

    /// \brief 析构函数
    virtual ~VdkWindow();

    /// \brief 为 VdkWindow 设置皮肤
    void Create(const VdkWindowInitializer &init_data);

    /// \brief 是否正在初始化，尚不可以作图
    bool IsInitializing() const {
        return TestState(VWST_INITING);
    }

    /// \brief 是否已提交销毁窗口命令，此时所有子控件应停止运行，同时准备
    /// 销毁自己。
    bool IsExiting() const {
        return TestState(VWST_EXITING);
    }

    /// \brief 若窗口处于已被重置，并正在重建控件列表的状态，则返回真
    bool IsReseting() const {
        return TestState(VWST_RESET);
    }

    /// \brief 初始化所有使用 wxRegion 作为作用域标志的 VdkButton
    void InitRegionButtons();

    /// \brief 安全地销毁一个 VdkWindow 对象
    template<class T>
    static void DestoryWindow(T &win) {
        if (win) {
            win->BeginExit();
            win->GetWindowHandle()->Destroy();
            win = NULL;
        }
    }

    /// \brief 获取面板句柄
    wxWindow *GetHandle() const {
        return m_panel;
    }

    /// \brief 获取外层包裹窗口的句柄
    wxWindow *GetWindowHandle() const {
        return m_this;
    }

    /// \brief 获取 wxTopLevelWindow 句柄
    ///
    /// \warning 返回的句柄可能为空。
    wxTopLevelWindow *GetTopLevelWindowHandle() const;

#ifdef __WXDEBUG__
    /// \brief 设置窗口标识符
    void SetDebugCaption(const wxString &debugCaption) {
        m_debugCaption = debugCaption;
    }

    /// \brief  获取窗口标识符
    wxString GetDebugCaption() const {
        return m_debugCaption;
    }
#endif

    //////////////////////////////////////////////////////////////////////////
    // 与窗口相关联比较紧密的一些 GUI 属性

    /// \brief 设置鼠标指针的形状
    void AssignCursor(const wxCursor &cursor);

    /// \brief 重置鼠标指针为默认形状
    void ResetCursor();

    /// \brief 获取与 VdkWindow 相关联的托盘图标
    TrayIcon *GetTrayIcon() const {
        return m_trayIcon;
    }

    /// \brief 设置与 VdkWindow 相关联的托盘图标
    void SetTrayIcon(TrayIcon *trayIcon);

    /// \brief 创建父窗口为自己的 VdkMenu
    /// \note 每个 GUI 窗体都需要一个父窗口。
    VdkMenu *CreateMenu(VdkMenu *parent = NULL, int mid = wxID_ANY);

    /// \brief 使 VdkMenu 依附到 VdkWindow 上，使得可以失去焦点时 VdkMenu 消失
    void AttachMenu(VdkMenu *menu) {
        m_menu = menu;
    }

    /// \brief 获取依附到 VdkWindow 上 VdkMenu
    VdkMenu *GetMenu() const {
        return m_menu;
    }

    /// \brief 在以当前窗口左上角为计量起点的(\a x, \a y )出显示弹出菜单\a menu
    ///
    /// 一个便利函数，因为\link VdkMenu::ShowContext \endlink
    /// 要求以屏幕左上角为为计量起点的显示坐标。
    /// \param delay 是否延迟显示
    void ShowContextMenu(VdkControl *pCtrl, int x, int y, bool delay = false);

    /// \brief 在以当前窗口左上角为计量起点的(\a Point.x, \a Point.y )
    /// 出显示弹出菜单\a menu
    void ShowContextMenu(VdkControl *pCtrl, const wxPoint &Point,
                         bool delay = false) {
        ShowContextMenu(pCtrl, Point.x, Point.y, delay);
    }

    /// \brief 隐藏当前正在显示的 VdkMenu
    static void HideMenu();

    /////////////////////////////////////////////////////////////////////////////////

    /// \brief 压入一个事件过滤器
    void PushEventFilter(VdkEventFilter *filter) {
        m_EventFilters.Append(filter);
    }

    /// \brief 弹出句柄为\a filter 的事件过滤器
    void PopEventFilter(VdkEventFilter *filter, bool del);

    /// \brief 处理鼠标事件
    bool HandleMouseEvent(wxMouseEvent &e, VdkMouseEventType evtCode);

    /// \brief 获取上次窗口所处理的鼠标事件
    VdkMouseEventType GetLastMouseEvent() const {
        return m_nLastMouseEvent;
    }

    /// \brief 获取当前处于鼠标指针之下的控件句柄置空
    static VdkControl *GetCtrlOnHover() {
        return ms_hovering;
    }

    /// \brief 获取当前具有输入焦点的控件
    VdkControl *GetCtrlOnFocus() const {
        return m_focus;
    }

    /// \brief 激活隐藏控件，准备接收键盘输入
    void FocusHiddenCtrl();

    /// \brief 获取隐藏控件句柄
    wxControl *GetHiddenCtrl() const {
        return m_hiddenCtrl;
    }

    //////////////////////////////////////////////////////////////////////////

    /// \brief 重绘窗口
    void Draw(wxDC &pdc);

    /// \brief 添加全部重绘窗口的事件到窗口的消息队列中
    void QueueRedrawEvent();

    /// \brief 最小化窗口
    void Minimize() {
        DoMinimize();
    }

    /// \brief 得到窗口初始大小
    ///
    /// 初始化某些 VdkControl 时可能用到。
    wxSize GetMinSize() const;

    /// \brief 得到窗口初始大小
    void GetMinSize(int *w, int *h) const;

    /// \brief 得到窗口的大小
    wxRect Rect00() const {
        int w, h;
        m_this->GetSize(&w, &h);

        return wxRect(0, 0, w, h);
    }

private:

    // 事件提供者轮询所有注册的事件过滤器
    // 返回值：是否希望立即终止后续处理
    bool FilterEvent(int evtCode, const wxMouseEvent &e);

    /// 事件提供者询问 VdkWindow 的派生类是否需要过滤此事件
    /// \return 是否希望立即终止后续处理
    virtual bool FilterEventBefore(wxMouseEvent &evt, int evtCode) {
        return false;
    }

    /// 事件提供者询问 VdkWindow 的派生类是否需要过滤此事件
    ///
    /// 本函数能对发生在窗体空白处的事件进行“马后炮”式处理，
    /// 即处理没有控件接管的事件
    /// \return 是否希望立即终止后续处理
    /// \TODO VdkMouseEvent ==> VdkEvent
    virtual bool FilterEventAfter(wxMouseEvent &evt, int evtCode) {
        return false;
    }

    // 执行实际绘制工作
    void DoDraw(wxDC &dc);

    /// 派生类可以对重画事件作出响应
    virtual void DoPaint(wxDC &dc) {}

    /// 派生类可以对发生在 VdkWindow 上的键盘事件作出响应
    /// \return 是否终止 VdkWindow 的后续处理
    virtual bool DoHandleKeyEvent(wxKeyEvent &e) {
        return false;
    }

protected:

    /// \brief 绑定事件处理函数
    void BindHandlers();

    wxWindow *m_this; ///< VdkWindow 的原生窗口句柄
    wxWindow *m_panel; ///< 所有子控件的直接父窗口（wxPanel 的句柄）

private:

    // 全局静态变量：当前处于鼠标指针之下的控件
    static VdkControl *ms_hovering;

    // 隐藏的控件，用于接收键盘输入（直接在窗体一级截获太没保障了）
    wxControl *m_hiddenCtrl;
    // 当前窗口具有输入焦点的控件
    VdkControl *m_focus;

#ifdef __WXDEBUG__
    // 窗口标识符
    wxString m_debugCaption;
#endif

    // 与整个窗口相关联的菜单
    VdkMenu *m_menu;

    // 与此窗口相关联的托盘图标
    TrayIcon *m_trayIcon;

    // 窗口当前的鼠标指针形状
    // 主要是用于 VDK 可能会内部性地改变鼠标指针的形状以保存
    // 用户自定义的形状用以恢复。
    wxCursor m_cursor;

    // 外部事件过滤器链表
    VdkEventFilterList m_EventFilters;

    // 上次 VdkControl 所处的状态
    int m_nLastCtrlState;

    // 本次鼠标事件不属于本窗口
    //
    // 例如拖动事件、滚轮事件这些可能会跳出所属窗口作用域的事件应该发送到当前
    // 具有输入焦点的窗口而不是鼠标指针正位于其上的那个窗口。
    bool m_mouseEventNotForMe;

    // 上次所处理的鼠标事件
    VdkMouseEventType m_nLastMouseEvent;

    // 在按住鼠标时持续发送产生相同的鼠标事件
    MouseHoldTimer *m_pMouseHoldTimer;

    // 窗口初始时刻的宽度
    int m_nMinWidth;

    // 窗口初始时刻的高度
    int m_nMinHeight;

private:

    void OnMouseMove(wxMouseEvent &e);
    void OnMouseWheel(wxMouseEvent &e);
    void OnMouseLost(wxMouseCaptureLostEvent &);
    void OnLeftDown(wxMouseEvent &e);
    void OnLeftUp(wxMouseEvent &e);
    void OnDLeftDown(wxMouseEvent &e);
    void OnRightUp(wxMouseEvent &e);
    void OnEnterWindow(wxMouseEvent &);
    void OnLeaveWindow(wxMouseEvent &);

    void OnKeyDown(wxKeyEvent &e);
    void OnKeyUp(wxKeyEvent &e);
    void OnChars(wxKeyEvent &e);

    // wxWidgets 会默认将“Windows context menu”按键事件转换成普通的右键弹起事件
    void OnWindowMenuKeyUp(wxMouseEvent &);

    // 统一处理三种不同意义的按键事件
    // 分别为按键按下、释放，以及已翻译的按键字符。
    void HandleKeys(VdkKeyEventType type, wxKeyEvent &e);

    void OnIconize(wxIconizeEvent &);
    void OnMaximize(wxMaximizeEvent &);
    void OnSize(wxSizeEvent &);
    void OnShow(wxShowEvent &e);

    void OnPaint(wxPaintEvent &e);
    void OnWindowPaintDummy(wxPaintEvent &);
    void OnEraseBackground(wxEraseEvent &) {}

    void OnWindowFocus(wxActivateEvent &e);
    void OnClose(wxCloseEvent &e);

    // 延迟显示右键菜单
    void OnDelayShowContextMenu(VdkVObjEvent &e);

    // 响应 VdkMenu 的隐藏消息
    void OnMenuHid(VdkVObjEvent &e);

    // 当前窗口是否为一个菜单实体窗体
    bool IsMenuImpl() {
        return TestStyle(VWS_MENU_IMPL);
    }

    // 事件是否可以激活当前控件
    //
    // 如单击、右击等。
    bool IsActivatableEvent(int evtCode);

    // 当上次激活的控件不在当前窗口内，则调用该窗口的这一函数。
    void RecoverCtrl(VdkControl *pCtrl);
    void HandleMouseHoldOn(int evtCode, const wxPoint &Point, VdkControl *pCtrl);

    // 设置当前处于鼠标指针之下的控件
    static void SetCtrlOnHover(VdkControl *pCtrl);

    // 将当前处于鼠标指针之下的控件句柄置空
    static void ResetCtrlOnHover();

    // 设置上次窗口所处理的鼠标事件
    void SetLastMouseEvent(VdkMouseEventType evtCode);

    // 设置当前具有输入焦点的控件
    void SetCtrlOnFocus(VdkControl *pCtrl);

    // 将当前具有输入焦点的控件句柄置空
    void ResetCtrlOnFocus();

    // 获取当前程序正在显示的菜单
    static VdkMenu *&GetMenuOnShow();

    // 设置当前程序正在显示的菜单
    static void SetMenuOnShow(VdkMenu *menu);

    //////////////////////////////////////////////////////////////////////////

    // 在窗口上按下鼠标左键时保存的初始位置
    // 用于模拟拖动窗口，保存开始拖动时鼠标指针相对窗口左上角的位置。
    wxPoint m_mouseOn;

    // 缓存的窗口作用域，用于拖动改变窗口改变大小以及处理窗口大小改变事件
    wxRect m_rectCached;

public:

    /// \brief 获取在窗口上按下鼠标左键时保存的初始位置
    wxPoint GetMouseOnForm() const {
        return m_mouseOn;
    }

#if 1
    /// \brief 设置窗口大小
    bool Resize(int w, int h);

    /// \brief 设置窗口大小
    bool Resize(const wxRect &rc) {
        return Resize(rc.x, rc.y, rc.width, rc.height);
    }

    /// \brief 设置窗口大小
    bool Resize(const wxSize &size) {
        return Resize(size.x, size.y);
    }

    /// \brief 设置窗口大小
    /// \attention 请勿直接调用 wxWindow::SetSize ，因为窗口可能为异形窗口。
    bool Resize(int x, int y, int w, int h, int sizeFlags = wxSIZE_AUTO);
#endif

    /// \brief 设置最小的宽高度
    ///
    /// 这个最小宽高度与实际物理最小宽高度不同。后者由皮肤提供的位图文件决定，\n
    /// 而这个值可由程序员编程指定，即用户改变窗口大小时所能达到的最小宽高度。\n
    /// \note 可以节省皮肤位图文件的大小，或者限制窗口的最小大小。
    void SetMinSize(int w, int h);

protected:

    /// \brief 通知下属各个 VdkControl 处理 VdkWindow 改变大小事件
    void LayoutWidgets(int dX, int dY);

private:

    // 重画前必须保证双缓冲位图是正确可用的
    bool IsBufferedBitmapOk();

    // 更改大小
    virtual void DoResize(int x, int y, int width, int height,
                          int sizeFlags = wxSIZE_AUTO) = 0;

    // 最小化
    virtual void DoMinimize() = 0;

private:

    // 处理改变窗口大小时的鼠标指针变化
    void HandleRzCursor(const wxPoint &Point);

public:

    /// \brief 给定一个以屏幕左上角为起点的坐标，将窗口放缩至该点处。
    void EmulateDragAndResize(const wxPoint &Point);

    /// \brief 模拟拖动窗口
    void EmulateDragAndMove(const wxPoint &Point);

    /// \brief 鼠标拖曳改变窗口大小时指针在窗口边界上的位置
    typedef int MousePtrPos;

    /// \brief 获取当前鼠标拖动拉伸窗口的类型
    MousePtrPos GetDragAndResizeType() const {
        return m_dragAndResizeType;
    }

    /// \brief 重置(取消)改变窗口大小时的特殊状态鼠标指针
    void ResetRzCursor();

    /// \brief 是否允许通过拖动窗口空白处实现拖动整个窗口
    bool CanDragBySpace() const {
        return m_dragBySpace;
    }

protected:

    VdkScalableBitmap m_bkCanvas; ///< 窗口的背景位图

private:

    bool m_dragBySpace; // 是否允许通过拖动窗口空白处实现拖动整个窗口

    // 决定如何去拉动改变窗口大小
    MousePtrPos m_dragAndResizeType;

    // 上次发送 Resize 事件的时间戳：Resize 事件每隔 40ms 发送一次
    wxMilliClock_t m_nLastTimeStamp;

public:

    /////////////////////////////////////////////////////////////////////////////////
    // VdkControl 相关

    /// \brief 重置窗口，重新设置皮肤
    void Reset();

    /// \brief 将\a pCtrl 添加到控件列表中
    ///
    /// \attention 应该由创建者调用，而不是被创建的控件本身
    void AddCtrl(VdkControl *pCtrl);

    /// \brief 从控件列表中移除指定控件
    ///
    /// 本函数不会移除\a pCtrl 的子控件。事实上它对\a pCtrl 的父子控件
    /// 一无所知。
    /// \return 是否成功移除了该控件。
    bool RemoveCtrl(VdkControl *pCtrl);

    /// \brief 查找名为\a strName 的 VdkControl
    VdkControl *FindCtrl(const wxString &strName);

    /// \brief 查找名为\a strName 的 VdkControl
    template<class T>
    T *FindCtrl(const wxString &strName) {
        return static_cast<T *>(FindCtrl(strName));
    }

    /// \brief 查找名为\a strName 的 VdkControl
    template<class T>
    T *operator [](const wxString &strName) {
        return static_cast<T *>(FindCtrl(strName));
    }

    /// \brief 确认句柄为\a pCtrl 的 VdkControl 是否已位于控件列表之中
    bool FindCtrl(VdkControl *pCtrl);

    typedef VdkCtrlList::iterator CtrlIter;

    /// \brief 获取所有控件的迭代器起点
    CtrlIter begin() {
        return m_Ctrls.begin();
    }

    /// \brief 获取所有控件的迭代器起点
    CtrlIter end() {
        return m_Ctrls.end();
    }

    //////////////////////////////////////////////////////////////////////////

    /// \brief 通知\a pCtrl 当前窗口大小相对最小值的改变量
    void LayoutCtrl(VdkControl *pCtrl);

    /// \brief 设置控件的可见性
    void ShowCtrl(const wxString &strName, bool bShow, wxDC *pDC);

    /// \brief 高亮指定控件（模拟鼠标置于其上）
    void HilightCtrl(VdkControl *pCtrl, wxDC &dc);

    /// \brief 设置具有输入焦点的控件
    /// \param pCtrl 要激活的控件，不可为空
    void FocusCtrl(VdkControl *pCtrl, wxDC *pDC);

    //////////////////////////////////////////////////////////////////////////

    /// \brief 设置缓冲双缓冲绘图上下文
    void SetCachedDC(VdkDC &vdc);

    /// \brief 设置已缓冲的双缓冲绘图上下文的某些初始属性
    virtual void PrepareCachedDC();

    /// \brief 重置已缓冲的双缓冲绘图上下文
    /// \param pdc 非缓冲绘图上下文
    void ResetCachedDC(wxDC &pdc);

    /// \brief 获取已缓冲的双缓冲绘图上下文
    VdkDC *GetCachedDC() const {
        return m_cachedDC;
    }

    /// \brief 添加绘图提交监视器
    void AddPostDrawListener(VdkDcPostListener *lsner);

    /// \brief 删除绘图提交监视器
    void RemovePostDrawListener(VdkDcPostListener *lsner);

    /// \brief 得到双缓冲绘图背景位图
    ///
    /// 可以减少每次使用双缓冲绘图时创建临时位图的开销。
    wxBitmap &GetBufferedBitmap() {
        return m_bmBuffered;
    }

    /// \brief 是否启用双缓冲
    bool IsDoubleBuffering() const;

    /// \brief 擦除\a rc 指定的矩形，恢复背景
    /// \param rc 坐标起点为父控件的左上角
    virtual void EraseBackground(wxDC &dc, const wxRect &rc);

private:

    // 初始化窗口状态
    void Init();

    // 设置具有输入焦点的控件
    // @param pCtrl 要激活的控件，不可为空
    // @param x 鼠标指针当前相对与窗口客户区的 X 坐标
    // @param y 鼠标指针当前相对与窗口客户区的 Y 坐标
    void FocusCtrl(VdkControl *pCtrl, wxDC *pDC, int x, int y);

private:

    VdkCtrlList m_Ctrls;

    // 处理使用 TAB 在窗口内来进行导航
    TabOrderIterator *m_tabOrderIter;

    wxBitmap m_bmBuffered;  // 双缓冲位图
    VdkDC *m_cachedDC; // 已缓冲的双缓冲绘图上下文
    wxVector<VdkDcPostListener *> *m_postListeners;

    // 表示当前窗口的设备上下文设备起始坐标
    int m_xDcOrigin, m_yDcOrigin;

public:

    //////////////////////////////////////////////////////////////////////////
    // 拙劣的设计：滚动窗口专用

    /// \brief 获取当前窗口的设备上下文设备起始坐标
    void GetDcOrigin(int *x, int *y) const;

    /// \brief 获取当前窗口的设备上下文设备起始坐标
    void SetDcOrigin(int x, int y);

    /// \brief 重置当前窗口的设备上下文设备起始坐标
    void ResetDcOrigin(wxDC &dc) const;

private:

    DECLARE_NO_COPY_CLASS(VdkWindow)   // 不可复制
};

//////////////////////////////////////////////////////////////////////////

/// \brief VdkWindow 重画事件的回调对象
/// \attention VdkWindow 的派生类不应该继承自本接口，
/// 否则会出现 this 指针层次错误，建议的用法是由 VdkControl 实现。
/// \note 派生类要实现类似功能可覆写\link VdkWindow::DoPaint \endlink 。
class RedrawCallback {
public:

    /// \brief 执行绘图操作
    virtual void DoRedraw(wxDC &dc) const = 0;
};
