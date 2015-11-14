/***************************************************************
 * Name:      VdkScalableBitmap.h
 * Purpose:   VdkScalableBitmap 的类声明
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-03-11
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkDefs.h" // for VdkResizeableBitmapType

/// \brief 可缩放位图的模拟实现
class VdkScalableBitmap
{
public:

	/// \brief 默认构造函数
	VdkScalableBitmap();

	/// \brief 构造函数
	VdkScalableBitmap(const wxBitmap& bm, 
			  	      VdkResizeableBitmapType resizeType, 
				      const wxRect& rcTile);

	/// \brief 设置图像属性
	void Set(const wxBitmap& bm, 
			 VdkResizeableBitmapType resizeType, 
			 const wxRect& rcTile);

	/// \brief 删除位图
	void Reset();

	/// \brief 检测拉伸区域是否能被原始位图包含，即可以用作改变位图大小
	bool CanResize() const;

	/// \brief 位图是否有效
	bool IsOk() const { return m_bitmap.IsOk(); }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 将位图缩放至指定的大小
	bool Rescale(const wxSize& newsize);

	/// \brief 获取位图缩放后的宽度
	int GetWidth() const { return m_size.x; }

	/// \brief 获取位图缩放后的高度
	int GetHeight() const { return m_size.y; }

	/// \brief 获取原始位图的大小
	wxSize GetMinSize() const {
		return wxSize( m_bitmap.GetWidth(), m_bitmap.GetHeight() );
	}

	/// \brief 获取位图缩放后的大小
	const wxSize& GetSize() const { return m_size; }

	/// \brief 获取位图拉伸区域
	const wxRect& GetTileArea() const { return m_rcTile; }

	/// \brief 将位图复制到指定 DC 的(\a x, \a y)处
	void Blit(wxDC& dc, wxCoord x, wxCoord y);

	/// \brief 将位图指定区域复制到指定 DC 的(\a rc.x, \a rc.y)处
	void BlitRect(wxDC& dc, const wxRect& rc);

	//! 创建透明窗体的方式
	enum WindowShapeMode {
		WSM_LAYERED, ///< 使用层次窗口创建透明，仅在 Windows 下可用
		WSM_REGION, ///< 使用 wxRegion 创建透明窗体
	};

	/// \brief 使用当前位图、当前大小设置指定非贵族窗口的作用域
	void WindowSetShape(wxTopLevelWindow* win, 
						const wxColour& maskColor,
						WindowShapeMode mode) const;

private:

	// 单独设置位图
	void SetBitmap(const wxBitmap& bm);

private:

	wxBitmap m_bitmap; // 原始位图
	wxBitmap m_rescaled; // 当前位图

	VdkResizeableBitmapType m_resizeType; // 位图缩放方式
	wxRect m_rcTile; // 拉伸区域
	wxRect m_rcTile_o; // 平铺拉伸时用以优化的可伸缩区域

	wxSize m_size; // 当前位图大小
};
