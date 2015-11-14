/***************************************************************
 * Name:      VdkScalableBitmap.cpp
 * Purpose:   Code for VdkScalableBitmap implementation
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-03-11
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkScalableBitmap.h"

#include "VdkUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkScalableBitmap::VdkScalableBitmap()
	: m_resizeType( RESIZE_TYPE_TILE )
{

}

VdkScalableBitmap::VdkScalableBitmap(const wxBitmap& bm, 
									 VdkResizeableBitmapType resizeType, 
									 const wxRect& rcTile)
	: m_bitmap( bm ),
	  m_rescaled( m_bitmap ),
	  m_resizeType( resizeType ),
	  m_rcTile( rcTile ),
	  m_rcTile_o( m_rcTile ),
	  m_size( bm.GetWidth(), bm.GetHeight() )
{
	
}

void VdkScalableBitmap::Set(const wxBitmap& bm, 
							VdkResizeableBitmapType resizeType, 
							const wxRect& rcTile)
{
	SetBitmap( bm );

	m_size.x = m_bitmap.GetWidth();
	m_size.y = m_bitmap.GetHeight();

	m_resizeType = resizeType;
	m_rcTile_o = m_rcTile = rcTile;
}

void VdkScalableBitmap::SetBitmap(const wxBitmap& bm)
{
	m_bitmap = bm;
	m_rescaled = bm;
}

void VdkScalableBitmap::Blit(wxDC& dc, wxCoord x, wxCoord y)
{
	if( IsOk() )
	{
		wxMemoryDC mdc( m_rescaled );
		dc.Blit( x, y, m_size.x, m_size.y, &mdc, 0, 0 );
	}
}

void VdkScalableBitmap::Reset()
{
	m_rescaled = m_bitmap = wxNullBitmap;
}

void VdkScalableBitmap::WindowSetShape(wxTopLevelWindow* win, 
									   const wxColour& maskColor, 
									   WindowShapeMode mode) 
									   const
{
#if defined( __WXMSW__ ) && 0
	if( mode == WSM_LAYERED )
	{
		HWND hwnd = GetHwndOf( win );

		DWORD dwExStyle = ::GetWindowLong( hwnd, GWL_EXSTYLE );
		if( (dwExStyle & WS_EX_LAYERED) != WS_EX_LAYERED )
		{
			dwExStyle |= WS_EX_LAYERED;
			::SetWindowLong( hwnd, GWL_EXSTYLE, dwExStyle );
		}

		wxUint32 crKey = maskColor.GetRGB();
		if( !::SetLayeredWindowAttributes( hwnd, crKey, 0, LWA_COLORKEY ) )
		{
			wxLogLastError( L"SetLayeredWindowAttributes" );
		}

		return;
	}
#endif

	if( IsOk() )
	{
		if( m_rescaled.GetMask() )
			win->SetShape( wxRegion( m_rescaled ) );
		else
			win->SetShape( wxRegion( m_rescaled, maskColor ) );
	}
}

bool VdkScalableBitmap::CanResize() const
{
	if( m_rcTile.IsEmpty() )
		return false;

	wxRect rcPrimitive( 0, 0, m_bitmap.GetWidth(), GetHeight() );
	if( !rcPrimitive.Contains( m_rcTile ) )
	{
		wxLogDebug( L"无效拉伸(平铺)区域:(%d,%d,%d,%d)/(%d,%d,%d,%d)",
					m_rcTile.x, m_rcTile.y,
					m_rcTile.width, m_rcTile.height,
					rcPrimitive.x, rcPrimitive.y, 
					rcPrimitive.width, 
					rcPrimitive.height );

		return false;
	}

	return true;
}

bool VdkScalableBitmap::Rescale(const wxSize& newsize)
{
	if( !CanResize() )
	{
		return false;
	}

	if( m_resizeType == RESIZE_TYPE_TILE )
	{
		// 优化
		int dx = newsize.x - m_rescaled.GetWidth(),
			dy = newsize.y - m_rescaled.GetHeight();

		if( (dx >= 0) && (dy >= 0) )
		{
			m_rescaled = VdkUtil::TileBitmap
				( m_rescaled, newsize.x, newsize.y, m_rcTile_o, false );

			m_rcTile_o.x += dx;
			m_rcTile_o.y += dy;
		}
		else
		{
			m_rescaled = wxNullBitmap; // 不能和 m_bitmap 同时使用
			m_rcTile_o = m_rcTile;

			m_rescaled = VdkUtil::TileBitmap
				( m_bitmap, newsize.x, newsize.y, m_rcTile, false );
		}
	}
	else
	{
		m_rescaled = wxNullBitmap; // 不能和 m_bitmap 同时使用
		m_rescaled = VdkUtil::InflateBitmapByCenter
			( m_bitmap, m_rcTile, newsize );
	}

	m_size = newsize;

	//----------------------------------------------------

	return true;
}

void VdkScalableBitmap::BlitRect(wxDC& dc, const wxRect& rc)
{
	if( IsOk() )
	{
		wxMemoryDC mdc( m_rescaled );
		dc.Blit( rc.x, rc.y, rc.width, rc.height, &mdc, rc.x, rc.y );
	}
}
