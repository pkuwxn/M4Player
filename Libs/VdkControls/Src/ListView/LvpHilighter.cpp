/***************************************************************
 * Name:      LvpHilighter.cpp
 * Purpose:   实现 VdkListView 鼠标滑过时高亮所在行
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#include "StdAfx.h"
#include "ListView/LvpHilighter.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

LvpHilighter::LvpHilighter
(ListView *list, const wxBrush &hilighted)
    : IListViewPlaugin(list), m_hilighted(hilighted) {

}

void LvpHilighter::OnEraseRow(int row, wxDC &dc) {

}
