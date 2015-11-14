/***************************************************************
 * Name:      OnMenuSlider.h
 * Purpose:   使用 VdkSlider 来设置窗口透明度
 * Author:    Ning (vanxining@139.com)
 * Created:   2011.04.02
 * Copyright: Wang Xiao Ning
 **************************************************************/
#pragma once
#include "VdkMenu.h"
#include "VdkSlider.h"

/// \brief 使用 VdkSlider 来设置窗口透明度
class OnMenuSlider : public VdkMenuXrcCtrlWrapper
{
public:

	/// \brief 构造函数
	OnMenuSlider(wxTopLevelWindow* window);

private:

	/// \brief 实例化要附加到 VdkMenu 上的控件
	virtual VdkControl* Implement(VdkWindow* menuWin, const wxPoint& pos);

	void OnSlider(VdkVObjEvent& e);

	// 保存控件状态
	virtual void SaveState();

	// 还原控件状态
	virtual void RestoreState();

	//////////////////////////////////////////////////////////////////////////
	
	double m_percentage; // 状态保持
	wxTopLevelWindow* m_window;
};
