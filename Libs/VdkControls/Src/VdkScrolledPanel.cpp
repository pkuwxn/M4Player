/***************************************************************
 * Name:      VdkScrolledPanel.cpp
 * Purpose:   滚动面板（能作为其他控件父控件的滚动窗口派生类）实现
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-01-04
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkScrolledPanel.h"
#include "VdkCtrlParserInfo.h"
#include "VdkDC.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


//////////////////////////////////////////////////////////////////////////

const static int VCN_SHOW_HIDE_CHILDREN = 9999;

VdkScrolledPanel::VdkScrolledPanel()
    : m_bUsingCachedSbDC(false) {

}

void VdkScrolledPanelBase::Create(wxWindow *parent, long style) {
    wxPanel::Create(parent, wxID_ANY, wxDefaultPosition,
                    wxDefaultSize,
                    style | wxCLIP_CHILDREN);

    // 至少要保持主窗口有两个原生控件
    new wxStaticText(parent, -1,
                     wxEmptyString,
                     wxPoint(-1, -1));

    //=====================================================

    // 设置面板不可拖动
    RemoveStyle(VWS_DRAGGABLE);
}

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT(VdkScrolledPanel);

void VdkScrolledPanel::Create(wxXmlNode *node) {
    DoXrcCreate(node);
    VdkScrolledPanelBase::Create(m_WindowImpl);

    //=====================================================

    TextInfo ti = GetXrcTextInfo(node, m_WindowImpl);
    m_Font = ti.font;
    wxPanel::SetFont(m_Font);

    VdkWindow::Create
    (VdkWindowInitializer().rect(GetAbsoluteRect()));
    VdkControl::ParseObjects
    (VdkCtrlParserInfo().window(this).node(node));
}

VdkScrolledPanel::~VdkScrolledPanel() {
    BeginExit();
}

void VdkScrolledPanel::DoDraw(wxDC &dc) {
    if (VdkControl::TestStyle(VCS_BORDER_NONE)) {
        return;
    }

    wxASSERT(false);

    static wxPen pen(wxColour(95, 232, 246));
    dc.SetPen(pen);

    wxRect rc(m_Rect);
    rc.Inflate(1);

    VdkScrollBar *psb = GetScrollBar();
    if (psb && psb->IsShown()) {
        rc.width += psb->GetRect().width;    // 加上滚动条的宽度
    }

    DrawRectangle(dc, rc);
}

void VdkScrolledPanel::OnScrollBarShowHide(const VdkScrollBar *sb) {
    int w = sb->GetRect().width;
    w = sb->IsOnShow() ? -w : w;
    m_Rect.width += w;

    VdkNotify notice(VCN_SIZE_CHANGED);
    notice.SetWparam(w);

    VdkNativeCtrl<VdkScrolledPanel>::HandleNotify(notice);
    OnNotify(notice);
}

void VdkScrolledPanel::DoHandleNotify(const VdkNotify &notice) {
    VdkNativeCtrl<VdkScrolledPanel>::DoHandleNotify(notice);
    Notify(notice);

    switch (notice.GetNotifyCode()) {
    case VCN_CREATE: {
        // 因为滚动条不属于滚动面板（属于滚动面板的父窗口），
        // 故缩小面板使滚动条显示出来
        VdkScrollBar *psb = GetScrollBar();
        if (psb) {
            VdkNotify n(VCN_SIZE_CHANGED);
            n.SetWparam(- psb->GetRect().width);
            VdkNativeCtrl<VdkScrolledPanel>::DoHandleNotify(n);

            VdkControl *pCtrl = NULL;
            VdkWindow::CtrlIter i;
            for (i = VdkWindow::begin(); i != VdkWindow::end() ; ++i) {
                pCtrl = *i;
                pCtrl->HandleRelativeAlign(pCtrl->GetAlign());
            }

        }

    }

    case VCN_SIZE_CHANGED:
    case VCN_WINDOW_RESIZED:

        CalcVirtualSize();
        LayoutWidgets(notice.GetWparam(), notice.GetLparam());

    // TODO:
    case VCN_SHOW_HIDE_CHILDREN: {
        VdkControl *pCtrl = NULL;
        wxRect rcThis(Rect00());
        GetViewStartCoord(&rcThis.x, &rcThis.y);

        SetDcOrigin(rcThis.x, rcThis.y);

        VdkWindow::CtrlIter i;
        for (i = VdkWindow::begin(); i != VdkWindow::end() ; ++i) {
            pCtrl = *i;

            if (rcThis.Intersects(pCtrl->GetAbsoluteRect())) {
                pCtrl->Thaw(NULL);

                VdkNotify n(VCN_WINDOW_ORIGIN_CHANGED);
                n.SetLparam(rcThis.y);
                pCtrl->HandleNotify(n);
            } else {
                pCtrl->Freeze();
            }
        }

        break;
    }

    default:
        break;
    }
}

bool VdkScrolledPanel::IsChildVisible(VdkControl *chd) const {
    wxASSERT(chd && chd->GetVdkWindow() == this);

    wxRect rcThis(Rect00());
    GetViewStartCoord(&rcThis.x, &rcThis.y);

    return rcThis.Intersects(chd->GetAbsoluteRect());
}

bool VdkScrolledPanel::FilterEventBefore(wxMouseEvent &evt, int evtCode) {
    bool ret = false;
#ifndef __WXMSW__
    ret = FilterEventAfter(evt, evtCode);
#endif

    // 不要处理离开窗口的鼠标事件
    if (Rect00().Contains(evt.GetX(), evt.GetY())) {
        int y;
        GetViewStartCoord(NULL, &y);

        evt.SetY(evt.GetY() + y);
    }

    return ret;
}

void VdkScrolledPanel::DoHandleMouseEvent(VdkMouseEvent &e) {
    FilterEventAfter(e);
}

bool IsScrollingEvent(int evtCode) {
    return (evtCode == SCROLLING_UP ||
            evtCode == SCROLLING_DOWN ||
            evtCode == WHEEL_DOWN ||
            evtCode == WHEEL_UP);
}

bool VdkScrolledPanel::FilterEventAfter(wxMouseEvent &evt, int evtCode) {
    if (IsScrollingEvent(evtCode)) {
        VdkDC dc(this, Rect00(), NULL);
        VdkMouseEvent e(evt, evtCode, dc);

        MouseEvent(e);

        return true;
    }

    return false;
}

bool VdkScrolledPanel::FilterEventAfter(VdkMouseEvent &evt) {
    if (IsScrollingEvent(evt.evtCode)) {
        MouseEvent(evt);
        return true;
    }

    return false;
}

void VdkScrolledPanel::OnMouseEvent(VdkMouseEvent &e) {
    switch (e.evtCode) {
    case WHEEL_UP:
    case WHEEL_DOWN: {
        wxPoint mousePos(e.mousePos);
        wxRect rc(GetAbsoluteRect());
        mousePos.x -= rc.x;
        mousePos.y -= rc.y;

        int y;
        GetViewStartCoord(NULL, &y);
        mousePos.y += y;

        //=====================================================

        CtrlIter ctrl;
        for (ctrl = VdkWindow::begin(); ctrl != VdkWindow::end(); ++ctrl) {
            if ((*ctrl)->HitTest(mousePos)) {
                break;
            }
        }

        if (ctrl != VdkWindow::end()) {
            if (*ctrl == GetCtrlOnHover()) {
                break;
            }

            VdkDC vdc(this, Rect00(), NULL);
            HilightCtrl(*ctrl, vdc);

            VdkWindow::GetHandle()->SetToolTip(wxEmptyString);
            wxASSERT(*ctrl == GetCtrlOnHover());
        }

        // 跳过本事件的后续处理，确保我们对 HilightCtrl 的调用的有效性
        e.Skip(true);

        break;
    }

    default:
        break;
    }
}

void VdkScrolledPanel::DoRefreshState(wxDC &) {
    int x, y;
    GetViewStartCoord(&x, &y);
    SetDcOrigin(x, y);

    DoHandleNotify(VdkNotify
                   (VdkCtrlNotifyCode(VCN_SHOW_HIDE_CHILDREN)));
}

void VdkScrolledPanel::CalcVirtualSize() {
    int h = 0, bottom = 0;

    VdkWindow::CtrlIter i;
    for (i = VdkWindow::begin(); i != VdkWindow::end(); ++i) {
        bottom = (*i)->GetRect().GetBottom();

        if (h < bottom) {
            h = bottom;
        }
    }

    VdkScrolled::SetVirtualSize(m_Rect.width, h);
}

void VdkScrolledPanel::PrepareCachedDC() {
    VdkScrolled::PrepareDC(*GetCachedDC());
}

void VdkScrolledPanel::DoPaint(wxDC &dc) {
    // 擦除正在显示的部分
    wxRect rc(Rect00());
    GetViewStartCoord(NULL, &rc.y);

    VdkWindow::EraseBackground(dc, rc);
}

wxDC *VdkScrolledPanel::GetScrollBarDC(wxDC *) const {
    wxASSERT(!m_bUsingCachedSbDC);
    VdkWindow *window = ParentWindow();
    VdkDC *cached = window->GetCachedDC();

    if (cached) {
        wxLogDebug(L"[%s:%d]实际不应该出现这种情况: "
                   L"使用已被缓存的滚动条窗口DC[%s:%d]。",
                   __FILE__, __LINE__,
                   window->GetDebugCaption(),
                   long(cached));

        m_bUsingCachedSbDC = true;
        return cached;
    }

    wxDC *dc = new VdkDC(window, GetScrollBar()->GetAbsoluteRect());
    window->ResetDcOrigin(*dc);

    return dc;
}

void VdkScrolledPanel::DestroyScrollBarDC(wxDC *dc) const {
    if (!m_bUsingCachedSbDC) {
        delete dc;
    } else {
        m_bUsingCachedSbDC = false;
    }
}

void VdkScrolledPanel::UpdateUI(wxDC &dc) {
    VdkWindow::SetAddinState(VWST_RESET);   // 强制刷新全部内容
    Refresh(true);
}
