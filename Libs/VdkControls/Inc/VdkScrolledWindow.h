#pragma once

class VdkWindow;
class VdkScrollBar;
#include "VdkControl.h"

/*!\brief 滚动窗口的实现基类
 * 
 * 次级派生类必须提供如下几个通知消息和鼠标事件：
 * 
 * 通知信息：
 * VCN_CREATE
 * VCN_SIZE_CHANGED
 * VCN_WINDOW_RESIZED
 * 
 * 鼠标事件：
 * SCROLLING_UP
 * SCROLLING_DOWN
 * WHEEL_UP
 * WHEEL_DOWN
**/
class VdkScrolled
{
public:

	/// \brief 默认构造函数
	VdkScrolled();

	/// \brief 从\a o 克隆
	void Clone(VdkScrolled* o, VdkScrollBar* scrollbar);

	/// \brief 刷新滚动窗口状态
	///
	/// 主要是滚动窗口时，需要更新滚动条位置。
	void RefreshState(wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 设置虚拟画布的大小
	void SetVirtualSize(int x, int y, wxDC* pDC = NULL);

	/// \brief 一个梯度的大小
	///
	/// 例如滚动一次滚轮虚拟画布滚动三个梯度，那么虚拟画布下移的像素数:
	/// 3 * 相应坐标轴的梯度
	void SetScrollRate(int xstep, int ystep);

	/// \brief 获取两个方向上鼠标滚轮梯度的大小
	/// \see SetScrollRate
	void GetScrollRate(int* xstep, int* ystep) const;

	/// \brief 每次鼠标滚轮滑动时翻过的行数
	void SetLinesUpDown(int lines) { m_linesUpDown = lines; }

	/// \brief 每次鼠标滚轮滑动时翻过的行数
	int GetLinesUpDown() const { return m_linesUpDown; }

	/// \brief 设置起始行号
	void SetViewStart(int x, int y, wxDC* pDC);

	/// \brief 获取起始行号
	///
	/// 行号 * yStep = 可视起始坐标。
	/// \see GetViewStartCoord
	virtual void GetViewStart(int* x, int* y) const;

	/// \brief 获取最大可能的起始序号
	virtual void GetMaxViewStart(int* x, int* y) const;

	/// \brief 获取虚拟画布的大小
	void GetVirtualSize(int* w, int* h) const;

	/// \brief 获取起始坐标
	/// \see GetViewStart
	void GetViewStartCoord(int* x, int* y) const;

	/// \brief 获取最大起始坐标
	/// \see GetMaxViewStart
	void GetMaxViewStartCoord(int* x, int* y) const;

	/// \brief 获取滚动条的只读句柄
	VdkScrollBar* GetScrollBar() const { return m_pScrollBar; }

	/// \brief 滚动到指定百分比
	/// \param progress 0 <= \a progress <= 1
	void GoTo(double progress, wxDC* pDC);

	/// \brief 获取当前可视位置的百分比
	virtual double GetProgress() const;

public:

	//////////////////////////////////////////////////////////////////////////
	// 这两个函数仅供内部调用，假如你不知道自己在干什么，不要调用它们！

	/// \brief 绘图前的准备工作
	///
	/// 例如正确设置 DC 的起始点。
	void PrepareDC(wxDC& dc);

	/// \brief 绘图后的清理、还原工作
	void RestoreDC(wxDC& dc);

protected:

	//////////////////////////////////////////////////////////////////////////
	// 滚动条相关

	/// \brief 更新滚动条状态
	void CalcScrollBarSize(wxDC* pDC);

	/// \brief 滚动条的风格属性对象定义
	struct ScrollBarStyle {

		wxString* strFileNames; ///< 滚动条所用的位图文件名数组
		int nExpl; ///< 滚动条手柄所用位图的分割份数
		VdkResizeableBitmapType nThumbTileType; ///< 设置滚动条手柄所用位图的拉伸方式
		int nThumbTile; ///< 滚动条手柄所用位图的拉伸叠放长度
	};

	/// \brief 更改滚动条皮肤
	virtual void SetScrollBarStyle(ScrollBarStyle& style);

private:

	/// \brief 计算最大允许起始坐标
	void CalcMaxViewStart();

protected:

	//////////////////////////////////////////////////////////////////////////
	// 由次级派生类分发的事件

	/// \brief 在此初始化 DC ，准备绘制
	///
	/// 派生类实现\link OnDraw \endlink 即可。
	void Paint(wxDC& dc);

	/// \brief 处理鼠标事件
	/// \warning VdkMouseEvent 中包含的鼠标位置应该以滚动窗口左上角为起始点。
	void MouseEvent(VdkMouseEvent& e);

	/// \brief 处理键盘按键事件
	void KeyEvent(VdkKeyEvent& e);

	/// \brief 接收、处理通知信息
	void Notify(const VdkNotify& notice);

private:

	//////////////////////////////////////////////////////////////////////////
	// 次级派生类必须覆写

	/// \brief 获取滚动条的父窗口
	virtual VdkWindow* ParentWindow() const = 0;

	/// \brief 滚动条的父控件(假如有)
	virtual VdkControl* ScrollBarParent() const = 0;

	/// \brief 滚动条的相对位置
	virtual wxRect ScrollBarRect() const = 0;

	/// \brief 获取滚动条对应的 DC
	virtual wxDC* GetScrollBarDC(wxDC* dc) const = 0;

	/// \brief 销毁从\link GetScrollBarDC \endlink 获得的 DC
	/// \see GetScrollBarDC
	virtual void DestroyScrollBarDC(wxDC* dc) const = 0;

	/// \brief 获取滚动窗口的名字
	virtual const wxString& Name() const = 0;

	/// \brief 获取/设置滚动窗口的风格属性集
	virtual long Style() const = 0;

	/// \brief 获取/设置滚动窗口相对于父控件的相对位置
	/// 
	/// 假如没有父控件，则返回值与\link AbsoluteRect() \endlink 相同。
	/// \see AbsoluteRect
	virtual wxRect& RelativeRect() = 0;

	/// \brief 获取/设置滚动窗口相对于顶层 VdkWindow 的绝对位置
	virtual wxRect AbsoluteRect() const = 0;

	/// \brief 在此更新/绘制滚动窗口
	virtual void UpdateUI(wxDC& dc) = 0;

	/// \brief 检测是否已在 XRC 文件中定义并创建了需要的滚动条
	///
	/// 用于滚动窗口创建缺省滚动条前。
	virtual VdkScrollBar* ScrollBarExists() = 0;

	/// \brief 响应滚动条显示/隐藏事件通知
	virtual void OnScrollBarShowHide(const VdkScrollBar* sb) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 次级派生类可以覆写

	/// \brief 绘图前的准备工作
	///
	/// 例如正确设置 DC 的起始点。
	/// \warning 此时 DC 的起始点为 (0, 0) 。
	virtual void DoPrepareDC(wxDC& dc) {}

	/// \brief 绘图后的清理、还原工作
	virtual void DoRestoreDC(wxDC& dc) {}

	//////////////////////////////////////////////////////////////////////////
	// 次次级派生类可以覆写

	/// \brief 刷新滚动窗口
	virtual void DoRefreshState(wxDC& dc) {}

protected: // 以下这些事件处理函数都需要实体派生类向基类分派

	/// \brief 在此绘图
	virtual void OnDraw(wxDC& dc) {}

	/// \brief 响应鼠标事件
	virtual void OnMouseEvent(VdkMouseEvent& e) {}

	/// \brief 响应键盘事件
	virtual void OnKeyEvent(VdkKeyEvent& e) {}

	/// \brief 接收、处理通知信息
	virtual void OnNotify(const VdkNotify& notice) {}

private:

	int					m_xStep;
	int					m_yStep;

	int					m_xVirtualSize;
	int					m_yVirtualSize;

	int					m_xViewStart;
	int					m_yViewStart;
	int					m_yViewStartMax;

	int					m_linesUpDown;		///< 每次鼠标滚轮滑动时翻过的行数

	//////////////////////////////////////////////////////////////////////////

	VdkScrollBar*		m_pScrollBar;
};

//////////////////////////////////////////////////////////////////////////

enum VdkScrolledWindowStyle {

