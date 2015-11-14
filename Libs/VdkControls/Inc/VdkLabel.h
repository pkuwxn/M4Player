#pragma once
#include "VdkControl.h"

/// \brief VdkLabel 的风格定义
enum VdkLabelStyle {

	VLBS_USER				= VCS_USER + 10, ///< \brief 派生类可用
	/// \brief 当作用域的宽高度小于文本实际尺寸时是否自动扩展作用域
	VLBS_AUTO_EXPAND		= 1 << ( VCS_USER + 0 ), 
};

/// \brief VdkLabel 的运行时属性集位定义
enum VdkLabelState {

	VLBST_USER				= VCST_USER + 10, ///< \brief 派生类可用
	/// \brief 当作用域的宽高度小于文本实际尺寸时裁剪绘图区域
	VLBST_CLIPPING_REGION	= 1 << ( VCST_USER + 0 ), 
};

//////////////////////////////////////////////////////////////////////////

/// \brief 为 VdkLabel 设置初始化信息
class VdkLabelInitializer : 
	public VdkCtrlInitializer< VdkLabelInitializer >
{
public:

	typedef VdkLabelInitializer Label;

	/// \brief 构造函数
	VdkLabelInitializer();

	/// \brief 标题
	Label& caption(const wxString& s) { Caption = s; return *this; }

	/// \brief 文本对齐属性
	Label& textAlign(align_type a) { TextAlign = a; return *this; }

	/// \brief 文本颜色
	Label& textColor(const wxColour& color) {
		TextColor = color; return *this;
	}

protected:

	wxString Caption; ///< 标题
	align_type TextAlign; ///< 文本对齐属性
	wxColour TextColor; ///< 文本颜色

	friend class VdkLabel;
};

//////////////////////////////////////////////////////////////////////////

class VdkLabelGhost;
/// \brief 最基本的 GUI 类——显示一小段纯文本
class VdkLabel : public VdkControl
{
public:

	/// \brief 默认构造函数
	VdkLabel();

	/// \brief 实际构建函数
	void Create(const VdkLabelInitializer& init_data);

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 析构函数
	~VdkLabel();

	/// \brief 确定 VdkLabel 的实际作用域
	///
	/// 受文本的本身性质限制，用户给出的数字很难精确。
	virtual void PrepareRect();

	//////////////////////////////////////////////////////////////////////////

	/// \brief 设置与之关联的 VdkLabelGhost
	/// \attention 不要从用户端调用。
	void SetGhost(VdkLabelGhost* ghost) { m_ghost = ghost; }

	/// \brief 获取标题文本
	wxString GetCaption() { return m_strCaption; }

	/// \brief 设置标题文本
	void SetCaption(const wxString& strCaption, wxDC* pDC);

private:

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc);

	/// \brief 接收、处理各种通知信息
	virtual void DoHandleNotify(const VdkNotify& notice);

protected:

	wxString			m_strCaption; ///< 要显示的文本

	int					m_xFix; ///< x方向的修正值，主要用于右对齐的情况
	int					m_yFix; ///< y方向的修正值，使文本垂直居中
	align_type			m_TextAlign; ///< 文本对齐属性

	wxColour			m_TextColor; ///< 文本颜色
	VdkLabelGhost*		m_ghost; ///< 延迟指定时间改变 VdkLabel 的标题

	DECLARE_DYNAMIC_VOBJECT
};

/// \brief 延迟指定时间改变 VdkLabel 的标题
///
/// 必须在堆上创建！（因为会调用delete this;删除自己）\n
/// 使用方法:
/// \code
/// wxString oldCaption(label->GetCaption());
/// label->SetCaption(newCaption);
/// new VdkLabelGhost(label, oldCaption, 1000); // 延迟1秒恢复显示
/// \endcode
/// 不要管 VdkLabelGhost 的释放
class VdkLabelGhost : public wxTimer
{
public:

	VdkLabelGhost(VdkLabel* label, const wxString& strNewText, int ms);

	/// \brief 在此恢复 VdkLabel 的标题
	virtual void Notify();

private:

	VdkLabel*			m_label;
	wxString			m_strNewText;
};
