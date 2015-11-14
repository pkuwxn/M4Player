#include "StdAfx.h"
#include "VdkFontPicker.h"

#include "VdkWindow.h"
#include "VdkButton.h"
#include "wxUtil.h"

#include <wx/fontdlg.h>
#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT( VdkFontPicker );

//////////////////////////////////////////////////////////////////////////

void VdkFontPicker::Create(wxXmlNode* node)
{
	SetAddinStyle( VBS_RESIZEABLE );
	VdkButton::Create( node );

	m_selectedFont = m_WindowImpl->GetFont();
	SetCaption( BuildString(), NULL );
}

void VdkFontPicker::DoHandleMouseEvent(VdkMouseEvent& e)
{
	if( e.evtCode == LEFT_UP && GetLastState() == PUSHED )
	{
		Update( NORMAL, &e.dc );

		wxFont font( wxGetFontFromUser( m_WindowImpl, m_selectedFont ) );
		if( font.IsOk() )
		{
			m_selectedFont = font;
			SetCaption( BuildString(), &e.dc );
		}

		if( IsReadyForEvent() )
			FireEvent( &e.dc, NULL );

		return;
	}

	VdkButton::DoHandleMouseEvent( e );
}

wxString VdkFontPicker::BuildString(wxFont* pFont)
{
	if( !pFont )
		pFont = &m_selectedFont;

	return wxString::Format( L"%s, %dpt", pFont->GetFaceName().c_str(),
                             pFont->GetPointSize() );
}

const wxFont& VdkFontPicker::SetSelectedFont(const wxString& strDesc, wxDC* pDC)
{
	wxFont font( GetFontFromDescString( strDesc ) );
	if( font.IsOk() )
	{
		m_selectedFont = font;
		SetCaption( strDesc, pDC );
	}

	return m_selectedFont;
}

const wxFont& VdkFontPicker::SetSelectedFont(const wxFont& font, wxDC* pDC)
{
	m_selectedFont = font;
	SetCaption( BuildString(), pDC );

	return font;
}

wxFont VdkFontPicker::GetFontFromDescString(const wxString& strDesc)
{
	wxString::size_type pos( strDesc.find( ',' ) );
	if( pos != wxString::npos )
	{
		wxString strFaceName( strDesc.Mid( 0, pos ) );
		int nPointSize( wxAtoi( strDesc.Mid( pos + 2 ) ) );

		return wxEasyCreatFont( strFaceName, nPointSize );
	}

	return wxEasyCreatFont();
}
