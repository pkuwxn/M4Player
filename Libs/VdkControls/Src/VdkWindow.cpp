/***************************************************************
 * Name:      VdkWindow.cpp
 * Purpose:   VdkWindow 的实现文件
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2011-02-13
 * Copyright: Wang Xiaoning
 **************************************************************/
#include "StdAfx.h"
#include "VdkWindow.h"

#include "VdkControl.h"
#include "VdkCtrlParserInfo.h"
#include "VdkMenu.h"
#include "VdkEvent.h"
#include "VdkDC.h"

#include "wxUtil.h"

#ifdef __WXMSW__
#   include "msw/TrayIcon.h"
#else
/// \brief 原有 wxTrayIcon 的扩展类，添加了图标闪烁等特性
class TrayIcon {};
#endif // __WXMSW__

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkWindowInitializer::VdkWindowInitializer()
    : BgColor(*wxWHITE),
      bResizeable(true),
      ResizeType(RESIZE_TYPE_TILE) {

}

//////////////////////////////////////////////////////////////////////////
// TAB order 管理

typedef unsigned int tab_order_t;

class TabOrderIterator {
public:

    // 添加一个控件
    void AddCtrl(VdkControl *pCtrl);

    // 删除一个控件
    bool RemoveCtrl(VdkControl *pCtrl);

    /// 清空所有已注册的控件
    void Clear();

    // 激活 TAB order 链中的下一个控件（转移焦点）
    //
    // @param current 若为空，则会激活控件链中的第一项
    // @return 若成功激活焦点链中的下一个控件则返回真。
    bool FocusNext(VdkControl *current);

private:

    // 查找指定句柄的控件
    VdkCtrlIter FindCtrl(VdkControl *pCtrl);

private:

    VdkCtrlList m_ctrlsOfTabTraversal;
};

void TabOrderIterator::AddCtrl(VdkControl *pCtrl) {
    wxASSERT(pCtrl->TestStyle(VCS_TAB_TRAVERSAL));

    if (FindCtrl(pCtrl) == m_ctrlsOfTabTraversal.end()) {
        m_ctrlsOfTabTraversal.push_back(pCtrl);
    }
}

bool TabOrderIterator::RemoveCtrl(VdkControl *pCtrl) {
    return m_ctrlsOfTabTraversal.DeleteObject(pCtrl);
}

void TabOrderIterator::Clear() {
    m_ctrlsOfTabTraversal.clear();
}

bool TabOrderIterator::FocusNext(VdkControl *current) {
    if (m_ctrlsOfTabTraversal.empty()) {
        return false;
    }

    VdkCtrlIter beg(m_ctrlsOfTabTraversal.begin());
    VdkCtrlIter end(m_ctrlsOfTabTraversal.end());
    VdkCtrlIter iter(end);

    if (current) {
        iter = FindCtrl(current);
        if (iter != end) {
            ++iter;
        }

        // TODO: 假如当前控件没有设置可转移焦点该如何处理？
    }

    if (iter == end) {
        iter = beg;
    }

    VdkCtrlList::size_type numCtrls = m_ctrlsOfTabTraversal.size();
    size_t tries = 0;

    while (tries++ < numCtrls) {
        VdkControl *pCtrl = *iter;

        if (pCtrl == current) {
            continue;
        }

        if (pCtrl->IsShown()) {
            VdkWindow *win = pCtrl->GetVdkWindow();
            VdkDC dc(win, win->Rect00(), NULL);

            win->FocusCtrl(pCtrl, &dc);
            return true;
        }

        //-----------------------------------------------------
        // 继续找下去

        ++iter;

        // 循环
        if (iter == end) {
            iter = beg;
        }
    }

    return false;
}

VdkCtrlIter TabOrderIterator::FindCtrl(VdkControl *pCtrl) {
    VdkCtrlIter iter(m_ctrlsOfTabTraversal.begin());
    for (; iter != m_ctrlsOfTabTraversal.end(); ++iter) {
        if (*iter == pCtrl) {
            break;
        }
    }

    return iter;
}

//////////////////////////////////////////////////////////////////////////

VDK_DEFINE_WX_PTR_LIST(VdkCtrlList);
VDK_DEFINE_WX_PTR_LIST(VdkWindowList);
VDK_DEFINE_WX_PTR_LIST(VdkEventFilterList);

enum {
    // 鼠标指针移出当前窗口时发送到窗口的伪鼠标位置的坐标值
    MEANNINGLESS_MOUSE_COORD = 9999999,
};

// 全局静态变量：当前处于鼠标指针之下的控件
VdkControl *VdkWindow::ms_hovering = NULL;

//////////////////////////////////////////////////////////////////////////

VdkWindow::VdkWindow(wxWindow *handle, long style)
    : VdkStyleAndStateOwner(style, VWST_INITING),
      m_this(handle),
      m_panel(m_this),
      m_hiddenCtrl(NULL),
      m_focus(NULL),
      m_menu(NULL),
      m_trayIcon(NULL),
      m_dragBySpace(false),
      m_tabOrderIter(NULL),
      m_cachedDC(NULL),
      m_postListeners(NULL)
      // 其它成员变量在 Init() 中初始化
      // TODO: 哪些应该在 Init() 中初始化？
{
    wxASSERT(m_this);
    Init();

#ifdef __WXMSW__
    m_this->SetFont(wxEasyCreatFont(L"宋体"));
#endif

    //-----------------------------------------------------------------

    // TODO: 用户改变了这个风格怎么办？
    m_dragBySpace = TestStyle(VWIS_DRAG_BY_SPACE);
}

void VdkWindow::Reset() {
    m_mouseOn.x = m_mouseOn.y = 0;

    //-----------------------------------------------------------------

    if (m_pMouseHoldTimer) {
        m_pMouseHoldTimer->Stop();
        delete m_pMouseHoldTimer;
    }

    if (ms_hovering && ms_hovering->GetVdkWindow() == this) {
        ResetCtrlOnHover();
    }

    ResetCtrlOnFocus();

    VdkMenu *menuOnShow = GetMenuOnShow();
    if (menuOnShow && menuOnShow->GetVdkWindow() == this) {
        HideMenu();
    }

    SetAddinState(VWST_RESET);

    //-----------------------------------------------------------------

    m_Ctrls.DeleteContents(true);
    m_Ctrls.Clear();
    m_Ctrls.DeleteContents(false);

    if (m_tabOrderIter) {
        m_tabOrderIter->Clear();
    }

    m_EventFilters.Clear();

    //-----------------------------------------------------------------

    Init();
}

void VdkWindow::Init() {
    m_nLastCtrlState = NORMAL;
    m_mouseEventNotForMe = false;
    m_nLastMouseEvent = NORMAL;
    m_pMouseHoldTimer = NULL;

    m_nMinWidth = -1;
    m_nMinHeight = -1;
    m_xDcOrigin = 0;
    m_yDcOrigin = 0;
}

#define BindVObjEvent( fx, mid ) m_panel->Bind( wxEVT_VOBJ, &VdkWindow::fx, this, mid )
#define BindToPanel( type, fx ) m_panel->Bind( type, &VdkWindow::fx, this )
#define BindToWindow( type, fx ) m_this->Bind( type, &VdkWindow::fx, this )

