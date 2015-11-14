/***************************************************************
 * Name:      LvpClick.cpp
 * Purpose:   实现 VdkListView 单选、多选
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#include "StdAfx.h"
#include "ListView/LvpClick.h"

#include "ListView/VdkListView.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

extern int CompareInts(int n1, int n2);

LvpClick::LvpClick(ListView* list, SelectMode sm, const wxBrush& selected)
	: IListViewPlaugin( list ), 
	  m_multiSel( sm == SM_MULTI ), 
	  m_selected( selected ),
	  m_selStart( wxNOT_FOUND ),
	  m_veryLastClicked( wxNOT_FOUND ),
	  m_selItems( CompareInts ),
	  m_clickListener( NULL )
{
	list->SetAddinStyle( VCS_KEY_EVENT );
}

void LvpClick::OnEraseRow(int row, wxDC& dc)
{
	if( m_selItems.Index( row ) != wxNOT_FOUND )
	{
		dc.SetBrush( m_selected );
	}
}

void LvpClick::OnClickDown(int rowAtPointer, VdkMouseEvent& e)
{
	VdkListView* list = GetListView();
	int shownItems = list->GetShownItems();

	if( e.shiftDown && m_multiSel )
	{
		// 用户点击了列表框最后的空白，直接丢弃这个事件
		if( rowAtPointer == wxNOT_FOUND )
			return;

		if( !m_selItems.empty() )
		{
			int nFirst, nLast;

			// 向下多选
			if( rowAtPointer >= m_selStart )
			{
				nFirst = m_selStart;
				nLast = rowAtPointer;
			}
			else if( rowAtPointer < m_selStart ) // 向上多选
			{
				nFirst = rowAtPointer;
				nLast = m_selStart;
			}

			int base; // 正在显示的第一行
			list->GetViewStart( NULL, &base );

			int i; // 是 m_selItems 保存的、属于 VdkListCtrl 的 index，实际数字
			// 清空 nFirst 之前不再是被选中的项目
			i = m_selItems[0];
			while( i < nFirst )
			{
				m_selItems.Remove( i );

				if( i >= base && i <= base + shownItems - 1 )
					list->UpdateRow( i, e.dc );

				i = m_selItems[0];
			}

			// 清空 nLast 之后不再是被选中的项目
			i = m_selItems[ m_selItems.size() - 1 ];
			while( i > nLast )
			{
				m_selItems.Remove( i );

				if( i >= base && i <= base + shownItems - 1 )
					list->UpdateRow( i, e.dc );

				i = m_selItems[ m_selItems.size() - 1 ];
			}

			for( int j = nFirst; j <= nLast; j++ )
			{
				// 不要重复添加条目
				if( m_selItems.Index( j ) == wxNOT_FOUND )
				{
					m_selItems.Add( j );

					if( j >= base && j <= base + shownItems - 1 )
						list->UpdateRow( j, e.dc );
				}
			}

		} // END if( !m_selItems.empty() )
		else
		{
			m_selItems.Add( rowAtPointer );
			m_selStart = rowAtPointer; // 最后被选中的一个条目作为起点

			list->UpdateRow( rowAtPointer, e.dc );
		}

		UpdateVeryLastSel( rowAtPointer );

	} // END if( e.shiftDown && m_multiSel )
	else
	{
		if( rowAtPointer != wxNOT_FOUND )
		{
			bool selected = (m_selItems.Index( rowAtPointer ) != wxNOT_FOUND);

			if( m_multiSel )
			{
				// 多选
				if( e.controlDown && selected )
				{
					m_selItems.Remove( rowAtPointer );
				}
				// TODO: 这里应该区分鼠标左键按下和弹起的情况，例如拖动多选
				// 项目
				else if( !selected )
				{
					if( !e.controlDown ) // 清空其他选择
					{
						SelectNone( &e.dc );
					}

					m_selItems.Add( rowAtPointer );
				}
			}
			else // END if( m_multiSel )
			{
				if( !selected )
				{
					SelectNone( &e.dc );
					m_selItems.Add( rowAtPointer );
				}
				else
				{
					return;
				}
			}

			m_selStart = rowAtPointer; // 将最后被选中的一个条目作为起点
			UpdateVeryLastSel( rowAtPointer );

			list->UpdateRow( rowAtPointer, e.dc );
		}
		else // 在最后的空白处单击，应该清空已选择的条目
		{
			SelectNone( &e.dc );
		}

	}
}

void LvpClick::OnClickUp(int rowAtPointer, VdkMouseEvent& e)
{
	if( !e.shiftDown && !e.controlDown && 
		(m_selItems.Index( rowAtPointer ) != wxNOT_FOUND) )
	{
		Select( rowAtPointer, &e.dc );
	}
}

void LvpClick::OnDClick(int rowAtPointer, VdkMouseEvent& e)
{
	if( m_clickListener )
	{
		m_clickListener->OnItemClick( this, rowAtPointer, e.dc );
	}
}

void LvpClick::Select(int index, wxDC* pDC)
{
	ListView* lv = GetListView();
	ListView::Adapter* adpater = lv->GetAdapter();
	if( !adpater || adpater->IsEmpty() )
		return;

	wxASSERT( index >= 0 );
	wxASSERT( size_t( index ) < adpater->GetRowCount() );

	SelectNone( NULL );

	m_selItems.Add( index );
	m_selStart = index;
	UpdateVeryLastSel( index );

	// 不能直接调用 Draw() ，因为要更新滚动条
	lv->RefreshState( pDC );
}

void LvpClick::Select(int beg, int end, wxDC* pDC)
{
	wxASSERT_MSG( false, L"未实现" );
}

void LvpClick::Select(const wxArrayInt& items, wxDC* pDC)
{
	wxASSERT_MSG( false, L"未实现" );
}

void LvpClick::SelectNone()
{
	wxASSERT_MSG( false, L"未实现" );
}

void LvpClick::SelectNone(wxDC* pDC)
{
	m_selStart = wxNOT_FOUND;
	UpdateVeryLastSel( wxNOT_FOUND );

	// 注意 if-else 结果的代码冗余！
	if( pDC )
	{
		ArrayOfSortedInts selItems( m_selItems );
		m_selItems.clear();

		VdkListView* list = GetListView();
		int numSel = selItems.size();
		for( int i = 0; i < numSel; i++ )
		{
			list->UpdateRow( selItems[i], *pDC );
		}
	}
	else
	{
		m_selItems.clear();
	}
}

void LvpClick::SetSelectMode(SelectMode sm, wxDC* pDC)
{
	// 注意这里的代码冗余！
	if( pDC )
	{
		VdkListView* list = GetListView();

		VdkDcDeviceOriginSaver saver( *pDC );
		list->PrepareDC( *pDC );

		SelectNone( pDC );

		list->RestoreDC( *pDC );
	}
	else
	{
		SelectNone( NULL );
	}
	
	m_multiSel = sm == SM_MULTI;
}

void LvpClick::OnKey(VdkKeyEvent& e)
{
	wxKeyEvent* nativeEvent = e.GetNativeObj();
	if( nativeEvent )
	{
		if( nativeEvent->GetEventType() != wxEVT_KEY_DOWN )
			return;
	}

	int kc = e.GetKeyCode();
	switch( kc )
	{
	case WXK_PAGEDOWN:

		OnPageDown( e );
		break;

	case WXK_PAGEUP:

		OnPageUp( e );
		break;

	case WXK_DOWN:

		OnDown( e );
		break;

	case WXK_UP:

		OnUp( e );
		break;

	case WXK_HOME:
	case WXK_END:

		if( e.controlDown )
		{
			OnCtrlPlusHomeEnd( e );
		}

		break;

	default:

		break;
	}
}

void LvpClick::OnPageDown(VdkKeyEvent& e)
{
	VdkListView* list = GetListView();

	int base;
	list->GetViewStart( NULL, &base );

	int shownItems = list->GetShownItems();
	// - 1 是为了将已选项移动到可视区域最后一行
	int bottom = base + shownItems - 1;
	if( list->GetRect().height % list->GetRowHeight() )
		bottom -= 1; // 最后一行显示不全

	int numRows = list->GetAdapter()->GetRowCount();
	if( bottom >= numRows )
	{
		bottom = numRows - 1;
	}

	int currSel = m_selItems.empty() ? wxNOT_FOUND : m_selItems[0];

	// 第一屏先将选择项移动到可视区域底部
	if( currSel == wxNOT_FOUND || currSel != bottom )
	{
		SelectNone( &e.dc );
		m_selItems.Add( bottom );

		list->UpdateRow( bottom, e.dc );
	}
	else
	{
		// 再下就超出范围了
		if( bottom == numRows - 1 )
			return;

		base += shownItems;
		bottom += shownItems;
		if( bottom >= numRows )
			bottom = numRows - 1;

		SelectNone( NULL );
		m_selItems.Add( bottom );

		list->SetViewStart( 0, base, &e.dc );
	}

	UpdateVeryLastSel( bottom );
	m_selStart = bottom;
}

void LvpClick::OnPageUp(VdkKeyEvent& e)
{
	VdkListView* list = GetListView();

	int base;
	list->GetViewStart( NULL, &base );

	int shownItems = list->GetShownItems();
	int currSel = m_selItems.empty() ? wxNOT_FOUND : m_selItems[0];

	// 第一屏先将选择项移动到可视区域顶部
	if( currSel == wxNOT_FOUND || currSel != base )
	{
		SelectNone( &e.dc );
		m_selItems.Add( base );

		list->UpdateRow( base, e.dc );
	}
	else
	{
		if( base == 0 )
			return;

		base -= shownItems;
		if( base < 0 )
			base = 0;

		SelectNone( NULL );
		m_selItems.Add( base );

		list->SetViewStart( 0, base, &e.dc );
	}

	UpdateVeryLastSel( base );
	m_selStart = base;
}

void LvpClick::OnCtrlPlusHomeEnd(VdkKeyEvent& e)
{
	ListView* lv = GetListView();
	ListView::Adapter* adapter = lv->GetAdapter();
	if( !adapter || adapter->IsEmpty() )
		return;

	if( e.GetKeyCode() == WXK_HOME )
	{
		lv->GoTo( 0, NULL );
		Select( 0, &e.dc );
	}
	else
	{
		lv->GoTo( 1, NULL );

		int last = adapter->GetRowCount() - 1;
		Select( last, &e.dc );
	}
}

bool LvpClick::IsAllSelected()
{
	ListView::Adapter* adapter = GetListView()->GetAdapter();
	if( !adapter )
	{
		return false;
	}

	return m_selItems.size() == adapter->GetRowCount();
}

void LvpClick::UpdateVeryLastSel(int row)
{
	m_veryLastClicked = row;
}

void LvpClick::OnShiftUpDownKeys(int currSel, wxDC& dc)
{
	VdkMouseEvent me( LEFT_DOWN, wxDefaultPosition, dc );
	me.shiftDown = true;

	OnClickDown( currSel, me );
}

void LvpClick::OnDown(VdkKeyEvent& e)
{
	VdkListView* list = GetListView();

	int base;
	list->GetViewStart( NULL, &base );

	int shownItems = list->GetShownItems();
	int currSel = wxNOT_FOUND;

	if( IsAllSelected() )
	{
		SelectNone( &e.dc );
		currSel = wxNOT_FOUND;
	}

	// 使用鼠标最后选择的一项来确定下一要选的条目
	// TODO: 全选时此值如何？
	if( m_veryLastClicked != wxNOT_FOUND )
		currSel = m_veryLastClicked;
	else
		currSel = base - 1;

	currSel++;

	// 超出范围
	if( size_t( currSel ) >= list->GetAdapter()->GetRowCount() )
		return;

	if( e.shiftDown )
	{
		if( !m_selItems.empty() )
		{
			OnShiftUpDownKeys( currSel, e.dc );
		}
		else // 原先没有任何选择
		{
			return;
		}
	}
	else // END if( e.shiftDown )
	{
		SelectNone( &e.dc );
		m_selItems.Add( currSel );
		m_selStart = currSel;
		UpdateVeryLastSel( currSel );

		list->UpdateRow( currSel, e.dc );
	}

	// 下移可视区域
	int bottom = base + shownItems - 1;
	// - 1 是为了将已选项移动到可视区域最后一行
	if( list->GetRect().height % list->GetRowHeight() )
		bottom -= 1; // 最后一行显示不全

	if( bottom < currSel )
	{
		list->SetViewStart( 0, currSel - shownItems + 1, &e.dc );
	}
}

void LvpClick::OnUp(VdkKeyEvent& e)
{
	VdkListView* list = GetListView();

	int base;
	list->GetViewStart( NULL, &base );

	int shownItems = list->GetShownItems();
	int currSel = wxNOT_FOUND;

	if( IsAllSelected() )
	{
		SelectNone( &e.dc );
		currSel = wxNOT_FOUND;
	}

	int bottom = base + shownItems - 1;
	// - 1 是为了将已选项移动到可视区域最后一行
	if( list->GetRect().height % list->GetRowHeight() )
		bottom -= 1; // 最后一行显示不全

	// 使用最后选择的一项来确定下一要选的条目
	if( m_veryLastClicked != wxNOT_FOUND )
		currSel = m_veryLastClicked;
	else
		currSel = bottom + 1;

	currSel--;

	// 超出范围
	if( currSel < 0 )
		return;

	if( e.shiftDown )
	{
		if( !m_selItems.empty() )
		{
			OnShiftUpDownKeys( currSel, e.dc );
		}
		else // 原先没有任何选择
		{
			return;
		}
	}
	else // END if( e.shiftDown )
	{
		SelectNone( &e.dc );
		m_selItems.Add( currSel );
		m_selStart = currSel;
		UpdateVeryLastSel( currSel );

		list->UpdateRow( currSel, e.dc );
	}

	// 上移可视区域
	if( base > currSel )
	{
		list->SetViewStart( 0, currSel, &e.dc );
	}
}

void LvpClick::OnItemAdd(size_t row)
{
	OnItemAddRemove( row, true );
}

void LvpClick::OnItemRemove(size_t row)
{
	OnItemAddRemove( row, false );
}

void LvpClick::OnItemAddRemove(size_t row, bool add)
{
	wxVector< int* > rowsMayUpdate;

	size_t numSel = m_selItems.size();
	for( size_t i = 0; i < numSel; i++ )
	{
		rowsMayUpdate.push_back( &m_selItems[i] );
	}

	rowsMayUpdate.push_back( &m_selStart );
	rowsMayUpdate.push_back( &m_veryLastClicked );

	//------------------------------------------------------

	wxVector< int* >::iterator iter( rowsMayUpdate.begin() );
	for( ; iter != rowsMayUpdate.end(); ++iter )
	{
		int& num = **iter;

		if( num >= (int) row )
		{
			if( add )
			{
				++num;
			}
			else
			{
				--num;
			}
		}
	}
}

void LvpClick::OnClear()
{
	m_selItems.clear();

	m_selStart = wxNOT_FOUND;
	m_veryLastClicked = wxNOT_FOUND;
}
