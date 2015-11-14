#pragma once
#include "VdkButton.h"

/// \brief 复选框
class VdkCheckBox : public VdkButton
{
public:

	/// \brief 构造函数
	VdkCheckBox();

	/// \brief 执行实际构建操作
	void Create(VdkWindow* Window, const wxString& strName, wxRect rc);

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 切换选中与否
	void Toggle(bool bToggled, wxDC* pDC);

	/// \brief 是否被选中
	bool IsChecked() const { return m_bToggled; }

	/// \brief 使能/禁用关联控件
	void EnableRelatedCtrls(bool bEnabled, wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 添加关联 VdkControl 到 CheckBox 中
	///
	/// 例如 VdkCheckBox 使控制一个文本框，则 VdkCheckBox 选中时文本框可用，
	/// 不选中时失活。
	void AddRelatedCtrl(VdkControl* relatedCtrl);

private:

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc);

	/// \brief 处理鼠标事件
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief 接收、处理通知信息
	virtual void DoHandleNotify(const VdkNotify& notice);
	
	/// \brief 复选框可以处于的状态
	enum State {

		NORMAL,
		HOVERING,
		PUSHED,
		CHECKED,
		CHECKED_HOVERING,
		NORMAL_DISABLED,
		THREE_STATE_DISABLED,
		CHECKED_DISABLED
	};

	/// \brief 将 VdkCheckBox::State 强制转型为 VdkButton::State
	void SetButtonState(State state) {
		Update( (VdkButton::State )state, NULL );
	}

	//////////////////////////////////////////////////////////////////////////

	bool					m_bToggled;
	VdkCtrlList				m_relatedCtrls;

	DECLARE_DYNAMIC_VOBJECT
};