void VdkWindow::BindHandlers() {
    // 假如窗体还没初始化完成（尚未绘制第一次），
    // 且不是处于重置窗体的过程之中
    // 绝对不能绑定两次相同的事件处理函数
    if (IsInitializing() && !TestState(VWST_RESET)) {
        // Windows 和 GTK 对键盘事件的处理有所不同。
        // Windows 下面通过 base panel 直接接收键盘事件很不靠谱，因此不使用
        // base panel 来截获按键事件，而是使用一个隐藏的 wxControl 类实例。
        // GTK 能使用隐藏控件的方法来截获按键事件，但是需要改变输入法状态窗口
        // 的位置时却又不正常了，无法解决，只能使用 base panel 这个方法。

#ifndef __WXMSW__
        if (TestStyle(VWS_BASE_PANEL)) {
            m_panel = new wxPanel(m_this, wxID_ANY,
                                  wxDefaultPosition, wxDefaultSize,
                                  wxTAB_TRAVERSAL |
                                  wxNO_BORDER |
                                  wxWANTS_CHARS);

            BindToPanel(wxEVT_KEY_DOWN, OnKeyDown);
            BindToPanel(wxEVT_KEY_UP, OnKeyUp);
            BindToPanel(wxEVT_CHAR, OnChars);

            // 不会出现默认背景
            m_panel->SetBackgroundStyle(wxBG_STYLE_CUSTOM);

            // 避免闪烁
            BindToWindow(wxEVT_ERASE_BACKGROUND, OnEraseBackground);
            BindToWindow(wxEVT_PAINT, OnWindowPaintDummy);
        }
#endif

        //-----------------------------------------------------------------

        m_this->SetBackgroundStyle(wxBG_STYLE_CUSTOM);   // 不会出现默认背景

        BindToPanel(wxEVT_LEFT_DOWN, OnLeftDown);
        BindToPanel(wxEVT_LEFT_DCLICK, OnDLeftDown);
        BindToPanel(wxEVT_LEFT_UP, OnLeftUp);
        BindToPanel(wxEVT_RIGHT_UP, OnRightUp);
        BindToPanel(wxEVT_MOTION, OnMouseMove);
        BindToPanel(wxEVT_MOUSE_CAPTURE_LOST, OnMouseLost);
        BindToPanel(wxEVT_LEAVE_WINDOW, OnLeaveWindow);

#ifdef __WXMSW__
        // 很多时候普通窗口也需要按键事件，如取消上下文菜单的显示
        BindToPanel(wxEVT_KEY_DOWN, OnKeyDown);
        BindToPanel(wxEVT_KEY_UP, OnKeyUp);
#endif

        BindToPanel(wxEVT_ERASE_BACKGROUND, OnEraseBackground);
        BindToPanel(wxEVT_PAINT, OnPaint);

        BindToWindow(wxEVT_MOUSEWHEEL, OnMouseWheel);   // TODO: Why here?
        BindToWindow(wxEVT_SHOW, OnShow);
        BindToWindow(wxEVT_ACTIVATE, OnWindowFocus);
        if (TestStyle(VWS_CLOSE_TO_DESTORY)) {
            BindToWindow(wxEVT_CLOSE_WINDOW, OnClose);
        }

        // 延迟显示菜单
        BindVObjEvent(OnDelayShowContextMenu, VEMC_SHOW_CONTEXT_MENU);

        // 知晓菜单隐藏
        BindVObjEvent(OnMenuHid, VEMC_MENU_HID);
    }
}

#undef BindVObjEvent
#undef BindToPanel
#undef BindToWindow

void VdkWindow::Create(const VdkWindowInitializer &init_data) {
    BindHandlers();

    //-----------------------------------------------------------------

    wxBitmap bkCanvas;
    if (!init_data.bmBkGnd.IsOk()) {
        if (!init_data.strFileName.IsEmpty()) {
            // 位图默认不设置透明色
            VdkUtil::ImRead(bkCanvas, init_data.strFileName);
        } else {
            m_this->SetBackgroundColour(init_data.BgColor);
        }
    } else {
        bkCanvas = init_data.bmBkGnd;
    }

    // 普通边框，没有背景位图
    if (!bkCanvas.IsOk()) {
        m_nMinWidth = init_data.Rect.width;
        m_nMinHeight = init_data.Rect.height;

        if (IsDoubleBuffering()) {
            m_bmBuffered = wxBitmap(init_data.Rect.width,
                                    init_data.Rect.height);
        }

        if (!init_data.Rect.IsEmpty()) {
            m_this->SetSize(init_data.Rect);
        }
    } else {
        m_nMinWidth = bkCanvas.GetWidth();
        m_nMinHeight = bkCanvas.GetHeight();

        m_bkCanvas.Set(bkCanvas, init_data.ResizeType, init_data.TileArea);
        bkCanvas = wxNullBitmap;

        if (!init_data.TileArea.IsEmpty()) {
            if (init_data.bResizeable) {   // nBorder > 0 时才会启用可改变窗口大小
                SetAddinStyle(VWS_RESIZEABLE);
            }

            int w = init_data.Rect.width;
            int h = init_data.Rect.height;

            if (w < m_nMinWidth) {
                w = m_nMinWidth;
            }
            if (h < m_nMinHeight) {
                h = m_nMinHeight;
            }

            Resize(w, h);
        } else {
            RemoveStyle(VWS_RESIZEABLE);
            Resize(m_nMinWidth, m_nMinHeight);
        }

        // 假如 init_data.Rect 为空就不要移动窗口了
        if (!init_data.Rect.IsEmpty() &&
                init_data.Rect.GetPosition() != wxDefaultPosition) {
            m_this->Move(init_data.Rect.x, init_data.Rect.y);
        }

    }
}

VdkWindow *VdkWindow::Create(wxXmlNode *node, MapOfCtrlIdInfo *ids) {
    wxXmlNode *chd = NULL;
    bool resizeable = true;
    wxRect ResizeRect;
    // TODO: 千千静听默认是拉伸
    VdkResizeableBitmapType nResizeType = RESIZE_TYPE_STRETCH;

    // 拉伸属性
    chd = FindChildNode(node, L"resize_rect");
    if (chd) {
        wxString strTmp(chd->GetNodeContent());
        ResizeRect = VdkControl::ParseRect(strTmp, PRT_RECTANGLE);

        if (ResizeRect.width == 0 || ResizeRect.height == 0) {
            resizeable = false;
        } else {
            chd = FindChildNode(node, L"resize_tile");
            if (XmlGetContentOfBoolean(chd, false)) {
                nResizeType = RESIZE_TYPE_TILE;
            }

            chd = FindChildNode(node, L"resizeable");
            if (!XmlGetContentOfBoolean(chd, true)) {
                resizeable = false;
            }
        }
    }

    // 字体属性
    TextInfo info(VdkUtil::GetXrcTextInfo(node, m_this));
    m_this->SetFont(info.font);
    if (m_panel != m_this) {
        m_panel->SetFont(info.font);
    }

    //-----------------------------------------------------------------
    // 初始化窗口

    wxString bgFile(XmlGetChildContent(node, L"image"));
    Create(VdkWindowInitializer().
           fileName(VdkControl::GetFilePath(bgFile)).
           rect(VdkControl::GetXrcRect(node)).
           resizeable(resizeable).
           tileType(nResizeType).
           tileArea(ResizeRect));

    if (TestStyle(VWS_RESIZEABLE)) {
        wxString strTmp(XmlGetChildContent(node, L"min-size"));
        if (!strTmp.IsEmpty()) {
            wxString::size_type pos(strTmp.find(','));
            if (pos != wxString::npos) {
                int w, h;
                w = wxAtoi(strTmp.Mid(0, pos));
                h = wxAtoi(strTmp.Mid(++pos));

                if (w > m_nMinWidth || h > m_nMinHeight) {
                    SetMinSize(w, h);
                }
            }
        }
    }

    //-----------------------------------------------------------------
    // 创建所有控件

    VdkControl::ParseObjects(VdkCtrlParserInfo().
                             window(this).
                             node(node).
                             ids(ids));

    // 补充控件地址
    if (ids) {
        VdkControl **addr = NULL;
        VdkControl *pCtrl = NULL;

        MapOfCtrlIdInfo::iterator i(ids->begin());
        for (; i != ids->end(); ++i) {
            pCtrl = FindCtrl(i->first);

            if (pCtrl) {
                addr = i->second->ptr();
                if (addr && !*addr) {
                    *addr = pCtrl;
                }

                pCtrl->SetID(i->second->id());
            }
        }
    }

    return this;
}

void VdkWindow::BeginExit() {
    // 假如还没初始化照样能自动退出
    SetAddinState(VWST_EXITING);

    // TODO: FindMenu
    if (FindCtrl(ms_hovering)) {
        ResetCtrlOnHover();
    }
}

VdkWindow::~VdkWindow() {
    Reset();

    wxDELETE(m_tabOrderIter);
    wxDELETE(m_trayIcon);
    wxDELETE(m_menu);
    wxDELETE(m_postListeners);
}

void VdkWindow::RecoverCtrl(VdkControl *pCtrl) {
    if (pCtrl) {
        wxPoint mousePos(MEANNINGLESS_MOUSE_COORD, MEANNINGLESS_MOUSE_COORD);
        VdkDC vdc(this, pCtrl->GetAbsoluteRect(), NULL);
        VdkMouseEvent fakeEvent(NORMAL, mousePos, vdc);

        pCtrl->HandleMouseEvent(fakeEvent);

        if (ms_hovering == pCtrl) {
            ResetCtrlOnHover();
        }
    }
}

