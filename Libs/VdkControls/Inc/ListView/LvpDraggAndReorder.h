/***************************************************************
 * Name:      LvpDraggAndReorder.h
 * Purpose:   实现 VdkListView 通过拖动改变项目排序
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#pragma once
#include "LvpClick.h"
#include <wx/vector.h>

/// 实现 VdkListView 通过拖动改变项目排序
class LvpDraggAndReorder : public LvpClick
{
public:

	/// 行号跟踪器
	/// 
	/// 保存一些行号，并对其进行跟踪，使之在拖动某行之后，
	/// 其余项目的行号仍然能在新列表中保持正确。
	class RowTracker
	{
	public:

		/// 监听行拖动事件
		class MoveListener
		{
		public:

			/// 行号为@a row 的行从@a row 拖到@a dst
			virtual void OnRowMove(int row, int dst) = 0;
		};

		/// 构造函数
		RowTracker(MoveListener& listener);

		/// 清空所有已注册的行号
		void Clear();

		/// 注册一行
		void AddRow(int& row);

		/// 将行号为@a row 的行从@a row 拖到@a dst
		void Move(int row, int dst);

	private:

		wxVector< int* > m_rows;
		MoveListener& m_listener;
	};
    
	/// 构造函数
	LvpDraggAndReorder(ListView* list, SelectMode sm, 
					   const wxBrush& selected, 
					   const wxPen& draggTarget,
					   RowTracker::MoveListener& listener);

private:

    virtual void OnDragg(int rowAtPointer, VdkMouseEvent& e);
    virtual void OnClickUp(int rowAtPointer, VdkMouseEvent& e);

	virtual void OnItemAdd(size_t row);
	virtual void OnItemRemove(size_t row);
	virtual void OnClear();

private:

	int m_lastDraggTarget; // 当拖动条目进行重排时当前的重排目标
	RowTracker m_rowTracker; // 行号跟踪器

	wxPen m_draggTarget; // 拖动项目时表示目标位置的线条颜色
};
