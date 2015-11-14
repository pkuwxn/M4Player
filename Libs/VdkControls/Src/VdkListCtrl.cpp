/***************************************************************
 * Name:      VdkListCtrl.cpp
 * Purpose:   Code for VdkListCtrl implementation
 * Author:    Ning (vanxining@139.com)
 * Created:   2010-02-01
 * Copyright: Ning
 **************************************************************/
#include "StdAfx.h"
#include "VdkListCtrl.h"

#include "VdkWindow.h"
#include "VdkScrollBar.h"
#include "VdkDC.h"
#include "wxUtil.h"

#include <wx/listbase.h>

#include <iterator> // for advance()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLONEABLE_VOBJECT( VdkListCtrl, VdkControl );

#include "Images/list_ctrl_header.xpm"

enum {
	// Ĭ���и�
	CNST_DEFAULT_ROW_HEIGHT = 20,
	// �������߶�
	CNST_HEADER_HEIGHT = 20,
};

//////////////////////////////////////////////////////////////////////////
// һЩ��� std::list �ı�������
// TODO: ���� STL �ĵ�Ч��ʩ

template< class ListT >
inline typename ListT::value_type at(ListT& lst, size_t index)
{
	typename ListT::iterator iter( lst.begin() );
	std::advance( iter, index );

	return *iter;
}

template< class ListT >
inline typename ListT::value_type at(const ListT& lst, size_t index)
{
	typename ListT::const_iterator iter( lst.begin() );
	std::advance( iter, index );

	return *iter;
}

template< class ListT >
size_t indexOf(const ListT& lst, typename ListT::value_type obj)
{
	size_t index = 0;
	typename ListT::const_iterator iter( lst.begin() );
	typename ListT::const_iterator e( lst.end() );

	for( ; iter != e; ++iter, ++index )
	{
		if( *iter == obj )
			return index;
	}

	return wxNOT_FOUND;
}

template< class ListT >
typename ListT::iterator insert
	(ListT& lst, size_t index, typename ListT::value_type obj)
{
	typename ListT::iterator insertPoint( lst.begin() );
	advance( insertPoint, index );

	return lst.insert( insertPoint, obj );
}

//////////////////////////////////////////////////////////////////////////

VdkListCtrl::VdkListCtrl(long style)
		   : VdkScrolledWindow( style | VCS_KEY_EVENT ),
			 m_rowHeight( 0 ),
			 m_shownItems( 0 ),
			 m_nShownItemsAddIn( 0 ),
			 m_pHeader( NULL ),
			 m_lastHilighted( wxNOT_FOUND ),
			 m_selStart( wxNOT_FOUND ),
			 m_lastDraggTarget( wxNOT_FOUND ),
			 m_selItems( CompareInts ),
			 m_cols( NULL )
{
	m_crossBrush1.SetColour( wxColour( 243,248,250 ) );
	m_crossBrush2 = *wxWHITE_BRUSH;

	wxColour hilight( 228,239,248 );
	m_hilighted.SetColour( hilight );

	wxColour selected( 255,254,223 );
	m_selected.SetColour( selected );

	m_draggTarget.SetColour( *wxBLACK );
	m_draggTarget.SetWidth( 2 );

	if( TestStyle( VLCS_HOVERING ) )
		RemoveStyle( VCS_ONESHOT_HOVERING );
}

VdkListCtrl::~VdkListCtrl()
{

}

void VdkListCtrl::OnXrcCreate(wxXmlNode* node)
{
	if( XmlGetContentOfBoolean( FindChildNode( node, L"header" ), true ) )
		SetAddinStyle( VLCS_HEADER );

	if( XmlGetContentOfBoolean( FindChildNode( node, L"multi-select" ), true ) )
		SetAddinStyle( VLCS_MULTI_SELECT );

	Create( m_Window, GetXrcName( node ), GetXrcRect( node ) );
	m_align = GetXrcAlign( node );
}

void VdkListCtrl::Create(VdkWindow* parent,
						 const wxString& strName,
						 const wxRect& rc)
{
	m_strName = strName;
	m_Rect = rc;
	SetVdkWindow( parent );

	//==============================================

	if( TestStyle( VLCS_HEADER ) )
	{
		m_Rect.y += CNST_HEADER_HEIGHT;
		m_Rect.height -= CNST_HEADER_HEIGHT;

		m_pHeader = new VdkLcHeader( m_Window,
									 m_strName + L"_Header",
									 wxRect( 0, - CNST_HEADER_HEIGHT,
											 m_Rect.width,
											 CNST_HEADER_HEIGHT ),
									 this,
									 m_cols );

		m_pHeader->SetParent( this );
		m_Window->AddCtrl( m_pHeader );
	}

	if( m_rowHeight == 0 )
		SetRowHeight( CNST_DEFAULT_ROW_HEIGHT );

	CalcShownItems();
}

void VdkListCtrl::Clone(VdkListCtrl* o)
{
	// ���ȸ�����������Ϊ���ǿ����� VdkScrolledWindow::Clone ���޸�
	// ��������һ�޸ı����ܷ�ӳ���������
	// ע�� wxList �Ǳ������ݵ�ָ��
	LcColIter i;
	for( i = o->m_cols.begin(); i != o->m_cols.end(); ++i )
	{
		VdkLcColumn* col = new VdkLcColumn;
		col->Clone( *(*i) );
		m_cols.push_back( col );
	}

	//==============================================

	VdkScrolledWindow::Clone( o );

	m_rowHeight = o->m_rowHeight;
	m_shownItems = o->m_shownItems;
	m_nShownItemsAddIn = o->m_nShownItemsAddIn;

	m_draggTarget = o->m_draggTarget;
	m_hilighted = o->m_hilighted;
	m_selected = o->m_selected;
	m_crossBrush1 = o->m_crossBrush1;
	m_crossBrush2 = o->m_crossBrush2;

	ChildIterator chd( begin() );
	for( ; chd != end(); ++chd )
	{
		m_pHeader = dynamic_cast< VdkLcHeader* >( chd.get() );
		if( m_pHeader )
			break;
	}
}

void VdkListCtrl::Attach(LcDataSet* dataSet)
{
	m_cols.Attach( dataSet );

	// TODO: �� InsertColumn ����һ�������Ƚ���ϸ��˵��
	// ע����� m_lastSelelected
	if( GetColumnCount() > 0 )
	{
		UpdateLastSelected( (*(m_cols.begin()))->cells.end() );

		if( !IsEmpty() )
		{
			UpdateSize( NULL );
		}
	}
}

LcDataSet* VdkListCtrl::Datach()
{
	return m_cols.Datach();
}

