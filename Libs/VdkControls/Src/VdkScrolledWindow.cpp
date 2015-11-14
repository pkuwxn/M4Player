///////////////////////////////////////////////////////////////////////////////
// Name:        VdkListCtrl.cpp
// Purpose:     Implementation of VdkListCtrl
// Author:      Wang Xiaoning
// Created:     2010
///////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "VdkScrolledWindow.h"
#include "VdkDC.h"
#include "VdkWindow.h"
#include "VdkScrollBar.h"
#include "VdkCtrlParserInfo.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkScrolled::VdkScrolled()
		   : m_xStep( 20 ),
		     m_yStep( 20 ),
			 m_xVirtualSize( 0 ),
			 m_yVirtualSize( 0 ),
			 m_xViewStart( 0 ),
			 m_yViewStart( 0 ),
			 m_yViewStartMax( 0 ),
			 m_linesUpDown( 3 ),
			 m_pScrollBar( NULL )
{

}

void VdkScrolled::Clone(VdkScrolled* o, VdkScrollBar* scrollbar)
{
	m_xStep = o->m_xStep;
	m_yStep = o->m_yStep;
	m_xVirtualSize = 0;
	m_yVirtualSize = 0;
	m_xViewStart = 0;
	m_yViewStart = 0;
	m_yViewStartMax = 0;
	m_linesUpDown = o->m_linesUpDown;
	m_pScrollBar = scrollbar;
}

void VdkScrolled::PrepareDC(wxDC& dc)
{
	wxRect rc( AbsoluteRect() );

	dc.SetDeviceOrigin( 0, 0 );
	DoPrepareDC( dc );
	dc.SetDeviceOrigin( - m_xViewStart * m_xStep + rc.x,
						- m_yViewStart * m_yStep + rc.y );
}

void VdkScrolled::RestoreDC(wxDC& dc)
{
	DoRestoreDC( dc );
}

void VdkScrolled::Paint(wxDC& dc)
{
	wxASSERT( m_xStep && m_yStep );
	//////////////////////////////////////////////////////////////////////////

	VdkDcDeviceOriginSaver saver( dc );
	PrepareDC( dc );

	OnDraw( dc );

	RestoreDC( dc );
}

void VdkScrolled::SetViewStart(int x, int y, wxDC* pDC)
{
	if( x != m_xViewStart || y != m_yViewStart )
	{
		if( x < 0 )	x = 0;
		if( y > m_yViewStartMax )
			y = m_yViewStartMax;
		else if( y < 0 )
			y = 0;

		m_xViewStart = x;
		m_yViewStart = y;
	}

	RefreshState( pDC );
}

void VdkScrolled::GetViewStart(int* x, int* y) const
{
	if( x ) *x = m_xViewStart;
	if( y ) *y = m_yViewStart;
}

void VdkScrolled::GetViewStartCoord(int* x, int* y) const
{
	if( x ) *x = m_xViewStart * m_xStep;
	if( y ) *y = m_yViewStart * m_yStep;
}

void VdkScrolled::GetMaxViewStart(int* x, int* y) const
{
	if( x ) *x = 0;
	if( y ) *y = m_yViewStartMax;
}

void VdkScrolled::GetMaxViewStartCoord(int* x, int* y) const
{
	if( x ) *x = 0; // TODO:
	if( y ) *y = m_yViewStartMax * m_yStep;
}

void VdkScrolled::GetVirtualSize(int* w, int* h) const
{
	if( w ) *w = m_xVirtualSize;
	if( h ) *h = m_yVirtualSize;
}

void VdkScrolled::SetScrollRate(int xstep, int ystep)
{
	m_xStep = xstep;
	m_yStep = ystep;

	CalcMaxViewStart();
}

void VdkScrolled::GetScrollRate(int* xstep, int* ystep) const
{
	if( xstep ) *xstep = m_xStep;
	if( ystep ) *ystep = m_yStep;
}

void VdkScrolled::SetVirtualSize(int x, int y, wxDC* pDC)
{
	if( x % m_xStep == 0 )
		m_xVirtualSize = x;
	else
		m_xVirtualSize = (x / m_xStep + 1) * m_xStep;

	if( y % m_yStep == 0 )
		m_yVirtualSize = y;
	else
		m_yVirtualSize = (y / m_yStep + 1) * m_yStep;

	CalcMaxViewStart();
	CalcScrollBarSize( pDC );

	RefreshState( pDC );
}

void VdkScrolled::CalcMaxViewStart()
{
	wxRect& rc = RelativeRect();

	if( m_yVirtualSize < rc.height )
	{
		// 更新 m_yViewStart
		m_yViewStart = m_yViewStartMax = 0;
		return;
	}

	double yViewStartMax = (m_yVirtualSize - rc.height) / double( m_yStep );
	m_yViewStartMax = yViewStartMax;
	if( yViewStartMax > m_yViewStartMax )
		m_yViewStartMax++;

	// 更新 m_yViewStart
	if( m_yViewStart > m_yViewStartMax )
		m_yViewStart = m_yViewStartMax;
}

