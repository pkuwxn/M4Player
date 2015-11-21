#include "StdAfx.h"
#include "VdkSwitcher.h"

#include "VdkDC.h"
#include "VdkWindow.h"
#include "VdkToggleButton.h"
#include "VdkEvent.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT(VdkSwitcher);

//////////////////////////////////////////////////////////////////////////

VdkSwitcher::VdkSwitcher(VdkWindow *parent,
                         const wxString &strName,
                         const wxString &strFileName,
                         const wxRect &rc,
                         int nExpl) {
    Init();

    VdkUtil::ImRead(m_bmp, strFileName);
    Create(parent, strName, rc, m_bmp, nExpl);
}

void VdkSwitcher::Init() {
    m_curr = -1;
    m_h = 0;
    m_nExpl = 3;
}

void VdkSwitcher::Create(wxXmlNode *node) {
    int nExpl(3);
    wxString strTmp(XmlGetChildContent(node, L"expl"));
    if (!strTmp.IsEmpty()) {
        strTmp.ToLong((long *) &nExpl);
    }

    GetXrcImage(node, m_bmp);
    m_align = GetXrcAlign(node);
    Create(m_Window, GetXrcName(node), GetXrcRect(node), m_bmp, nExpl);
}

void VdkSwitcher::Create(VdkWindow *win,
                         const wxString &strName,
                         const wxRect &rc,
                         const wxBitmap &bmp,
                         int nExpl) {
    m_strName = strName;
    m_Rect = rc;
    SetVdkWindow(win);

    m_nExpl = nExpl;
    m_h = m_bmp.GetHeight();
    m_last = NULL;
    m_bmp = wxBitmap(bmp.ConvertToImage().
                     Rescale(m_Rect.GetWidth() * m_nExpl, m_h));
}

void VdkSwitcher::Switch(int id, wxDC *pDC) {
    tab_map::iterator i;
    for (i = m_tabs.begin(); i != m_tabs.end(); ++i) {
        if (i->second.id == id) {
            Switch(i->second.button, pDC);
            break;
        }
    }
}

void VdkSwitcher::Switch(VdkToggleButton *btn, wxDC *pDC) {
    if (btn == m_last) {
        return;
    }

    TabInfo &ti = m_tabs[ btn ];
    wxRect rc;
    int id(ti.id);

    if (m_last) {
        int idLast = m_tabs[m_last].id;
        if (idLast > id) {   // ↓
            ResetCoords(id+1);
        } else { // ↑
            ResetCoords(-id);
        }

        m_last->Toggle(false, NULL);
    }

    if (id > 0) {
        rc = btn->GetRect();
        rc.y = m_Rect.y + m_bmp.GetHeight()*id;

        btn->SetRect(rc, pDC);
    }

    btn->Toggle(true, NULL);

    tab_map::iterator i;
    for (i = m_tabs.begin(); i != m_tabs.end(); ++i) {
        i->second.button->Draw(*pDC);
    }

    if (m_last) {
        m_tabs[m_last].win->Freeze();
    }
    if (ti.win) {
        ti.win->Thaw(pDC);
    }

    if (IsReadyForEvent()) {
        FireEvent(pDC, (void *) id);
    }

    m_last = btn;
    m_curr = id;
}

void VdkSwitcher::Disable(bool bDisabled, wxDC *pDC) {
    tab_map::iterator i;
    for (i = m_tabs.begin(); i != m_tabs.end(); ++i) {
        i->second.button->Show(!bDisabled, pDC);

        if (pDC) {
            i->second.button->Draw(*pDC);
        }
    }
}

