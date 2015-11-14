/***************************************************************
 * Name:      OnMenuSlider.h
 * Purpose:   ʹ�� VdkSlider �����ô���͸����
 * Author:    Ning (vanxining@139.com)
 * Created:   2011.04.02
 * Copyright: Wang Xiao Ning
 **************************************************************/
#pragma once
#include "VdkMenu.h"
#include "VdkSlider.h"

/// \brief ʹ�� VdkSlider �����ô���͸����
class OnMenuSlider : public VdkMenuXrcCtrlWrapper
{
public:

	/// \brief ���캯��
	OnMenuSlider(wxTopLevelWindow* window);

private:

	/// \brief ʵ����Ҫ���ӵ� VdkMenu �ϵĿؼ�
	virtual VdkControl* Implement(VdkWindow* menuWin, const wxPoint& pos);

	void OnSlider(VdkVObjEvent& e);

	// ����ؼ�״̬
	virtual void SaveState();

	// ��ԭ�ؼ�״̬
	virtual void RestoreState();

	//////////////////////////////////////////////////////////////////////////
	
	double m_percentage; // ״̬����
	wxTopLevelWindow* m_window;
};