void VdkWindow::HandleMouseHoldOn(int evtCode,
                                  const wxPoint &Point,
                                  VdkControl *pCtrl) {
    if (evtCode != NORMAL) {
        SetAddinState(VWST_MOUSE_HOLD_ON);

        if (!m_pMouseHoldTimer) {
            m_pMouseHoldTimer = new MouseHoldTimer;
        }

        m_pMouseHoldTimer->Start(pCtrl, evtCode, Point);
    } else if (m_pMouseHoldTimer && m_pMouseHoldTimer->IsRunning()) {
        RemoveState(VWST_MOUSE_HOLD_ON);
        m_pMouseHoldTimer->Stop();

        wxPoint mousePos(wxDefaultPosition);
        VdkDC vdc(this, Rect00(), NULL);
        VdkMouseEvent fakeEvent(MOUSE_HOLD_ON_RELEASED, mousePos, vdc);

        m_pMouseHoldTimer->GetCtrl()->HandleMouseEvent(fakeEvent);
    }
}

void VdkWindow::OnWindowPaintDummy(wxPaintEvent &) {
    wxASSERT(m_panel != m_this);
    wxPaintDC dc(m_this);
}

void VdkWindow::OnPaint(wxPaintEvent &) {
    wxPaintDC pdc(m_panel);
    Draw(pdc);
}

void VdkWindow::Draw(wxDC &pdc) {
    if (TestState(VWST_EXITING)) {
        return;
    }

    // TODO: 是否必须？
    bool isFirstDraw = false;

    //-----------------------------------------------------------------

    if (IsDoubleBuffering()) {
        if (!IsBufferedBitmapOk()) {
            if (m_bkCanvas.IsOk()) {
                m_bkCanvas.Blit(pdc, 0, 0);
            }

            return;
        }

        // 注意：VdkDC vdc 宏必须在 VWST_INITING 标志位被取消后
        // 才会返回正确的 DC 句柄
        if (TestState(VWST_INITING)) {
            isFirstDraw = true;
            RemoveState(VWST_INITING);
        }

        // TODO: 这里为什么要这么干？
        VdkDC vdc(this, Rect00(), &pdc);

        // 假如只是不是第一次绘制，直接从双缓冲位图中复制
        // 因为我们所有的绘图操作都是先在双缓冲位图中进行
        if (!TestStyle(VWS_ALWAYS_REFRESH) &&    // 假如总是需要刷新
                !TestState(VWST_REDRAW_ALL) &&   // 假如不是本次需要刷新
                !TestState(VWST_RESET) &&    // 假如不是重置了窗口
                !isFirstDraw) {
            return;
        }

        DoDraw(vdc);
    } else {
        DoDraw(pdc);
    }

    if (isFirstDraw) {
        if (TestStyle(VWS_FIRST_SHOWN_EVT)) {
            VdkWindowFirstShownEvent *e =
                new VdkWindowFirstShownEvent(this);

            wxQueueEvent(m_this, e);
        }
    }

    RemoveState(VWST_INITING | VWST_RESET | VWST_REDRAW_ALL);
}

void VdkWindow::DoDraw(wxDC &dc) {
    EraseBackground(dc, Rect00());

    // 派生类可以对重画事件作出响应
    DoPaint(dc);

    // 当快速改变 VdkWindow 的大小时，滚动条手柄可能会覆盖掉“向下滚动”按钮，
    // 所以从后往前画，先画手柄，再画按钮。奇怪
    VdkControl *pCtrl(NULL);
    VdkCtrlList::reverse_iterator i, rEnd(m_Ctrls.rend());
    for (i = m_Ctrls.rbegin(); i != rEnd; ++i) {
        pCtrl = *i;

        if (!pCtrl->IsShown() || pCtrl->GetParent()) {
            continue;
        }

        pCtrl->Draw(dc);
    }
}

void VdkWindow::QueueRedrawEvent() {
    SetAddinState(VWST_REDRAW_ALL);
    m_this->Refresh(false);
    m_this->Update();
}

void VdkWindow::OnShow(wxShowEvent &e) {
#if 0
    VdkNotify notice(e.IsShown() ? VCN_PARENT_SHOW : VCN_PARENT_HID);

    VdkCtrlIter it(m_Ctrls.begin()), itEnd(m_Ctrls.end());
    for (; it != itEnd; ++it) {
        (*it)->HandleNotify(notice);
    }
#endif

    e.Skip(true);
}

void VdkWindow::OnIconize(wxIconizeEvent &) {
    // 我们需要还原最小化按钮的正常状态
    wxMouseEvent e(wxEVT_LEAVE_WINDOW);
    e.SetX(-1);
    e.SetY(-1);

    OnLeaveWindow(e);
}

void VdkWindow::OnMaximize(wxMaximizeEvent &) {
    SetAddinState(VWST_MAXIMIZING);
}
#if 0
void VdkWindow::OnSize(wxSizeEvent &e) {
    wxSize size(e.GetSize());

    // 假如是窗体大小与背景位图大小不同还是要调用 Resize
    if (!TestState(VWST_INITING) &&
            m_bkCanvas.IsOk() && // 假如不存在背景位图，则无须响应改变大小事件
            size != m_bkCanvas.GetSize()) {  // 目的大小与当前背景位图大小不符
        if (!Resize(size.x, size.y)) {
            if (IsZoomed()) {
                // 提交更改
                wxRect rc(wxDisplay().GetClientArea());
                DoResize(rc.width, rc.height);

                m_this->SetMaxClientSize(rc.GetSize());

                RemoveState(VWST_MAXIMIZING);
            }
        }
    } else {
        if (!IsBufferedBitmapOk()) {
            m_bmBuffered = wxBitmap(size.x, size.y);
            SetAddinState(VWST_REDRAW_ALL);
        }
    }
}
#lse
void VdkWindow::OnSize(wxSizeEvent &e) {

}
#endif

void VdkWindow::OnWindowFocus(wxActivateEvent &e) {
    if (!e.GetActive()) {
        if (GetMenuOnShow()) {
            // 菜单窗口本身不应该接受 LOSTFOCUS 事件
            wxASSERT(!IsMenuImpl());
            // 得到焦点的窗口不能就是当前正在显示的菜单实体实现窗体自身
            wxASSERT(wxWindow::FindFocus() !=
                     (wxWindow *)GetMenuOnShow()->GetImpl());

            HideMenu();
        }
    }
#ifdef __WXMSW__
#if 0
    else {
        // 窗口从失活状态进入激活状态不会自动激活面板，
        // 导致面板上的 VdkEdit 无法获得输入焦点
        if (m_this != m_panel) {
            m_panel->SetFocus();
        }
    }
#endif
#endif // __WXMSW__
}

void VdkWindow::OnClose(wxCloseEvent &) {
    // 假如是菜单窗体，取消操作
    if (IsMenuImpl()) {
        return;
    }

    BeginExit();
    m_this->Destroy();
}

void VdkWindow::OnLeftDown(wxMouseEvent &e) {
    HandleMouseEvent(e, LEFT_DOWN);

    // 保存相对窗口左上角的位置，用于模拟拖动窗口
    SetAddinState(VWST_LEFT_DOWN_SKIPPED);
    m_mouseOn = e.GetPosition();

    SetLastMouseEvent(LEFT_DOWN);

#   ifdef __WXMSW__
    if (!m_panel->HasCapture()) {
        m_panel->CaptureMouse();
    }
#   endif // __WXMSW__
}

inline void VdkWindow::OnLeftUp(wxMouseEvent &e) {
    HandleMouseEvent(e, LEFT_UP);
    SetLastMouseEvent(LEFT_UP);

#   ifdef __WXMSW__
    if (m_panel->HasCapture()) {
        m_panel->ReleaseMouse();
    }
#   endif // __WXMSW__
}

inline void VdkWindow::OnDLeftDown(wxMouseEvent &e) {
    HandleMouseEvent(e, DLEFT_DOWN);
    SetLastMouseEvent(DLEFT_DOWN);
}

inline void VdkWindow::OnRightUp(wxMouseEvent &e) {
    HandleMouseEvent(e, RIGHT_UP);
    SetLastMouseEvent(RIGHT_UP);
}