void VdkListCtrl::SetRowHeight(int height, wxDC* pDC)
{
	if( m_rowHeight == height )
		return;

	int ch = m_WindowImpl->GetCharHeight();
	if( ch > height )
		height = ch;

	m_rowHeight = height;

	//==============================================

	if( m_cols.IsOk() )
	{
		for( LcColIter i( m_cols.begin() ); i != m_cols.end(); ++i )
		{
			(*i)->height = m_rowHeight;
		}
	}

	SetScrollRate( 20, m_rowHeight );
	UpdateSize( NULL );
	CalcShownItems();

	if( pDC )
	{
		Draw( *pDC );
	}
}

void VdkListCtrl::UpdateSize(wxDC* pDC)
{
	SetVirtualSize( m_Rect.width, m_rowHeight * GetRowCount(), pDC );
}

bool VdkListCtrl::ScrollList(int dX, int dY, wxDC* pDC)
{
	return false;
}

bool VdkListCtrl::AjustCollumn(size_t w, size_t adjust, size_t adjustFrom)
{
	VdkLcColumn* col = at( m_cols, adjust );
	VdkLcColumn* colFrom = at( m_cols, adjustFrom );

	int dw = w - col->width;
	if( colFrom->width <= dw )
		return false;

	col->UpdateWidth( w );
	colFrom->UpdateWidth( colFrom->width - dw );

	if( col->x < colFrom->x )
		colFrom->x += dw;  // Դ�����ұߣ�Դ������
	else
		col->x -= dw; // Դ������ߣ�Ŀ��������

	return true;
}

void VdkListCtrl::OnDraw(wxDC& dc)
{
	int numRows = GetRowCount();
	int y, maxY;
	GetViewStart( NULL, &y );
	GetMaxViewStart( NULL, &maxY );

	// ��������һ��ʱ��յײ��ѻ�����
	if( y == maxY )
	{
		int lh, yBlank; // �հ�����ĸ߶�
		GetVirtualSize( NULL, &lh );
		yBlank = m_Rect.height - (numRows - y) * GetRowHeight();

		wxRect rc( 0, lh, m_Rect.width, yBlank );
		dc.SetBrush( m_crossBrush1 );
		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.DrawRectangle( rc );
	}

	if( numRows == 0 )
		return;

	//==============================================

	dc.SetFont( m_Font );

	LcCellIter it( GetCellIterator( *(m_cols.begin()), y ) );
	for( int i = y; (i < (y + m_shownItems)) && (i < numRows); ++i, ++it )
		EraseRow( it, i, dc );

	int w = 0; // �п���ۼ�
	LcColIter i( m_cols.begin() ), e( m_cols.end() );
	int colIndex = 0;
	for( ; i != e; ++i, ++colIndex )
	{
		DrawColumn( *(*i), colIndex, dc, y, y + m_shownItems );
		w += (*i)->width;
	}
}

void VdkListCtrl::DrawColumn(const VdkLcColumn& col, 
							 int colIndex, 
							 wxDC& dc,
							 unsigned from,
							 unsigned to)
{
	VdkLcHilightState state;
	LcCellIter cell( GetCellIterator( (VdkLcColumn *) &col, from ) );
	size_t numCells = col.cells.size();

	for( unsigned i = from; (i < to) && (i < numCells); ++i, ++cell )
	{
		state = VDKLC_HS_NORMAL;

		if( TestState( VLCST_SELECT_ALL ) ||
			(m_selItems.Index( i ) != wxNOT_FOUND) )
		{
			state = VDKLC_HS_SELECTED;
		}
		else if( i == (unsigned) m_lastHilighted )
		{
			state = VDKLC_HS_HILIGHT;
		}

		dc.SetTextForeground( col.textColor );

		if( DoDrawCellText( *cell, colIndex, i, dc, state ) == VCCDRF_DODEFAULT )
		{
			(*cell)->DrawLabel( dc, col.x + col.leftPadding, i * col.height );
		}
	}
}

int VdkListCtrl::EraseRow(const LcCellIter& it, int index, wxDC& dc)
{
	if( it == GetColumnEnd( 0 ) )
		return wxNOT_FOUND;

	if( index == wxNOT_FOUND )
	{
		index = std::distance( (*(m_cols.begin()))->cells.begin(), it );
	}

	if( TestState( VLCST_SELECT_ALL ) || 
		(m_selItems.Index( index ) != wxNOT_FOUND) )
	{
		dc.SetBrush( m_selected );
	}
	else if( index == m_lastHilighted )
	{
		dc.SetBrush( m_hilighted );
	}
	else if( index % 2 ==0 )
	{
		dc.SetBrush( m_crossBrush1 );
	}
	else
	{
		dc.SetBrush( m_crossBrush2 );
	}

	//-------------------------------------------------------

	dc.SetPen( *wxTRANSPARENT_PEN );

	if( DoEraseRow( it, index, dc ) == VCCDRF_DODEFAULT )
	{
		dc.DrawRectangle( 0, m_rowHeight * index, 
						  m_Rect.width, m_rowHeight );
	}

	return index;
}

void VdkListCtrl::DrawRowText(int index, wxDC& dc)
{
	if( m_cols.empty() )
		return;

	dc.SetFont( m_Font );

	LcColIter i( m_cols.begin() );
	int colIndex = 0;
	for( ; i != m_cols.end(); ++i, ++colIndex )
		DrawColumn( *(*i), colIndex, dc, index, index + 1 );
}

void VdkListCtrl::UpdateRow(int index, wxDC& dc)
{
	if( index == GetRowCount() )
		return;

	if( index == wxNOT_FOUND )
	{
		if( m_lastHilighted != wxNOT_FOUND )
		{
			index = m_lastHilighted;
			m_lastHilighted = wxNOT_FOUND;
		}
		else
		{
			return;
		}
	}

	LcCellIter it( GetCellIterator( *(m_cols.begin()), index ) );
	EraseRow( it, index, dc );
	DrawRowText( index, dc );
}

void VdkListCtrl::UpdateRow(const LcCellIter& it, wxDC& dc)
{
	if( it == GetColumnEnd( 0 ) )
		return;

	int index = EraseRow( it, wxNOT_FOUND, dc );
	DrawRowText( index, dc );
}

void VdkListCtrl::GetIndex(int y, int& index, int& indexMayOverflow)
{
	indexMayOverflow = index = 0;

	int itemsCount( GetRowCount() );
	if( itemsCount == 0 )
		return;

	int base; // ������ʾ�ĵ�һ��
	GetViewStart( NULL, &base );

	indexMayOverflow = base + y / m_rowHeight;
	index = indexMayOverflow;

	if( index >= itemsCount )
	{
		index = -1;
		return;
	}

	if( index < base )
		index = base;

	if( index >= base + m_shownItems )
		index = base + m_shownItems - 1;
}

