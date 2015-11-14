/***************************************************************
 * Name:      VdkShortcutCtrl.h
 * Purpose:   以可视化的方式定义键盘快捷键
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-18
 **************************************************************/
#pragma once
#include "VdkEdit.h"

/// 以可视化的方式定义键盘快捷键
class VdkShortcutCtrl : public VdkEdit
{
public:

	/// 默认构造函数
	VdkShortcutCtrl();

	/// XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// 设置当前显示的快捷键
	void SetShortcut(int modifiers, int keycode, wxDC* pDC);

	/// 清空当前快捷键
	void Clear(wxDC* pDC);

private:

	// 处理键盘按键事件
	virtual void DoHandleKeyEvent(VdkKeyEvent& e);

private:

	int m_modifiers;
	int m_keycode;

	DECLARE_DYNAMIC_VOBJECT
};