void VdkScrolled::CalcScrollBarSize(wxDC* dc)
{
	if( m_pScrollBar )
	{
		wxRect& rc = RelativeRect();
		wxDC* pDC = NULL;
		if( dc )
			pDC = GetScrollBarDC( dc );

		if( rc.height < m_yVirtualSize )
		{
			if( Style() & VSWS_ALWAYS_SHOW_SCROLLBAR )
			{
				if( !m_pScrollBar->IsEnabled() )
					m_pScrollBar->Enable( true, pDC );
			}
			else
			{
				if( !m_pScrollBar->IsOnShow() )
				{
					VdkControl* sbparent = ScrollBarParent();
					if( ( sbparent && sbparent->IsOnShow() ) || !sbparent )
					{
						m_pScrollBar->Show( true, pDC );
						OnScrollBarShowHide( m_pScrollBar );
					}
				}
			}

			int h;
			m_pScrollBar->GetSize( NULL, &h );

			// 本函数只负责建议滚动条的高度，至于具体的操作，
			// 即滚动条是否接受由 VdkScrollBar::SetThumbHeight 负责
			m_pScrollBar->SetThumbHeight
				( h * ( static_cast< double >( rc.height ) / m_yVirtualSize ), NULL);
			// 更新滚动条位置（滚动窗口的最大显示起点可能已改变）
			m_pScrollBar->GoTo( GetProgress(), pDC );
		}
		else // 一屏足够显示
		{
			if( Style() & VSWS_ALWAYS_SHOW_SCROLLBAR )
			{
				m_pScrollBar->Enable( false, pDC );
			}
			else
			{
				if( m_pScrollBar->IsOnShow() )
				{
					m_pScrollBar->Hide( pDC );
					OnScrollBarShowHide( m_pScrollBar );
				}
			}
		}

		if( dc )
			DestroyScrollBarDC( pDC );
	}
}

void VdkScrolled::RefreshState(wxDC* pDC)
{
	if( pDC )
	{
		DoRefreshState( *pDC );
		UpdateUI( *pDC );
	}

	if( m_pScrollBar )
	{
		wxDC* dc = GetScrollBarDC( pDC );
		m_pScrollBar->GoTo( GetProgress(), dc );
		DestroyScrollBarDC( dc );
	}
}

void VdkScrolled::Notify(const VdkNotify& notice)
{
	wxRect& rc = RelativeRect();

	switch( notice.GetNotifyCode() )
	{
	case VCN_CREATE:
	{
		if( !m_pScrollBar && !( Style() & VSWS_NO_SCROLLBAR ) )
		{
			// 检测是否已在 XRC 文件中定义并创建了需要的滚动条
			m_pScrollBar = ScrollBarExists();
			if( !m_pScrollBar )
			{
	  			ScrollBarStyle style = { NULL, 4, RESIZE_TYPE_TILE, 8 };
	  			SetScrollBarStyle( style );

	  			wxBitmap* bmArray = 
					m_pScrollBar->GetBitmapArray( style.strFileNames, style.nExpl );

	  			m_pScrollBar = new VdkScrollBar;
	  			m_pScrollBar->Create( VdkScrollBarInitializer().
	  								  window( ParentWindow() ).
	  								  name( Name() + L"_scrollbar" ).
	  								  rect( ScrollBarRect() ).
	  								  align( ALIGN_SYNC_Y | ALIGN_RIGHT ).
	  								  addToWindow( true ).
	  								  parent( ScrollBarParent() ).
	  								  bitmapArray( bmArray, true ).
	  								  thumbExpl( style.nExpl ).
	  								  thumbResizeType( style.nThumbTileType ).
	  								  thumbTileLen( style.nThumbTile ).
	  								  castTo< VdkScrollBarInitializer >() );
			}
		}

		// 为滚动条腾出空间
		if( m_pScrollBar )
		{
			if( Style() & VSWS_ALWAYS_SHOW_SCROLLBAR )
			{
				int w;
				m_pScrollBar->GetSize( &w, NULL );
				rc.width -= w;
				CalcScrollBarSize( NULL );
			}
			else
				m_pScrollBar->Hide( NULL );
		}

		break;
	}

	case VCN_SIZE_CHANGED:
	case VCN_WINDOW_RESIZED:

		CalcMaxViewStart();

		if( m_pScrollBar )
		{
			CalcScrollBarSize( NULL );
		}

		break;

	default:

		break;
	}

	OnNotify( notice );
}

