#pragma once
#include "VdkHotArea.h"

class VdkWindow;
/// \brief 显示一个超链接
class VdkHyperLink : public VdkHotArea
{
public:

	/// \brief 默认构造函数
	VdkHyperLink();

	/// \brief 执行实际构建操作
	void Create(VdkWindow* Window,
                const wxString& strName,
                const wxRect& rc,
                const wxString& strUrl,
                const wxString& strCaption);

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

private:

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc);

	wxString					m_strCaption;
	wxPen						m_pen;
	int							m_yFix;

	DECLARE_DYNAMIC_VOBJECT
};
