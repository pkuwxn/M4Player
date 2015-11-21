/***************************************************************
 * Name:      VdkMenu.cpp
 * Purpose:   一个基于 VdkWindow 的富形式自绘菜单
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-02-26
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkMenu.h"
#include "VdkEvent.h"
#include "VdkWindowImpl.h"
#include "VdkCtrlParserInfo.h"
#include "VdkButton.h"
#include "VdkDC.h"
#include "wxUtil.h"

#include <wx/popupwin.h>
#include <wx/artprov.h>
#include <wx/sstream.h>

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

// TODO: 在 wxGTK 下面窗口隐藏是异步调用，不能及时反馈回来，
// 当再次显示时可能上次的隐藏操作还没完成，造成混乱。
#if !defined( __WXMSW__ )
#   define DESTORY_MENU_IMPL_AT_ONCE
#endif

#ifdef __WXMSW__
// Windows 专用的弹出窗口类，解决多个窗口重绘时相互覆盖的问题
class PopupWindowMSW : public wxPopupWindow {
public:

    // 构造函数
    PopupWindowMSW() {

    }

    // 实际构建函数
    bool Create(wxWindow *parent) {
        wxCHECK_MSG(parent, false, L"can't create wxWindow without parent");

        // popup windows are created hidden by default
        Hide();

        //////////////////////////////////////////////////////////////////////////

        int style = wxFULL_REPAINT_ON_RESIZE | wxPOPUP_WINDOW;
        if (!CreateBase
                (parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style)) {
            return false;
        }

        parent->AddChild(this);

        WXDWORD exstyle;
        DWORD msflags = MSWGetCreateWindowFlags(&exstyle);

        // "#32768" 是 Windows 下菜单窗口的特有类名
        if (!MSWCreate(L"#32768", NULL,
                       wxDefaultPosition, wxDefaultSize, msflags, exstyle)) {
            return false;
        }

        InheritAttributes();

        return true;
    }

    // return the style to be used for the popup windows
    virtual WXDWORD MSWGetStyle(long flags, WXDWORD *exstyle) const {
        // we only honour the border flags, the others don't make sense for us
        WXDWORD style = wxPopupWindow::MSWGetStyle(flags, exstyle);

        // WS_CLIPSIBLINGS 这一风格属性是解决重画覆盖的关键
        return (style | WS_CLIPSIBLINGS);
    }
};
#endif // __WXMSW__

//////////////////////////////////////////////////////////////////////////
// 富形式自绘菜单的实现基类
//
// 简化设计压力，所以加多了一层继承。同时有很奇怪的问题，
// 在 Windows 下，鼠标在弹出窗口上移动时，会自动提升弹出窗口的
// Z-Order，这样会导致显示的子窗口的一部分内容被破坏。
class VdkMenuImplBase :
#ifndef __WXMSW__
    public VdkWindowImpl<wxPopupWindow>
#else
    public VdkWindowImpl<PopupWindowMSW>
#endif // !__WXMSW__
{
public:

    // 构造函数
    VdkMenuImplBase(wxWindow *parent) {
#   ifndef __WXMSW__
        wxPopupWindow::Create(parent);
#   else
        PopupWindowMSW::Create(parent);
#   endif // !__WXMSW__

        // 设置菜单不可拖动
        RemoveStyle(VWS_DRAGGABLE);
    }

private:

    // 改变窗体大小
    virtual void DoResize(int x, int y, int width, int height,
                          int sizeFlags = wxSIZE_AUTO) {
        SetSize(x, y, width, height, sizeFlags);
    }

    // 最小化
    virtual void DoMinimize() {}
};

//////////////////////////////////////////////////////////////////////////
// 菜单的实际窗体
class VdkMenuImpl : public VdkMenuImplBase {
public:

    // 构造函数
    VdkMenuImpl(VdkMenu *menu, wxWindow *parent)
        : VdkMenuImplBase(parent), m_menu(menu), m_currSubMenu(NULL) {
        SetAddinStyle(VWS_MENU_IMPL | VWS_DISMISS_BY_ESC);
#       ifdef __WXDEBUG__
        SetDebugCaption(L"VdkMenuImpl");
#       endif // __WXDEBUG__

        // 绑定事件处理函数
        BindHandlers();

#       ifdef __WXMSW__
        if (m_menu->TestStyle(VMS_ERASE_ALL) && IsDoubleBuffering()) {
            m_panel->Bind(wxEVT_PAINT, &VdkMenuImpl::EraseAll, this);
        }
#       endif // __WXMSW__
    }

    // 获取菜单句柄
    VdkMenu *GetMenu() const {
        return m_menu;
    }

    // 设置当前正在显示的子菜单
    void SetCurrSubMenu(VdkMenuItemImpl *subMenu) {
        m_currSubMenu = subMenu;
    }

    // 获取当前正在显示的子菜单
    VdkMenuItemImpl *GetCurrSubMenu() const {
        return m_currSubMenu;
    }

private:

#   ifdef __WXMSW__
    void EraseAll(wxPaintEvent &e) {
        wxPaintDC dc(this);
        dc.SetBackground(GetBackgroundColour());
        dc.Clear();

        Draw(dc);
    }
#   endif // __WXMSW__

    // 绘制左边渐变色条
    virtual void EraseBackground(wxDC &dc, const wxRect &rc);

    //////////////////////////////////////////////////////////////////////////

    VdkMenu *m_menu;
    VdkMenuItemImpl *m_currSubMenu; //  当前正在显示的子菜单
};

void VdkMenuImpl::EraseBackground(wxDC &dc, const wxRect &rc) {
    // 不要清空背景，由菜单项进行
    if (rc == Rect00()) {
        VdkMenuImplBase::EraseBackground(dc, rc);

        // 绘制菜单边框
        dc.SetPen(m_menu->GetBorderPen());
        DrawRectangle(dc, Rect00());

        if (m_menu->TestStyle(VMS_BITMAP)) {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(m_menu->GetBitmapRegionBrush());

            int border = m_menu->GetBorderWeight();
            dc.DrawRectangle(wxRect(border, border,
                                    m_menu->GetBitmapRegionWidth(),
                                    rc.height - border * 2));
        }

        return;
    }

    // 始终需要清空背景
    VdkUtil::ClrBkGnd(dc, wxBrush(GetBackgroundColour()), rc);

    if (m_menu->TestStyle(VMS_BITMAP)) {
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(m_menu->GetBitmapRegionBrush());
        dc.DrawRectangle(wxRect(m_menu->GetBorderWeight(), rc.y,
                                m_menu->GetBitmapRegionWidth(),
                                rc.height));
    }
}

//////////////////////////////////////////////////////////////////////////
// 菜单项的实际可显示对象
class VdkMenuItemImpl : public VdkButton {
public:

    // 构造函数
    VdkMenuItemImpl(VdkMenu *menu, VdkMenuItem *item)
        : m_menu(menu), m_item(item) {
        m_nPaddingX = m_menu->GetTextPaddingToLeft();
        SetAddinStyle(VCS_ERASE_BG);
    }

    // 获取父菜单句柄
    VdkMenu *GetParentMenu() const {
        return m_menu;
    }

    // 获取子菜单句柄
    VdkMenu *GetSubMenu() const {
        return m_item->subMenu();
    }

    // 获取菜单项句柄
    VdkMenuItem *GetMenuItem() const {
        return m_item;
    }

private:

    // 绘制控件
    virtual void DoDraw(wxDC &dc);

    // 处理鼠标事件
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);

    //////////////////////////////////////////////////////////////////////////

    VdkMenu *m_menu;
    VdkMenuItem *m_item;
};

void VdkMenuItemImpl::DoDraw(wxDC &dc) {
    bool disabled = m_item->disabled();
    Enable(!disabled, NULL);

    //////////////////////////////////////////////////////////////////////////

    // 禁用时不绘制高亮区域
    if (!disabled && GetButtonState() != NORMAL) {
        dc.SetBrush(m_menu->GetHilightBrush());
        dc.SetPen(m_menu->GetHilightBorderPen());

        wxRect rc(m_Rect);
        rc.Deflate(1) ;
        if (m_menu->GetSubMainGap() < 0) {   // 向左缩进
            rc.width += m_menu->GetSubMainGap();
        }
        dc.DrawRectangle(rc);
    }

    //////////////////////////////////////////////////////////////////////////
    // 绘制文本

    VdkButton::DoDraw(dc);
    DrawHotKeyCaption(m_item->caption(), dc,
                      m_Rect.x + m_nPaddingX, m_Rect.y + m_nPaddingY);

    if (!m_item->accelString().IsEmpty()) {
        dc.DrawText(m_item->accelString(),
                    m_Rect.x + m_Rect.width -
                    m_menu->GetRightArrowSize() -
                    m_item->accelStringWidth(),
                    m_Rect.y + m_nPaddingY);
    }

    // 绘制右箭头
    if (m_item->subMenu()) {
        dc.DrawText(L"\x25B6",
                    m_Rect.x + m_Rect.width - m_menu->GetRightArrowSize(),
                    m_Rect.y + m_nPaddingY);
    }

    //////////////////////////////////////////////////////////////////////////
    // 绘制左侧位图/对勾

    enum {
        CONST_LEFT_BITMAP_SIZE = 16, // 左边小位图的尺寸
    };

    int bitmapWidth = m_menu->GetBitmapRegionWidth();
    int x = m_Rect.x + (bitmapWidth - CONST_LEFT_BITMAP_SIZE) / 2;
    int y = m_Rect.y + (m_menu->GetRowHeight() - CONST_LEFT_BITMAP_SIZE) / 2;

    if (m_item->checked()) {
        dc.SetPen(m_menu->GetHilightBorderPen());

        if (GetButtonState() != NORMAL) {
            dc.SetBrush(m_menu->GetCheckedHilightBrush());
        } else {
            dc.SetBrush(m_menu->GetCheckedBrush());
        }

        int edgetSize = CONST_LEFT_BITMAP_SIZE + 4;
        dc.DrawRectangle(x - 2, y - 2, edgetSize, edgetSize);

        if (!m_item->isBitmapOk()) {
            m_menu->DrawCheckMark(dc, x, y);
        }
    }

    if (m_item->isBitmapOk()) {
        m_item->bmpArrayID().BlitTo(dc, x, y);
    }
}

void VdkMenuItemImpl::DoHandleMouseEvent(VdkMouseEvent &e) {
    VdkMenu *subMenu = m_item->subMenu();
    VdkMenuImpl *menuImpl = (VdkMenuImpl *) m_WindowImpl;

    // 在菜单项上拖动
    if (e.evtCode == DRAGGING) {
        if (HitTest(e.mousePos)) {
            e.evtCode = HOVERING;
        } else {
            // 失活当前菜单项
            e.Skip(true);
            menuImpl->HilightCtrl(NULL, e.dc);

            return;
        }
    }

    VdkButton::DoHandleMouseEvent(e);

    //////////////////////////////////////////////////////////////////////////

    switch (e.evtCode) {
    case HOVERING: {
        VdkMenuItemImpl *subMenu0 = menuImpl->GetCurrSubMenu();
        if (subMenu0 && subMenu0 != this) {
            subMenu0->GetSubMenu()->Return();
            subMenu0->Update(NORMAL, &e.dc);
            menuImpl->SetCurrSubMenu(NULL);
        }

        if (subMenu) {
            if (!subMenu->IsShown()) {
                wxRect rc(m_WindowImpl->GetScreenRect());
                subMenu->ShowContext(rc.x + rc.width + m_menu->GetSubMainGap(),
                                     rc.y + GetAbsoluteRect().y,
                                     NULL);

                menuImpl->SetCurrSubMenu(this);
            }
        }

    }

    break;

    case NORMAL:

        if (subMenu) {
            // 鼠标指针进入了子菜单里
            if (subMenu->ContainsPointerGlobally()) {
                Update(VdkButton::HOVERING, &e.dc);

                break;
            }

            // 假如指针离开菜单作用域又不在子菜单里，
            // 又或是移动到其他菜单项上
            subMenu->Return();
            menuImpl->SetCurrSubMenu(NULL);
        }

        break;

    default:

        break;
    }
}

//////////////////////////////////////////////////////////////////////////

class VdkMenuItemSeperator : public VdkCtrlHandler {
public:

    // 构造函数
    VdkMenuItemSeperator(VdkWindow *win, wxRect rc) {
        m_strName = L"VdkMenuItemSeperator";
        m_Rect = rc;
        SetVdkWindow(win);

        SetAddinStyle(VCS_IGNORE_ALL_EVENTS | VCS_ERASE_BG);
        m_Window->AddCtrl(this);
    }

private:

    // 绘制控件
    virtual void DoDraw(wxDC &dc) {
        wxPen Pen(*wxGREY_PEN);
        Pen.SetWidth(1);
        dc.SetPen(Pen);

        int x = m_Rect.x + ((VdkMenuImpl *) m_WindowImpl)->GetMenu()->
                GetTextPaddingToLeft();
        int y = m_Rect.y + (m_Rect.height - 2) / 2;
        dc.DrawLine(x, y, x + m_Rect.width, y);
    }
};

//////////////////////////////////////////////////////////////////////////

VdkCtrlId VdkMenu::m_fakeId(VdkGetUniqueId());
const static int gs_invalidBestWidth = -1;
// 通知主窗口菜单已隐藏
NEW_EVENT(wxEVT_MENU_HID)

VdkMenu::VdkMenu(int mid, VdkWindow *win, VdkMenu *parent)
    : m_window(win),
      m_parent(parent),
      m_id(mid),
      m_width(0),
      m_height(0),
      m_bestWidth(gs_invalidBestWidth),
      m_maxTextWidth(0),
      m_pRelativeCtrl(NULL),
      m_impl(this),
      m_sstyle(StyleFactory::GetStyle(MSOFFICE_2003)) {
    wxASSERT(m_window);
    //////////////////////////////////////////////////////////////////////////

    // 设置默认风格
    SetAddinStyle(VMS_BITMAP);
}

VdkMenu::~VdkMenu() {
    m_items.DeleteContents(true);
    m_items.Clear();

    m_attached.DeleteContents(true);
    m_attached.Clear();

    m_window = NULL; // 指示菜单已被销毁
}

void VdkMenu::AppendItem(VdkMenuItem *pitem) {
    int nCaptionWidth = 0;

    switch (pitem->type()) {
    case MIT_NORMAL: {
        // 获取标题文本宽度
        wxMemoryDC mdc;
        if (!pitem->strAccel.IsEmpty()) {
            mdc.GetTextExtent(pitem->strAccel, &pitem->nAcceStrWidth,
                              NULL, 0, 0, &m_sstyle->menuFont);

            // 添加其与右箭头之间的间隙宽度
            pitem->nAcceStrWidth += m_sstyle->componentPaddingCount;
        }


        mdc.GetTextExtent(pitem->strCaption, &nCaptionWidth,
                          NULL, 0, 0, &m_sstyle->menuFont);

        // 加上标题文本与右箭头之间的间隙宽度
        nCaptionWidth += m_sstyle->componentPaddingCount +
                         pitem->nAcceStrWidth;

        break;
    }

    case MIT_USER_DEFINED: {

        break;
    }

    default:

        break;
    }

    if (nCaptionWidth > m_maxTextWidth) {
        m_maxTextWidth = nCaptionWidth;
    }

    //////////////////////////////////////////////////////////////////////////

    m_items.push_back(pitem);
}

void VdkMenu::AppendItem(const wxString &strCaption) {
    VdkMenuItem *item = new VdkMenuItem();
    item->caption(strCaption);

    AppendItem(item);
}

void VdkMenu::AppendSeperator() {
    m_items.push_back(new VdkMenuItem(MIT_SEPERATOR));
}

void VdkMenu::ShowContext(int x, int y, VdkControl *pCtrl) {
    if (m_items.size() == 0) {
        return;
    }

    m_impl.Implement();
    ImplementMenu();

    // 发送菜单预显示事件
    SendPreOperateMessage(VMS_SEND_PRESHOW_MSG);

    //////////////////////////////////////////////////////////////////////////

    int w, h;
    wxDisplaySize(&w, &h);
    if (x + m_width > w) {   // 右边放不下
        if (m_parent) {
            int sx;
            VdkMenuImpl *rootmenu = GetRootMenu()->GetImpl();
            rootmenu->GetScreenPosition(&sx, NULL);

            x = sx - m_width + abs(GetSubMainGap());
        } else {
            x = w - m_width;
        }
    }
    if (y + m_height > h) {
        y = h - m_height;
    }

    m_impl->Resize(m_width, m_height);
    m_impl->Move(x, y);

    //////////////////////////////////////////////////////////////////////////

    m_impl->Show();
    m_pRelativeCtrl = pCtrl;
}

void VdkMenu::ImplementMenu() {
    VdkWindow *win = m_impl.GetImpl();
    if (IsInitialized()) {   // 已然初始化
        return;
    }

    // 发送菜单预创建事件
    SendPreOperateMessage(VMS_SEND_PRECREATE_MSG);

    // 计算宽度
    CalcMenuWidth();

    m_height = 0;
    int x = m_sstyle->borderWeight,
        y = 0,
        w = m_width - m_sstyle->borderWeight * 2,
        h = m_sstyle->rowHeight;

    wxString strDummy(L" ");
    VdkMenuItemImpl *impl = NULL;
    wxWindow *winimp = win->VdkWindow::GetHandle();
    VdkMenuItem *item = NULL;
    ItemIter i;
    for (i = m_items.begin(); i != m_items.end(); ++i) {
        item = *i;
        y = m_height + m_sstyle->borderWeight;

        switch (item->nType) {
        case MIT_NORMAL:

            impl = new VdkMenuItemImpl(this, item);
            impl->VdkButton::Create(VdkButtonInitializer().
                                    // 伪文本，我们自己画文本
                                    caption(strDummy).
                                    textAlign(TEXT_ALIGN_LEFT).
                                    name(L"VdkMenuItemImpl").
                                    id(m_fakeId).
                                    rect(wxRect(x, y, w, h)).
                                    window(win).
                                    addToWindow(true)
                                   );

            if (item->disabled()) {
                impl->Enable(false, NULL);
            }

            winimp->Bind(wxEVT_VOBJ, &VdkMenu::OnSelect, m_fakeId);
            m_height += m_sstyle->rowHeight;

            break;

        case MIT_SEPERATOR:

            new VdkMenuItemSeperator
            (win, wxRect(x, y, w - GetTextPaddingToLeft() - 4, 4));

            m_height += 4;

            break;

        case MIT_USER_DEFINED: {
            VdkMenuCtrlHandler *hdler =
                reinterpret_cast<VdkMenuCtrlHandler *>
                (item->GetClientData());

            wxASSERT(hdler);
            //////////////////////////////////////////////////////////

            VdkControl *pCtrl = NULL;
            pCtrl = hdler->Implement
                    (win, wxPoint(x + GetTextPaddingToLeft(), y));

            if (pCtrl) {
                m_height = pCtrl->GetRect().GetBottom() + 1;

                int l, t, r, b;
                hdler->GetPaddings(&l, &t, &r, &b);

                m_height += b;
            }

            break;
        }

        default:

            break;
        }
    }

    m_height += m_sstyle->borderWeight * 2; // 添加边框

    // 更新菜单项宽度
    if (!m_attached.empty()) {
        int dX = MeasureOnMenuCtrls();
        if (dX) {
            ItemIter i;
            VdkWindow::CtrlIter ci;
            VdkMenuItem *item;
            VdkControl *notUserDefined;

            for (i = begin(), ci = win->begin(); i != end(); ++i, ++ci) {
                // 必须要坚持一个原则：一个菜单项只能有一个没有父控件的 VdkControl
                while ((*ci)->GetParent()) {
                    ++ci;
                }

                item = *i;
                if (item->type() != MIT_USER_DEFINED) {
                    notUserDefined = *ci;

                    int w, h;
                    notUserDefined->GetSize(&w, &h);
                    notUserDefined->SetSize(w + dX, h);
                }
            }
        }
    }

    // 默认背景色
    winimp->SetBackgroundColour(m_sstyle->bgColor);

    //////////////////////////////////////////////////////////////////////////

    SetAddinState(VMST_INITIALIZED);
}

void VdkMenu::OnSelect(VdkVObjEvent &e) {
    VdkMenuItemImpl *itemImpl = e.GetCtrl<VdkMenuItemImpl>();
    if (!itemImpl->GetSubMenu()) {
        VdkMenu *menu = itemImpl->GetParentMenu();
        VdkMenuItem *item = itemImpl->GetMenuItem();
        if (item->checkable()) {
            item->checked(!item->checked());
        }

        menu->FireEvent(item);
        // 取消菜单的显示
        menu->GetRootMenu()->Return();
    }
}

int VdkMenu::GetTextPaddingToLeft() const {
    return GetTextPaddingToRight() +
           (TestStyle(VMS_BITMAP) ? GetBitmapRegionWidth() : 0);
}

int VdkMenu::GetTextPaddingToRight() const {
    return wxNORMAL_FONT->GetPointSize() * m_sstyle->textPaddingCount;
}

bool VdkMenu::ContainsPointer(const wxPoint &mousePos) const {
    if (mousePos.x > 0 && mousePos.x < m_width &&
            mousePos.y > 0 && mousePos.y < m_height) {
        return true;
    }

    return false;
}

bool VdkMenu::ContainsPointerGlobally() const {
    VdkMenuImpl *implWin = m_impl.GetImpl();
    return implWin && implWin->GetScreenRect().
           Contains(wxGetMousePosition());
}

void VdkMenu::Return() {
    // 还没有实现
    if (!m_impl.GetImpl()) {
        return;
    }

    if (m_impl->GetCurrSubMenu()) {
        m_impl->GetCurrSubMenu()->GetSubMenu()->Return();
    }

    m_impl.Release(false);

    //////////////////////////////////////////////////////////////////////////

    // 发送选中事件、菜单已隐藏事件
    VdkVObjEvent e(VEMC_MENU_HID);
    e.SetMenu(this);
    e.SetCtrl(m_pRelativeCtrl);
    m_pRelativeCtrl = NULL;

    // 最好先处理菜单已隐藏事件才进行后续处理。
    // 例如菜单栏隐藏当前菜单，显示一个新的菜单，
    // 此时 VdkWindow 获得了正确的状态，当 VdkWindow 处理本消息时，
    // 绝对不能会覆盖掉先前的状态，使当前菜单句柄为 NULL 。
    // 也可：wxPostEvent( m_window->GetWindowHandle(), e );
    m_window->GetHandle()->ProcessWindowEvent(e);
}

VdkMenu *VdkMenu::GetRootMenu() {
    if (m_parent) {
        VdkMenu *menu(m_parent);
        while (menu->GetParentMenu()) {
            menu = menu->GetParentMenu();
        }

        return menu;
    }

    return this;
}

void VdkMenu::CalcMenuWidth() {
    /*   文本边距                右箭头
    ||__|_|___________________|_|_|| <-边框
    | 小位图 文本区域      文本边距
    |__ 边框
    */

    m_width = m_maxTextWidth + m_sstyle->borderWeight * 2;

    if (!TestStyle(VMS_NO_EXTRA_SPACE)) {
        if (TestStyle(VMS_BITMAP)) {
            m_width += GetBitmapRegionWidth();
        }

        m_width += GetTextPaddingToRight() * 2 +
                   m_sstyle->rightArrowSize;
    }

    if (m_bestWidth != gs_invalidBestWidth &&
            m_width < m_bestWidth) {
        m_width = m_bestWidth;
    }
}

