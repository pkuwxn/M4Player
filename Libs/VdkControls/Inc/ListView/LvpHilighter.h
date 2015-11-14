/***************************************************************
 * Name:      LvpHilighter.h
 * Purpose:   实现 VdkListView 鼠标滑过时高亮所在行
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#pragma once
#include "IListViewPlaugin.h"

/// 实现 VdkListView 鼠标滑过时高亮所在行
class LvpHilighter : public IListViewPlaugin
{
public:

	/// 构造函数
	LvpHilighter(ListView* list, const wxBrush& hilighted);
    
private:

    virtual void OnEraseRow(int row, wxDC& dc);
    virtual void OnDragg(int rowAtPointer, VdkMouseEvent& e) {}
    virtual void OnDClick(int rowAtPointer, VdkMouseEvent& e) {}
    virtual void OnClickDown(int rowAtPointer, VdkMouseEvent& e) {}
    virtual void OnClickUp(int rowAtPointer, VdkMouseEvent& e) {}
    virtual void OnRightUp(int rowAtPointer, VdkMouseEvent& e) {}
    virtual void OnKey(VdkKeyEvent& ke) {}
    virtual void OnMove(int dx, int dy) {}
    virtual void OnSize(int dx, int dy) {}

private:

	wxBrush m_hilighted; // 高亮项的背景画刷
};