void VdkWindow::OnMouseMove(wxMouseEvent &e) {
    // 不知道为什么当在窗体中点击一下激活窗体时会发送
    // 上次失活时的一个 MOVING 鼠标事件
    if (e.GetPosition() != m_this->ScreenToClient(wxGetMousePosition())) {
        return;
    }

    if (e.Dragging() && e.LeftIsDown()) {
        // 最大化后不允许再拖动
        if (!HandleMouseEvent(e, DRAGGING) &&
                TestStyle(VWS_DRAGGABLE) &&
                (m_nLastMouseEvent == LEFT_DOWN ||
                 m_nLastMouseEvent == DRAGGING)) {
            EmulateDragAndMove(m_this->ClientToScreen(e.GetPosition()));
        }

        SetLastMouseEvent(DRAGGING);
        return;
    }

    HandleMouseEvent(e, HOVERING);
    SetLastMouseEvent(HOVERING);
}

void VdkWindow::OnMouseWheel(wxMouseEvent &e) {
    if (e.m_wheelRotation < 0) {
        HandleMouseEvent(e, WHEEL_DOWN);
        SetLastMouseEvent(WHEEL_DOWN);
    } else {
        HandleMouseEvent(e, WHEEL_UP);
        SetLastMouseEvent(WHEEL_UP);
    }
}

void VdkWindow::OnLeaveWindow(wxMouseEvent &e) {
    // 不管什么情况，只要鼠标指针离开窗口，就还原鼠标指针为默认箭头，
    // 重入时再行设置。但当拖拉边框改变窗口大小时不要重置。
    if (!wxGetMouseState().LeftIsDown()) {
        ResetCursor();
    }

    if ((m_nLastCtrlState == DRAGGING) ||  // 拖动事件时指针移出窗口
            // 接收到的“鼠标离开窗口”事件后于“进入其他窗口”事件被处理，舍弃
            (ms_hovering && (ms_hovering->GetVdkWindow() != this))) {
        return;
    }

    HandleMouseEvent(e, MOUSE_LEAVE_WINDOW);
    SetLastMouseEvent(MOUSE_LEAVE_WINDOW);
}

void VdkWindow::OnMouseLost(wxMouseCaptureLostEvent &) {}

void VdkWindow::OnKeyDown(wxKeyEvent &e) {
    HandleKeys(KEY_DOWN, e);
}

void VdkWindow::OnKeyUp(wxKeyEvent &e) {
    HandleKeys(KEY_UP, e);
}

void VdkWindow::OnChars(wxKeyEvent &e) {
    HandleKeys(KEY_CHAR, e);
}

void VdkWindow::HandleKeys(VdkKeyEventType type, wxKeyEvent &e) {
    // 先分派给派生类处理
    if (DoHandleKeyEvent(e)) {
        return;
    }

    bool isMenuOnShow = (GetMenuOnShow() != NULL);
    int keyCode = e.GetKeyCode();

    // 处理“Windows context menu”按键，实际是打开右键菜单
    if (!isMenuOnShow && (keyCode == WXK_WINDOWS_MENU) && (type == KEY_UP)) {
        wxMouseEvent fakeEvent(wxEVT_RIGHT_UP);
        fakeEvent.SetRightDown(true);

        // 假如有焦点控件，由焦点控件来处理，否则由窗口来处理
        wxPoint pos(0, 0);
        if (m_focus) {
            pos = m_focus->GetAbsoluteRect().GetPosition();
        }

        fakeEvent.SetPosition(pos);

        //----------------------------------------------

        OnRightUp(fakeEvent);

        // Comsume it.
        return;
    }

    // 处理键盘导航
    if (!isMenuOnShow && m_tabOrderIter &&
            (type == KEY_DOWN) && (keyCode == WXK_TAB) &&
            (!m_focus || !m_focus->TestStyle(VCS_WANTS_ALL_CHARS))) {
        if (m_tabOrderIter->FocusNext(m_focus)) {
            return;
        }
    }

    bool skip = (type == KEY_DOWN);
    if (!isMenuOnShow && m_focus && m_focus->TestStyle(VCS_KEY_EVENT)) {
        VdkDC vdc(this, Rect00(), NULL);
        VdkKeyEvent vke(type, e, vdc);

        m_focus->HandleKeyEvent(vke);
        skip = (vke.GetSkipped() || e.GetSkipped());
    }

    if (skip) {
        VdkMenu *menuOnShow = GetMenuOnShow();
        VdkWindow *win = menuOnShow ? menuOnShow->GetVdkWindowImpl() : this;

        // 使用 ESC 关闭窗口/菜单
        if ((type == KEY_DOWN) && keyCode == WXK_ESCAPE &&
                win->TestStyle(VWS_DISMISS_BY_ESC) &&
                // 使用 ESC 取消输入法状态窗口
                !IsImeWindowVisiable(win->m_this)) {
            // Hide() or Close() ?
            // 可能是 Close() 比较合适，毕竟很多时候对话框都是“一次性”的
            if (!menuOnShow) {
                m_this->Close();
            } else {
                menuOnShow->Return();
            }

            // Eat this event.
            return;
        }

        e.Skip(true);
    }
}

void VdkWindow::OnWindowMenuKeyUp(wxMouseEvent &e) {
    // 不要重复显示菜单
    if (GetMenuOnShow()) {
        return;
    }

    // 假如有焦点控件，由焦点控件来处理，否则由窗口来处理
    wxPoint pos(0, 0);
    if (m_focus) {
        pos = m_focus->GetAbsoluteRect().GetPosition();
    }

    e.SetPosition(pos);

    //----------------------------------------------

    OnRightUp(e);
}

void VdkWindow::PopEventFilter(VdkEventFilter *filter, bool del) {
    wxASSERT(filter);

    m_EventFilters.remove(filter);
    if (del) {
        delete filter;
    }
}

bool VdkWindow::FilterEvent(int evtCode, const wxMouseEvent &e) {
    VdkEventFilter *filter;
    EventFilterIter i(m_EventFilters.begin());
    for (; i != m_EventFilters.end(); ++i) {
        filter = *i;

        if (filter->IsActive()) {
            if (filter->FilterEvent(VdkEventFilter::EventForFiltering
                                    (this, evtCode).mouseEvent(e))) {
                return true;
            }
        }
    }

    return false;
}

#if 0
#define DC_INSTANCE_HERE() \
    wxLogDebug( L"[%s:%03d]new VdkDC instance.", \
                m_debugCaption, __LINE__ );\
    VdkDC dc( this, Rect00(), NULL )
#else
#define DC_INSTANCE_HERE() \
    VdkDC dc( this, Rect00(), NULL )
#endif

#if 0
#define RETURN( ret ) \
{\
    if( evtCode != HOVERING ) \
    {\
        wxLogDebug( L"[%s]HandleMouseEvent: %s(#%d)", m_debugCaption, \
                    GetEventName( evtCode ), __LINE__ );\
    }\
    \
    return ret;\
}
#else
#define RETURN( ret ) \
    return ret
#endif