void VdkScrolled::MouseEvent(VdkMouseEvent& e)
{
	switch( e.evtCode )
	{
	case SCROLLING_UP:
	case SCROLLING_DOWN:

		if( m_pScrollBar )
		{
			double percentage( m_pScrollBar->GetProgress() );
			int newIndex( m_yViewStartMax * percentage );

			if( newIndex != m_yViewStart )
			{
				m_yViewStart = newIndex;
				RefreshState( &e.dc );
			}
		}

		break;

	case WHEEL_DOWN:
	
		// m_linesUpDown == 0 表明不需要处理鼠标滚轮事件
		if( m_linesUpDown == 0 || m_yViewStart == m_yViewStartMax )
			break;

		for( int k = m_linesUpDown; k >= 1; --k )
		{
			if( m_yViewStart + k <= m_yViewStartMax )
			{
				m_yViewStart += k;
				break;
			}
		}

		goto PROCESS_SCROLLING;

	case WHEEL_UP:

		if( m_linesUpDown == 0 || m_yViewStart == 0 )
			break;

		for( int k = m_linesUpDown; k >= 1; --k )
		{
			if( m_yViewStart - k >= 0 )
			{
				m_yViewStart -= k;
				break;
			}
		}

PROCESS_SCROLLING:

		RefreshState( &e.dc );

		break;

	default:

		break;
	}

	// 滚动窗口位于同一个窗口时
	VdkDcDeviceOriginSaver saver( e.dc );
	PrepareDC( e.dc );
	
	OnMouseEvent( e );

	RestoreDC( e.dc );
}

void VdkScrolled::KeyEvent(VdkKeyEvent& e)
{
	// TODO:
	VdkDcDeviceOriginSaver saver( e.dc );
	PrepareDC( e.dc );

	OnKeyEvent( e );

	RestoreDC( e.dc );
}

void VdkScrolled::SetScrollBarStyle(ScrollBarStyle& style)
{

}

void VdkScrolled::GoTo(double progress, wxDC* pDC)
{
	// +1 是为了避免浮点运算的精度损失
	int yViewStart = m_yViewStartMax * progress;
	if( yViewStart )
		yViewStart++;

	SetViewStart( m_xViewStart, yViewStart, pDC );
}

double VdkScrolled::GetProgress() const
{
	if( m_yViewStartMax > 0 )
		return static_cast< double >( m_yViewStart ) / m_yViewStartMax;
	else
		return 1;
}

//////////////////////////////////////////////////////////////////////////

VdkScrolledWindow::VdkScrolledWindow(long style)
	: m_borderColor( 81, 163, 206 )
{
	SetAddinStyle( style );
}

void VdkScrolledWindow::Create(wxXmlNode* node)
{
	OnXrcCreate( node );
	ParseObjects( VdkCtrlParserInfo().
				  window( m_Window ).
				  node( node ).
				  parent( this ) );
}

void VdkScrolledWindow::Clone(VdkScrolledWindow* o)
{
	wxASSERT_MSG( CheckSourceCtrl< VdkScrollBar >( m_firstChild ), 
		L"错误的滚动条。" );

	VdkScrollBar* scrollbar = (VdkScrollBar *) m_firstChild;
	VdkScrolled::Clone( o, scrollbar );
	m_borderColor = o->m_borderColor;

	// 初始状态是不显示滚动条的，将滚动窗口修正为正确的作用域
	if( !TestStyle( VSWS_ALWAYS_SHOW_SCROLLBAR ) &&
		scrollbar->IsOnShow() )
	{
		scrollbar->Hide( NULL );
		OnScrollBarShowHide( scrollbar );
	}
}

wxRect VdkScrolledWindow::GetLogicalRect() const
{
	wxRect rc( m_Rect );
	VdkScrollBar* psb = GetScrollBar();

	if( psb && psb->IsOnShow() )
		rc.width += psb->GetRect().width;

	return rc;
}

VdkScrollBar* VdkScrolledWindow::ScrollBarExists()
{
	// 滚动条是否已然存在
	ChildIterator chd;
	VdkScrollBar* psb = NULL;
	for( chd = begin(); chd != end(); ++chd )
	{
		psb = dynamic_cast< VdkScrollBar* >( chd.get() );
		if( psb )
			break;
	}

	return psb;
}

void VdkScrolledWindow::DoPrepareDC(wxDC& dc)
{
	dc.SetClippingRegion( GetAbsoluteRect() );
}

void VdkScrolledWindow::DoRestoreDC(wxDC& dc)
{
	dc.DestroyClippingRegion();
}

void VdkScrolledWindow::DoDraw(wxDC& dc)
{
	if( TestStyle( VCS_BORDER_SIMPLE ) )
	{
		wxPen borderPen( m_borderColor );
		dc.SetPen( borderPen );

		DrawRectangle( dc, GetLogicalRect().Inflate( 1 ) );
	}

	// 向基类分发重画事件
	Paint( dc );
}

void VdkScrolledWindow::DoHandleMouseEvent(VdkMouseEvent& e)
{
	// 将鼠标位置改为以滚动面板左上角为起始点
	e.mousePos = DeTranslatePoint( e.mousePos );
	MouseEvent( e );
}

void VdkScrolledWindow::DoRefreshState(wxDC& dc)
{
	m_Window->ResetDcOrigin( dc );
}

void VdkScrolledWindow::OnScrollBarShowHide(const VdkScrollBar* sb)
{
	int w = sb->GetRect().width;
	w = sb->IsOnShow() ? -w : w;
	m_Rect.width += w;

	VdkNotify notice( VCN_SIZE_CHANGED );
	notice.SetWparam( w );

	OnNotify( notice );
}
