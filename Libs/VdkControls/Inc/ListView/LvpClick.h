/***************************************************************
 * Name:      LvpClick.h
 * Purpose:   实现 VdkListView 单选、多选
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#pragma once
#include "IListViewPlaugin.h"

/// 实现 VdkListView 单选、多选
class LvpClick : public IListViewPlaugin
{
public:

	//! 单选/多选
	enum SelectMode {
		SM_SINGLE, ///< 单选
		SM_MULTI, ///< 多选
	};

	/// 构造函数
	LvpClick(ListView* list, SelectMode sm, const wxBrush& selected);

	/// 选中第@a index 项
	///
	/// 当前所有选择会被清空。
	void Select(int index, wxDC* pDC);

	/// 选中[@a index, end) 区间中的所有项目
	///
	/// 当前所有选择会被清空，且仅在多选模式中有效。
	/// @attention 区间为右开区间。
	void Select(int beg, int end, wxDC* pDC);

	/// 选中指定数组中的所有项目
	///
	/// 当前所有选择会被清空。
	void Select(const wxArrayInt& items, wxDC* pDC);

	/// 清除已选择的列表项
	///
	/// 不会重画控件，必须显式调用 VdkListView::RefreshState() 。
	void SelectNone();

	/// 获取选择模式
	SelectMode GetSelectMode() const {
		return m_multiSel ? SM_MULTI : SM_SINGLE;
	}

	/// 设置选择模式
	///
	/// 这个函数一般从外部直接调用，内部实现会临时重设 pDC 的绘图起始点，
	/// 并在返回时恢复。
	/// 副作用：会清空当前所有选择。
	void SetSelectMode(SelectMode sm, wxDC* pDC);

	/// 存取选中项的背景画刷
	wxBrush GetSelectedBrush() const { return m_selected; }
	void SetSelectedBrush(const wxBrush& brush) { m_selected = brush; }

public:

	/// Interface definition for a callback to be invoked when an item in this 
	/// ListView has been clicked. 
	class OnItemClickListener
	{
	public:

		/// Callback method to be invoked when an item in this ListView 
		/// has been clicked. 
		/// @param parent The ListView where the click happened.
		/// @param index The row id of the item that was clicked.
		/// @param dc The device context of this ListView.
		virtual void OnItemClick(LvpClick* parent, int index, wxDC& dc) = 0;
	};

	/// 设置项目点击监听器
	void SetOnItemClickListener(OnItemClickListener* listener) {
		m_clickListener = listener;
	}

public:

	WX_DEFINE_SORTED_ARRAY_INT( int, ArrayOfSortedInts );
	/// 获取被选中的行
	const ArrayOfSortedInts& GetSelItems() const { return m_selItems; }
    
protected:

    virtual void OnEraseRow(int row, wxDC& dc);
    virtual void OnDragg(int rowAtPointer, VdkMouseEvent& e) {}
    virtual void OnDClick(int rowAtPointer, VdkMouseEvent& e);
    virtual void OnClickDown(int rowAtPointer, VdkMouseEvent& e);
    virtual void OnClickUp(int rowAtPointer, VdkMouseEvent& e);
    virtual void OnRightUp(int rowAtPointer, VdkMouseEvent& e) {}

	virtual void OnItemAdd(size_t row);
	virtual void OnItemRemove(size_t row);
	virtual void OnClear();

private:

    virtual void OnKey(VdkKeyEvent& e);
	void OnPageDown(VdkKeyEvent& e);
	void OnPageUp(VdkKeyEvent& e);
	void OnDown(VdkKeyEvent& e);
	void OnUp(VdkKeyEvent& e);
	void OnCtrlPlusHomeEnd(VdkKeyEvent& e);

	void OnItemAddRemove(size_t row, bool add);

private:

	// 处理按住 Shift 键时按动上下导航键的按键事件
	//
	// 模拟鼠标点击，由鼠标事件处理函数来处理。
	void OnShiftUpDownKeys(int currSel, wxDC& dc);

	/// 是否处于全选的状态中
	bool IsAllSelected();

	// 清除已选择的列表项，内部调用
	void SelectNone(wxDC* pDC);

	// 更新最后点击的选项
	// 
	// 主要是为了只保留一个修改 m_lastSelected 变量的入口。
	void UpdateVeryLastSel(int row);

protected:

	/// 获取多项选择时指定的第一项的 ID
	int& GetSelStart() { return m_selStart; }

	/// 获取最后点击的选项
	int& GetVeryLastClicked() { return m_veryLastClicked; }

	/// 获取被选中的行
	ArrayOfSortedInts& GetSelItems() { return m_selItems; }

private:

	bool m_multiSel; // 是否启用多选

	wxBrush m_selected; // 选中项的背景画刷
	int m_selStart; // 多项选择时指定的第一项的 ID
	int m_veryLastClicked; // 最后点击的选项，用于“上”“下”键的处理

	ArrayOfSortedInts m_selItems;

	//============================================

	OnItemClickListener* m_clickListener;
};
