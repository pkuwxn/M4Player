#pragma once
#include "VdkLabel.h"

/// \brief 一段具有渐变效果的分割线
class VdkGradientLine : public VdkLabel
{
public:

	/// \brief 默认构造函数
	VdkGradientLine();

private:

	// 绘制控件
	virtual void DoDraw(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	int				m_dxLine; // 渐变色条 x 轴上的绘图起始点
	int				m_dyLine;

	wxBitmap		m_gradientLine;

	DECLARE_DYNAMIC_VOBJECT
};