void VdkMenu::SetBestWidth(int w) {
    m_bestWidth = w;

    VdkWindow *win = m_impl.GetImpl();
    // 实现窗体必须已然初始化
    if (win && !win->TestState(VWST_INITING)) {
        CalcMenuWidth();
    }
}

//////////////////////////////////////////////////////////////////////////

enum DestoryDelayTimeout {
    DESTORY_AT_ONCE = 50, // 立刻删除
    DELAY_DESTORY = // 延迟删除
#ifdef __WXDEBUG__
        1000,
#else
        600000,
#endif // __WXDEBUG__
};

VdkMenu::VdkMenuPtr::VdkMenuPtr(VdkMenu *menu)
    : m_menu(menu), m_impl(NULL) {

}

VdkMenuImpl *VdkMenu::VdkMenuPtr::operator->() {
    Implement();
    return m_impl;
}

VdkMenu::VdkMenuPtr::~VdkMenuPtr() {
    Release(true);
}

void VdkMenu::VdkMenuPtr::Release(bool delAtOnce) {
    if (m_impl) {
#ifdef DESTORY_MENU_IMPL_AT_ONCE
        delAtOnce = true;
#endif
        if (delAtOnce) {
            // TODO：此时窗口必须不能正在处理各种事件，例如鼠标事件（如何判断？）
            m_impl->Destroy();
        } else {
            m_impl->Hide();

            // 取消高亮项
            VdkMenuItemImpl *itemImpl = m_impl->GetCurrSubMenu();
            if (itemImpl) {
                // 必须提供正确的 DC ，否则双缓冲位图中保存的内容是旧的
                VdkDC vdc(m_impl, itemImpl->GetAbsoluteRect(), NULL);
                itemImpl->Update(VdkButton::NORMAL, &vdc);
            }

            // 只需要一次 Timer 事件
            Start(DELAY_DESTORY, true);
        }

    }
}

