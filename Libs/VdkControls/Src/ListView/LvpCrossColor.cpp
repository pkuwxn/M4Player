/***************************************************************
 * Name:      LvpCrossColor.cpp
 * Purpose:   实现 VdkListView 的隔行变色特性
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-3-3
 **************************************************************/
#include "StdAfx.h"
#include "ListView/LvpCrossColor.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

LvpCrossColor::LvpCrossColor
(ListView *list, const wxBrush &b1, const wxBrush &b2)
    : IListViewPlaugin(list), m_crossBrush1(b1), m_crossBrush2(b2) {

}

void LvpCrossColor::OnEraseRow(int row, wxDC &dc) {
    if (row % 2 ==0) {
        dc.SetBrush(m_crossBrush1);
    } else {
        dc.SetBrush(m_crossBrush2);
    }
}
