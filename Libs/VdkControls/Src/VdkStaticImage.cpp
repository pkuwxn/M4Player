#include "StdAfx.h"
#include "VdkStaticImage.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT( VdkStaticImage );

//////////////////////////////////////////////////////////////////////////

void VdkStaticImage::Create(wxXmlNode* node)
{
	GetXrcImage( node, m_image );
	Create( m_Window, 
			GetXrcName( node ), 
			GetXrcRect( node ), 
			m_image,
			XmlGetContentOfBoolean( FindChildNode( node, L"rescale" ), 
			false ) );

	m_align = GetXrcAlign( node );
}

void VdkStaticImage::Create(VdkWindow* Window, 
							const wxString& strName, 
							const wxRect& rc, 
							const wxBitmap& bm,
							bool rescaleBitmap)
{
	m_strName = strName;
	m_Rect = rc;
	SetVdkWindow( Window );

	SetAddinStyle( VCS_IGNORE_ALL_EVENTS );
	m_image = bm;
	
	// TODO:
	if( !m_image.IsOk() )
	    return;

	int w = m_image.GetWidth(), h = m_image.GetHeight();
	if( rescaleBitmap && ( w > m_Rect.width || h > m_Rect.height ) )
	{
		double rx = double( w ) / m_Rect.width, 
			ry = double( h ) / m_Rect.height;
		double r = wxMax( rx, ry );

		int nw = w / r, nh = h / r;
		m_image = wxBitmap( m_image.ConvertToImage().
							Rescale( nw, nh ) );
	}

}

void VdkStaticImage::DoDraw(wxDC& dc)
{
	if( m_image.IsOk() )
		dc.DrawBitmap( m_image, m_Rect.x, m_Rect.y, true );
}