void VdkMenu::VdkMenuPtr::Implement() {
    if (!m_impl) {
        wxWindow *parent = NULL;
        VdkMenu *menu0 = m_menu->GetParentMenu();
        if (menu0) {
            parent = menu0->GetImpl();
        }
        if (!parent) {
            parent = m_menu->GetVdkWindow()->GetWindowHandle();
        }

        m_impl = new VdkMenuImpl(m_menu, parent);
        m_impl->Bind(wxEVT_DESTROY, &VdkMenuPtr::OnImplDestroy, this);
    } else {
        // 继续使用当前实体窗体，停止 Timer
        Stop();
    }
}

bool VdkMenu::VdkMenuPtr::IsShown() const {
    return m_impl != NULL && m_impl->IsShown();
}

void VdkMenu::VdkMenuPtr::Notify() {
    Release(true);
}

void VdkMenu::VdkMenuPtr::OnImplDestroy(wxWindowDestroyEvent &) {
    wxASSERT(m_impl);

    m_impl = NULL;
    m_menu->OnMenuImplDestroy();
}

void VdkMenu::FireEvent(VdkMenuItem *item) {
    VdkVObjEvent *e = NULL;

    if (item && item->id() != wxID_ANY) {   // 消息时关于菜单项的
        e = new VdkVObjEvent(item->id());
    } else if (m_id != wxID_ANY) {   // 消息是关于菜单的
        e = new VdkVObjEvent(m_id);
    }

    if (e) {
        e->SetMenu(this);
        e->SetMenuItem(item);
        wxQueueEvent(m_window->GetWindowHandle(), e);
    }
}

