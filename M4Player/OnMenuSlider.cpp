/***************************************************************
 * Name:      OnMenuSlider.cpp
 * Purpose:   使用 VdkSlider 来设置窗口透明度
 * Author:    Ning (vanxining@139.com)
 * Created:   2011.04.02
 * Copyright: Wang Xiao Ning
 **************************************************************/
#include "StdAfx.h"
#include "OnMenuSlider.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

OnMenuSlider::OnMenuSlider(wxTopLevelWindow *window)
    : VdkMenuXrcCtrlWrapper
      (wxString::Format
       (L"<menuCtrl>"
        L"<object name=\"progress\" class=\"VdkSlider\">"
        L"<rect>0,25,193,20</rect>"
        L"<thumb_image>../../App/progress_thumb_menu.bmp</thumb_image>"
        L"<fill_image>../../App/progress_fill_menu.bmp</fill_image>"
        L"</object>"
        L"<object name=\"desc\" class=\"VdkLabel\">"
        L"设置窗口透明度:"
        L"<rect>38,4,193,%d</rect>"
        L"<text-align>center_y</text-align>"
        L"</object>"
        L"</menuCtrl>", window->GetCharHeight() + 4)),
      m_percentage(100),
      m_window(window) {

}

VdkControl *OnMenuSlider::Implement(VdkWindow *menuWin, const wxPoint &pos) {
    VdkMenuXrcCtrlWrapper::Implement(menuWin, pos);

    //-------------------------------------------------------

    m_pCtrl->SetAddinStyle(VCS_HAND_CURSOR);
    m_window->Bind(wxEVT_VOBJ, &OnMenuSlider::OnSlider, this, m_id);

    return m_pCtrl;
}

void OnMenuSlider::OnSlider(VdkVObjEvent &e) {
    if ((int) e.GetClientData() == DRAGGING) {
        VdkSlider *slider = (VdkSlider *) m_pCtrl;
        wxByte nonTransp = 255 * slider->GetProgress();

        m_window->SetTransparent(nonTransp);

        wxWindow *win;
        wxWindowList &winlist(m_window->GetChildren());
        wxWindowList::iterator i;
        for (i = winlist.begin(); i != winlist.end(); ++i) {
            win = *i;
            win->SetTransparent(nonTransp);
        }
    }
}

void OnMenuSlider::SaveState() {
    VdkSlider *slider = (VdkSlider *) m_pCtrl;
    m_percentage = slider->GetProgress();
}

void OnMenuSlider::RestoreState() {
    VdkSlider *slider = (VdkSlider *) m_pCtrl;
    slider->GoTo(m_percentage, NULL, false);
}
