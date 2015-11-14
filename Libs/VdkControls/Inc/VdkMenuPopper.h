/***************************************************************
 * Name:      VdkMenuPopper.h
 * Purpose:   Code for VdkMenuPopper declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2010-12-03
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkToggleButton.h"

class VdkMenu;
class VdkWindow;

/// \brief 按下按钮，弹出菜单
class VdkMenuPopper : public VdkToggleButton, public VdkEventFilter
{
public:

	/// \brief XRC 动态创建的关键函数
	///
	/// 一个 VdkControl 的派生类假如要实现动态创建，则应该实现这个函数
	/// \todo 父控件
	virtual void Create(wxXmlNode* node);

	/// \brief 弹出相关联的菜单
	void PopupMenu();

protected:

	/// \brief 设置针对父窗口的鼠标钩子
	///
	/// 一旦下拉菜单已被显示，继续点击按钮不会取消菜单，而只有在按钮外部
	/// 点击才会。
	void HookMouseEvent();

	/// \brief 某些准备工作
	/// \warning 派生类构建时必须调用
	void Prepare(wxDC* pDC);

	/// \brief 过滤事件
	virtual bool FilterEvent
		(const VdkEventFilter::EventForFiltering& e);

	/// \brief 处理鼠标事件
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief 接收、处理通知信息
	virtual void DoHandleNotify(const VdkNotify& notice);

	DECLARE_DYNAMIC_VOBJECT
};
