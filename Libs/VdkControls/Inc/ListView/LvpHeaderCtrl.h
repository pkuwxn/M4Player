/***************************************************************
 * Name:      LvpHeaderCtrl.h
 * Purpose:   VdkListView 标题栏
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-26
 **************************************************************/
#pragma once
#include "IListViewPlaugin.h"
#include "VdkControl.h"

/// 实现 VdkListView 鼠标滑过时高亮所在行
class LvpHeaderCtrl : public VdkControl, public IListViewPlaugin
{
public:

	/// 构造函数
	LvpHeaderCtrl(ListView* listview);

private:

	virtual bool CanDestoryByListView() const { return false; }

	// 绘制控件
	virtual void DoDraw(wxDC& dc);

private:

    virtual void OnMove(int dx, int dy);
    virtual void OnSize(int dx, int dy);

private:

	int					m_yFix;
	wxBitmap			m_bmBkGnd;
};