	/// \brief 派生类风格属性的起始 ID
	VSWS_USER					= ( VCS_USER + 10 ),
	/// \brief 不创建滚动条
	VSWS_NO_SCROLLBAR			= ( 1 << ( VCS_USER + 0 ) ),
	/// \brief 始终显示滚动条（不满一屏时禁用滚动条而不是隐藏）
	VSWS_ALWAYS_SHOW_SCROLLBAR	= ( 1 << ( VCS_USER + 1 ) ),
};

/// \brief VdkScrolledWindow 的运行时属性集位定义
enum VdkScrolledWindowState {

	/// \brief 派生类风格属性的起始 ID
	VSWST_USER				= ( VCST_USER + 10 ),
};

/*!\brief 滚动窗口的 VDK 实现
 *
 * 以前不懂滚动窗口的巨大意义，而傻乎乎地使用非原生滚动窗口来模拟这种效果，
 * 事实证明，这种努力太没意义！平白无故地极大地增加了基于滚动窗口的控件的实现复杂性。\n
 * 血泪的教训。
 *
 * \attention 必须设置 m_physicRect，尤其是修改了 m_Rect 的情况下！
**/
class VdkScrolledWindow : public VdkControl, public VdkScrolled
{
public:

	/// \brief 默认构造函数
	VdkScrolledWindow(long style = 0);

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 从\a o 克隆
	void Clone(VdkScrolledWindow* o);