void VdkListCtrl::OnMouseEvent(VdkMouseEvent& e)
{
	int itemsCount = GetRowCount();
	if( itemsCount == 0 )
		return;

	int base; // ������ʾ�ĵ�һ��
	GetViewStart( NULL, &base );

	int index, indexMayOverflow;
	GetIndex( e.mousePos.y, index, indexMayOverflow );

	//==============================================

	bool selected = (!m_selItems.empty()) && // �Ƿ��ѱ�ѡ��
				  (m_selItems.Index( index ) != wxNOT_FOUND);

	bool multi = TestStyle( VLCS_MULTI_SELECT );
	bool hovering = TestStyle( VLCS_HOVERING );
	bool selectAll = IsAllSel();

	switch( e.evtCode )
	{
	case HOVERING:

		if( !hovering )
			break;

		if( m_lastHilighted != index )
		{
			// ��Ҫ���²�����ʾ��Χ����
			if( m_lastHilighted >= base )
				UpdateRow( wxNOT_FOUND, e.dc );

			// ��Ҫ�����ѱ�ѡ�е���
			if( !selected )
			{
				m_lastHilighted = index;
				UpdateRow( index, e.dc );
			}
		}

		break;

	case LEFT_DOWN:
		{
			if( TestStyle( VLCS_NO_SELECT ) )
				break;

			if( selectAll )
			{
				SelectNone( &e.dc );
				selected = false;
			}

			if( e.shiftDown && multi )
			{
				// �û�������б�����Ŀհף�ֱ�Ӷ�������¼�
				if( index == -1 )
					break;

				if( !m_selItems.empty() )
				{
					int nFirst, nLast;

					// ���¶�ѡ
					if( index >= m_selStart )
					{
						nFirst = m_selStart;
						nLast = index;
					}
					else if( index < m_selStart ) // ���϶�ѡ
					{
						nFirst = index;
						nLast = m_selStart;
					}

					int i; // �� m_selItems ����ġ�
						   // ���� VdkListCtrl �� index��ʵ������

					// ��� nFirst ֮ǰ�����Ǳ�ѡ�е���Ŀ
					i = m_selItems[0];
					while( i < nFirst )
					{
						m_selItems.Remove( i );

						if( (i >= base) && (i <= base + m_shownItems - 1) )
							UpdateRow( i, e.dc );

						i = m_selItems[0];
					}

					// ��� nLast ֮�����Ǳ�ѡ�е���Ŀ
					i = m_selItems[m_selItems.size() - 1];
					while( i > nLast )
					{
						m_selItems.Remove( i );

						if( (i >= base) && (i <= base + m_shownItems - 1) )
							UpdateRow( i, e.dc );

						i = m_selItems[m_selItems.size() - 1];
					}

					for( int j = nFirst; j <= nLast; j++ )
					{
						// ��Ҫ�ظ������Ŀ
						if( m_selItems.Index( j ) == wxNOT_FOUND )
						{
							m_selItems.Add( j );

							if( (j >= base) && (j <= base + m_shownItems - 1) )
								UpdateRow( j, e.dc );
						}
					}

				}
				else
				{
					if( !selected )
						m_selItems.Add( index );

					m_selStart = index; // ���ѡ�е�һ����Ŀ��Ϊ���

					//==============================================

					UpdateRow( index, e.dc );
				}

				UpdateLastSelected( GetCellIterator( 0, index ) );

			} // if( multi && e.shiftDown )
			else
			{
				if( indexMayOverflow < itemsCount )
				{
					// ��ѡ
					if( e.controlDown && selected )
					{
						m_selItems.Remove( index );
					}
					// TODO: ����Ӧ���������������º͵��������������϶���ѡ
					// ��Ŀ
					else if( !selected )
					{
						if( !e.controlDown ) // ��ѡʱ�������ѡ��
						{
							SelectNone( &e.dc );
						}

						m_selItems.Add( index );
					}

					m_selStart = index; // �����ѡ�е�һ����Ŀ��Ϊ���
					UpdateLastSelected( GetCellIterator( 0, index ) );

					if( !hovering )
					{
						UpdateRow( index, e.dc );
					}

				}
				else // �����Ŀհ״�������Ӧ�������ѡ�����Ŀ
				{
					SelectNone( &e.dc );
				}

			}
		}

		break;

	case NORMAL:

		UpdateRow( wxNOT_FOUND, e.dc );

		// ������������֮��ſ������Ϸŵ�ָ�룬
		// ��ᷢ�� NORMAL �¼������� LEFT_UP
		if( m_lastDraggTarget == wxNOT_FOUND )
			break;

	case LEFT_UP:

		if( m_lastDraggTarget != wxNOT_FOUND )
		{
			m_lastDraggTarget = wxNOT_FOUND;

			// ���Լ��ƶ�����ѡ��Ŀ����λ�ã�
			if( selected )
			{
				break;
			}

			// ��֪��Ϊʲô����� size == 0 �����
			int numSel = m_selItems.size();
			if( numSel == 0 )
				break;

			//==============================================

			// ���ȱ��� Shift ��ѡ�����
			int selStartIndex = wxNOT_FOUND;
			if( m_selStart != wxNOT_FOUND )
				selStartIndex = m_selItems.Index( m_selStart );

			// �������������ţ���Ϊ���Ǵ�ԭ���ĵط���ȡ�� size ����Ŀ������
			// ���Ҫ���ֲ���λ�ò��䣬�����Ҫ��Ӧ����ĳ��ȷ���Ĵ���
			// ����Ϊ��ѡʱ���ܰ�ס Ctrl �������˷�ɢ�Ķ�ѡ��
			// ���û�ǡǡ�뽫��ѡ����Ŀ���뵽���еĿյ��
			int dstIndex = index;
			if( dstIndex > m_selItems[0] )
			{
				int i = 0; // TODO: 1 or 0 ?
				// index ������λ����ѡ��Ŀ�ļ�����
				while( i < numSel && dstIndex > m_selItems[i] )
				{
					i++;
					dstIndex++;
				}
			}

			// ����Ҫ�ƶ�����Ŀ�ĵ�����
			LcCellIter* selItemIters = new LcCellIter[numSel];

			LcColIter walker( m_cols.begin() );
			for( ; walker != m_cols.end(); ++walker )
			{
				VdkLcColumn* col = *walker;

				// �ȱ���Ҫ�ƶ������Ϊһ����ʼ�ƶ�����ѡ��������
				// ��ֵ���ò���ȷ��Ҫ������������Ƚϸ���
				for( int i = 0; i < numSel ; i++ )
				{
					if( i == 0 )
					{
						selItemIters[i] = col->cells.begin();
						std::advance( selItemIters[i], m_selItems[i] );
					}
					else
					{
						selItemIters[i] = selItemIters[i - 1];
						size_t delta =  m_selItems[i] - m_selItems[i - 1];
						std::advance( selItemIters[i], delta );
					}
				}

				LcCellIter dst( col->cells.begin() );
				std::advance( dst, dstIndex );

				for( int i = 0; i < numSel ; i++ )
				{
					col->cells.splice( dst, col->cells, selItemIters[i] );
				}

			}

			delete [] selItemIters;
			selItemIters = NULL;

			//-----------------------------------------------------------

			// �޸� m_selItems ��ֵ
			for( int i = 0; i < numSel ; i++ )
				m_selItems[i] = index + i;

			// ��ԭ Shift ��ѡ�����
			if( m_selStart != wxNOT_FOUND )
				m_selStart = m_selItems[selStartIndex];

			RefreshState( &e.dc );

			// ����֪ͨ�¼�
			if( IsReadyForEvent() )
			{
				// TODO: No corresponding END_DRAG?
				wxListEvent lstEvent( wxEVT_COMMAND_LIST_BEGIN_DRAG, m_id );
				wxPostEvent( m_WindowImpl, lstEvent );
			}

		} // END m_lastDraggTarget != wxNOT_FOUND
		else
		{
			if( !e.shiftDown && !e.controlDown && selected )
			{
				Select( index, &e.dc );
			}
		}

		break;

	case DLEFT_DOWN:

		if( TestStyle( VLCS_NO_SELECT ) )
			break;

		if( IsReadyForEvent() )
		{
			RestoreDC( e.dc );
			e.dc.SetDeviceOrigin( 0, 0 );

			FireEvent( &e.dc, (void *) index );

			PrepareDC( e.dc );
		}

		break;

	case RIGHT_UP:

		if( m_menu && (indexMayOverflow < itemsCount) )
		{
			if( !TestStyle( VLCS_NO_SELECT ) )
			{
				if( !selected )
				{
					if( !m_selItems.empty() )
						SelectNone( &e.dc );

					m_selItems.Add( index );
					m_selStart = index;
				}

				// �������ѡ����
				UpdateLastSelected( GetCellIterator( 0, index ) );

				// �� UpdateRow �ŵ����ŵ���(�ȴ�����״̬������)
				UpdateRow( index, e.dc );

			} // END if( !TestStyle( VLCS_NO_SELECT ) )

			// �����Ҽ��˵�
			wxPoint menupos( AbsoluteRect().GetPosition() );
			menupos.x += e.mousePos.x;
			menupos.y += e.mousePos.y;

			m_Window->ShowContextMenu( this, menupos );
		}

		// �����Ŀհ״��һ���Ӧ�������ѡ�����Ŀ
		if( index == -1 )
			SelectNone( &e.dc );

		break;

	case DRAGGING:
		{
			if( TestStyle( VLCS_NO_DRAGG ) ||
				TestStyle( VLCS_NO_SELECT ) ||
				m_selItems.size() == 0 )
			{
				break;
			}

			// �����򿴲����ĵط��϶�
			if( m_lastDraggTarget == index )
			{
				if( (index == base || index >= base + m_shownItems - 1) &&
					index != 0 && index != itemsCount - 1 )
				{
					if( index == base )
					{
						index--;
						e.evtCode = WHEEL_UP;
					}
					else
					{
						index++;
						e.evtCode = WHEEL_DOWN;
					}

					/* ���ﲻ���� VdkDcDeviceOriginSaver����Ӧ��ֱ�������趨 DC ��
					 * ����������ȷ�Ĳü����� */
					HandleMouseEvent( e );
					e.evtCode = DRAGGING;

					// �����趨 DC
					PrepareDC( e.dc );
				}
				else
				{
					break;
				}
			}

			if( m_lastDraggTarget != wxNOT_FOUND )
				UpdateRow( m_lastDraggTarget, e.dc );

			// �����϶��¼�̫��������Ŀ��Ϊ��ѡ��ʱ��Ҫ������
			if( !selected )
			{
				e.dc.SetPen( m_draggTarget );
				e.dc.DrawLine( 0, m_rowHeight * index + 1,
					m_Rect.width,
					m_rowHeight * index + 1 );
			}

			m_lastDraggTarget = index;
			break;
		}

	default:

		break;
	}
}