void VdkSwitcher::AddTab(const wxString &strCaption, VdkControl *win) {
    TabInfo ti = { (int) m_tabs.size(), strCaption, NULL, win };

    int y = m_Rect.y;
    if (ti.id) {
        WalkCoords(ti.id);
        y = m_Rect.y + m_Rect.height - m_h;
    }

    ti.rc = wxRect(0, m_h * (ti.id + 1),
                   m_Rect.width,
                   m_Rect.height - m_h * (ti.id + 1));

    wxBitmap bm(m_bmp.GetSubBitmap
                (wxRect(0, 0, m_bmp.GetWidth(), m_bmp.GetHeight())));
    int id = VdkGetUniqueId();
    ti.button = new VdkToggleButton;
    ti.button->Create(VdkButtonInitializer().
                      id(id).
                      window(m_Window).
                      name(wxString::Format(L"%s_%d", m_strName.c_str(), ti.id)).
                      rect(wxRect(m_Rect.x, y, m_Rect.GetWidth(), m_h)).
                      caption(strCaption).
                      bitmap(bm).
                      explode(m_nExpl).
                      resizeable(true));

    ti.button->SetOwnerControl(true);
    m_WindowImpl->Bind(wxEVT_VOBJ, &VdkSwitcher::OnSwitchTabs, this, id);

    //////////////////////////////////////////////////////////////////////////

    if (win) {
        m_Window->AddCtrl(win);
        ti.rc.width = win->GetRect().width; // 需要减去滚动条的宽度

        win->SetParent(this);
        win->SetRect(ti.rc);
    }

    if (ti.id == 0) {
        m_curr = 0;
        m_last = ti.button;

        ti.button->Toggle(true, NULL, false);
    } else if (win) {
        win->Freeze();
    }

    m_Window->AddCtrl(ti.button);
    m_tabs[ ti.button ] = ti;
}

void VdkSwitcher::WalkCoords(int id) {
    wxRect rc;

    tab_map::iterator i;
    for (i = m_tabs.begin(); i != m_tabs.end(); ++i) {
        TabInfo &ti = i->second;
        if (ti.id < id) {
            if (ti.id > 0) {
                rc = ti.button->GetRect();
                rc.y -= m_h;

                ti.button->SetRect(rc, NULL);
            }

            ti.win->GetSize(&ti.rc.width, NULL);
            ti.rc.height -= m_h;
            ti.win->SetRect(ti.rc);
        }
    }
}

void VdkSwitcher::ResetCoords(int id) {
    wxRect rc;

    tab_map::iterator i;
    for (i = m_tabs.begin(); i != m_tabs.end(); ++i) {
        if (id > 0) {   // 此ID及此ID以下恢复
            if (i->second.id >= id) {
                rc = i->second.button->GetRect();
                rc.y = m_Rect.y + m_Rect.height - m_h*(m_tabs.size()-i->second.id);

                i->second.button->SetRect(rc, NULL);
            }
        } else { // 此ID及此ID以上恢复
            if (i->second.id <= -id) {
                rc = i->second.button->GetRect();
                rc.y = m_Rect.y + m_h*i->second.id;

                i->second.button->SetRect(rc, NULL);
            }
        }
    }
}

void VdkSwitcher::OnSwitchTabs(VdkVObjEvent &e) {
    VdkToggleButton *btn = e.GetCtrl<VdkToggleButton>();
    if (btn == m_last) {
        return;
    }

    wxDC *pDC = e.GetVObjDC();
    Switch(btn, pDC);
}

void VdkSwitcher::DoHandleNotify(const VdkNotify &notice) {
    switch (notice.GetNotifyCode()) {
    case VCN_WINDOW_RESIZED:

        if (m_align) {
            if (notice.GetWparam()) {
                tab_map::iterator i;
                for (i = m_tabs.begin(); i != m_tabs.end(); ++i) {
                    i->second.button->Resize(m_Rect.width, 0);
                }
            }

            if (notice.GetLparam()) {
                wxRect rc;
                tab_map::iterator i;
                for (i = m_tabs.begin(); i != m_tabs.end(); ++i) {
                    TabInfo &ti = i->second;
                    if (ti.id > m_curr) {
                        rc = ti.button->GetRect();
                        rc.y += notice.GetLparam();
                        ti.button->SetRect(rc, NULL);
                    }
                }

            }

        }

        break;

    default:

        break;
    }

}
