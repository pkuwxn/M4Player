#pragma once
#include "VdkNativeCtrl.h"
#include <wx/richtext/richtextctrl.h>

/// \brief 封装富文本编辑框
class VdkRichTextCtrl : public wxRichTextCtrl, 
						public VdkNativeCtrl< VdkRichTextCtrl >
{
public:

	/// \brief 默认构造函数
	VdkRichTextCtrl(){ Init(); }

	/// \brief 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 构建控件
	void Create( VdkWindow* win, 
				 const wxString& strName, 
				 const wxRect& rc, 
				 long style );

private:

	/// \brief 初始化控件
	void Init();

	/// \brief 绘制控件
	virtual void DoDraw(wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	long				m_nativeStyle;
	int					m_borderWeight;
	wxPen				m_borderPen;

	DECLARE_DYNAMIC_VOBJECT
};
