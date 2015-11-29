/***************************************************************
 * Name:      OOPWindow.cpp
 * Purpose:   定义 OOPWindow
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-2-27
 **************************************************************/
#include "StdAfx.h"
#include "OOPWindow.h"

#include "OOPApp.h"
#include "OOPConf.h" // for OOPStickyWindowState

#include "VdkButton.h" // for close button

#include "wxUtil.h" // for FindChildNode(), RectToString() and XmlSetContent()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

extern OOPlayerApp *g_app;

OOPWindow::OOPWindow(wxWindow *parent,
                     const wxString &uuid,
                     const wxString &title,
                     long style,
                     long vstyle)
    : Super(parent, title, style, vstyle),
      m_uuid(uuid),
      m_stickyToMain(false) {
    g_app->AddToWindowManager(this);
#ifdef __WXDEBUG__
    SetDebugCaption(title);
#endif
    Bind(wxEVT_VOBJ, &OOPWindow::OnResetSkin, this, OOPEMC_RESET_WINDOW);
    Bind(wxEVT_CLOSE_WINDOW, &OOPWindow::OnCloseNative, this);

    wxWindowID wid = GetId();
    Bind(wxEVT_STICKY_WINDOW_ATTACHED, &OOPWindow::OnAttachToMain, this, wid);
    Bind(wxEVT_STICKY_WINDOW_DETACHED, &OOPWindow::OnAttachToMain, this, wid);
}

OOPWindow::~OOPWindow() {
    g_app->RemoveFromWindowManager(this);
}

void OOPWindow::HackXrc(wxXmlNode *winRoot) {
    wxASSERT(winRoot);

    OOPStickyWindowState *sstate = g_app->GetPanelStickyState(m_uuid);
    if (sstate) {
        wxASSERT(!sstate->m_rect.IsEmpty());

        m_stickyToMain = sstate->m_stickyToMain;
        if (m_stickyToMain) {
            m_relativePos = sstate->m_rect.GetPosition();
        }
    }

    //-----------------------------------------------------------

    wxXmlNode *rectNode = FindChildNode(winRoot, L"rect");
    if (rectNode) {
        wxRect rcScreen;

        wxWindow *parent = GetParent();
        if (parent) {
            wxRect rcMain(parent->GetScreenRect());

            if (sstate) {
                rcScreen = sstate->m_rect;
                if (m_stickyToMain) {
                    rcScreen.x += rcMain.x;
                    rcScreen.y += rcMain.y;
                }
            } else {
                wxString strRect(rectNode->GetNodeContent());
                rcScreen = VdkUtil::ParseRect(strRect, PRT_RECTANGLE);

                // 皮肤中的面板位置定义应该都是按照已粘附到主窗口来计算的
                // 假如窗口一开始是显示的，但是后来被隐藏了，再就是一个
                // 换肤操作，当这个窗口被再次显示时，它本该与主窗口粘附的，
                // 但若这里不设置则不会是粘附状态，造成显示错位。
                m_stickyToMain = true;
                m_relativePos = rcScreen.GetPosition();

                rcScreen.x += rcMain.x;
                rcScreen.y += rcMain.y;
            }
        }

        XmlSetContent(rectNode, RectToString(rcScreen));
    }
}

void OOPWindow::BindCloseBtnEvent(const wxString &objName) {
    VdkControl *btnClose = FindCtrl(objName);
    if (btnClose) {
        VdkCtrlId idCloseBtn = VdkGetUniqueId();
        btnClose->SetID(idCloseBtn);

        Bind(wxEVT_VOBJ, &OOPWindow::OnClose, this, idCloseBtn);
    }
}

void OOPWindow::NotifyShowHideAndStick(bool show) {
    wxWindow *winMain = GetParent();
    if (!winMain) {
        return;
    }

    wxASSERT(winMain->IsShown());
    wxPoint ptMain(winMain->GetScreenPosition());

    if (show) {
        if (m_stickyToMain) {
            Move(ptMain.x + m_relativePos.x, ptMain.y + m_relativePos.y);
        }
    }
}

void OOPWindow::SetStickyToMainState(bool sticky) {
    m_stickyToMain = sticky;

    if (m_stickyToMain) {
        wxWindow *winMain = GetParent();
        wxPoint ptMain(winMain->GetScreenPosition());
        wxPoint ptThis(GetScreenPosition());

        m_relativePos.x = ptThis.x - ptMain.x;
        m_relativePos.y = ptThis.y - ptMain.y;
    }
}

void OOPWindow::SetSkin() {
    DoSetSkin();

    if (IsReseting()) {
        g_app->RemoveFromWindowManager(this);
        g_app->AddToWindowManager(this);

        if (GetParent() == NULL) {
            g_app->GetWindowManager().SetMainWindow(this);
        }
    }
}

void OOPWindow::OnResetSkin(VdkVObjEvent &) {
    ResetSkin();
}

void OOPWindow::ResetSkin() {
    m_stickyToMain = false;
    ZeroPoint(m_relativePos);

    PreResetSkin();
    Reset();
    SetSkin();
}

void OOPWindow::DoMinimize() {
    Hide();
}

void OOPWindow::OnCloseNative(wxCloseEvent &e) {
    DoCloseWindow();
    e.Skip(true);
}

void OOPWindow::OnClose(VdkVObjEvent &e) {
    VdkButton *btnClose = (VdkButton *) e.GetCtrl();
    btnClose->Update(VdkButton::NORMAL, e.GetVObjDC());

    DoCloseWindow();
}

void OOPWindow::DoCloseWindow() {
    NotifyShowHideAndStick(false);
    g_app->HidePanel(this);
}

wxRect OOPWindow::GetRectToSerialize() const {
    wxRect rcThis(GetScreenRect());

    if (m_stickyToMain) {
        wxASSERT(GetParent());
        rcThis.SetPosition(m_relativePos);
    }

    return rcThis;
}

void OOPWindow::OnAttachToMain(wxCommandEvent &) {
    SetStickyToMainState(true);
}

void OOPWindow::OnDetachFromMain(wxCommandEvent &) {
    SetStickyToMainState(false);
}

void OOPWindow::UpdateStickyState(OOPStickyWindowState &sstate) const {
    sstate.m_stickyToMain = m_stickyToMain;
    sstate.m_rect = GetRectToSerialize();
}