bool VdkWindow::HandleMouseEvent(wxMouseEvent &evt, VdkMouseEventType evtCode) {
    // 现时返回值仅在判断拖动窗口时使用
    if (TestState(VWST_INITING) || TestState(VWST_EXITING)) {
        RETURN(true);
    }

    // 处理内部事件过滤器（派生类）
    if (FilterEventBefore(evt, evtCode)) {
        RETURN(true);
    }

    // 处理外部事件过滤器（控件,etc.）
    if (!m_EventFilters.empty()) {
        if (FilterEvent(evtCode, evt)) {
            RETURN(true);
        }
    }

    // 右键菜单是否在显示
    if (GetMenuOnShow() && !IsMenuImpl()) {
#ifdef __WXMSW__
        // 在 wxMSW 下面，当一个菜单在显示时，发生的滚轮事件会被
        // 发送到菜单实体窗体的父窗口的消息队列中
        if (evtCode == WHEEL_UP || evtCode == WHEEL_DOWN) {
            VdkMenu *menu = GetMenuOnShow()->GetLastShownMenu();
            VdkWindow *menuImpl = menu->GetVdkWindowImpl();

            wxASSERT(menuImpl != this);
            if (menuImpl) {
                wxGetMousePosition(&evt.m_x, &evt.m_y);
                menuImpl->GetHandle()->ScreenToClient(&evt.m_x, &evt.m_y);

                // 当然，鼠标指针需要在这个活动菜单实体窗体里
                if (menuImpl->Rect00().Contains(evt.m_x, evt.m_y)) {
                    m_mouseEventNotForMe = true;
                    menuImpl->HandleMouseEvent(evt, evtCode);

                    RETURN(true);
                }
            }
        }
#endif //__WXMSW__

        // 鼠标按键事件是其他一切有意义事件的基础，因此只需要
        // 截取左右键按下的事件，在其中取消菜单的显示即可。
        if (IsActivatableEvent(evtCode)) {
            HideMenu();
        } else { // 拦截其他一切事件
            RETURN(true);
        }
    }

    //----------------------------------------------------------

    wxPoint mousePos(evt.GetPosition());

    //----------------------------------------------------------
    // 拖动改变窗口大小

    switch (evtCode) {
    case HOVERING: {
        if (TestStyle(VWS_RESIZEABLE)) {
            // 不知道为什么系统会在拖动的时候发送鼠标移动事件，
            // 偏偏我的程序检测不到鼠标是位于窗口边界上的
            if (TestState(VWST_DRAG_AND_RESIZING) &&
                    wxGetMouseState().LeftIsDown()) {
                RETURN(false);
            }

            HandleRzCursor(mousePos);
        }

        break;
    }

    case DRAGGING: {
        if (TestState(VWST_DRAG_AND_RESIZING)) {
            if (GetMenuOnShow()) {
                HideMenu();
            }

            EmulateDragAndResize(mousePos);
            RETURN(true);
        } else if (TestState(VWST_DRAG_AND_MOVING)) {
            RETURN(false);
        }

        break;
    }

    case LEFT_UP: {
        if (TestState(VWST_DRAG_AND_RESIZING)) {
            ResetRzCursor();

            // 这里不能直接 RETURN(true) ，否则在一个靠近边缘
            // 的控件上按下鼠标，控件改变了其自身状态，却无法
            // 获取按键释放的事件。
        }

        RemoveState(VWST_DRAG_AND_MOVING);

        break;
    }

    default:

        break;
    }

    //----------------------------------------------------------
    // 查找 VdkControl

    VdkCtrlList::reverse_iterator iContainer(m_Ctrls.rend()),   // 可能改变
                rEnd(iContainer);
    wxRect rect;
    VdkControl *pCtrl;

    // 假如不是主窗口失去焦点或者将鼠标移出主窗口
    if (mousePos.x != MEANNINGLESS_MOUSE_COORD) {
        // 假如上次事件时正在拖动 VdkSlider ，继续发送到那个 VdkSlider
        if (evtCode == DRAGGING &&
                ms_hovering &&
                !TestState(VWST_MOUSE_HOLD_ON)) {
            pCtrl = ms_hovering;
            VdkWindow *win(pCtrl->GetVdkWindow());

            if (this != win) {
                wxPoint Another(win->m_this->GetScreenPosition());
                wxPoint This(m_this->GetScreenPosition());

                mousePos.x += This.x - Another.x;
                mousePos.y += This.y - Another.y;

                m_mouseEventNotForMe = true;
                win->HandleMouseEvent(evt, evtCode);

                RETURN(true);   // 这个事件是不属于我们的
            }
        } else {
            bool bContains(false);

            VdkCtrlList::reverse_iterator i;
            for (i = m_Ctrls.rbegin(); i != rEnd; ++i) {
                pCtrl = *i;

                if (!pCtrl->CanHandleEvent() ||
                        pCtrl->IsCtrlHandler()) {
                    continue;
                }

                bContains = pCtrl->HitTest(mousePos);
                if (bContains) {
                    // 假如是 container，继续查找其他控件，但保留 container 以备用
                    if (pCtrl->TestStyle(VCS_CTRL_CONTAINER)) {
                        // 只保留第一次找到的那个 container
                        if (iContainer == rEnd) {
                            iContainer = i;
                        }

                        continue;
                    } else if (pCtrl->TestStyle(VCS_IGNORE_ALL_EVENTS)) {
                        continue;
                    } else {
                        break;
                    }
                }
            } // end for

            // 不直接返回是因为可能要处理没有还原的
            // 前一个 VdkControl
            if (i == rEnd) {
                pCtrl = NULL;
            }
        }

        //----------------------------------------------------------

        // 持续按住鼠标事件
        // <---不要在线程里面启动或者停止一个 Timer ！---->
        if (TestState(VWST_MOUSE_HOLD_ON)) {
            // 在持续按住的同一个 VdkControl 里允许拖动指针
            if (evtCode == DRAGGING && pCtrl == ms_hovering) {
                RETURN(true);
            }

            HandleMouseHoldOn(NORMAL, mousePos, NULL);
            // 返回是避免再次破坏状态
            RETURN(true);
        }
    }

    // 必须同时考虑 Container 类型的控件，
    // 因为它们也可能被作为当前具有悬浮状态的控件
    VdkControl *pCtrl2 = pCtrl;
    if (!pCtrl2 && iContainer != rEnd) {
        pCtrl2 = *iContainer;
    }

    // 是否是相同的 VdkControl
    if (pCtrl2 &&
            evtCode == HOVERING &&
            pCtrl2 == ms_hovering &&
            pCtrl2->TestStyle(VCS_ONESHOT_HOVERING)) {
        RETURN(true);
    }

    //----------------------------------------------------------

    // 还原上一个 VdkControl
    if (ms_hovering && pCtrl2 != ms_hovering &&
            ms_hovering->CanHandleEvent()) {  // 诸如控件被禁用，不要发送还原事件
        // 处理当 pCtrl 属于另一个窗口的情况
        VdkWindow *lastwindow(ms_hovering->GetVdkWindow());

        if (lastwindow != this) {
            lastwindow->RecoverCtrl(ms_hovering);
            goto PROCESSING_EVENTS;
        }

        VdkControl *hovering = ms_hovering;
        ResetCtrlOnHover();
        m_nLastCtrlState = NORMAL;

        DC_INSTANCE_HERE();
        VdkMouseEvent e(NORMAL, mousePos, dc);
        hovering->HandleMouseEvent(e);
    }

    // 处理具有输入焦点的控件
    if (IsActivatableEvent(evtCode) &&
            pCtrl2 && // 在窗口空白处点击不失活当前焦点控件
            m_focus != pCtrl2) {
        DC_INSTANCE_HERE();
        FocusCtrl(pCtrl2, &dc, mousePos.x, mousePos.y);
    }

    //----------------------------------------------------------
    // 真正开始处理事件

PROCESSING_EVENTS:

    if (pCtrl) {   // 真的在 VdkControl 的区域里，并且不是在调整窗口大小
        m_nLastCtrlState = evtCode;
        SetCtrlOnHover(pCtrl);

        DC_INSTANCE_HERE();
        VdkMouseEvent e(evt, evtCode, dc);

        pCtrl->HandleMouseEvent(e);

        // TODO: 是否应该重新找另一个符合要求的 VdkControl 呢？
        if (e.GetSkipped()) {
            goto FILTEREVENTAFTER;
        }

        // 执行的是一个关闭/重置窗口的命令
        if (TestState(VWST_EXITING) || TestState(VWST_RESET)) {
            RETURN(true);
        }
    } else if (evtCode == RIGHT_UP && m_menu) {   // 显示与本 VdkWindow 相关联的菜单
        if (GetMenuOnShow()) {
            HideMenu();
        }

        ShowContextMenu(NULL, mousePos.x, mousePos.y);

        RETURN(true);
    }
    // 纯粹在窗口空白处上移动
    else if (iContainer == rEnd) {
        ResetCtrlOnHover();
        m_nLastCtrlState = NORMAL;
    }

    if (pCtrl2) {
        // 这段代码是为了 container 而设计的
        if ((evtCode == LEFT_DOWN || evtCode == DLEFT_DOWN) &&
                pCtrl2->TestStyle(VCS_HONLD_ON)) {
            // 持续按住按钮
            HandleMouseHoldOn(LEFT_DOWN, mousePos, pCtrl2);
        }
        // 将事件导向 container
        else if (pCtrl2->TestStyle(VCS_CTRL_CONTAINER) && iContainer != rEnd) {
            DC_INSTANCE_HERE();
            VdkMouseEvent e(evt, evtCode, dc);

            pCtrl2->HandleMouseEvent(e);

            if (e.GetSkipped()) {
                goto FILTEREVENTAFTER;
            } else {
                // 即使是 container 也要保持好状态
                // 例如直接拖动 VdkSlider ，而不是拖动手柄，
                // 就一定要能持续拖动（鼠标离开滚动条作用域后不能
                // 取消状态）
                SetCtrlOnHover(pCtrl2);
                m_nLastCtrlState = evtCode;
            }
        }

    } else if (!GetMenuOnShow()) {   // 纯粹在窗口空白处上移动
FILTEREVENTAFTER:
        if (!FilterEventAfter(evt, evtCode)) {     // 不可在窗体空白上拖动窗口
            RETURN(!CanDragBySpace());
        }
    }

    RETURN(true);
}

