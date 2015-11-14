/***************************************************************
 * Name:      VdkSearchCtrl.h
 * Purpose:   Code for VdkSearchCtrl declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-03-13
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkControl.h"
#include "VdkBitmapArray.h"

/// \brief 为 VdkSearchCtrl 设置初始化信息
class VdkSearchCtrlInitializer : 
	public VdkCtrlInitializer< VdkSearchCtrlInitializer >
{
public:
private:

	friend class VdkSearchCtrl;
};

//////////////////////////////////////////////////////////////////////////

/// \brief VdkSearchCtrl 的风格属性集
enum VdkSearchCtrlStyle {
};

class VdkEdit;
class VdkButton;
/// \brief 标准化的搜索框
class VdkSearchCtrl : public VdkCtrlHandler
{
public:

	/// \brief 构造函数
	VdkSearchCtrl();

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 实际构建函数
	void Create(const VdkSearchCtrlInitializer& init_data);

	/// \brief 获取文本框
	VdkEdit* GetEditCtrl() { return m_edit; }

	/// \brief 获取文本框的控件标识符
	VdkCtrlId GetEditID() const;

private:

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc);

	/// \brief 擦除背景
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	/// \brief 接收、处理通知信息
	virtual void DoHandleNotify(const VdkNotify& notice);

	//////////////////////////////////////////////////////////////////////////

	VdkEdit* m_edit;
	VdkButton* m_button;

	VdkBitmapArray m_bmpBorder; // 边框位图

	int m_borderWeight; // 边框宽度
	int m_radius; // 圆角矩形小曲圆半径

	wxPen m_borderPen; // 正常模式下边框
	wxPen m_hilightPen; // 高亮边框

	DECLARE_DYNAMIC_VOBJECT
};