void VdkListCtrl::OnKeyEvent(VdkKeyEvent& ke)
{
	// ȫ������
	ke.Skip( true );

	//===============================================

	if( ke.evtCode != KEY_DOWN )
		return;

	if( IsEmpty() )
		return;

	wxDC* pDC( &ke.dc );

	//===============================================

	int base, shownItems;
	GetViewStart( NULL, &base );
	shownItems = static_cast< float >( m_Rect.height ) / GetRowHeight();
	// - 1 ��Ϊ�˽���ѡ���ƶ��������������һ��
	int bottom = base + shownItems;
	if( m_Rect.height % GetRowHeight() )
		bottom -= 1; // ���һ����ʾ��ȫ

	int count = GetItemCount();
	int sel = wxNOT_FOUND, selCount( m_selItems.size() );
	if( selCount > 0 )
	{
		sel = m_selItems[0];
	}

	bool selectAll = IsAllSel();

	//===============================================

	switch( ke.GetKeyCode() )
	{
	case WXK_PAGEDOWN:
		{
			// ��һ���Ƚ�ѡ�����ƶ�����������ײ�
			if( sel == wxNOT_FOUND || sel != bottom )
			{
				SelectNone( NULL );
				m_selItems.Add( bottom );

				RefreshState( pDC );
			}
			else
			{
				// ���¾ͳ�����Χ��
				if( bottom == count - 1 )
					break;

				base += shownItems;
				bottom += shownItems;
				if( bottom >= count )
					bottom = count - 1;

				SelectNone( NULL );
				m_selItems.Add( bottom );

				SetViewStart( 0, base, pDC );
			}

			UpdateLastSelected( GetCellIterator( 0, bottom ) );
			m_selStart = bottom;

			break;
		}

	case WXK_PAGEUP:
		{
			// ��һ���Ƚ�ѡ�����ƶ����������򶥲�
			if( sel == wxNOT_FOUND || sel != base )
			{
				SelectNone( NULL );
				m_selItems.Add( base );

				RefreshState( pDC );
			}
			else
			{
				if( base == 0 )
					break;

				base -= shownItems;
				if( base < 0 )
					base = 0;

				SelectNone( NULL );
				m_selItems.Add( base );

				SetViewStart( 0, base, pDC );
			}

			UpdateLastSelected( GetCellIterator( 0, base ) );
			m_selStart = base;

			break;
		}

	case WXK_DOWN:
		{
			if( selectAll )
			{
				SelectNone( pDC );
				sel = wxNOT_FOUND;
				selCount = 0;
			}

			// ʹ��������ѡ���һ����ȷ����һҪѡ����Ŀ
			if( m_lastSelected != GetColumnEnd( 0 ) )
				sel = IndexOf( *m_lastSelected );
			else
				sel = base - 1;

			sel++;

			// ������Χ
			if( sel >= count )
				break;

			if( ke.shiftDown )
			{
				if( selCount )
				{
					OnShiftUpDownKeys( sel, *pDC );
				}
				else // ԭ��û���κ�ѡ��
				{
					break;
				}
			}
			else // !e.ShiftDown()
			{
				SelectNone( pDC );
				m_selItems.Add( sel );
				m_selStart = sel;

				UpdateLastSelected( GetCellIterator( 0, sel ) );
			}

			if( bottom < sel )
			{
				SetViewStart( 0, sel - shownItems + 1, pDC );
			}

			break;
		}

	case WXK_UP:
		{
			if( selectAll )
			{
				SelectNone( pDC );
				sel = wxNOT_FOUND;
				selCount = 0;
			}

			// ʹ�����ѡ���һ����ȷ����һҪѡ����Ŀ
			if( m_lastSelected != GetColumnEnd( 0 ) )
				sel = IndexOf( *m_lastSelected );
			else
				sel = bottom + 1;

			sel--;

			// ������Χ
			if( sel < 0 )
				break;

			if( ke.shiftDown )
			{
				if( selCount )
				{
					OnShiftUpDownKeys( sel, *pDC );
				}
				else // ԭ��û���κ�ѡ��
				{
					break;
				}
			}
			else // !e.ShiftDown()
			{
				SelectNone( pDC );
				m_selItems.Add( sel );
				m_selStart = sel;

				UpdateLastSelected( GetCellIterator( 0, sel ) );
			}

			if( base > sel )
			{
				SetViewStart( 0, sel, pDC );
			}

			break;
		}

	case WXK_HOME:
	case WXK_END:

		if( ke.controlDown )
		{
			OnCtrlPlusHomeEnd( ke );
			RefreshState( pDC );
		}

		break;

	case 'A':

		if( !TestStyle( VLCS_NO_SELECT ) )
		{
			if( ke.controlDown )
			{
				SelectAll( pDC );
			}
		}

		break;

	default:

		break;
	}
}

