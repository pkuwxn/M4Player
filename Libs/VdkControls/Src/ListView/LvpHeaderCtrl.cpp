/***************************************************************
 * Name:      LvpHeaderCtrl.cpp
 * Purpose:   VdkListView 标题栏
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-26
 **************************************************************/
#include "StdAfx.h"
#include "ListView/LvpHeaderCtrl.h"

#include "ListView/VdkListView.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

#include "../Images/list_ctrl_header.xpm"

//////////////////////////////////////////////////////////////////////////

enum {
    STD_HEIGHT = 20, // 标题栏高度
};

LvpHeaderCtrl::LvpHeaderCtrl(ListView *listview)
    : IListViewPlaugin(listview), m_yFix(wxNOT_FOUND) {
    m_strName = listview->GetName() + L"_header";

    wxRect rcListView(listview->GetRect());
    m_Rect = rcListView;
    m_Rect.height = STD_HEIGHT;

    rcListView.y += STD_HEIGHT;
    rcListView.height -= STD_HEIGHT;
    listview->SetRect(rcListView);

    // 我们自己画边框
    listview->RemoveStyle(VCS_BORDER_SIMPLE);

    SetVdkWindow(listview->GetVdkWindow());
    SetParent(listview->GetParent());
    m_Window->AddCtrl(this);

    //==============================================

    m_bmBkGnd = wxBitmap(LCHEADER_xpm);
    if (m_bmBkGnd.GetWidth() < m_Rect.width) {
        int quarter(m_bmBkGnd.GetWidth() / 4);
        wxRect rcTile(0, 0, quarter * 2, m_bmBkGnd.GetHeight());
        m_bmBkGnd = TileBitmap(m_bmBkGnd, m_Rect.width, rcTile.height, rcTile);
    }
}

void LvpHeaderCtrl::DoDraw(wxDC &dc) {
    ListView *listview = GetListView();

    // 绘制边框
    wxPen borderPen(listview->GetBorderColour());
    dc.SetPen(borderPen);

    wxRect rc(m_Rect);
    rc.height += listview->GetRect().GetHeight();
    DrawRectangle(dc, rc.Inflate(1));

    //////////////////////////////////////////////////////////////////////////

    dc.DrawBitmap(m_bmBkGnd, m_Rect.x, m_Rect.y);

    if (m_yFix == wxNOT_FOUND) {
        m_yFix = (STD_HEIGHT - dc.GetCharHeight()) / 2;
    }

    int x = 0;

    ListView::Adapter *adapter = listview->GetAdapter();
    for (size_t i = 0; i < adapter->GetColumnCount(); ++i) {
        ListView::Adapter::ColumnStyle &cs = adapter->GetColumnStyle(i);
        int xFix = 5;

        if (cs.textAlign & TEXT_ALIGN_CENTER_X_Y) {
            int w;
            dc.GetTextExtent(cs.heading, &w, NULL);
            xFix = (cs.width - w) / 2;
        }

        dc.SetFont(m_Font);
        dc.SetTextForeground(*wxBLACK);
        dc.DrawText(cs.heading,
                    m_Rect.x + xFix + x,
                    m_Rect.y + m_yFix);

        dc.SetPen(*wxLIGHT_GREY_PEN);
        dc.DrawLine(m_Rect.x + x + cs.width,
                    m_Rect.y + 2,
                    m_Rect.x + x + cs.width,
                    m_Rect.y + STD_HEIGHT - 2);

        x += cs.width;
    }
}

void LvpHeaderCtrl::OnMove(int dx, int dy) {
    // TODO:
}

void LvpHeaderCtrl::OnSize(int dx, int dy) {
    // TODO:
}
