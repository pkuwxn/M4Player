/***************************************************************
 * Name:      StickyWindows.cpp
 * Purpose:   WinAMP ���ճ������ʵ��
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-03-07
 * Copyright: Ning
 **************************************************************/
#include "StdAfx.h"
#include "StickyWindows.h"

#include "VdkWindow.h"
#include "VdkDefs.h" // for vdkSOUTH, vdkALIGN_TOP, etc.

#include "wxUtil.h" // for RightOf()��BottomOf()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT( wxEVT_STICKY_WINDOW_ATTACHED, wxCommandEvent );
wxDEFINE_EVENT( wxEVT_STICKY_WINDOW_DETACHED, wxCommandEvent );

StickyWindows::StickyWindows(int criticalDistance)
	: m_criticalDistance( criticalDistance )
{

}

void StickyWindows::AddWindow(VdkWindow* win)
{
#ifdef __WXGTK__
    VdkWindowList::iterator i;
    for( i = m_windows.begin(); i != m_windows.end(); ++i )
    {
        if( *i == win )
            return;
    }
#else
	if( m_windows.Member( win ) )
		return;
#endif // __WXGTK__

	win->PushEventFilter( this );
	m_windows.push_back( win );
}

void StickyWindows::SetMainWindow(VdkWindow* win)
{
	wxASSERT( m_main == m_windows.end() );

	for( m_main = m_windows.begin();
		 m_main != m_windows.end();
		 ++m_main )
	{
		if( *m_main == win )
		{
			// ������������¼����ǲ����٣������޷����ʼ�ƶ�������֮ǰ��
			// ��֪����Щ�Ӵ���ճ����������Ϊ VDK �ṩ��ģ���ƶ�״̬�ڿ�ʼ
			// ��һ���ƶ�֮��Ż���Ч
			win->GetWindowHandle()->Bind( wxEVT_LEFT_DOWN,
										  &StickyWindows::OnMainLeftDown,
										  this );

			win->GetWindowHandle()->Bind( wxEVT_MOVE,
										  &StickyWindows::LinklyMove,
										  this );

			break;
		}
	}
}

void StickyWindows::RemoveWindow(VdkWindow* win)
{
	if( m_main != m_windows.end() && *m_main == win )
	{
		win->GetWindowHandle()->Unbind( wxEVT_LEFT_DOWN,
									    &StickyWindows::OnMainLeftDown,
									    this );

		win->GetWindowHandle()->Unbind( wxEVT_MOVE,
									    &StickyWindows::LinklyMove,
									    this );

		m_main = m_windows.end();
	}

	win->PopEventFilter( this, false );

	RemoveChild( win );
	m_windows.DeleteObject( win );
}

bool StickyWindows::FilterEvent(const EventForFiltering& e)
{
	switch( e.evtCode() )
	{
	case DRAGGING:
		{
			VdkWindow* win = e.window();
			wxPoint mousePos( e.mouseEvent().GetPosition() );

			// �������ƶ��¼��������� wxWidgets ԭ�������ṩ
			if( win != *m_main )
			{
				if( win->TestState( VWST_DRAG_AND_MOVING ) )
				{
				    wxASSERT( !win->TestState( VWST_DRAG_AND_RESIZING ) );
				    
					return HandleChildMove( win, mousePos );
				}
			}

			if( win->TestState( VWST_DRAG_AND_RESIZING ) )
			{
			    wxASSERT( !win->TestState( VWST_DRAG_AND_MOVING ) );
			    
				return HandleResize( win, mousePos );
			}

			break;
		}

	case LEFT_UP:

		m_stickyToMain.clear();

		break;

	default:

		break;
	}

	return false;
}

// rc1 �� rc2 �Ƿ���Ȼ���һ����������һ��
bool IsContains(const wxRect& rc1, const wxRect& rc2)
{
	return (rc1 == rc2) || rc1.Contains( rc2 ) || rc2.Contains( rc1 );
}