void VdkMenu::DrawCheckMark(wxDC &dc, int x, int y) const {
    if (m_sstyle->checkMark.IsOk()) {
        dc.DrawBitmap(m_sstyle->checkMark, x, y, true);
    } else
        dc.DrawBitmap(wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_MENU),
                      x, y, true);
}

bool VdkMenu::SelectStaticStyle(MenuStyle style) {
    StaticStyle *ss = StyleFactory::GetStyle(style);
    if (ss) {
        m_sstyle = ss;
        return true;
    }

    return false;
}

VdkMenuItem *VdkMenu::FindCheckedItem() {
    ItemIter i;
    for (i = m_items.begin(); i != m_items.end(); ++i) {
        if ((*i)->checked()) {
            return *i;
        }
    }

    return NULL;
}

void VdkMenu::SendPreOperateMessage(VdkMenuStyle style) {
    if (TestStyle(style) && m_id != wxID_ANY) {
        VdkVObjEvent e(m_id);
        e.SetMenu(this);
        m_window->GetWindowHandle()->ProcessWindowEvent(e);
    }
}

void VdkMenu::AttachCtrl(VdkMenuCtrlHandler *hdler) {
    wxASSERT(hdler);

    m_attached.push_back(hdler);
    VdkMenuItem *item = new VdkMenuItem(MIT_USER_DEFINED);
    item->SetClientData(hdler);

    AppendItem(item);
}

