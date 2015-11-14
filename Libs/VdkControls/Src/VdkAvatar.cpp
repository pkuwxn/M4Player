#include "StdAfx.h"
#include "VdkAvatar.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


IMPLEMENT_DYNAMIC_VOBJECT( VdkAvatar );

//////////////////////////////////////////////////////////////////////////

VdkAvatar::VdkAvatar() : m_radius( 0 ), m_ptrState( NORMAL )
{
	RemoveStyle( VCS_IGNORE_ALL_EVENTS );
}

void VdkAvatar::Create(wxXmlNode* node)
{
	VdkStaticImage::Create( node );
	//------------------------------

	m_radius = XmlGetContentOfNum( FindChildNode( node, L"radius" ) );

	wxString v( XmlGetChildContent( node, L"border-color" ) );
	if( !v.IsEmpty() )
	{
		m_borderPen.SetColour( wxColour( v ) );
	}
}

void VdkAvatar::DoDraw(wxDC& dc)
{
	const wxBitmap& image = GetImage();
	if( !image.IsOk() )
		return;

	if( m_radius != 0 )
	{
		int w( image.GetWidth() ), h( image.GetHeight() );

		wxBitmap obm( w, h );
		wxMemoryDC odc( obm );

		odc.SetBackground( *wxBLACK );
		odc.Clear();
		odc.SetPen( *wxWHITE_PEN );
		odc.SetBrush( *wxWHITE_BRUSH );
		odc.DrawRoundedRectangle( 0, 0, w, h, m_radius );

		wxMemoryDC mdc;
		mdc.SelectObjectAsSource( image );
		odc.Blit( 0, 0, w, h, &mdc, 0, 0, wxAND );

		if( m_borderPen.IsOk() )
		{
			odc.SetPen( m_borderPen );
			odc.SetBrush( *wxTRANSPARENT_BRUSH );
			odc.DrawRoundedRectangle( 0, 0, w, h, m_radius );
		}

		dc.SetPen( *wxBLACK_PEN );
		dc.SetBrush( *wxBLACK_BRUSH );
		dc.DrawRoundedRectangle( m_Rect.x, m_Rect.y, w, h, m_radius );
		dc.Blit( m_Rect.x, m_Rect.y, w, h, &odc, 0, 0, wxOR );
	}
	else
	{
		VdkStaticImage::Draw( dc );
	}
}

void VdkAvatar::DoHandleMouseEvent(VdkMouseEvent& e)
{
	switch( e.evtCode )
	{
	case LEFT_UP:

		if( m_ptrState == LEFT_DOWN && IsReadyForEvent() )
			FireEvent( &e.dc, NULL );

		break;

	default:

		break;	
	}

	m_ptrState = (VdkMouseEventType) e.evtCode;
}