// ���@a moving �Ƿ����ճ����@a still ��ĳ��������
// @param alignment ����Ϊ�գ����Ի�ȡ���߽߱������Ϣ
unsigned FindDirection(const wxRect& moving, const wxRect& still,
					   unsigned* alignment,
					   int criticalDistance)
{
	wxASSERT( criticalDistance >= 0 );

	// ���ע�⣺���ܶԸ������� |=
	unsigned direction = vdkDIRECTION_INVALID;

	// �������
	if( abs( RightOf( moving ) - still.x ) < criticalDistance )
	{
		direction |= vdkWEST;
	}

	// �����Ҳ�
	if( abs( moving.x - RightOf( still ) ) < criticalDistance )
	{
		direction |= vdkEAST;
	}

	// �����ϲ�
	if( abs( BottomOf( moving ) - still.y ) < criticalDistance )
	{
		direction |= vdkNORTH;
	}

	// �����²�
	if( abs( moving.y - BottomOf( still ) ) < criticalDistance )
	{
		direction |= vdkSOUTH;
	}

	if( alignment ) // �߽����
	{
		// ��ʼ��
		*alignment = vdkALIGN_INVALID;

		if( abs( moving.x - still.x ) < criticalDistance )
		{
			*alignment |= vdkALIGN_LEFT;
		}

		if( abs( RightOf( moving ) - RightOf( still ) ) < criticalDistance )
		{
			*alignment |= vdkALIGN_RIGHT;
		}

		if( abs( moving.y - still.y ) < criticalDistance )
		{
			*alignment |= vdkALIGN_TOP;
		}

		if( abs( BottomOf( moving ) - BottomOf( still ) ) < criticalDistance )
		{
			*alignment |= vdkALIGN_BOTTOM;
		}
	}

	return direction;
}

bool StickyWindows::HandleResize(VdkWindow* win, const wxPoint& mousePosClient)
{
	VdkWindow::MousePtrPos mousePtrPosOnEdge( win->GetDragAndResizeType() );
	if( mousePtrPosOnEdge == 0 )
		return false;

	wxWindow* winMoving = win->GetWindowHandle();
	wxPoint mousePosScreen( winMoving->ClientToScreen( mousePosClient ) );
	wxRect rcMovingUnamended( winMoving->GetScreenRect() );
	// �Ѹ������ָ����������Ĵ���������
	wxRect rcMoving( rcMovingUnamended );

	// �������ָ���� VdkWindow ��Ե��λ������ rcMoving ��ֵ��ʹ֮��ָ��ͬ��
	if( mousePtrPosOnEdge & vdkWEST )
	{
		rcMoving.x = mousePosScreen.x;
		rcMoving.width = RightOf( rcMovingUnamended ) - mousePosScreen.x;
	}

	if( mousePtrPosOnEdge & vdkEAST )
	{
		rcMoving.width = mousePosScreen.x - rcMovingUnamended.x;
	}

	if( mousePtrPosOnEdge & vdkNORTH )
	{
		rcMoving.y = mousePosScreen.y;
		rcMoving.height = BottomOf( rcMovingUnamended ) - mousePosScreen.y;
	}

	if( mousePtrPosOnEdge & vdkSOUTH )
	{
		rcMoving.height = mousePosScreen.y - rcMovingUnamended.y;
	}

	int minWidth, minHeight;
	win->GetMinSize( &minWidth, &minHeight );

	if( rcMoving.width < minWidth || rcMoving.height < minHeight )
		return false;

	//-------------------------------------------------

	// �����µĴ�С
	wxRect rcNew( rcMoving );

	VdkWindowIter i( m_windows.begin() );
	for( ; i != m_windows.end(); ++i )
	{
		wxWindow* winStill = (*i)->GetWindowHandle();
		if( winMoving == winStill || !winStill->IsShown() )
		{
			continue;
		}

		wxRect rcStill( winStill->GetScreenRect() );

		// �����ǰ�����ϵ���˳�����ѭ���������ƻ�״̬
		if( IsContains( rcMoving, rcStill ) )
			continue;

		unsigned alignment = vdkALIGN_INVALID;
		unsigned direction =
			FindDirection( rcMoving, rcStill, &alignment, m_criticalDistance );

		if( direction != vdkDIRECTION_INVALID || alignment != vdkALIGN_INVALID )
		{
			if( mousePtrPosOnEdge & vdkWEST )
			{
				wxCoord x = rcNew.x;

				if( direction & vdkEAST  )
				{
					rcNew.x = RightOf( rcStill );
				}
				else if( alignment & vdkALIGN_LEFT )
				{
					rcNew.x = rcStill.x;
				}

				// ���뱣�ִ����ұ�Ե����
				if( x != rcNew.x )
				{
					rcNew.width = RightOf( rcMoving ) - rcNew.x;
				}
			}

			if( mousePtrPosOnEdge & vdkEAST )
			{
				if( direction & vdkWEST )
				{
					rcNew.width = rcStill.x - rcMoving.x;
				}
				else if( alignment & vdkALIGN_RIGHT )
				{
					rcNew.width = RightOf( rcStill ) - rcMoving.x;
				}

				// ����û�и߶�/��ȸı����ж��ˣ���Ϊ���Ǿ���Ҫ�ı�����
			}

			if( mousePtrPosOnEdge & vdkNORTH )
			{
				wxCoord y = rcNew.y;

				if( direction & vdkSOUTH )
				{
					rcNew.y = BottomOf( rcStill );
				}
				else if( alignment & vdkALIGN_TOP )
				{
					rcNew.y = rcStill.y;
				}

				// ���뱣�ִ����±�Ե����
				if( y != rcNew.y )
				{
					rcNew.height = BottomOf( rcMoving ) - rcNew.y;
				}
			}

			if( mousePtrPosOnEdge & vdkSOUTH )
			{
				if( direction & vdkNORTH )
				{
					rcNew.height = rcStill.y - rcMoving.y;
				}
				else if( alignment & vdkALIGN_BOTTOM )
				{
					rcNew.height = BottomOf( rcStill ) - rcMoving.y;
				}
			}

		} // direction != vdkDIRECTION_INVALID

	}

	if( rcNew != rcMoving )
	{
		win->Resize( rcNew );
		// ����û��Ҫ���� UpdateStickyChildren() ����ճ���Ӵ��ڣ�
		// ��Ϊÿ��������������ʱ������и���

		return true;
	}

	return false;
}