void VdkWindow::EmulateDragAndMove(const wxPoint &screen) {
    SetAddinState(VWST_DRAG_AND_MOVING);

    wxCoord x = screen.x - m_mouseOn.x;
    wxCoord y = screen.y - m_mouseOn.y;

#ifdef __WXMSW__
    m_this->Move(x, y);
#elif defined( __WXGTK__ )
    MoveWindowSynchronously(m_this, x, y);
#endif
}

void VdkWindow::OnDelayShowContextMenu(VdkVObjEvent &e) {
    ShowContextMenu(e.GetCtrl(), e.GetInt(), e.GetExtraLong(), false);
}

void VdkWindow::ShowContextMenu(VdkControl *pCtrl, int x, int y, bool delay) {
    if (delay) {
        VdkVObjEvent e(VEMC_SHOW_CONTEXT_MENU);
        e.SetCtrl(pCtrl);
        e.SetInt(x);
        e.SetExtraLong(y);

        wxPostEvent(m_panel, e);
        return;
    }

    VdkMenu *menu(NULL);

    if (pCtrl) {
        menu = pCtrl->GetMenu();
        pCtrl->SetMenuState(true);
    } else if (!pCtrl) {
        menu = m_menu;
        if (!menu) {
            return;
        }
    }

    // 不知道为什么不能移到 ShowContext 上面
    SetMenuOnShow(menu);

    m_this->ClientToScreen(&x, &y);
    menu->ShowContext(x, y, pCtrl);
}

/*static*/
void VdkWindow::HideMenu() {
    VdkMenu *menuOnShow = GetMenuOnShow();
    if (menuOnShow) {
        // m_MenuOnShow = NULL 在 OnMenuHid 里进行
        menuOnShow->Return();
    }
}

void VdkWindow::OnMenuHid(VdkVObjEvent &e) {
    // 一定要进行判断！
    if (GetMenuOnShow() == e.GetMenu()) {
        SetMenuOnShow(NULL);
    }

    //----------------------------------------------------------

    VdkControl *pCtrl = e.GetCtrl();
    if (pCtrl) {
        VdkDC dc(this, Rect00(), NULL);
        pCtrl->SetMenuState(false, &dc);
    }
}

void VdkWindow::AssignCursor(const wxCursor &cursor) {
#ifdef __WXMSW__
    if (m_cursor.GetHCURSOR() == cursor.GetHCURSOR()) {
        return;
    }
#endif

    m_cursor = cursor;
    m_panel->SetCursor(m_cursor);
}

void VdkWindow::ResetCursor() {
#ifdef __WXMSW__
    if (m_cursor.GetHCURSOR() == wxSTANDARD_CURSOR->GetHCURSOR()) {
        return;
    }
#endif

    m_cursor = *wxSTANDARD_CURSOR;
    m_panel->SetCursor(m_cursor);
}

void VdkWindow::SetTrayIcon(TrayIcon *trayIcon) {
    if (m_trayIcon) {
        delete m_trayIcon;
    }
    m_trayIcon = trayIcon;
}

wxTopLevelWindow *VdkWindow::GetTopLevelWindowHandle() const {
    if (m_this->IsTopLevel()) {
        return dynamic_cast<wxTopLevelWindow *>(m_this);
    }

    return NULL;
}

void VdkWindow::SetMinSize(int w, int h) {
    wxASSERT(w >= m_bkCanvas.GetMinSize().x &&
             h >= m_bkCanvas.GetMinSize().y);

    m_nMinWidth = w;
    m_nMinHeight = h;

    //----------------------------------------------------------

    int ww, wh;
    m_panel->GetSize(&ww, &wh);
    if ((ww < m_nMinWidth) || (wh < m_nMinHeight)) {
        Resize(wxMax(m_nMinWidth, ww),
               wxMax(m_nMinHeight, wh));
    }
}

wxSize VdkWindow::GetMinSize() const {
    wxSize size;
    GetMinSize(&size.x, &size.y);

    return size;
}

void VdkWindow::GetMinSize(int *w, int *h) const {
    // 这里为什么要弄得这么复杂呢，因为对于一些窗体背景图片，虽然比实际的呈现
    // 要小，但它缩放至所需的大小之后是不需要再改变窗体大小的了。所以得到最小
    // 最小的大小并无意义。因为 XRC 文件中的控件大小、位置定义是根据缩放之后
    // 的目标窗口的大小来放置控件位置的，所以我们返回当前窗口大小。
    if (TestStyle(VWS_RESIZEABLE)) {
        if (w) {
            *w = m_nMinWidth;
        }
        if (h) {
            *h = m_nMinHeight;
        }
    } else {
        m_panel->GetSize(w, h);
    }
}

//////////////////////////////////////////////////////////////////////////
// 原 VdkWindowResizer 的代码
//////////////////////////////////////////////////////////////////////////

void VdkWindow::HandleRzCursor(const wxPoint &mousePos) {
    wxASSERT_MSG(TestStyle(VWS_RESIZEABLE), L"窗口必须具有 VWS_RESIZEABLE 风格！");

    wxStockCursor cursorType(wxCURSOR_NONE);
    m_rectCached = m_this->GetScreenRect();

    int x = mousePos.x, y = mousePos.y;
    int minX = 0,
        minY = 0,
        maxX = m_rectCached.width,
        maxY = m_rectCached.height;

    enum {
        GS_WINDOW_BORDER = 5, // 鼠标指针移动到这些 Border 组成的区域时会改变
    };

    m_dragAndResizeType = 0;

    if ((x <= minX + GS_WINDOW_BORDER) && (x >= minX)) {
        m_dragAndResizeType |= vdkWEST;
    }

    if ((x >= maxX - GS_WINDOW_BORDER) && (x <= maxX)) {
        m_dragAndResizeType |= vdkEAST;
    }

    if ((y <= minY + GS_WINDOW_BORDER) && (y >= minY)) {
        m_dragAndResizeType |= vdkNORTH;
    }

    if ((y >= maxY - GS_WINDOW_BORDER) && (y <= maxY)) {
        m_dragAndResizeType |= vdkSOUTH;
    }

    bool north = (m_dragAndResizeType & vdkNORTH) != 0;
    bool west = (m_dragAndResizeType & vdkWEST) != 0;
    bool east = (m_dragAndResizeType & vdkEAST) != 0;
    bool south = (m_dragAndResizeType & vdkSOUTH) != 0;

    if ((north && west) || (east && south)) {
        cursorType = wxCURSOR_SIZENWSE;
    } else if ((north && east) || (west && south)) {
        cursorType = wxCURSOR_SIZENESW;
    } else if (west || east) {
        cursorType = wxCURSOR_SIZEWE;
    } else if (north || south) {
        cursorType = wxCURSOR_SIZENS;
    }

    if (m_dragAndResizeType != 0) {
        wxASSERT(cursorType != wxCURSOR_NONE);

        AssignCursor(wxCursor(cursorType));
        SetAddinState(VWST_DRAG_AND_RESIZING);
    } else {
        ResetRzCursor();
    }
}

void VdkWindow::ResetRzCursor() {
    if (TestState(VWST_DRAG_AND_RESIZING)) {
        RemoveState(VWST_DRAG_AND_RESIZING);
        m_dragAndResizeType = 0;

        ResetCursor();
    }
}

