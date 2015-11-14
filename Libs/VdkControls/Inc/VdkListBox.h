#pragma once
#include "VdkListCtrl.h"

/// \brief 为 VdkListBox 设置初始化信息
class VdkListBoxInitializer : public 
	VdkCtrlInitializer< VdkListBoxInitializer > {
public:

	typedef VdkListBoxInitializer ListBox;

	/// \brief 设置文本对齐方式
	ListBox& textAlign(align_type align) {
		TextAlign = align; return *this;
	}

private:

	align_type TextAlign;
	friend class VdkListBox;
};

//////////////////////////////////////////////////////////////////////////

/// \brief 只有一列的 VdkListCtrl
///
/// 这是一个便利类。
class VdkListBox : public VdkListCtrl
{
public:

	/// \brief 构造函数
	/// \param style 列表框的风格组合
	VdkListBox(long style = VCS_BORDER_SIMPLE | VLCS_HOVERING)
		: VdkListCtrl( style )
	{

	}

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 执行实际构建操作
	void Create(const VdkListBoxInitializer& init_data);

private:

	DECLARE_DYNAMIC_VOBJECT
};