void VdkListCtrl::OnShiftUpDownKeys(int sel, wxDC& dc)
{
	int base;
	GetViewStart( NULL, &base );

	PrepareDC( dc );

	wxPoint mousePos( 1, 1 + GetRowHeight() * ( sel - base ) );
	VdkMouseEvent me( LEFT_DOWN, mousePos, dc );
	me.shiftDown = true;

	OnMouseEvent( me );
}

void VdkListCtrl::InsertColumn(const VdkLcColumnInitializer& init_data)
{
	wxASSERT_MSG( m_cols.IsOk(), L"�����Ȱ󶨿��õ����ݼ���" );

	//----------------------------------------------------

	int numCols = m_cols.size();

	int x = 0;
	int index = init_data.Index;
	if( index == wxNOT_FOUND ) // Ĭ�ϴ�ĩβ����
		index = numCols;

	LcColIter dst( m_cols.begin() );
	std::advance( dst, index );

	// ������ȷ��λ��
	if( index > 0 )
	{
		LcColIter lastCol( dst );
		--lastCol;

		VdkLcColumn* last = *lastCol;
		x = last->x + last->width;
	}

	if( numCols > 0 && index != numCols ) // ���м����
	{
		LcColIter iter( dst );
		for( ; iter != m_cols.end(); ++iter )
		{
			(*iter)->x += init_data.Width;
		}
	}

	VdkLcColumn* column = new VdkLcColumn;
	column->percentage = TestWidthArragements( init_data.Percentage );
	column->leftPadding = init_data.LeftPadding;
	column->textAlign = init_data.TextAlign;
	column->textColor = init_data.TextColor;
	column->font = init_data.Font;
	column->heading = init_data.Heading;
	column->x = x;
	column->width = init_data.Width;
	column->height = m_rowHeight;

	//----------------------------------------------------

	dst = m_cols.insert( dst, column );

	if( numCols == 0 )
	{
		UpdateLastSelected( (*dst)->cells.end() );
	}
}

VdkLcCell* VdkListCtrl::InsertRow(size_t index, const wxString& label, bool updateSize)
{
	return DoInsertRow( index, label, updateSize );
}

VdkLcCell* VdkListCtrl::Append(const wxString& label, bool updateSize)
{
	return DoInsertRow( -1, label, updateSize );
}

VdkLcCell* VdkListCtrl::DoInsertRow
	(int index, const wxString& label, bool updateSize)
{
	wxASSERT_MSG( !m_cols.empty(), L"�����Ȳ����У�" );

	// ��һ�����еĵ�Ԫ����֯��һ����ʱ�����ظ�������
	VdkLcCell *head = NULL, *prev = NULL;
	wxString cellLabel( label );
	LcColIter iter( m_cols.begin() );

	for( ; iter != m_cols.end(); ++iter )
	{
		VdkLcColumn* col = *iter;
		VdkLcCell* cell = new VdkLcCell( col, cellLabel, NULL );

		// �������
		if( index == -1 )
		{
			col->cells.push_back( cell );
		}
		else
		{
			insert( col->cells, index, cell );
		}

		if( prev )
		{
			prev->SetClientData( cell );
		}
		else // prev Ϊ�գ���������һ���е�һ����Ԫ��
		{
			head = cell;
		}
		
		cellLabel.clear();
		prev = cell;
	}

	if( updateSize )
	{
		UpdateSize( NULL );

		// ����֪ͨ�¼�
		if( IsReadyForEvent() )
		{
			wxListEvent lstEvent( wxEVT_COMMAND_LIST_INSERT_ITEM, m_id );
			lstEvent.m_itemIndex = index;

			wxPostEvent( m_WindowImpl, lstEvent );
		}
	}

	return head;
}

void VdkListCtrl::RemoveRow(int index, bool updateSize, wxDC* pDC)
{
	// ���ܴ��������ѡ������
	int lastSel = std::distance( at( m_cols, 0 )->cells.begin(), m_lastSelected );
	if( lastSel == index )
	{
		UpdateLastSelected( GetColumnEnd( 0 ) );
	}

	// ���ܴ�������ѡ�б���
	if( !m_selItems.IsEmpty() )
	{
		int selIndex = m_selItems.Index( index );
		if( selIndex != wxNOT_FOUND )
		{
			m_selItems.RemoveAt( selIndex );
		}

		// ��������֮�����ѡ����
		int numSel = m_selItems.size();
		for( int i = 0; i < numSel; i++ )
		{
			if( m_selItems[i] > index )
				m_selItems[i]--;
		}
	}

	if( m_lastHilighted == index )
	{
		m_lastHilighted = wxNOT_FOUND;
	}

	if( m_selStart == index )
	{
		m_selStart = wxNOT_FOUND;
	}

	wxASSERT( m_lastDraggTarget == wxNOT_FOUND );

	//==============================================

	LcColIter i( m_cols.begin() );
	LcColIter e( m_cols.end() );

	for( ; i != e; ++i )
	{
		LcCellIter cell( GetCellIterator( *i, index ) );
		delete *cell;

		(*i)->cells.erase( cell );
	}

	if( updateSize )
	{
		UpdateSize( pDC );

		// ����֪ͨ�¼�
		if( IsReadyForEvent() )
		{
			wxListEvent lstEvent( wxEVT_COMMAND_LIST_DELETE_ITEM, m_id );
			lstEvent.m_itemIndex = index;

			wxPostEvent( m_WindowImpl, lstEvent );
		}
	}
}