	/// \brief 获取包含滚动条的作用域
	///
	/// m_Rect 默认不包含滚动条的作用域。
	wxRect GetLogicalRect() const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief 设置边界颜色
	void SetBorderColour(const wxColour& color){ m_borderColor = color; }

	/// \brief 获取边界颜色
	wxColour GetBorderColour() const { return m_borderColor; }

private:

	//////////////////////////////////////////////////////////////////////////
	// 次级派生类必须覆写

	/// \brief 获取滚动条的父窗口
	virtual VdkWindow* ParentWindow() const { return m_Window; }

	/// \brief 滚动条的父控件(假如有)
	virtual VdkControl* ScrollBarParent() const {
		return (VdkControl *)( this );
	}

	/// \brief 滚动条的相对位置
	virtual wxRect ScrollBarRect() const {
		return wxRect( 0, 0, m_Rect.width, m_Rect.height );
	}

	/// \brief 获取滚动条对应的 DC
	virtual wxDC* GetScrollBarDC(wxDC* dc) const { return dc; }

	/// \brief 销毁从\link GetScrollBarDC \endlink 获得的 DC
	virtual void DestroyScrollBarDC(wxDC* dc) const {}

	/// \brief 获取滚动窗口的名字
	virtual const wxString& Name() const { return m_strName; }

	/// \brief 获取/设置滚动窗口的风格属性集
	virtual long Style() const { return GetStyle(); }

protected:

	/// \brief 获取/设置滚动窗口相对于父控件的相对位置
	/// 
	/// 假如没有父控件，则返回值与\link AbsoluteRect() \endlink 相同。
	/// \see AbsoluteRect
	virtual wxRect& RelativeRect() { return m_Rect; }

	/// \brief 获取/设置滚动窗口相对于顶层 VdkWindow 的绝对位置
	virtual wxRect AbsoluteRect() const { return GetAbsoluteRect(); }

private:

	/// \brief XRC 动态创建
	virtual void OnXrcCreate(wxXmlNode* node) {}

	/// \brief 在此更新/绘制滚动窗口
	virtual void UpdateUI(wxDC& dc) { Draw( dc ); }

	/// \brief 检测是否已在 XRC 文件中定义并创建了需要的滚动条
	///
	/// 用于滚动窗口创建缺省滚动条前。
	virtual VdkScrollBar* ScrollBarExists();

	/// \brief 响应滚动条显示/隐藏事件通知
	virtual void OnScrollBarShowHide(const VdkScrollBar* sb);

	//////////////////////////////////////////////////////////////////////////
	// 次级派生类可以覆写

	/// \brief 绘图前的准备工作
	///
	/// 例如正确设置 DC 的起始点。
	/// \warning 此时 DC 的起始点为 (0, 0) 。
	virtual void DoPrepareDC(wxDC& dc);

	/// \brief 绘图后的清理、还原工作
	virtual void DoRestoreDC(wxDC& dc);

	/// \brief 刷新滚动窗口
	virtual void DoRefreshState(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////
	// 继承自 VdkControl

	/// \brief 在此绘制全部内容
	virtual void DoDraw(wxDC& dc);

	/// \brief 处理鼠标事件
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief 处理键盘按键事件
	virtual void DoHandleKeyEvent(VdkKeyEvent& e) { KeyEvent( e ); }

	/// \brief 接收、处理通知信息
	virtual void DoHandleNotify(const VdkNotify& notice) {
		Notify( notice );
	}

private:

	wxColour m_borderColor;
};
