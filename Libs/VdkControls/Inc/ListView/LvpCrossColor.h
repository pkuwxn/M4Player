/***************************************************************
 * Name:      LvpCrossColor.h
 * Purpose:   ʵ�� VdkListView �ĸ��б�ɫ����
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#pragma once
#include "IListViewPlaugin.h"

/// ʵ�� VdkListView �ĸ��б�ɫ����
class LvpCrossColor : public IListViewPlaugin
{
public:

	/// ���캯��
	LvpCrossColor(ListView* list, const wxBrush& b1, const wxBrush& b2);

private:

	virtual void OnEraseRow(int row, wxDC& dc);

private:

	wxBrush m_crossBrush1; // ������ɫ 1
	wxBrush m_crossBrush2; // ������ɫ 2
};
