#pragma once
#include "VdkStaticImage.h"

/// \brief 显示一个具有圆角效果头像
class VdkAvatar : public VdkStaticImage
{
public:

	/// \brief 默认构造函数
	VdkAvatar();

	/// \brief 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 设置曲率
	void SetRadius(int radius) { m_radius = radius; }

private:

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc);

	/// \brief 处理鼠标事件
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	int						m_radius;
	wxPen					m_borderPen;

	VdkMouseEventType		m_ptrState; // 当前鼠标指针的状态

	DECLARE_DYNAMIC_VOBJECT
};