void VdkWindow::EmulateDragAndResize(const wxPoint &mousePosClient) {
    // 不接收突如其来的拖动事件
    if ((m_nLastMouseEvent != LEFT_DOWN) &&
            (m_nLastMouseEvent != DRAGGING)) {
        return;
    }

    // 对于 GTK 下的异型窗口，不能将窗口移出屏幕范围，对于所有 GTK 窗口，
    // 不能用 Move() 将其移出屏幕：奇怪的规定

#ifdef __WXGTK__
    // 不能太频繁改变窗口大小，我们取 Windows 的值：最低 40 ms
    wxMilliClock_t nTimeStamp = wxGetLocalTimeMillis();
    if ((nTimeStamp - m_nLastTimeStamp) < 40) {
        return;
    }
#endif

    wxRect rcNew(m_this->GetScreenRect());
    wxPoint mousePosScreen(m_this->ClientToScreen(mousePosClient));

    if (m_dragAndResizeType & vdkEAST) {
        rcNew.width = mousePosScreen.x - m_rectCached.x;
    }

    if (m_dragAndResizeType & vdkWEST) {
        rcNew.x = mousePosScreen.x;
        rcNew.width = RightOf(m_rectCached) - rcNew.x;
    }

    if (m_dragAndResizeType & vdkNORTH) {
        rcNew.y = mousePosScreen.y;
        rcNew.height = BottomOf(m_rectCached) - rcNew.y;
    }

    if (m_dragAndResizeType & vdkSOUTH) {
        rcNew.height = mousePosScreen.y - m_rectCached.y;
    }

    if (rcNew.width < m_nMinWidth) {
        rcNew.width = m_nMinWidth;

        // 这时候 rc.x 已然越过窗口的左边框，窗口不能向右移
        if (m_dragAndResizeType & vdkWEST) {
            rcNew.x = RightOf(m_rectCached) - rcNew.width;
        }
    }

    if (rcNew.height < m_nMinHeight) {
        rcNew.height = m_nMinHeight;

        // 这时候 rc.y 已然越过窗口的上边框，窗口不能向下移
        if (m_dragAndResizeType & vdkNORTH) {
            rcNew.y = BottomOf(m_rectCached) - rcNew.height;
        }
    }

    Resize(rcNew);
#ifdef __WXGTK__
    m_nLastTimeStamp = wxGetLocalTimeMillis();
#endif
}

#if 1
bool VdkWindow::Resize(int w, int h) {
    return Resize(wxDefaultCoord, wxDefaultCoord, w, h, wxSIZE_USE_EXISTING);
}

bool VdkWindow::Resize(int x, int y, int w, int h, int sizeFlags) {
    /*
        这里有个非常奇怪的逻辑。
        SetSize 遇到的情形有几个类型：
        1、不可更改大小，提供相同大小的背景位图；
        2、不可更改大小，提供小于要求大小的背景位图；
        3、可更改大小

        有一个原则就是，一定要保证 m_bmBuffered 是有效的。所以最后加了一个判断。
    */

    bool resizeable = TestStyle(VWS_RESIZEABLE);

    if (m_bkCanvas.CanResize()) {
        // 这个特性在程序初始化时也能用（ wxWidgets 会给窗口以默认大小），
        // 因为此时还没有创建任何控件，故调用 LayoutWidgets 对程序无影响
        int ww, hh;
        m_this->GetSize(&ww, &hh);

        if (w != 0 && w < m_nMinWidth) {
            w = m_nMinWidth;
        }

        if (h != 0 && h < m_nMinHeight) {
            h = m_nMinHeight;
        }

        if (w == 0) {
            w = ww;
        }
        if (h == 0) {
            h = hh;
        }

        int dX, dY;
        dX = w - m_bkCanvas.GetWidth();
        dY = h - m_bkCanvas.GetHeight();

        // 是否需要调整背景位图的大小
        if (dX || dY) {
            if (!m_bkCanvas.Rescale(wxSize(w, h))) {
                return false;
            }

            LayoutWidgets(dX, dY);
        }
    } else if (resizeable) {
        RemoveStyle(VWS_RESIZEABLE);
        resizeable = false;

        return false;
    }

    // 提交更改
    DoResize(x, y, w, h, sizeFlags);

    if (!IsBufferedBitmapOk()) {
        int buffw(resizeable ? w * 2 : w),
            buffh(resizeable ? h * 2 : h), disx, disy;

        wxDisplaySize(&disx, &disy);

        if (buffw > disx) {
            buffw = disx;
        }
        if (buffh > disy) {
            buffh = disy;
        }

        m_bmBuffered = wxBitmap(buffw, buffh);
    }

    RemoveState(VWST_MAXIMIZING);   // TODO:
    SetAddinState(VWST_REDRAW_ALL);   // 全部重画

    return true;
}
#else
void VdkWindow::OnSize(wxSizeEvent &e) {
    wxSize newSize(e.GetSize());

    /*
        这里有个非常奇怪的逻辑。
        SetSize 遇到的情形有几个类型：
        1、不可更改大小，提供相同大小的背景位图；
        2、不可更改大小，提供小于要求大小的背景位图；
        3、可更改大小

        有一个原则就是，一定要保证 m_bmBuffered 是有效的。所以最后加了一个判断。
    */

    bool resizeable = TestStyle(VWS_RESIZEABLE);

    if (m_bkCanvas.CanResize()) {
        // 这个特性在程序初始化时也能用（ wxWidgets 会给窗口以默认大小），
        // 因为此时还没有创建任何控件，故调用 LayoutWidgets 对程序无影响
        int ww, hh;
        m_this->GetSize(&ww, &hh);

        if ((w != 0) && (w < m_nMinWidth)) {
            w = m_nMinWidth;
        }

        if ((h != 0) && h < m_nMinHeight) {
            h = m_nMinHeight;
        }

        if (w == 0) {
            w = ww;
        }
        if (h == 0) {
            h = hh;
        }

        int dX, dY;
        dX = w - m_bkCanvas.GetWidth();
        dY = h - m_bkCanvas.GetHeight();

        // 是否需要调整背景位图的大小
        if (dX || dY) {
            if (!m_bkCanvas.Rescale(wxSize(w, h))) {
                return false;
            }

            LayoutWidgets(dX, dY);
        }
    } else if (resizeable) {
        RemoveStyle(VWS_RESIZEABLE);
        resizeable = false;

        return false;
    }

    // 提交更改
    DoResize(x, y, w, h, sizeFlags);

    if (!IsBufferedBitmapOk()) {
        int buffw(resizeable ? w * 2 : w),
            buffh(resizeable ? h * 2 : h), disx, disy;

        wxDisplaySize(&disx, &disy);

        if (buffw > disx) {
            buffw = disx;
        }
        if (buffh > disy) {
            buffh = disy;
        }

        m_bmBuffered = wxBitmap(buffw, buffh);
    }

    RemoveState(VWST_MAXIMIZING);
    SetAddinState(VWST_REDRAW_ALL);   // 全部重画
}
#endif

bool VdkWindow::IsBufferedBitmapOk() {
    if (!IsDoubleBuffering()) {
        return true;
    }

    wxCoord w, h;
    m_this->GetSize(&w, &h);

    return m_bmBuffered.IsOk() &&
           (m_bmBuffered.GetWidth() >= w) &&
           (m_bmBuffered.GetHeight() >= h);
}

void VdkWindow::LayoutWidgets(int dX, int dY) {
    VdkControl *pCtrl;
    CtrlIter walker(m_Ctrls.begin()), e(m_Ctrls.end());

    for (; walker != e; ++walker) {
        pCtrl = *walker;
        if (pCtrl->GetParent()) {
            continue;
        }

        pCtrl->HandleResize(dX, dY);
    }
}

//////////////////////////////////////////////////////////////////////////
// 原 VdkCtrlContainer 的代码
//////////////////////////////////////////////////////////////////////////

bool VdkWindow::RemoveCtrl(VdkControl *pCtrl) {
    if (m_Ctrls.DeleteObject(pCtrl)) {
        if (m_tabOrderIter) {
            m_tabOrderIter->RemoveCtrl(pCtrl);
        }

        if (ms_hovering == pCtrl) {
            ResetCtrlOnHover();
        }

        if (m_focus == pCtrl) {
            ResetCtrlOnFocus();
        }

        return true;
    }

    return false;
}

void VdkWindow::AddCtrl(VdkControl *pCtrl) {
    wxASSERT(pCtrl);

    m_Ctrls.push_back(pCtrl);

    // 键盘导航事件
    if (pCtrl->TestStyle(VCS_TAB_TRAVERSAL)) {
        if (!m_tabOrderIter) {
            m_tabOrderIter = new TabOrderIterator;
        }

        m_tabOrderIter->AddCtrl(pCtrl);
    }

    //----------------------------------------------------------

    // 一切就绪，发送“CREATE”事件
    pCtrl->HandleNotify(VdkNotify(VCN_CREATE));

    // 克隆而来的控件不需知晓 VdkWindow 当前大小与初始大小之间的差值。
    if (!pCtrl->TestState(VCST_CLONING)) {
        LayoutCtrl(pCtrl);
    } else {
        pCtrl->RemoveState(VCST_CLONING);
    }
}

void VdkWindow::LayoutCtrl(VdkControl *pCtrl) {
    wxSize sz(m_this->GetSize()), szMin(GetMinSize());

    int dX(sz.x - szMin.x),
        dY(sz.y - szMin.y);

    if ((dX || dY) &&
            pCtrl->GetAlign() &&
            !pCtrl->GetParent()) {
        pCtrl->HandleResize(dX, dY);
    }
}