wxString VdkListCtrl::GetString(int i)
{
	return GetCellLabel( i, 0 );
}

wxString VdkListCtrl::GetCellLabel(int row, int col)
{
	return at( at( m_cols, col )->cells, row )->GetLabel();
}

void VdkListCtrl::SetCellLabel(int row, int col, const wxString& label)
{
	at( at( m_cols, col )->cells, row )->SetLabel( label );
}

void VdkListCtrl::SetCellLabel(LcCellIter cell, const wxString& label)
{
	(*cell)->SetLabel( label );
}

void VdkListCtrl::SetCellClientData(int row, int col, void* clientData)
{
	at( at( m_cols, col )->cells, row )->SetClientData( clientData );
}

void* VdkListCtrl::GetCellClientData(int row, int col)
{
	return at( at( m_cols, col )->cells, row )->GetClientData();
}

bool VdkListCtrl::IsEmpty() const
{
	return m_cols.empty() || at( m_cols, 0 )->cells.empty();
}

int VdkListCtrl::GetRowCount() const
{
	if( m_cols.empty() )
		return 0;

	return at( m_cols, 0 )->cells.size();
}

int VdkListCtrl::GetColumnCount() const
{
	return m_cols.IsOk() ? m_cols.size() : 0;
}

void VdkListCtrl::CalcShownItems()
{
	double shownItems( double( m_Rect.height ) / m_rowHeight );
	m_shownItems = shownItems;
	if( shownItems > m_shownItems )
		m_shownItems++;

	m_shownItems += m_nShownItemsAddIn;
}

int VdkListCtrl::TestWidthArragements(int percentage)
{
	int sum( 0 );

	LcColIter i( m_cols.begin() );
	for( ; i != m_cols.end(); ++i )
		sum += (*i)->percentage;

	if( sum + percentage > 100 )
		percentage = 100 - sum;

	return percentage;
}

void VdkListCtrl::Clear(wxDC* pDC)
{
	if( IsEmpty() )
		return;

	DoClear( pDC );

	//==============================================

	m_lastHilighted = wxNOT_FOUND;
	m_selItems.clear();
	UpdateLastSelected( GetColumnEnd( 0 ) );

	// TODO: Ӧ���� LcDataSet::RemoveAllRows()
	LcColIter i( m_cols.begin() );
	for( ; i != m_cols.end(); ++i )
	{
		WX_CLEAR_LIST( ListOfLcCell, (*i)->cells );
	}

	RemoveState( VLCST_SELECT_ALL );
	SetVirtualSize( 0, 0, NULL );
	SetViewStart( 0, 0, NULL );

	if( pDC )
	{
		// ���ﲻ������ Draw
		VdkUtil::ClrBkGnd( *pDC, m_crossBrush1, AbsoluteRect() );
	}
}

void VdkListCtrl::Select(int first, int last, wxDC* pDC)
{
	wxASSERT( first >= 0 && first < GetRowCount() );

	if( last == wxNOT_FOUND )
	{
		last = GetRowCount();
	}

	SelectNone( pDC );
	for( int i = first; i < last; i++ )
		m_selItems.Add( i );

	// ����ѡ�����
	m_selStart = first;
	UpdateLastSelected( GetCellIterator( 0, m_selStart ) );

	if( pDC )
	{
		int base; // ������ʾ�ĵ�һ��
		GetViewStart( NULL, &base );

		for( int i = wxMax( first, base ); i < last; i++ )
		{
			if( i >= base + m_shownItems )
				break;

			UpdateRow( i, *pDC );
		}
	}
}

void VdkListCtrl::SelectAll(wxDC* pDC)
{
	UpdateLastSelected( GetColumnEnd( 0 ) );
	m_selItems.Clear();

	if( TestState( VLCST_SELECT_ALL ) )
		return;

	SetAddinState( VLCST_SELECT_ALL );

	// ֱ���ػ������ؼ�������Ҫ���¹�����
	RefreshContent( pDC );
}

void VdkListCtrl::SelectNone(wxDC* pDC)
{
	m_selItems.Clear();
	UpdateLastSelected( GetColumnEnd( 0 ) );
	RemoveState( VLCST_SELECT_ALL );
	
	// ֱ���ػ������ؼ���һ��һ�еػ�̫���ˣ�����Ҫ���¹�����
	RefreshContent( pDC );
}

void VdkListCtrl::SelectReverse(wxDC* pDC)
{
	if( TestState( VLCST_SELECT_ALL ) )
		return SelectNone( pDC );

	if( m_selItems.IsEmpty() )
		return SelectAll( pDC );

	ArrayOfSortedInts newselect( CompareInts );
	unsigned size = GetRowCount(),
			 size0 = m_selItems.size(),
			 size1 = size - size0;

	newselect.Alloc( size1 );

    unsigned j = 0, k = 0;
	for( int i = 0; k < size1 ; i++, k++ )
	{
		if( (j < size0) && (i == m_selItems[j]) )
		{
			j++;
			continue;
		}

		newselect.Add( i );
	}

	m_selItems = newselect;

	//==============================================

	RefreshContent( pDC );
}

void VdkListCtrl::SetColumnTextColor(int index, const wxColour& color)
{
	wxASSERT( (index >= 0) && ((unsigned) index < m_cols.size()) );

	at( m_cols, index )->textColor = color;
}

VdkCusdrawReturnFlag VdkListCtrl::DoEraseRow
	(const LcCellIter& it, int index, wxDC& dc)
{
	return VCCDRF_DODEFAULT;
}

VdkCusdrawReturnFlag VdkListCtrl::DoDrawCellText(const VdkLcCell* cell,
	int col_index, int index, wxDC& dc, VdkLcHilightState state)
{
	return VCCDRF_DODEFAULT;
}

LcCellIter VdkListCtrl::GetCellIterator(int col, int row)
{
	VdkLcColumn* column = at( m_cols, col );
	return GetCellIterator( column, row );
}

LcCellIter VdkListCtrl::GetCellIterator(VdkLcColumn* col, int row)
{
	wxASSERT( row >= 0 );

	LcCellIter i( col->cells.begin() );
	std::advance( i, row );

	return i;
}

LcCellIter VdkListCtrl::GetColumnEnd(int col) const
{
	return at( m_cols, col )->cells.end();
}

