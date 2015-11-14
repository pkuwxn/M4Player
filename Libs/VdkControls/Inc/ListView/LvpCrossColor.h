/***************************************************************
 * Name:      LvpCrossColor.h
 * Purpose:   实现 VdkListView 的隔行变色特性
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#pragma once
#include "IListViewPlaugin.h"

/// 实现 VdkListView 的隔行变色特性
class LvpCrossColor : public IListViewPlaugin
{
public:

	/// 构造函数
	LvpCrossColor(ListView* list, const wxBrush& b1, const wxBrush& b2);

private:

	virtual void OnEraseRow(int row, wxDC& dc);

private:

	wxBrush m_crossBrush1; // 交错颜色 1
	wxBrush m_crossBrush2; // 交错颜色 2
};