VdkControl *VdkWindow::FindCtrl(const wxString &strName) {
    CtrlIter i;
    for (i = m_Ctrls.begin(); i != m_Ctrls.end(); ++i) {
        if (strName == (*i)->GetName()) {
            return *i;
        }
    }

    return NULL;
}

bool VdkWindow::FindCtrl(VdkControl *pCtrl) {
    CtrlIter i;
    for (i = m_Ctrls.begin(); i != m_Ctrls.end(); ++i) {
        if (*i == pCtrl) {
            return true;
        }
    }

    return false;
}

void VdkWindow::ShowCtrl(const wxString &strName, bool bShow, wxDC *pDC) {
    VdkControl *pCtrl(FindCtrl(strName));
    if (pCtrl) {
        pCtrl->Show(bShow, pDC);
    }
}

void VdkWindow::HilightCtrl(VdkControl *pCtrl, wxDC &dc) {
    if (pCtrl == ms_hovering) {
        return;
    }

    if (ms_hovering) {
        VdkWindow *lastwin = ms_hovering->GetVdkWindow();
        if (lastwin != this) {
            VdkDC vdc(lastwin, lastwin->Rect00(), NULL);
            lastwin->HilightCtrl(NULL, vdc);
        } else {
            wxPoint fakeMousePos(MEANNINGLESS_MOUSE_COORD,
                                 MEANNINGLESS_MOUSE_COORD);

            VdkMouseEvent fakeEvent(NORMAL, fakeMousePos, dc);
            ms_hovering->HandleMouseEvent(fakeEvent);

            ResetCtrlOnHover();
            m_nLastCtrlState = NORMAL;
        }
    }

    //----------------------------------------------------

    if (pCtrl) {
        SetCtrlOnHover(pCtrl);
        m_nLastCtrlState = HOVERING;

        VdkMouseEvent e(HOVERING, pCtrl->GetPosition(), dc);
        pCtrl->HandleMouseEvent(e);
    }
}

void VdkWindow::FocusCtrl(VdkControl *pCtrl, wxDC *pDC) {
    FocusCtrl(pCtrl, pDC, MEANNINGLESS_MOUSE_COORD, MEANNINGLESS_MOUSE_COORD);
}

void VdkWindow::FocusCtrl(VdkControl *pCtrl, wxDC *pDC, int x, int y) {
    wxASSERT_MSG(pCtrl, L"必须提供有效的控件句柄。");

    if (m_focus == pCtrl) {
        // 保险一点，重新激活隐藏控件
        if (m_focus) {
            FocusHiddenCtrl();
        }

        return;
    }

    //----------------------------------------------------

    if (m_focus) {
        VdkControl *focus = m_focus;
        ResetCtrlOnFocus();

        VdkNotify n(VCN_LOST_FOCUS);
        n.SetVObjDC(pDC);
        focus->HandleNotify(n);
    }

    if (pCtrl) {
        SetCtrlOnFocus(pCtrl);
        // 一定要先激活隐藏控件
        FocusHiddenCtrl();

        VdkNotify n(VCN_FOCUS);
        n.SetWparam(x);
        n.SetLparam(y);
        n.SetVObjDC(pDC);

        pCtrl->HandleNotify(n);
    }
}

void VdkWindow::FocusHiddenCtrl() {
#ifdef __WXMSW__
    // 不能将焦点移至菜单窗体上
    if (IsMenuImpl()) {
        return;
    }

    if (!m_hiddenCtrl) {
        m_hiddenCtrl = new wxControl(m_panel, wxIdManager::ReserveId(),
                                     wxPoint(-100, -100),
                                     wxSize(1, 1),
                                     wxWANTS_CHARS);

        // 必须保证窗口中有两个控件
        new wxControl(m_panel, wxIdManager::ReserveId(),
                      wxPoint(-200, -200),
                      wxSize(1, 1),
                      wxWANTS_CHARS);

        m_hiddenCtrl->Bind(wxEVT_KEY_DOWN, &VdkWindow::OnKeyDown, this);
        m_hiddenCtrl->Bind(wxEVT_KEY_UP, &VdkWindow::OnKeyUp, this);
        m_hiddenCtrl->Bind(wxEVT_CHAR, &VdkWindow::OnChars, this);

        // wxWidgets 会默认将“Windows context menu”按键事件转换成普通的
        // 右键弹起事件
        m_hiddenCtrl->Bind(wxEVT_RIGHT_UP,
                           &VdkWindow::OnWindowMenuKeyUp,
                           this);
    }

    m_hiddenCtrl->SetFocus();
#endif
}

void VdkWindow::EraseBackground(wxDC &dc, const wxRect &rc) {
    if (m_bkCanvas.IsOk()) {
        m_bkCanvas.BlitRect(dc, rc);
    } else {
        wxBrush brush(m_this->GetBackgroundColour());
        dc.SetBrush(brush);
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(rc);
    }
}

bool VdkWindow::IsDoubleBuffering() const {
#ifndef __VDK_NOT_USE_DOUBLE_BUFFERING__
    return true;
#else
    return false;
#endif // __VDK_NOT_USE_DOUBLE_BUFFERING__
}

void VdkWindow::SetCtrlOnHover(VdkControl *pCtrl) {
    ms_hovering = pCtrl;
}

void VdkWindow::ResetCtrlOnHover() {
    ms_hovering = NULL;
}

inline void VdkWindow::SetLastMouseEvent(VdkMouseEventType evtCode) {
    if (!m_mouseEventNotForMe) {
        m_nLastMouseEvent = evtCode;
    } else {
        m_mouseEventNotForMe = false;
    }
}

inline void VdkWindow::SetCtrlOnFocus(VdkControl *pCtrl) {
    m_focus = pCtrl;
}

inline void VdkWindow::ResetCtrlOnFocus() {
    m_focus = NULL;
}

VdkMenu *&VdkWindow::GetMenuOnShow() {
    static VdkMenu *s_menuOnShow = NULL;
    return s_menuOnShow;
}

void VdkWindow::SetMenuOnShow(VdkMenu *menu) {
    VdkMenu *&menuOnShow = GetMenuOnShow();

    // 显示之前没有将正在显示的另外一个菜单隐藏？
    if (menu && menuOnShow && menu != menuOnShow) {
#ifdef __WXDEBUG__
        wxASSERT(false);
#else
        menuOnShow->Return();
#endif // __WXDEBUG__
    }

    menuOnShow = menu;
}

VdkMenu *VdkWindow::CreateMenu(VdkMenu *parent, int mid) {
    return new VdkMenu(mid, this, parent);
}

inline bool VdkWindow::IsActivatableEvent(int evtCode) {
    return evtCode == LEFT_DOWN || evtCode == RIGHT_UP;
}

//////////////////////////////////////////////////////////////////////////

void VdkWindow::GetDcOrigin(int *x, int *y) const {
    if (x) {
        *x = m_xDcOrigin;
    }

    if (y) {
        *y = m_xDcOrigin;
    }
}

void VdkWindow::SetDcOrigin(int x, int y) {
    m_xDcOrigin = x;
    m_yDcOrigin = y;
}

void VdkWindow::ResetDcOrigin(wxDC &dc) const {
    dc.SetDeviceOrigin(m_xDcOrigin, m_yDcOrigin);
}

//////////////////////////////////////////////////////////////////////////

void VdkWindow::SetCachedDC(VdkDC &vdc) {
    m_cachedDC = &vdc;
}

void VdkWindow::PrepareCachedDC() {

}

void VdkWindow::ResetCachedDC(wxDC &pdc) {
    m_cachedDC = NULL;

    if (m_postListeners) {
        wxVector<VdkDcPostListener *>::iterator i;
        for (i = m_postListeners->begin(); i != m_postListeners->end(); ++i) {
            (*i)->OnPost(pdc);
        }
    }
}

void VdkWindow::AddPostDrawListener(VdkDcPostListener *lsner) {
    if (!m_postListeners) {
        m_postListeners = new wxVector<VdkDcPostListener *>;
    }

    m_postListeners->push_back(lsner);
}

void VdkWindow::RemovePostDrawListener(VdkDcPostListener *lsner) {
    if (m_postListeners) {
        wxVector<VdkDcPostListener *>::iterator i(m_postListeners->begin());
        for (; i != m_postListeners->end(); ++i) {
            if (*i == lsner) {
                m_postListeners->erase(i);
                return;
            }
        }
    }
}
