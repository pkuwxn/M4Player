#pragma once

class VdkWindow;
#include "VdkControl.h"

/*!\brief 一个静态图像类
 * 
 * 可以实现类似可换肤软件的“底纹”功能。\n
 * 为实现底纹功能，建议将 XRC 文件中本类定义的位置放到文件最开头，\n
 * 这样便不会覆盖后续子控件的绘制和事件处理。\n
 * 图像文件可以是 .BMP、.PNG、.JPG/.JPEG、.GIF 四种格式之一。
**/
class VdkStaticImage : public VdkControl
{
public:

	/// \brief 默认构造函数
	VdkStaticImage() {}

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 构建控件
	/// \param Window 父窗口
	/// \param strName 控件标识符
	/// \param rc 控件作用域
	/// \param bm 要显示的位图
	/// \param rescaleBitmap 是否缩放位图以适应控件原定作用域
	void Create(VdkWindow* Window, 
				const wxString& strName, 
				const wxRect& rc, 
				const wxBitmap& bm,
				bool rescaleBitmap);

protected:

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc);

	/// \brief 获取要显示的图像
	const wxBitmap& GetImage() const { return m_image; }

	/// \brief 设置要显示的图像
	void SetImage(const wxBitmap& image) { m_image = image; }

private:

    // 要显示的图像
	wxBitmap		m_image;

	DECLARE_DYNAMIC_VOBJECT
};