void VdkMenu::DeAttachCtrl(VdkMenuCtrlHandler *hdler) {
    wxASSERT(hdler);


}

int VdkMenu::MeasureOnMenuCtrls() {
    int w = 0, tmp;
    VdkMenuCtrlHandler *hdler;
    OnMenuCtrlIter i;
    for (i = m_attached.begin(); i != m_attached.end(); ++i) {
        hdler = *i;
        tmp = hdler->GetCtrl()->GetRect().width;

        int l, t, r, b;
        hdler->GetPaddings(&l, &t, &r, &b);

        tmp += r;

        if (tmp > w) {
            w = tmp;
        }
    }

    if (w > m_maxTextWidth) {
        int dX = w - m_maxTextWidth;
        m_maxTextWidth = w;

        CalcMenuWidth();

        return dX;
    }

    return 0;
}

void VdkMenu::OnMenuImplDestroy() {
    RemoveState(VMST_INITIALIZED);

    VdkMenuCtrlHandler *hdler;
    OnMenuCtrlIter i;
    for (i = m_attached.begin(); i != m_attached.end(); ++i) {
        hdler = *i;
        hdler->Reset();
    }
}

VdkWindow *VdkMenu::GetVdkWindowImpl() const {
    return m_impl.GetImpl();
}

VdkMenu *VdkMenu::GetLastShownMenu() const {
    VdkMenuImpl *winImpl = m_impl.GetImpl();
    if (winImpl) {
        VdkMenuItemImpl *itemImpl = winImpl->GetCurrSubMenu();
        if (itemImpl) {
            VdkMenu *shown = itemImpl->GetSubMenu();
            if (shown) {
                return shown->GetLastShownMenu();
            }
        } else {
            return const_cast<VdkMenu *>(this);
        }
    }

    return NULL;
}

