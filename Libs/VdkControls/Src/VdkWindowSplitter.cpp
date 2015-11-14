#include "StdAfx.h"
#include "VdkWindowSplitter.h"
#include "VdkWindow.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


IMPLEMENT_DYNAMIC_VOBJECT( VdkWindowSplitter )

//////////////////////////////////////////////////////////////////////////

void VdkWindowSplitter::Init()
{
	m_win1 = m_win2 = NULL;
	m_bVertical = true;
	m_lastX = m_lastY = m_mousePos = wxNOT_FOUND;
	m_min = 100;

#if 0
	// 必须要去除这个属性，否则鼠标指针的处理会很不灵敏
	RemoveStyle( VCS_ONESHOT_HOVERING );
#endif
}

void VdkWindowSplitter::Create(VdkWindow* parent,
							   const wxString& strName,
							   const wxRect& rc,
							   VdkControl* win1,
							   VdkControl* win2,
							   bool bVertical)
{
	m_strName = strName;
	m_Rect = rc;
	SetVdkWindow( parent );

	//////////////////////////////////////////////////////////////////////////

	wxASSERT( win1 );
	wxASSERT( win2 );
	wxASSERT( win1->GetParent() == win2->GetParent() );

	m_win1 = win1;
	m_win2 = win2;
	m_bVertical = bVertical;
}

void VdkWindowSplitter::Create(wxXmlNode* node)
{
	DoXrcCreate( node );
}

void VdkWindowSplitter::DoHandleMouseEvent(VdkMouseEvent& e)
{
	wxASSERT( m_win1 );
	wxASSERT( m_win2 );

	switch( e.evtCode )
	{
	case NORMAL:
	case LEFT_UP:

		if( !GetAbsoluteRect().Contains( e.mousePos ) )
			m_Window->AssignCursor( wxNullCursor );

		m_lastX = m_lastY =	m_mousePos = wxNOT_FOUND;

		break;

	case HOVERING:
		{
			wxStockCursor cursor( wxCURSOR_SIZENS );
			if( !m_bVertical )
				cursor = wxCURSOR_SIZEWE;

			m_Window->AssignCursor( wxCursor( cursor ) );

			break;
		}

	case LEFT_DOWN:

		if( m_bVertical )
		{
			m_lastY = e.mousePos.y;
			m_mousePos = e.mousePos.y - GetAbsoluteRect().y;
		}

		break;

	case DRAGGING:
		{
			if( m_lastY == wxNOT_FOUND && m_mousePos == wxNOT_FOUND )
				break;

			if( m_bVertical )
			{
				// 下面的顺序绝对不能错，dY 在两个两个窗口的处理过程中必须相同！
				int dY( e.mousePos.y - m_lastY );
				wxRect rc1( m_win1->GetRect() );
				wxRect rc2( m_win2->GetRect() );

				// 向上拖动
				if( dY < 0 && rc1.height == m_min )
					return;
				// 向下拖动
				if( dY > 0 && rc2.height == m_min )
					return;

				// 向上拖动使得上面的控件高度小于最小值
				if( rc1.height + dY < m_min )
					dY = m_min - rc1.height;

				// 向下拖动使得下面的控件高度小于最小值
				if( rc2.height - dY < m_min )
					dY = rc2.height - m_min;

				rc1.height += dY;
				m_win1->SetRect( rc1, &e.dc );

				rc2.y += dY;
				rc2.height -= dY;
				m_win2->SetRect( rc2, &e.dc );

				// 修改自己的作用域，当且仅当父控件不是
				// 当前正在拉锯的两个控件之一
				if( m_parent != m_win1 && m_parent != m_win2 )
					m_Rect.y += dY;

				// 保存当前鼠标位置
				m_lastY = e.mousePos.y;

				// 重画控件
				Draw( e.dc );
			}

			break;
		}

	default:
		break;
	}
}

void VdkWindowSplitter::DoDraw(wxDC& dc)
{
	dc.SetBrush( *wxBLACK_BRUSH );
	dc.DrawRectangle( m_Rect );
}
