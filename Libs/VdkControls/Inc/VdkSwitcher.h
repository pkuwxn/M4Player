#pragma once
#include "VdkControl.h"
#include <wx/hashmap.h>

class VdkVObjEvent;
class VdkToggleButton;
/// \brief 类似老版 QQ 的联系人分组切换列表
class VdkSwitcher : public VdkCtrlHandler
{
public:

	/// \brief 默认构造函数
	VdkSwitcher(){ Init(); }

	/// \brief 构造函数
	VdkSwitcher( VdkWindow* parent,
			     const wxString& strName,
			     const wxString& strFileName,
			     const wxRect& rt,
				 int nExpl = 3 );

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 实际构建函数
	void Create( VdkWindow* win,
				 const wxString& strName,
				 const wxRect& rc,
				 const wxBitmap& bmp,
				 int nExpl );

	//////////////////////////////////////////////////////////////////////////

	/// \brief 禁用 VdkSwitcher
	void Disable(bool bDisabled, wxDC* pDC);

	/// \brief 切换到指定项
	void Switch(int id, wxDC* pDC);

	/// \brief 切换到指定项
	void Switch(VdkToggleButton* btn, wxDC* pDC);

	/// \brief 获取当前活动的选项页
	int GetCurrentId() const { return m_curr; }

	/// \brief 添加选项页
	void AddTab(const wxString& strCaption, VdkControl* win);

	/// \brief 获取切换条的大小
	int GetTabHeight() const { return m_h; };

private:

	/// \brief 初始化控件
	void Init();

	/// \brief 接收、处理通知信息
	virtual void DoHandleNotify(const VdkNotify& notice);

	/// \brief 切换标签页时用
	void OnSwitchTabs(VdkVObjEvent&);

	void WalkCoords(int id);
	void ResetCoords(int id);

	//////////////////////////////////////////////////////////////////////////

    struct TabInfo {

        int							id;
        wxString					strCaption;
        VdkToggleButton*			button;
        VdkControl*					win;
        wxRect						rc;
    };

	typedef VdkToggleButton* BtnPtr;
	WX_DECLARE_VOIDPTR_HASH_MAP( TabInfo, tab_map );

	wxBitmap						m_bmp;

	int								m_h;
	int								m_nExpl;

	tab_map							m_tabs;
	BtnPtr							m_last;

	int								m_curr;

	DECLARE_DYNAMIC_VOBJECT
};
