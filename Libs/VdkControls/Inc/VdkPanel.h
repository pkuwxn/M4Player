/***************************************************************
 * Name:      VdkPanel.h
 * Purpose:   Code for VListCtrl implementation
 * Author:    Ning (vanxining@139.com)
 * Created:   2010-09-11
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkControl.h"
class RedrawCallback;

/*!\brief 类似于 wxPanel 的子控件组合控件
 *
 * VdkPanel 往往用于组织一组逻辑性较强的控件。例如选项卡切换显示另一套控件。
**/
class VdkPanel : public VdkCtrlHandler
{
public:

	/// \brief 默认构造函数
	VdkPanel();
	/// \brief 析构函数
	~VdkPanel();

	/// \brief 执行实际构建操作
	void Create(VdkWindow* parent, const wxString& strName, const wxRect& rc);

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/////////////////////////////////////////////////////////////////////////

	/// \brief 设置回调绘制对象
	void SetRedrawCallback(RedrawCallback* Redrawer) { m_redrawer = Redrawer; }

private:

	/// \brief 擦除背景
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	RedrawCallback*				m_redrawer;

	wxBrush						m_bgBrush;
	GradientBgInfo*				m_gbi;
	BackgroundInfo*				m_bi;

	DECLARE_DYNAMIC_VOBJECT
};
