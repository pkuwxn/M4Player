#pragma once
#include "VdkControl.h"

class VdkEdit;

/// \brief 类似 HTML 中的“<file>”标签
class VdkFileSelector : public VdkCtrlHandler
{
public:

	/// \brief 打开的类型
	enum SEL_TYPE {

		ST_DIR, ///< “文件”
		ST_FILE ///< “文件夹”
	};

	/// \brief 默认构造函数
	VdkFileSelector();

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 执行实际构建操作
	void Create(VdkWindow* Window, const wxString& strName, const wxRect& rc);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 获取得到的文件/文件夹路径
	wxString GetPath() const;

	/// \brief 设置当前显示路径
	void SetPath(const wxString& strPath);

	/// \brief 设置“打开”通用对话框显示的文件类型
	void SetExt(const wxString& strDefaultFileName, const wxString& strExtPrompt);

	/// \brief 当前打开的类型是“文件”还是“文件夹”
	SEL_TYPE GetType() const { return m_type; }

	/// \brief 设置当前打开的类型
	void SetType(SEL_TYPE type) { m_type = type; }

	/// \brief 设置“打开”通用对话框的标题
	void SetAlertMessage(const wxString& alertMsg) { m_alertMsg = alertMsg; }

	/// \brief 获取“打开”通用对话框的标题
	wxString GetAlertMessage() const { return m_alertMsg; }

private:

	// 显示“打开”通用对话框(消息转发器)
	void OnSelectorRed(VdkVObjEvent&);

	// 显示“打开”通用对话框
	void OnSelector(wxCommandEvent&);

	//////////////////////////////////////////////////////////////////////////

	VdkEdit*		m_edit;

	SEL_TYPE		m_type;
	wxString		m_alertMsg;

	wxString		m_extPrompt;
	wxString		m_defaultFileName;

	DECLARE_DYNAMIC_VOBJECT
};
