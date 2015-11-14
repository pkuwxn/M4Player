#pragma once
#include "VdkLabel.h"
#include <wx/vector.h>

/// \brief VdkRichLabel 里包含的 URL
struct Url {

	wxString	strUrl; ///< URL 地址
	wxString	strUrlDisplay; ///< 显示的实际 URL 文本（可能截短）
	wxString	strDummy; ///< VdkRichLabel 里用来标记 URL 位置的特殊文本

	wxRect		Rect; ///< 虚拟画布上的位置
};

/// \brief 富文本标签
///
/// 当前具有的功能：
/// ** 多行显示；
/// ** URL 解释。
class VdkRichLabel : public VdkLabel
{
public:

	/// \brief 构造函数
	VdkRichLabel();

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 确定 VdkLabel 的实际作用域
	virtual void PrepareRect();

	//////////////////////////////////////////////////////////////////////////

	/// \brief 设置 URL 显示风格（文本颜色、是否显示下划线等）
	void SetUrlStyle(wxString strStyle);

	/// \brief 得到指定序号的 URL
	wxString GetUrl(unsigned id = 0);

private:

	/// \brief 一段彩色文本
	struct ColorTextNode {

		wxString	strText; ///< 文本
		int			width; ///< 文本宽度
		int			x, y; ///< 文本的相对位置
		wxColour	color; ///< 文本颜色
	};

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc);

	/// \brief 处理鼠标事件
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief 接收、处理通知信息
	virtual void DoHandleNotify(const VdkNotify& notice);

	/// \brief 分析 UBB 代码，分解属性值
	bool GetUbbNode( wxString& strUbb, 
					 wxString& strName, 
					 wxString& strAttriValue,
					 wxString& strText );

	/// \brief 绘制 URL
	void DrawUrl(int& UrlIndex, const wxString& strUrlAll, wxDC& dc, int x, int y);
	/// \brief 随机生成与 URL 同长、用来标记其位置的特殊文本
	void GetRandomString(wxString& strOut, int len);

	//////////////////////////////////////////////////////////////////////////

	wxColour				m_UrlColor;

	_WX_DECLARE_VECTOR(Url, VectorOfUrl, );
	_WX_DECLARE_VECTOR(ColorTextNode, VectorOfColorTextNode, );

	VectorOfUrl				m_Urls;				// 文本中包含的所有URL
	wxArrayString			m_strBreaks;		// 多行文本用以分行、分词
	VectorOfColorTextNode	m_colorTexts;		// 着色文本

	DECLARE_DYNAMIC_VOBJECT
};
