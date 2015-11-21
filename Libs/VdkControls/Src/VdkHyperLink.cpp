#include "StdAfx.h"
#include "VdkHyperLink.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT(VdkHyperLink)

VdkHyperLink::VdkHyperLink()
    : m_pen(*wxBLUE), m_yFix(0) {
    SetAddinStyle(VCS_ERASE_BG);
}

void VdkHyperLink::Create(wxXmlNode *node) {
    VdkHotArea::Create(node);

    Create(m_Window, m_strName, m_Rect,
           XmlGetChildContent(node, L"url", wxEmptyString),
           GetXrcTextBlock(node));
}

void VdkHyperLink::Create(VdkWindow *Window,
                          const wxString &strName,
                          const wxRect &rc,
                          const wxString &strUrl,
                          const wxString &strCaption) {
    m_strName = strName;
    m_Rect = rc;
    SetVdkWindow(Window);

    m_strUrl = strUrl;
    m_strCaption = strCaption;

    // TODO: 是否应该继承自 VdkLabel ？否则文本居中部分的代码无法重用
    /**/
    if (!m_strCaption.IsEmpty()) {
        int w, h;
        m_WindowImpl->GetTextExtent(m_strCaption, &w, &h, 0, 0, &m_Font);

        m_Rect.width = w;
        m_Rect.height = h;

        // m_yFix = (m_Rect.height - h) / 2;
    }

    StartSense();
}

void VdkHyperLink::DoDraw(wxDC &dc) {
    if (!m_strCaption.IsEmpty()) {
        dc.SetFont(m_Font);

        dc.SetTextForeground(m_pen.GetColour());
        dc.DrawText(m_strCaption, m_Rect.x, m_Rect.y + m_yFix);

        int y = m_Rect.GetBottom() - m_yFix + 1; // +1 是增加一点距离
        dc.SetPen(m_pen);
        dc.DrawLine(m_Rect.x, y, m_Rect.x + m_Rect.width, y);
    }
}
