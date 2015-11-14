/***************************************************************
 * Name:      LvpDraggAndReorder.cpp
 * Purpose:   实现 VdkListView 通过拖动改变项目排序
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#include "StdAfx.h"
#include "ListView/LvpDraggAndReorder.h"

#include "ListView/VdkListView.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

LvpDraggAndReorder::RowTracker::RowTracker(MoveListener& listener)
	: m_listener( listener )
{

}

void LvpDraggAndReorder::RowTracker::Move(int row, int dst)
{
	wxASSERT( row != dst );
	int delta = row - dst;

	// 向下拖动
	if( delta < 0 )
	{
		wxVector< int* >::iterator iter( m_rows.begin() );
		for( ; iter != m_rows.end(); ++iter )
		{
			if( **iter > row && **iter < dst )
			{
				--(**iter);
			}
			else if( **iter == row )
			{
				**iter = dst;
			}
			else if( **iter == dst )
			{
				++(**iter);
			}
		}
	}
	else
	{
		wxVector< int* >::iterator iter( m_rows.begin() );
		for( ; iter != m_rows.end(); ++iter )
		{
			// 目标(含)与源之间
			if( **iter >= dst && **iter < row )
			{
				++(**iter);
			}
			else if( **iter == row )
			{
				**iter = dst;
			}
		}
	}

	m_listener.OnRowMove( row, dst );
}

void LvpDraggAndReorder::RowTracker::Clear()
{
	m_rows.clear();
}

void LvpDraggAndReorder::RowTracker::AddRow(int& row)
{
	m_rows.push_back( &row );
}

//////////////////////////////////////////////////////////////////////////

LvpDraggAndReorder::LvpDraggAndReorder(ListView* list, SelectMode sm, 
									   const wxBrush& selected, 
									   const wxPen& draggTarget,
									   RowTracker::MoveListener& listener)
   : LvpClick( list, sm, selected ), 
     m_lastDraggTarget( wxNOT_FOUND ),
	 m_rowTracker( listener ),
     m_draggTarget( draggTarget ) // TODO:
{

}

void LvpDraggAndReorder::OnDragg(int rowAtPointer, VdkMouseEvent& e)
{
	const ArrayOfSortedInts& selItems = GetSelItems();
	if( selItems.empty() )
		return;

	VdkListView* list = GetListView();

	// 继续向看不到的地方拖动
	if( m_lastDraggTarget == rowAtPointer )
	{
		int shownItems = list->GetShownItems();

		int base;
		list->GetViewStart( NULL, &base );

		if( ( (rowAtPointer == base) && (rowAtPointer != 0) ) || 
			( (rowAtPointer >= base + shownItems - 1) &&
			   rowAtPointer != list->GetAdapter()->GetRowCount() - 1 ) )
		{
			// 模拟发送滑轮事件到控件
			int fakeEventCode;

			if( rowAtPointer == base )
			{
				rowAtPointer--;
				fakeEventCode = WHEEL_UP;
			}
			else
			{
				rowAtPointer++;
				fakeEventCode = WHEEL_DOWN;
			}

			/* 这里不能用 VdkDcDeviceOriginSaver，而应该直接重新设定 DC ，
			 * 重新设置正确的裁剪区域 */
			VdkMouseEvent fakeEvent( fakeEventCode, wxDefaultPosition, e.dc );
			list->HandleMouseEvent( fakeEvent );

			// 重新设定 DC
			list->PrepareDC( e.dc );
		}
		else
		{
			return;
		}
	}

	if( m_lastDraggTarget != wxNOT_FOUND )
	{
		list->UpdateRow( m_lastDraggTarget, e.dc );
	}

	// 避免拖动事件太过灵敏，目标为已选项时不要画白线
	if( selItems.Index( rowAtPointer ) == wxNOT_FOUND )
	{
		int rowHeight = list->GetRowHeight();
		int y = rowHeight * rowAtPointer + 1;

		e.dc.SetPen( m_draggTarget );
		e.dc.DrawLine( 0, y, list->GetRect().width, y );
	}

	m_lastDraggTarget = rowAtPointer;
}

void LvpDraggAndReorder::OnClickUp(int rowAtPointer, VdkMouseEvent& e)
{
	if( m_lastDraggTarget == wxNOT_FOUND )
	{
		// 基类需要一定的处理
		LvpClick::OnClickUp( rowAtPointer, e );

		return;
	}

	//---------------------------------------------------------------

	m_lastDraggTarget = wxNOT_FOUND;

	VdkListView* list = GetListView();
	ArrayOfSortedInts& selItems = GetSelItems();
	wxASSERT( !selItems.empty() );

	// 将自己移动到已选条目的现位置？
	if( selItems.Index( rowAtPointer ) != wxNOT_FOUND )
	{
		// 取消已画在界面上的横线
		list->UpdateRow( rowAtPointer, e.dc );
		return;
	}

	//---------------------------------------------------------------
	// 保存要跟踪的行号

	m_rowTracker.Clear();

	int numSel = selItems.size();
	for( int i = 0; i < numSel; i++ )
	{
		m_rowTracker.AddRow( selItems[i] );
	}

	m_rowTracker.AddRow( GetSelStart() );
	m_rowTracker.AddRow( GetVeryLastClicked() );

	int dst = rowAtPointer;
	m_rowTracker.AddRow( dst );

	int insertPoint = dst;

	// 开始移动
	int forValidating = dst - 1;
	for( int i = 0; i < numSel; i++ )
	{
		int origin = selItems[i];
		m_rowTracker.Move( selItems[i], insertPoint );
		
		// 向上拖动
		if( origin > insertPoint )
		{
			insertPoint++;
		}

		//// 确保移动成功
		//wxASSERT( selItems[i] == ++forValidating );
	}

	list->RefreshState( &e.dc );
}

void LvpDraggAndReorder::OnItemAdd(size_t row)
{
	if( m_lastDraggTarget >= int( row ) )
	{
		m_lastDraggTarget++;
	}
}

void LvpDraggAndReorder::OnItemRemove(size_t row)
{
	if( m_lastDraggTarget >= int( row ) )
	{
		m_lastDraggTarget--;
	}
}

void LvpDraggAndReorder::OnClear()
{
	m_lastDraggTarget = wxNOT_FOUND;
}
