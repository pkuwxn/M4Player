#pragma once
#include "VdkStaticImage.h"

class VdkWindow;
/// \brief 负责 OOPlayer 的图标生成、异常管理等等
class OOPIcon : public VdkStaticImage
{
public:

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 设置使用的图标
	void SetFrame(wxTopLevelWindow* frame);

private:

	// 绘制控件
	virtual void DoDraw(wxDC& dc);

private:

	wxIcon m_icon;
	wxIconBundle m_icons;

	DECLARE_DYNAMIC_VOBJECT
};
