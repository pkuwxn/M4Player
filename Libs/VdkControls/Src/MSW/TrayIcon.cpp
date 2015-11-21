#include "StdAfx.h"
#include "MSW/TrayIcon.h"

#ifdef _MSC_VER
#   pragma warning(disable:4800)
#endif // _MSC_VER

#include "VdkWindow.h"
#include "MSW/ExtMenu.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


//////////////////////////////////////////////////////////////////////////

// 通知主线程激发Timer
extern const wxEventType wxEVT_TI_TIMERUSER;
const wxEventType wxEVT_TI_TIMERUSER = wxNewEventType();

//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(TrayIcon, wxTaskBarIcon)

    EVT_MENU(TI_MENU_EXIT, TrayIcon::OnQuit)
    EVT_TASKBAR_LEFT_DCLICK(TrayIcon::OnLeftDoubleClick)

    EVT_TIMER(TI_FLASH_TIMER, TrayIcon::OnTimer)
    EVT_COMMAND(wxID_ANY, wxEVT_TI_TIMERUSER, TrayIcon::OnTimerUser)

END_EVENT_TABLE()

enum { // Timer users
    TU_FLASH
};

//////////////////////////////////////////////////////////////////////////

TrayIcon::TrayIcon(VdkWindow *frame,
                   const wxIcon &icon,
                   wxString strTrunkTooltip)
    : m_mainFrame(frame),
      m_bNull(false),
      m_state(TI_STATE_NOMAL),
      m_timer(this, TI_FLASH_TIMER) {
    frame->SetTrayIcon(this);
    SetIcon(icon, strTrunkTooltip);
    m_strTrunkTooltip = strTrunkTooltip;

    m_nid.cbSize = sizeof(m_nid);
    m_nid.hIcon = (HICON)m_icon.GetHICON();
    m_nid.hWnd = GetHwndOf((wxWindow *)m_win);
    m_nid.uCallbackMessage = RegisterWindowMessage(L"wxTaskBarIconMessage");
    m_nid.uID = 99;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    m_nid.dwInfoFlags = NIIF_INFO;
    if (!strTrunkTooltip.IsEmpty()) {
        wxStrncpy(m_nid.szTip, strTrunkTooltip.c_str(), WXSIZEOF(m_nid.szTip));
    }

    Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}

void TrayIcon::SetMainFrame(VdkWindow *frame) {
    m_mainFrame = frame;
}

void TrayIcon::SetToolTip(wxString strTooltip) {
    if (!strTooltip.IsEmpty()) {
        if (strTooltip.Mid(0, m_strTrunkTooltip.Length()) != m_strTrunkTooltip) {
            strTooltip = m_strTrunkTooltip + L"\n" + strTooltip;
        }
    } else {
        strTooltip = m_strTrunkTooltip;
    }

    wxStrncpy(m_nid.szTip, strTooltip.c_str(), WXSIZEOF(m_nid.szTip));
    Shell_NotifyIcon(NIM_MODIFY, &m_nid);

    m_strTooltip = strTooltip;
}

void TrayIcon::Flash(bool bEnabled) {
    if (!wxThread::IsMain()) {
        wxCommandEvent e(wxEVT_TI_TIMERUSER);
        e.SetId(TI_FLASH_TIMER);
        e.SetExtraLong(bEnabled);
        wxPostEvent(this, e);

        return;
    }

    if (!bEnabled) {
        if (m_timer.IsRunning()) {
            m_timer.Stop();
            wxTimerEvent e;
            if (m_bNull) {
                OnTimer(e);
            }

            m_state = TI_STATE_NOMAL;
            SetToolTip(wxEmptyString);
        }
    } else {
        if (!m_timer.IsRunning()) {
            m_state = TI_STATE_FLASHING;
            m_timer.Start(500);
        }
    }
}

void TrayIcon::Hide(bool bToHide) {
    if (bToHide) {
        m_state = TI_STATE_HID;
        Shell_NotifyIcon(NIM_DELETE, &m_nid);
    } else {
        m_state = TI_STATE_NOMAL;
        Shell_NotifyIcon(NIM_ADD, &m_nid);
    }
}

void TrayIcon::OnTimer(wxTimerEvent &) {
    if (m_bNull) {
        m_nid.hIcon = (HICON)m_icon.GetHICON();
        Shell_NotifyIcon(NIM_MODIFY, &m_nid);
    } else {
        m_nid.hIcon = (HICON)NULL;
        Shell_NotifyIcon(NIM_MODIFY, &m_nid);
    }

    m_bNull = !m_bNull;
}

void TrayIcon::OnTimerUser(wxCommandEvent &e) {
    switch (e.GetId()) {
    case TI_FLASH_TIMER:

        Flash(e.GetExtraLong());
        break;

    default:
        break;
    }
}

void TrayIcon::OnLeftDoubleClick(wxTaskBarIconEvent &) {
    if (m_mainFrame) {
        m_mainFrame->GetWindowHandle()->Show();
        if (m_timer.IsRunning()) {
            Flash(false);
        }
    }
}

wxMenu *TrayIcon::CreatePopupMenu() {
    ExtMenu *popup = new ExtMenu;

    popup->Append(TI_MENU_EXIT, L"退出(&E)");
    return popup;
}

void TrayIcon::OnQuit(wxCommandEvent &) {
    RemoveIcon();
    if (m_mainFrame) {
        m_mainFrame->GetWindowHandle()->Close(true);
    }
}