// �����Ƿ����ճ������������
bool StickyWindows::HandleChildMove(VdkWindow* win, const wxPoint& mousePosClient)
{
	wxWindow* winMoving = win->GetWindowHandle();

	wxPoint mousePosScreen( winMoving->ClientToScreen( mousePosClient ) );
	// �϶�����ʱ�������ڸ���ָ��һ���ƶ�����ôָ��Ӧ�ÿ���������
	// һֱ�ڴ����ϵ�ͬһλ��
	wxPoint mousePosClientFixed( win->GetMouseOnForm() );

	// �϶����µ�λ�ã����ܻ�ճ�����µĴ��ڶ���������
	wxRect rcNew;
	winMoving->GetSize( &rcNew.width, &rcNew.height );
	rcNew.x = mousePosScreen.x - mousePosClientFixed.x;
	rcNew.y = mousePosScreen.y - mousePosClientFixed.y;

	//=======================================================
	// �ƶ��Ӵ���

	// ����ճ�����µ�λ��
	wxRect rcSticked( rcNew );

	VdkWindowIter i( m_windows.begin() );
	for( ; i != m_windows.end(); ++i )
	{
		wxWindow* winStill = (*i)->GetWindowHandle();
		if( winMoving == winStill || !winStill->IsShown() )
		{
			continue;
		}

		wxRect rcStill( winStill->GetScreenRect() );
		// ����ճ������ֹһ������
		TryStickToAgent( rcSticked, rcStill );

	} // ends fori

	if( rcSticked != rcNew )
	{
		winMoving->Move( rcSticked.x, rcSticked.y );

		wxRect rcMain( (*m_main)->GetWindowHandle()->GetScreenRect() );
		UpdateStickyToMainState( rcMain, win );

		return true;
	}

	return false;
}

