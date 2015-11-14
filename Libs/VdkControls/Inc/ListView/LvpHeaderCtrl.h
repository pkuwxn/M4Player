/***************************************************************
 * Name:      LvpHeaderCtrl.h
 * Purpose:   VdkListView ������
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-26
 **************************************************************/
#pragma once
#include "IListViewPlaugin.h"
#include "VdkControl.h"

/// ʵ�� VdkListView ��껬��ʱ����������
class LvpHeaderCtrl : public VdkControl, public IListViewPlaugin
{
public:

	/// ���캯��
	LvpHeaderCtrl(ListView* listview);

private:

	virtual bool CanDestoryByListView() const { return false; }

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

private:

    virtual void OnMove(int dx, int dy);
    virtual void OnSize(int dx, int dy);

private:

	int					m_yFix;
	wxBitmap			m_bmBkGnd;
};
