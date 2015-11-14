/***************************************************************
 * Name:      VdkToolBar.h
 * Purpose:   Code for VdkToolBar declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-04-07
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkControl.h"
#include "VdkBitmapArray.h"

/// \brief VdkToolBar 的风格属性集
enum VdkToolBarStyle {

};

/// \brief VdkToolBar 的运行时状态属性集
enum VdkToolBarState {

};

class VdkToolBarEntry;

//////////////////////////////////////////////////////////////////////////

/// \brief 便利工具栏项目添加器
class VdkToolBarEntryAdder
{
public:

	/// \brief 构造函数
	VdkToolBarEntryAdder()
		: Menu( NULL )
	{

	}

	/// \brief 设置标题文本
	VdkToolBarEntryAdder& caption(const wxString& c) {
		Caption = c;
		return *this;
	}

	/// \brief 设置下拉菜单
	VdkToolBarEntryAdder& menu(VdkMenu* m) {
		Menu = m;
		return *this;
	}

	/// \brief 设置相关联的小位图
	VdkToolBarEntryAdder& bid(const VdkBitmapArrayId& i) {
		BId = i;
		return *this;
	}

private:

	wxString Caption;
	VdkMenu* Menu;
	VdkBitmapArrayId BId;

	friend class VdkToolBar;
};

/// \brief 工具栏的静态风格
class VdkToolBarStaticStyle
{
public:

	/// \brief 构造函数
	VdkToolBarStaticStyle();

	/// \brief 设置边框颜色
	VdkToolBarStaticStyle& borderColor(const wxColour& color) {
		m_border.SetColour( color );
		return *this;
	}

	/// \brief 获取边框画笔
	wxPen& borderPen() { return m_border; }

	/// \brief 设置鼠标指针置于其上时按钮的高亮颜色
	VdkToolBarStaticStyle& hilightColor(const wxColour& color) {
		m_hilight.SetColour( color );
		return *this;
	}

	/// \brief 获取高亮画刷
	wxBrush& hilightBrush() { return m_hilight; }

	/// \brief 设置鼠标指针按下时按钮的高亮颜色
	VdkToolBarStaticStyle& pushedColor(const wxColour& color) {
		m_pushed.SetColour( color );
		return *this;
	}

	/// \brief 获取按下时的画刷
	wxBrush& pushedBrush() { return m_pushed; }

	/// \brief 设置工具栏背景画刷
	VdkToolBarStaticStyle& bgBrush(wxBrush brush) {
		m_bg = brush;
		return *this;
	}

	/// \brief 获取工具栏背景画刷
	wxBrush& bgBrush() { return m_bg; }

	/// \brief 设置工具栏按钮的各向边距
	VdkToolBarStaticStyle& bitmapAddin(int addin) {
		m_bitmapAddin = addin;
		return *this;
	}

	/// \brief 获取工具栏按钮的各向边距
	int bitmapAddin() const { return m_bitmapAddin; }

private:

	wxPen m_border;
	wxBrush m_hilight;
	wxBrush m_pushed;
	wxBrush m_bg; // 背景画刷

	int m_bitmapAddin;
};

class VdkToolBarLeftSideHandle;
class VdkToolBarRightSideHandle;

/// \brief 类似 Office 2003 风格的工具栏
class VdkToolBar : public VdkCtrlHandler
{
public:

	/// \brief 默认构造函数
	VdkToolBar();

	/// \brief 析构函数
	~VdkToolBar();

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 往工具栏末尾添加一项
	VdkToolBar& Append(const VdkToolBarEntryAdder& adder);

	/// \brief 往工具栏末尾添加一个分隔符
	VdkToolBar& AppendSeperator();

	/// \brief 获取静态风格
	VdkToolBarStaticStyle* GetStaticStyle() const {
		return m_sstyle;
	}

private:

	// 擦除背景
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	//////////////////////////////////////////////////////////////////////////
	// 控件状态定义

	class VdkToolBarLeftSideHandle* m_leftHandle;
	class VdkToolBarRightSideHandle* m_rightHandle;

	//////////////////////////////////////////////////////////////////////////
	// 风格定义
	VdkToolBarStaticStyle* m_sstyle;

	DECLARE_CLONEABLE_VOBJECT( VdkToolBar )
};