size_t VdkMenu::GetItemCount() const {
    return m_items.size();
}

bool VdkMenu::IsEmpty() const {
    return m_items.empty();
}

VdkMenuItem *VdkMenu::GetItem(size_t i) {
    return m_items[i];
}

VdkMenuItem *VdkMenu::FindItem(VdkCtrlId id) {
    // TODO: 未实现
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

void VdkMenuCtrlHandler::Reset() {
    SaveState();
    m_pCtrl = NULL;
}

VdkControl *VdkMenuXrcCtrlWrapper::Implement(VdkWindow *menuWin, const wxPoint &pos) {
    wxStringInputStream sstream(m_strXmlData);
    wxXmlDocument doc(sstream);
    wxXmlNode *root = doc.GetRoot();
    wxASSERT(root);

    VdkControl *pCtrl;
    m_pCtrl = pCtrl = VdkControl::ParseObjects(VdkCtrlParserInfo().
                      window(menuWin).
                      node(root));

    while (pCtrl) {
        if (pos.x == 0 && pos.y == 0) {
            break;
        }

        wxPoint pold(pCtrl->GetPosition());
        pCtrl->Move(pold.x + pos.x + m_paddingLeft,
                    pold.y + pos.y + m_paddingRight);

        pCtrl = pCtrl->GetNext();
    }

    m_pCtrl->SetID(m_id);

    //////////////////////////////////////////////////////////////////////////

    RestoreState();
    return m_pCtrl;
}

VdkControl *VdkMenuCloneableCtrlWrapper::Implement
(VdkWindow *menuWin, const wxPoint &pos) {
    VdkControl *o = m_Window->FindCtrl(m_strName);
    wxASSERT(o);

    m_pCtrl = o->GetImitation();
    m_pCtrl->Clone(o, o->GetParent());
    m_pCtrl->Move(pos);
    m_pCtrl->SetID(m_id);

    menuWin->AddCtrl(m_pCtrl);

    //////////////////////////////////////////////////////////////////////////

    RestoreState();
    return m_pCtrl;
}

//////////////////////////////////////////////////////////////////////////

VdkMenuItem::~VdkMenuItem() {
    wxDELETE(pSubMenu);
}

VdkMenuItem &VdkMenuItem::checkable(bool b) {
    if (b) {
        SetAddinStyle(VMIS_CHECKABLE);
    } else {
        RemoveStyle(VMIS_CHECKABLE);
    }

    return *this;
}

VdkMenuItem &VdkMenuItem::checked(bool b) {
    if (!checkable()) {
        checkable(true);
    }

    if (b) {
        SetAddinState(VMIST_CHECKED);
    } else {
        RemoveState(VMIST_CHECKED);
    }

    return *this;
}

VdkMenuItem &VdkMenuItem::disabled(bool d) {
    if (d != disabled()) {
        if (d) {
            SetAddinState(VMIST_DISABLED);
        } else {
            RemoveState(VMIST_DISABLED);
        }
    }

    return *this;
}

//////////////////////////////////////////////////////////////////////////

VdkMenu::StaticStyle *VdkMenu::StyleFactory::GetStyle(MenuStyle style) {
    switch (style) {
    case MSOFFICE_2003:
        return GetOffice2003Style();
    case TM2009:
        return GetTM2009Style();
    default:
        return NULL;
    }
}

void VdkMenu::StyleFactory::InitStyle(StaticStyle &sstyle) {
    sstyle.rowHeight = 24;
    sstyle.borderWeight = 1; // 边框宽度
    sstyle.padding = 1; // 高亮条距边框的宽度
    sstyle.textPaddingCount = 1; // 高亮条距边框的宽度
    sstyle.bitmapRegion = 26;
    sstyle.rightArrowSize = 16; // 代表子菜单的右边箭头宽度
    sstyle.subMainGap = -2; // 子菜单与父菜单之间的间隙宽度
    // 菜单文本各部分之间的间隙宽度，如右箭头与加速键之间
    sstyle.componentPaddingCount = 2;

    sstyle.bgColor = *wxWHITE;
    sstyle.menuFont = *wxNORMAL_FONT;
    sstyle.bitmapRegionBrush.SetColour(*wxWHITE);
}

VdkMenu::StaticStyle *VdkMenu::StyleFactory::GetOffice2003Style() {
    static VdkMenu::StaticStyle sstyle;
    if (!sstyle.bitmapRegionBrush.IsOk()) {
        InitStyle(sstyle);

        wxBitmap stipple(sstyle.bitmapRegion, 1);
        wxMemoryDC mdc(stipple);
        mdc.GradientFillLinear(wxRect(0, 0, stipple.GetWidth(), 1),
                               wxColour(227,239,255), wxColour(142,179,231));
        sstyle.bitmapRegionBrush.SetStipple(stipple);

        sstyle.borderPen.SetColour(wxColour(0,45,150));
        sstyle.hilightBorderPen = *wxBLUE_PEN;

        sstyle.checkedBrush.SetColour(wxColour(255,197,118));
        sstyle.checkedHilightBrush.SetColour(wxColour(254,128,62));
        sstyle.hilightBrush.SetColour(wxColour(255,238,194));
    }

    return &sstyle;
}

VdkMenu::StaticStyle *VdkMenu::StyleFactory::GetTM2009Style() {
    static VdkMenu::StaticStyle sstyle;
    if (!sstyle.bitmapRegionBrush.IsOk()) {
        InitStyle(sstyle);

        sstyle.borderPen.SetColour(wxColour(131,165,187));
        sstyle.hilightBorderPen = *wxTRANSPARENT_PEN;

        sstyle.checkedBrush.SetColour(wxColour(255,197,118));
        sstyle.checkedHilightBrush.SetColour(wxColour(254,128,62));
        sstyle.bitmapRegionBrush.SetColour(wxColour(213,226,236));
        sstyle.hilightBrush.SetColour(wxColour(120,175,210));
    }

    return &sstyle;
}

//////////////////////////////////////////////////////////////////////////

VdkMenu *GenerateMenu(int mid, VdkWindow *parent, const wxString &strMenuTree) {
    VdkMenu *menu = new VdkMenu(mid, parent, NULL);
    wxString strItem;
    size_t nPos = strMenuTree.find(L';');
    size_t nLastPos = 0;

    while (nPos != wxString::npos) {
        strItem = strMenuTree.Mid(nLastPos, nPos - nLastPos);
        if (strItem == L"<SEP>") {
            menu->AppendSeperator();
        } else {
            menu->AppendItem(strItem);
        }

        nLastPos = nPos + 1;
        nPos = strMenuTree.find(L';', nLastPos);
    }

    strItem = strMenuTree.Mid(nLastPos, wxString::npos);
    if (!strItem.IsEmpty()) {
        menu->AppendItem(strItem);
    }

    return menu;
}

VdkMenu *GenerateMenu(VdkWindow *parent, const MenuGenerator &menuTree) {
    VdkMenu *menu(new VdkMenu(menuTree.mid, parent, menuTree.parent));

    MenuGenItemMap::const_iterator i;
    for (i = menuTree.itemMap.begin(); i != menuTree.itemMap.end(); i++) {
        if (i->second == L"<SEP>") {
            menu->AppendSeperator();
        } else
            menu->AppendItem(&(new VdkMenuItem())->id(i->first).
                             caption(i->second));
    }

    return menu;
}

void MenuGenerator::Clear() {
    itemMap.clear();
    parent = NULL;
}

//////////////////////////////////////////////////////////////////////////

VDK_DEFINE_INTERNAL_WX_PTR_LIST(VdkMenu, item_list)
VDK_DEFINE_INTERNAL_WX_PTR_LIST(VdkMenu, OnMenuCtrls)
