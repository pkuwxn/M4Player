/***************************************************************
 * Name:      StickyWindows.h
 * Purpose:   WinAMP 风格粘附窗口实现
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-03-07
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkEvent.h"
#include <wx/vector.h>

/*! 子窗口粘附到主窗口 */
wxDECLARE_EVENT( wxEVT_STICKY_WINDOW_ATTACHED, wxCommandEvent );
/*! 子窗口从主窗口脱离 */
wxDECLARE_EVENT( wxEVT_STICKY_WINDOW_DETACHED, wxCommandEvent );

/// \brief WinAMP 风格粘附窗口实现
class StickyWindows : public VdkEventFilter
{
public:

	/// \brief 构造函数
	StickyWindows(int criticalDistance = 10);

	/// \brief 添加一个受管理的窗口
	void AddWindow(VdkWindow* win);

	/// \brief 设置主窗口
	///
	/// 是否主窗口，假如是拖动主窗口时与之粘附的子窗口会一起联动。
	void SetMainWindow(VdkWindow* win);

	/// \brief 查找子窗口是否已然黏附到主窗口
	bool IsStickyToMain(VdkWindow* chd);

	/// \brief 更新当前与主窗口相粘附的子窗口
	/// \param rcMain 主窗口的屏幕作用域
	void UpdateStickyChildren(const wxRect& rcMain);

	/// \brief 移除一个受管理的窗口
	void RemoveWindow(VdkWindow* win);

private:

	// 拦截发送给 VdkWindow 的事件
	virtual bool FilterEvent(const EventForFiltering& e);

	// 处理移动子窗口事件（主窗口移动事件处理器为 LinklyMove() ）
	bool HandleChildMove(VdkWindow* win, const wxPoint& mousePosClient);

	// 处理改变窗口大小
	bool HandleResize(VdkWindow* win, const wxPoint& mousePosClient);

	// 在真正移动之前更新粘附子窗口
	void OnMainLeftDown(wxMouseEvent&);

	// 处理父子窗口联动
	void LinklyMove(wxMoveEvent& e);

private:

	// 计算子窗口是否直接或者间接地黏附到主窗口
	//
	// 所谓间接，即通过一个已粘附到主窗口的代理子窗口（兄弟子窗口）\n
	// 与主窗口粘附。
	// @param rcMain 主窗口在屏幕上的作用域。
	bool RecalcLinkageToMain(const wxRect& rcMain, VdkWindow* chd);

	// 检测\a moving 是否确实粘附到\a still 的某个方向上
	bool ReallySticky(const wxRect& moving, const wxRect& still);

	// 重新计算与主窗口的粘附属性
	// 
	// 同时更新内部状态信息。
	void UpdateStickyToMainState(const wxRect& rcMain, VdkWindow* win);

	// 根据\a rcThis 和\a rcAgence 的可粘附状态更新\a rcThis
	bool TryStickToAgent(wxRect& rcThis, const wxRect& rcAgent);

private:

	// 窗口粘附的临界值
	// 这里不能用 unsigned 类型，因为 std::abs() 返回 int ，用了
	// 会导致大量警告
	int m_criticalDistance;

	VdkWindowList m_windows;
	// 主窗口迭代器
	VdkWindowIter m_main;

	// 粘附的子窗口的相关信息
	class StickyInfo {
	public:

		VdkWindow* win;

		wxCoord offsetX; // 与主窗口在 X 轴上的距离
		wxCoord offsetY; // 与主窗口在 Y 轴上的距离

		bool operator == (const StickyInfo& rhs) {
			return win == rhs.win;
		}
	};

	typedef wxVector< StickyInfo >::iterator InfoIter;
	typedef wxVector< StickyInfo >::const_iterator InfoIter_Const;
	wxVector< StickyInfo > m_stickyToMain; // 四个方向上粘附到主窗口的子窗口

	// 查找子窗口是否已然黏附
	InfoIter FindStickyChild(VdkWindow* chd);

	// 移除黏附的子窗口
	void RemoveChild(VdkWindow* chd);
};