// TODO: Member() ?
StickyWindows::InfoIter StickyWindows::FindStickyChild(VdkWindow* chd)
{
	InfoIter i;
	for( i = m_stickyToMain.begin(); i != m_stickyToMain.end(); ++i )
    {
        if( i->win == chd )
            return i;
    }

    return i;
}

void PostDetachEvent(wxWindow* win)
{
	wxWindowID windowId = win->GetId();
	wxCommandEvent e( wxEVT_STICKY_WINDOW_DETACHED, windowId );

	wxPostEvent( win, e );
}

void StickyWindows::UpdateStickyChildren(const wxRect& rcMain)
{
	// ����������յ�ǰ����ճ���Ӵ��ڣ������ƶ�һ���Ӵ��ڣ�ʹ֮��������
	// ������κ�ͨ�������ճ���������ڵ��Ӵ�����Ȼ�ᱣ���������ڵ�ճ��
	// ״̬�����ǲ���ȷ�ġ���ô�˺��ƶ������ڣ�ʹ�øոմ��������������
	// �����ٴ�ճ������ô��������ճ���������ڣ��ⲻ�٣������ڽ������ĵ���
	// ���ҹ��̻᲻��ȷ��ճ����ճ��״̬��ʧЧ����Щ�Ӵ�������ȥ��

	InfoIter_Const it( m_stickyToMain.begin() );
	for( ; it != m_stickyToMain.end(); ++it )
	{
		PostDetachEvent( it->win->GetWindowHandle() );
	}

	m_stickyToMain.clear();

	//========================================================

	size_t numStickyWindows = 0;

	do
	{
		numStickyWindows = m_stickyToMain.size();

		// ֻ������δճ�����Ӵ��ڣ���Ϊ������ʼʱ�����Ӵ��ڶ�����δճ��
		// ��״̬���������ճ��ʧЧ��Ҫ����(ʹ֮����)�����
		VdkWindowIter i( m_windows.begin() );
		for( ; i != m_windows.end(); ++i )
		{
			wxWindow* winstill = (*i)->GetWindowHandle();

			if( !winstill->IsShown() || *m_main == *i ||
				IsStickyToMain( *i ) )
			{
				continue;
			}

			UpdateStickyToMainState( rcMain, *i );
		}
	}
	while( numStickyWindows != m_stickyToMain.size() );
}

void StickyWindows::UpdateStickyToMainState(const wxRect& rcMain, VdkWindow* win)
{
	wxWindow* winNative = win->GetWindowHandle();
	InfoIter This( FindStickyChild( win ) );

	// �����������ڵ�ճ������
	if( RecalcLinkageToMain( rcMain, win ) )
	{
		wxCoord x, y, offsetX, offsetY;
		winNative->GetPosition( &x, &y );

		offsetX = x - rcMain.x;
		offsetY = y - rcMain.y;

		if( This == m_stickyToMain.end() )
		{
			StickyInfo info = { win, offsetX, offsetY };
			m_stickyToMain.push_back( info );

			wxWindowID windowId = winNative->GetId();
			wxCommandEvent e( wxEVT_STICKY_WINDOW_ATTACHED, windowId );
			wxPostEvent( winNative, e );
		}
		else
		{
			This->offsetX = offsetX;
			This->offsetY = offsetY;
		}
	}
	else
	{
		if( This != m_stickyToMain.end() )
		{
			m_stickyToMain.erase( This );
			PostDetachEvent( winNative );
		}
	}
}

bool StickyWindows::IsStickyToMain(VdkWindow* chd)
{
	wxASSERT( chd != *m_main );

	return FindStickyChild( chd ) != m_stickyToMain.end();
}