int VdkListCtrl::IndexOf(const VdkLcCell* cell) const
{
	int index;
	ListOfLcColumn::const_iterator i( m_cols.begin() );
	for( ; i != m_cols.end(); ++i )
	{
		index = indexOf( (*i)->cells, const_cast< VdkLcCell* >( cell ) );
		if( index != wxNOT_FOUND )
			return index;
	}

	return wxNOT_FOUND;
}

VdkLcCell* VdkListCtrl::GetCell(size_t row, size_t col) const
{
	return at( at( m_cols, col )->cells, row );
}

void VdkListCtrl::GetViewStart(int* x, int* y) const
{
	VdkScrolledWindow::GetViewStart( x, y );
	FixViewStart( x, y );
}

void VdkListCtrl::FixViewStart(int* x, int* y) const
{
	if( y )
	{
		int yStep;
		GetScrollRate( NULL, &yStep );
		*y /= ( m_rowHeight / yStep );
	}
}

void VdkListCtrl::GetMaxViewStart(int* x, int* y) const
{
	VdkScrolledWindow::GetMaxViewStart( x, y );
	FixViewStart( x, y );
}

void VdkListCtrl::OnNotify(const VdkNotify& notice)
{
	switch( notice.GetNotifyCode() )
	{
	case VCN_FONT_CHANGED:
		{
			LcColIter i( m_cols.begin() );
			for( ; i != m_cols.end(); ++i )
			{
				(*i)->SetFont( m_Font );
			}

			break;
		}

	case VCN_WINDOW_RESIZED:
	case VCN_SIZE_CHANGED:
		{
			int dX = notice.GetWparam();
			int dY = notice.GetLparam();

			if( dX  )
			{
				VdkLcColumn* col = NULL;
				int sum = 0, d = 0;

				for( LcColIter i( m_cols.begin() ); i != m_cols.end(); i++ )
				{
					col = *i;

					col->x += sum;
					if( col->percentage )
					{
						d = double( dX ) * (col->percentage / 100);
						sum += d;

						col->UpdateWidth( col->width + d );
						// 100% ʱ��Ҫ�˳�ѭ��������
					}
				} // END for

				// һЩʣ��Ŀ�ȣ������ظ����һ��
				if( col && (sum != dX) )
					col->UpdateWidth( col->width + dX - sum );
			}

			if( dY  )
			{
				CalcShownItems();
			}

			break;
		}

	default:

		break;
	}

}

void VdkListCtrl::RefreshContent(wxDC* pDC)
{
	if( pDC )
	{
		PrepareDC( *pDC );
		OnDraw( *pDC );
	}
}

void VdkListCtrl::Sort(int col, CellComparer comp)
{
	at( m_cols, col )->cells.sort( comp );
}

void VdkListCtrl::OnCtrlPlusHomeEnd(VdkKeyEvent& e)
{
	wxASSERT( !IsEmpty() );

	if( e.GetKeyCode() == WXK_HOME )
	{
		GoTo( 0, NULL );
		Select( 0, &e.dc );
	}
	else
	{
		GoTo( 1, NULL );

		int last = GetRowCount() - 1;
		Select( last, &e.dc );
	}
}

void VdkListCtrl::UpdateLastSelected(const LcCellIter& iter)
{
	m_lastSelected = iter;
}

LcCellIter VdkListCtrl::GetLastSel() const
{
	return m_lastSelected;
}

int VdkListCtrl::GetLastSelIndex() const
{
	if( m_lastSelected == GetColumnEnd( 0 ) )
		return wxNOT_FOUND;

	return std::distance( (*(m_cols.begin()))->cells.begin(), m_lastSelected );
}

bool VdkListCtrl::IsAllSel() const
{
	return TestState( VLCST_SELECT_ALL ) ||
		( !m_selItems.empty() &&
		  (m_selItems.size() == GetItemCount()) );
}

bool VdkListCtrl::HasSelected() const
{
	return !m_selItems.empty() || TestState( VLCST_SELECT_ALL );
}

//////////////////////////////////////////////////////////////////////////

VdkLcCell::VdkLcCell(VdkLcColumn* col,
					 const wxString& label,
					 void* clientData)
		 : m_col( col ), m_label( label ), m_xFix( 0 ),
		   m_drawingVersion( 0 )
{
	SetClientData( clientData );

	if( !m_label.empty() )
	{
		DecreaseDrawingVersion();
	}
}

void VdkLcCell::DecreaseDrawingVersion()
{
	m_drawingVersion = m_col->GetDrawingVersion() - 1;
}

void VdkLcCell::DrawLabel(wxDC& dc, int x, int y) const
{
	SyncDrawingVersion();

	x += m_xFix;
	y += (m_col->height - dc.GetCharHeight()) / 2;

	dc.DrawText( m_labelOverflow, x, y );
}

void VdkLcCell::SetLabel(const wxString& label)
{
	if( m_label != label )
	{
		m_label.assign( label );

		// ���¼���������Լ�
		DecreaseDrawingVersion();
	}
}

void VdkLcCell::CalcTextPos()
{
	m_labelOverflow.assign( m_label );

	wxMemoryDC mdc;
	mdc.SetFont( m_col->font );

	int w;
	mdc.GetTextExtent( m_labelOverflow, &w, NULL );

	if( w > m_col->width )
	{
		wxArrayInt widths;
		mdc.GetPartialTextExtents( m_labelOverflow, widths );

        wxString dots( L"..." );

        int dotsWidth;
        mdc.GetTextExtent( dots, &dotsWidth, NULL );
        
        //============================================

		int suitable = m_col->width - dotsWidth;
        int i = 0;
        
		while( widths[i] <= suitable )
		    i++;
		    
		// TODO: ����Ҫ��Ҫ ��i--;����
		// wxDC::GetPartialTextExtents():
		// ...from the beginning of text to the corresponding character of text

		m_labelOverflow = m_label.Mid( 0, i );
		m_labelOverflow += dots;

		m_xFix = 0;
	}
	else if( m_col->textAlign & TEXT_ALIGN_CENTER_X_Y )
	{
		m_xFix = (m_col->width - w) / 2;
	}
	else if( m_col->textAlign & TEXT_ALIGN_RIGHT )
	{
		m_xFix = m_col->width - w;
	}
}

int VdkLcCell::GetX_Padding() const
{
	SyncDrawingVersion();

	return m_xFix;
}

wxString VdkLcCell::GetLabel(int* xPadding) const
{
	if( xPadding )
		*xPadding = GetX_Padding();

	return m_label;
}

void VdkLcCell::SyncDrawingVersion() const
{
	int colDrawingVersion = m_col->GetDrawingVersion();
	if( m_drawingVersion != colDrawingVersion )
	{
		const_cast< VdkLcCell* >( this )->CalcTextPos();

		m_drawingVersion = colDrawingVersion;
	}
}

//////////////////////////////////////////////////////////////////////////

