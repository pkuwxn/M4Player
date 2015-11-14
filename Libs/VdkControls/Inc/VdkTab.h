#pragma once
#include "VdkControl.h"

class VdkToggleButton;
class VdkPanel;

/// \brief VdkTab 风格属性
enum VdkTabStyle {

	/// \brief 派生类风格位定义的起始位序号
	VTABS_USER					= ( VCS_USER + 5 ),

	/// \brief 是否绘制选项卡边框
	VTABS_TAB_BORDER			= 1 << ( VCS_USER + 0 ),
};

/*!\brief 选项卡
 *
 * 一个时刻只允许一个页面被激活，其他处于隐藏的状态。
**/
class VdkTab : public VdkCtrlHandler, public VdkCtrlParserCallback
{
public:

	/// \brief 默认构造函数
	VdkTab();

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 析构函数
	~VdkTab();

	/// \brief 添加一个控件到第\a page 个页面
	/// 
	/// 同时会将控件添加到页面面板( \link VdkPanel \endlink )的控件列表中。
	void AddCtrlToPage(int page, VdkControl* pCtrl);

	/// \brief 添加一个新的页面
	/// 
	/// \attention 此前选项卡必须已有页面存在。
	/// \return 返回新页面的句柄。
	VdkPanel* AddPage(const wxString& caption);

	/// \brief 移除第\a id 个页面
	void RemovePage(int id, wxDC* pDC);

	/// \brief 切换到第\a id 个页面
	void Toggle(int id, wxDC* pDC);

	/// \brief 得到页面数目
	int GetPageCount() const;

	/// \brief 获取当前选中的页面
	int GetValue() const;

	/// \brief 回调通知函数
	///
	/// 来自 \link VdkCtrlParserCallback \endlink
	virtual void Notify(VdkWindow* win, 
						const wxString& clName,
						VdkControl* pCtrl);

private:

	// 由相关联的按钮得到页面 ID
	int GetPageIdFromHandle(VdkToggleButton* pCtrl) const;

	// 绘制控件
	virtual void DoDraw(wxDC& dc);

	// 处理鼠标事件
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	// 工具栏按钮的事件处理函数
	void OnToolBarEvents(VdkVObjEvent&);

	// 发送标准 wx 事件
	void SendWxEvent(int index, int lastindex) const;

	//////////////////////////////////////////////////////////////////////////

	// 选项卡信息集合
	struct PageInfo {

        VdkToggleButton* btn; // 相关联的按钮
        VdkPanel* panel; // 相关联的页面
    };

	wxVector<PageInfo *> m_pages; // 所有页面

	//////////////////////////////////////////////////////////////////////////

	int m_padding; // 按钮之间的间隙

	// 当前激活的按钮
	VdkToggleButton* m_currToggled;
	
	DECLARE_DYNAMIC_VOBJECT
};

//////////////////////////////////////////////////////////////////////////
/// \brief VdkTab 专用事件通知类
class VdkTabEvent : public VdkVObjEvent
{
public:

	/// \brief 构造函数
	VdkTabEvent(VdkTab* tab, int index, int lastindex);

private:

	VdkTab* m_tab;

	int m_index;
	int m_lastindex;
};
