#include "StdAfx.h"

#include "Optional/VdkRichTextCtrl.h"
#include "VdkWindow.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT(VdkRichTextCtrl);

//////////////////////////////////////////////////////////////////////////

void VdkRichTextCtrl::Init()
{
	m_nativeStyle = 0L;
	m_borderWeight = 0;
}

void VdkRichTextCtrl::Create(wxXmlNode* node)
{
	long style( wxRE_MULTILINE | wxWANTS_CHARS );

	wxString strTmp;
	strTmp = XmlGetChildContent( node, L"border" );
	if( strTmp.CmpNoCase( L"simple" ) == 0 )
	{
		style |= wxBORDER_SIMPLE;

		strTmp = XmlGetChildContent( node, L"border-weight" );
		if( !strTmp.IsEmpty() )
		{
			m_borderWeight = wxAtoi( strTmp );
			m_borderPen.SetWidth( m_borderWeight );
		}

		strTmp = XmlGetChildContent( node, L"border-color" );
		if( !strTmp.IsEmpty() )
		{
			wxColour color( strTmp );
			m_borderPen.SetColour( color );
		}
	}
	else if( strTmp.CmpNoCase( L"raise" ) == 0 )
		style |= wxBORDER_RAISED;
	else if( strTmp.CmpNoCase( L"none" ) == 0 )
		style |= wxBORDER_NONE;

	Create( m_Window, GetXrcName( node ), GetXrcRect( node ), style );
	m_align = GetXrcAlign( node );
}

void VdkRichTextCtrl::Create(VdkWindow* win, 
							 const wxString& strName, 
							 const wxRect& rect, 
							 long style)
{
	// TODO:
	m_strName = strName;
	m_Rect = rect;
	SetVdkWindow( win );

	m_nativeStyle = style;

	//////////////////////////////////////////////////////////////////////////

	wxRect rc( GetAbsoluteRect() );
	if( TestStyle( wxBORDER_SIMPLE ) )
	{
		style ^= wxBORDER_SIMPLE;
		style |= wxBORDER_NONE;
		rc.Deflate( m_borderWeight );
	}

	wxRichTextCtrl::Create( m_WindowImpl, 
						    wxID_ANY, 
						    wxEmptyString, 
						    rc.GetPosition(), 
						    rc.GetSize(), 
						    style );
}

void VdkRichTextCtrl::DoDraw(wxDC* pDC)
{
	if( TestStyle( wxBORDER_SIMPLE ) )
	{
		wxRect rc( m_Rect );
		rc.Inflate( m_borderWeight );

		pDC->SetPen( m_borderPen );
		pDC->SetBrush( *wxTRANSPARENT_BRUSH );
		DrawRectangle( *pDC, rc );
	}
}
