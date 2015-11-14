#include "StdAfx.h"
#include "VdkCheckBox.h"

#include "VdkWindow.h"
#include "VdkToggleButton.h"
#include "VdkDC.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT( VdkCheckBox );
#include "Images/checkbox_all.xpm"

VdkCheckBox::VdkCheckBox()
{
	m_TextAlign = 0;
}

void VdkCheckBox::Create(wxXmlNode* node)
{
	wxBitmap bm( checkbox_all_xpm );
	wxMask* mask = new wxMask( bm, wxColour( 255, 0, 255 ) );
	bm.SetMask( mask );

	XmlInsertChild( node, L"x-padding", L"20" );
	XmlInsertChild( node, L"y-padding", L"0" );

	VdkButton::Create( node, bm, 8 );
	Create( m_Window, m_strName, m_Rect );
}

void VdkCheckBox::Create(VdkWindow* Window, const wxString& strName, wxRect rc)
{
	m_strName = strName;
	m_Rect = rc;
	SetVdkWindow( Window );

	int w, h;
	m_WindowImpl->GetTextExtent( m_strCaption, &w, &h, 0, 0, &m_Font );
	m_Rect.width = w += 20;

	m_nPaddingY = ( m_bmArray.GetHeight() - h ) / 2;
	m_bToggled = false;

	// TODO: 头疼的问题：英文不会被精确地画到正确的位置上
	SetAddinStyle( VCS_ERASE_BG );
}

void VdkCheckBox::DoDraw(wxDC& dc)
{
	if( m_bToggled )
	{
		if( GetLastState() == VdkButton::NORMAL )
			SetButtonState( CHECKED );
	}

	VdkButton::DoDraw( dc );
}

void VdkCheckBox::DoHandleNotify(const VdkNotify& notice)
{
	switch( notice.GetNotifyCode() )
	{
	case VCN_DISABLED:
	case VCN_ENABLED:
		{
			bool enable = IsEnabled();
			if( m_bToggled )
			{
				if( !enable )
					SetButtonState( CHECKED_DISABLED );
				else
					SetButtonState( CHECKED );
			}
			else
			{
				if( !enable )
					SetButtonState( NORMAL_DISABLED );
				else
					SetButtonState( NORMAL );
			}

			EnableRelatedCtrls( enable, notice.GetVObjDC() );

			break;
		}

	default:

		break;
	}
}

void VdkCheckBox::EnableRelatedCtrls(bool bEnabled, wxDC* pDC)
{
	VdkWindow::CtrlIter i;
	for( i = m_relatedCtrls.begin(); i != m_relatedCtrls.end(); ++i )
		(*i)->Enable( m_bToggled, pDC );
}

void VdkCheckBox::DoHandleMouseEvent(VdkMouseEvent& e)
{
	State state = (State) e.evtCode;
	switch( state )
	{
	case NORMAL:

		if( !m_bToggled )
			state = NORMAL;
		else
			state = CHECKED;

		break;

	case LEFT_UP:
	{
		state = PUSHED;
		m_bToggled = !m_bToggled;

		if( m_bToggled )
			state = CHECKED_HOVERING;
		else
			state = HOVERING;

		EnableRelatedCtrls( m_bToggled, &e.dc );

		if( IsReadyForEvent() )
			FireEvent( &e.dc, NULL );

		break;
	}

	case HOVERING:

		if( m_bToggled )
			state = CHECKED_HOVERING;

		break;

	default:

		e.Skip( true );

		return;
	}

	Update( ( VdkButton::State ) state, &e.dc );
}

void VdkCheckBox::Toggle(bool bToggled, wxDC* pDC)
{
	if( bToggled == m_bToggled )
		return;

	m_bToggled = bToggled;

	State state( CHECKED );
	if( m_Rect.Contains( m_WindowImpl->ScreenToClient(wxGetMousePosition()) ) )
		state = CHECKED_HOVERING;

	EnableRelatedCtrls( m_bToggled, pDC );
	Update( ( VdkButton::State ) state, pDC );
}

void VdkCheckBox::AddRelatedCtrl(VdkControl* relatedCtrl)
{
	wxASSERT( relatedCtrl );

	m_relatedCtrls.push_back( relatedCtrl );
	relatedCtrl->Enable( false, NULL );
}
