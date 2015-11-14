#include "StdAfx.h"
#include "VdkPanel.h"

#include "VdkWindow.h"
#include "VdkCtrlParserInfo.h"
#include "VdkDC.h"

#include "wxUtil.h"
#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT( VdkPanel );

VdkPanel::VdkPanel()
		 : m_redrawer( NULL ), 
		   m_bgBrush( *wxWHITE_BRUSH ), 
		   m_gbi( NULL ),
		   m_bi( NULL )
{
	SetAddinStyle( VCS_ERASE_BG );
}

void VdkPanel::Create(wxXmlNode* node)
{
	Create( m_Window, GetXrcName(node), GetXrcRect(node) );

	// VdkTab 附加的 VdkPanel 不需要提供 <rect> 结点，
	// 因此 VdkPanel 在初始化时 m_Rect 是空的
	if( m_Rect.IsEmpty() )
	{
		m_Rect = m_parent->GetRect();
		m_Rect.x = m_Rect.y = 0;
	}

	ParseObjects( VdkCtrlParserInfo().
				  window( m_Window ).
				  node( node ).
				  parent( this ) );

	m_gbi = GetXrcGradientBgInfo( node );
	m_bi = GetXrcBgInfo( node );
	m_align = GetXrcAlign( node );
}

void VdkPanel::Create(VdkWindow* parent, const wxString& strName, const wxRect& rc)
{
	m_strName = strName;
	m_Rect = rc;

	SetVdkWindow( parent );
}

VdkPanel::~VdkPanel()
{
    wxDELETE( m_bi );
    wxDELETE( m_gbi );
}

void VdkPanel::DoEraseBackground(wxDC& dc, const wxRect& rc)
{
	if( !m_gbi )
	{
		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.SetBrush( m_bgBrush );

		if( m_bi )
		{
			if( m_bi->borderPen.IsOk() )
				dc.SetPen( m_bi->borderPen );

			if( m_bi->bgColor.IsOk() )
			{
				wxBrush bgBrush( m_bi->bgColor );
				dc.SetBrush( bgBrush );
			}
		}

		dc.DrawRectangle( rc );
	}
	else
	{
        VdkDcClippingRegionDestroyer destroyer( dc, rc );
		m_Window->ResetDcOrigin( dc );
		TranslateDC( dc );

		dc.GradientFillLinear( m_Rect, m_gbi->beg, m_gbi->end, m_gbi->direction );
	}
}

void VdkPanel::DoDraw(wxDC& dc)
{
	if( m_redrawer )
		m_redrawer->DoRedraw( dc );
}
