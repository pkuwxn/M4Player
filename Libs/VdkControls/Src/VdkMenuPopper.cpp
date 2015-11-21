/***************************************************************
 * Name:      VdkMenuPopper.cpp
 * Purpose:   Code for VdkMenuPopper implementation
 * Author:    vanxining (vanxining@139.com)
 * Created:   2010-12-03
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkMenuPopper.h"
#include "VdkMenu.h"
#include "VdkWindow.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT(VdkMenuPopper);

void VdkMenuPopper::Create(wxXmlNode *node) {
    VdkToggleButton::Create(node);
    wxString s(XmlGetChildContent(node, L"menu"));
    if (!s.IsEmpty()) {
        m_menu = GenerateMenu(wxID_ANY, m_Window, s);
    }

    //===================================================

    HookMouseEvent();
    Prepare(NULL);
}

void VdkMenuPopper::HookMouseEvent() {
    m_Window->PushEventFilter(this);
}

void VdkMenuPopper::Prepare(wxDC *pDC) {
    PushButton(false, pDC);
    PushButton(true, NULL);
}

void VdkMenuPopper::DoHandleMouseEvent(VdkMouseEvent &e) {
    if (IsMenuOnShow()) {
        return;
    }

    if (m_menu && !m_menu->IsEmpty()) {
        VdkToggleButton::DoHandleMouseEvent(e);

        switch (e.evtCode) {
        case LEFT_DOWN:

            // 因为弹出菜单后不再发送 LEFT_UP 事件，因此我们事先补充
            e.evtCode = LEFT_UP;
            VdkToggleButton::DoHandleMouseEvent(e);

            wxASSERT(IsPushed());
            wxASSERT(IsToggled());
            wxASSERT(GetButtonState() == VdkButton::PUSHED);

            if (!m_menu->IsShown()) {
                PopupMenu();
            }

            break;

        default:

            break;
        }
    } else {
        VdkButton::DoHandleMouseEvent(e);
    }
}

void VdkMenuPopper::PopupMenu() {
    wxPoint p(GetAbsoluteRect().GetPosition());
    p.y += m_Rect.height;

    // 延迟显示菜单
    m_Window->ShowContextMenu(this, p);
}

void VdkMenuPopper::DoHandleNotify(const VdkNotify &notice) {
    switch (notice.GetNotifyCode()) {
    case VCN_MENU_HID:

        Prepare(notice.GetVObjDC());

        break;

    default:

        break;
    }
}

bool VdkMenuPopper::FilterEvent(const VdkEventFilter::EventForFiltering &e) {
    const wxMouseEvent &me(e.mouseEvent());
    if (HitTest(me.GetPosition())) {
        if (e.evtCode() == LEFT_DOWN) {
            if (IsMenuOnShow()) {
                return true;
            }
        }
    }

    return false;
}