bool StickyWindows::TryStickToAgent(wxRect& rcThis, const wxRect& rcAgent)
{
	unsigned align;
	unsigned d = FindDirection( rcThis, rcAgent, &align, m_criticalDistance );

	if( d & vdkEAST )
	{
		rcThis.x = RightOf( rcAgent );
	}
	else if( d & vdkWEST )
	{
		rcThis.x = rcAgent.x - rcThis.width;
	}

	if( d & vdkNORTH )
	{
		rcThis.y = rcAgent.y - rcThis.height;
	}
	else if( d & vdkSOUTH )
	{
		rcThis.y = BottomOf( rcAgent );
	}

	//---------------------------------------------
	// �߽����

	if( align & vdkALIGN_TOP )
	{
		rcThis.y = rcAgent.y;
	}
	else if( align & vdkALIGN_BOTTOM )
	{
		rcThis.y = BottomOf( rcAgent ) - rcThis.height;
	}

	if( align & vdkALIGN_LEFT )
	{
		rcThis.x = rcAgent.x;
	}
	else if( align & vdkALIGN_RIGHT )
	{
		rcThis.x = RightOf( rcAgent ) - rcThis.width;
	}

	return (d != vdkDIRECTION_INVALID) || (align != vdkALIGN_INVALID);
}

bool StickyWindows::RecalcLinkageToMain(const wxRect& rcMain, VdkWindow* chd)
{
	wxASSERT( chd != *m_main );

	wxRect rcChild( chd->GetWindowHandle()->GetScreenRect() );
	wxRect rcNew( rcChild );

	bool sticky = ReallySticky( rcNew, rcMain ) &&
				  TryStickToAgent( rcNew, rcMain );

	if( !sticky )
	{
		// ���������Ѿ�ճ���������ڵ��Ӵ���
		VdkWindowIter i( m_windows.begin() );
		for( ; i != m_windows.end(); ++i )
		{
			if( i == m_main || *i == chd ||
				!(*i)->GetWindowHandle()->IsShown() )
			{
				continue;
			}

			wxRect rcAgent( (*i)->GetWindowHandle()->GetScreenRect() );
			// ��һ���Ѿ�ճ���������ڵĴ��������ճ����������
			if( IsStickyToMain( *i ) || ReallySticky( rcAgent, rcMain ) )
			{
				sticky = ReallySticky( rcNew, rcAgent ) &&
						 TryStickToAgent( rcNew, rcAgent );

				if( sticky )
					break;
			}
		} // END for i
	}

	if( rcNew != rcChild )
	{
		chd->GetWindowHandle()->Move( rcNew.x, rcNew.y );
	}

	return sticky;
}

void StickyWindows::RemoveChild(VdkWindow* chd)
{
    InfoIter i = FindStickyChild( chd );
    if( i != m_stickyToMain.end() )
        m_stickyToMain.erase( i );
}

bool StickyWindows::ReallySticky(const wxRect& rc1, const wxRect& rc2)
{
	// ���߱��������Ӵ�
	wxRect rcInflated( rc1 );
	return rcInflated.Inflate( m_criticalDistance ).Intersects( rc2 );
}

void StickyWindows::OnMainLeftDown(wxMouseEvent& e)
{
	// ����ճ���Ӵ���
	UpdateStickyChildren( (*m_main)->GetWindowHandle()->GetScreenRect() );

	e.Skip( true );
}

void StickyWindows::LinklyMove(wxMoveEvent& e)
{
	wxASSERT( m_main != m_windows.end() );

	//------------------------------------------------

	wxRect rcMain( (*m_main)->GetWindowHandle()->GetScreenRect() );

	if( !m_stickyToMain.empty() )
	{
		wxWindow* chd = NULL;
		wxCoord newX, newY;

		InfoIter i( m_stickyToMain.begin() );
		for( ; i != m_stickyToMain.end(); ++i )
		{
			chd = i->win->GetWindowHandle();
			if( !chd->IsShown() )
				continue;

			newX = rcMain.x + i->offsetX;
			newY = rcMain.y + i->offsetY;

			chd->Move( newX, newY );
		}
	}

	// �ƶ�������ʱ�����ܻ���δ𤸽�������ڵ��Ӵ��ڽӽ������ڣ�
	// �����������Ҫ��ʱ����ճ���б�
	UpdateStickyChildren( rcMain );

	e.Skip( true );
}
