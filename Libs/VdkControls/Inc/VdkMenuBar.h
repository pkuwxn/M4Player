/***************************************************************
 * Name:      VdkMenuBar.h
 * Purpose:   Code for VdkMenuBar declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2010-12-02
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkControl.h"
#include "VdkButton.h"
#include "VdkMenuPopper.h"
#include "VdkEvent.h"

class VdkMenuBarEntryStyle;

/// \brief VdkMenuBarEntry 所需的初始化信息
class VdkMenuBarEntryInitializer : public VdkButtonInitializer
{
public:

	typedef VdkMenuBarEntryInitializer MenuBarEntry;

	/// \brief 构造函数
	VdkMenuBarEntryInitializer();

	/// \brief 设置所属的菜单
	MenuBarEntry& menu(VdkMenu* m) { Menu = m; return *this; }

	/// \brief 从\a s 中动态生成新的菜单
	MenuBarEntry& menuTree(const wxString& s) { MenuTree = s; return *this; }

	/// \brief 设置绘制时所用的风格
	MenuBarEntry& drawStyle(VdkMenuBarEntryStyle* s) { DrawStyle = s; return *this; }

private:

	VdkMenu* Menu;
	wxString MenuTree;
	VdkMenuBarEntryStyle* DrawStyle;

	friend class VdkMenuBarEntry;
};

/// \brief 为 VdkMenuBar 设置皮肤
class VdkMenuBarInitializer : 
	public VdkCtrlInitializer< VdkMenuBarInitializer >
{
public:

	typedef VdkMenuBarInitializer MenuBar;

	/// \brief 构造函数
	VdkMenuBarInitializer();

	/// \brief 设置所有的菜单项
	MenuBar& menus(VdkMenuBarEntryInitializer menus[]) {
		menuArray = menus; return *this;
	}

private:

	VdkMenuBarEntryInitializer* menuArray;
	friend class VdkMenuBar;
};

//////////////////////////////////////////////////////////////////////////

class VdkMenuBarEntry;
WX_DECLARE_LIST_PTR( VdkMenuBarEntry, ListOfEntry );

/// \brief 菜单栏
class VdkMenuBar : public VdkCtrlHandler, public VdkEventFilter
{
public:

	/// \brief 构造函数
	VdkMenuBar();

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 构建控件
	void Create(const VdkMenuBarInitializer& init_data);

	/// \brief 析构函数
	~VdkMenuBar();
	
	/// \brief 往菜单栏中添加一项
	/// \param strCaption 标题文本
	/// \param menu 与之相关联的菜单句柄
	/// \param strMenuTree 若\a menu 为空，则可指定本参数进行动态创建
	VdkMenuBar& Append(wxString strCaption, 
					   VdkMenu* menu, 
					   const wxString& strMenuTree = wxEmptyString);

public:

	/// \brief 过滤菜单弹出后鼠标移动事件
	virtual bool FilterEvent(const EventForFiltering& e);

private:

	// 从堆上创建一个新项的 C++ 对象
	virtual VdkMenuBarEntry* CreateNewEntry(int& w, int& h);

	// 擦除背景
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	// 处理通知信息
	virtual void DoHandleNotify(const VdkNotify& notice);

	unsigned				m_total;
	ListOfEntry				m_entries;
	VdkMenuBarEntryStyle*	m_drawStyle;

	DECLARE_DYNAMIC_VOBJECT
};

//////////////////////////////////////////////////////////////////////////

enum VdkMenuBarEntryStyleEnum {

	VMBES_OFFICE2003,
};

/// \brief 菜单栏项目的共用风格对象
class VdkMenuBarEntryStyle
{
public:

	/// \brief 设置一项被高亮时边框表现所用的颜色
	VdkMenuBarEntryStyle& hilightBorderColour(const wxColour& color) {
		m_hilightBorderPen.SetColour(color);
		return *this;
	}

	/// \brief 设置一项被选中时边框表现所用的颜色
	VdkMenuBarEntryStyle& selectedBorderColour(const wxColour& color) {
		m_selectedBorderPen.SetColour(color);
		return *this;
	}

	/// \brief 设置一项正常状态下的背景颜色
	VdkMenuBarEntryStyle& bgBrush(const wxBrush& brush) {
		m_background = brush;
		return *this;
	}

	/// \brief 设置一项被高亮时表现所用的颜色
	VdkMenuBarEntryStyle& hilightColour(const wxColour& color) {
		m_hilight.SetColour( color );
		return *this;
	}

	/// \brief 设置一项被选中时表现所用的颜色
	VdkMenuBarEntryStyle& selectedColour(const wxColour& color) {
		m_selected.SetColour(color);
		return *this;
	}

private:

	wxPen m_hilightBorderPen;
	wxPen m_selectedBorderPen;
	wxBrush m_background;
	wxBrush m_hilight;
	wxBrush m_selected;

	friend class VdkMenuBarEntry;
};

/// \brief 菜单栏中的一个下拉菜单按钮
class VdkMenuBarEntry : public VdkMenuPopper
{
public:

	/// \brief 默认构造函数
	VdkMenuBarEntry();

	/// \brief 承担实际构建工作
	void Create(const VdkMenuBarEntryInitializer& init_data);

	/// \brief 析构函数
	~VdkMenuBarEntry();

private:

	/// \brief 构建控件
	void Init();

	/// \brief 擦除\a rc 指定的矩形
	/// \param rc 坐标起点为父控件的左上角
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	//////////////////////////////////////////////////////////////////////////

	wxString m_strCaption;  // 菜单项标题文本
	VdkMenuBarEntryStyle* m_drawStyle;
};

/// \brief 获取指定菜单风格的定义
VdkMenuBarEntryStyle* GetMenuBarEntryStyle(VdkMenuBarEntryStyleEnum sid);