VdkListCtrl::VdkLcHeader::VdkLcHeader(VdkWindow* win,
									  const wxString& strName,
									  const wxRect& rc,
									  VdkListCtrl* listctrl,
									  ColumnListAdapter& cols)
		   : m_listctrl( listctrl ), m_cols( cols )
{
	m_strName = strName;
	m_Rect = rc;
	SetVdkWindow( win );

	m_yFix = wxNOT_FOUND;

	//==============================================

	m_bmBkGnd = wxBitmap( LCHEADER_xpm );
	if( m_bmBkGnd.GetWidth() < m_Rect.width )
	{
		int qurd( m_bmBkGnd.GetWidth() / 4 );
		wxRect rcTile( 0, 0, qurd * 2, m_bmBkGnd.GetHeight() );
		m_bmBkGnd = TileBitmap
			( m_bmBkGnd, m_Rect.width, rcTile.height, rcTile );
	}
}

void VdkListCtrl::VdkLcHeader::DoDraw(wxDC& dc)
{
	if( m_listctrl->TestStyle( VCS_BORDER_SIMPLE ) )
	{
		wxPen borderPen( m_listctrl->GetBorderColour() );
		dc.SetPen( borderPen );

		wxRect rc( m_Rect );
		rc.Inflate( 1 );

		// ���
		dc.DrawLine
			( rc.x, rc.y, rc.x, rc.y + rc.height );
		// �ϡ�
		dc.DrawLine
			( rc.x, rc.y, rc.GetRight(), rc.y );
		// �ҡ�
		dc.DrawLine
			( rc.GetRight(), rc.y, rc.GetRight(), rc.y + rc.height );
	}

	//==============================================

	dc.DrawBitmap( m_bmBkGnd, m_Rect.x, m_Rect.y );

	if( m_yFix == wxNOT_FOUND )
	{
		m_yFix = ( m_Rect.height - dc.GetCharHeight() ) / 2;
	}

	LcColIter i( m_cols.begin() ), e( m_cols.end() );
	for( ; i != e; ++i )
	{
		VdkLcColumn* col = *i;
		int xFix = 5;

		if( col->textAlign & TEXT_ALIGN_CENTER_X_Y )
		{
			int w;
			dc.GetTextExtent( col->heading, &w, NULL );
			xFix = ( col->width - w ) / 2;
		}

		dc.SetFont( m_Font );
		dc.SetTextForeground( *wxBLACK );
		dc.DrawText( col->heading, 
					 m_Rect.x + xFix + col->x, 
					 m_Rect.y + m_yFix );

		dc.SetPen( *wxLIGHT_GREY_PEN );
		dc.DrawLine( m_Rect.x + col->x + col->width, 
					 m_Rect.y + 2,
					 m_Rect.x + col->x + col->width, 
					 m_Rect.y + CNST_HEADER_HEIGHT - 2 );
	}
}

//////////////////////////////////////////////////////////////////////////

VdkLcColumn::VdkLcColumn()
	: percentage( 0 ),
	  leftPadding( 0 ),
	  textAlign( TEXT_ALIGN_CENTER_X_Y ),
	  x( 0 ),
	  width( 0 ),
	  height( 0 ),
	  m_drawingVersion( 0 )
{

}

VdkLcColumn::VdkLcColumn(const VdkLcColumn& rhs)
{
	Clone( rhs );
}

VdkLcColumn& VdkLcColumn::operator = (const VdkLcColumn& rhs)
{
	Clone( rhs );
	return *this;
}

void VdkLcColumn::Clone(const VdkLcColumn &rhs)
{
	percentage = rhs.percentage;
	leftPadding = rhs.leftPadding;
	textAlign = rhs.textAlign;
	textColor = rhs.textColor;
	heading = rhs.heading;
	x = rhs.x;
	width = rhs.width;
	height = rhs.height;
}

void VdkLcColumn::SetFont(const wxFont& f)
{
	font = f;

	// ���¼���������Լ�
	IncreaseDrawingVersion();
}

void VdkLcColumn::UpdateWidth(size_t w)
{
    if( width != w )
    {
	    width = w;

		// ��Ԫ���ı���Ҫ���¼��������ʼλ�ú����
	    IncreaseDrawingVersion();
    }
}

//////////////////////////////////////////////////////////////////////////

VdkLcColumnInitializer::VdkLcColumnInitializer(VdkListCtrl* lst)
	: Index( wxNOT_FOUND ),
	  Percentage( 0 ),
	  LeftPadding( 0 ),
	  Width( 0 ),
	  TextAlign( TEXT_ALIGN_LEFT ),
	  Font( lst->GetFont() )
{

}

//////////////////////////////////////////////////////////////////////////

VdkListCtrl::ColumnListAdapter::ColumnListAdapter(LcDataSet* dataSet)
	: m_dataSet( dataSet )
{

}

void VdkListCtrl::ColumnListAdapter::Attach(LcDataSet* dataSet)
{
	m_dataSet = dataSet;
}

LcDataSet* VdkListCtrl::ColumnListAdapter::Datach()
{
	LcDataSet* ret = m_dataSet;
	m_dataSet = NULL;

	return ret;
}

bool VdkListCtrl::ColumnListAdapter::IsOk() const
{
	return (m_dataSet != NULL);
}

LcColIter VdkListCtrl::ColumnListAdapter::begin()
{
	return m_dataSet->cols.begin();
}

ListOfLcColumn::const_iterator VdkListCtrl::ColumnListAdapter::begin() const
{
	return m_dataSet->cols.begin();
}

LcColIter VdkListCtrl::ColumnListAdapter::end()
{
	return m_dataSet->cols.end();
}

ListOfLcColumn::const_iterator VdkListCtrl::ColumnListAdapter::end() const
{
	return m_dataSet->cols.end();
}

bool VdkListCtrl::ColumnListAdapter::empty() const
{
	return !m_dataSet || m_dataSet->cols.empty();
}

ListOfLcColumn::size_type VdkListCtrl::ColumnListAdapter::size() const
{
	return m_dataSet ? m_dataSet->cols.size() : 0;
}

LcColIter VdkListCtrl::ColumnListAdapter::insert(LcColIter it, VdkLcColumn* col)
{
	return m_dataSet->cols.insert( it, col );
}

void VdkListCtrl::ColumnListAdapter::push_back(VdkLcColumn* col)
{
	m_dataSet->cols.push_back( col );
}

//////////////////////////////////////////////////////////////////////////

LcDataSet::~LcDataSet()
{
	RemoveAllColumns();
}

void LcDataSet::RemoveAllColumns()
{
	RemoveAllRows();

	LcColIter i;
	for( i = cols.begin(); i != cols.end(); ++i )
		delete *i;
}

void LcDataSet::RemoveAllRows()
{
	LcColIter i;
	for( i = cols.begin(); i != cols.end(); ++i )
	{
		ListOfLcCell& cells( (*i)->cells );
		WX_CLEAR_LIST( ListOfLcCell, cells );
	}
}
