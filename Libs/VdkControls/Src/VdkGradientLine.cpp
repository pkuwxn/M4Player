#include "StdAfx.h"
#include "VdkGradientLine.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT( VdkGradientLine );

//////////////////////////////////////////////////////////////////////////

VdkGradientLine::VdkGradientLine() : m_dxLine( -1 ), m_dyLine( 0 )
{
	m_Font.SetWeight( wxFONTWEIGHT_BOLD );
	SetAddinStyle( VCS_IGNORE_ALL_EVENTS );
}

void VdkGradientLine::DoDraw(wxDC& dc)
{
	dc.SetPen( *wxBLACK_PEN );
	dc.SetFont( m_Font );
	dc.SetTextForeground( *wxBLACK );

	if( m_dxLine == -1 )
	{
		int w = 0, h = 0;
		if( m_strCaption.empty() )
		{
			h = dc.GetCharHeight();
			m_dxLine = 0;
		}
		else
		{
			dc.GetTextExtent( m_strCaption, &w, &h );
			m_dxLine = w + dc.GetCharWidth(); // 加一个空白符用以分割			
		}

		m_dyLine = ( h - 1 ) / 2;
		w = m_Rect.width - m_dxLine;

		if( w > 0 )
		{
			m_gradientLine = wxBitmap( w, 1 );
			wxMemoryDC mdc( m_gradientLine );
			wxRect rcGradi( 0, 0, w, 1 );
			wxColour begColor( *wxLIGHT_GREY );
			mdc.GradientFillLinear( rcGradi, begColor, *wxWHITE );
		}
	}

	dc.DrawText( m_strCaption, m_Rect.x, m_Rect.y + m_yFix );

	if( m_gradientLine.IsOk() )
	{
		dc.DrawBitmap( m_gradientLine, m_Rect.x + m_dxLine, 
					   m_Rect.y + m_yFix + m_dyLine );
	}
}
