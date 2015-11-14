/***************************************************************
 * Name:      VdkSearchCtrl.cpp
 * Purpose:   Code for VdkSearchCtrl implementation
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-03-13
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkSearchCtrl.h"
#include "VdkEdit.h"
#include "VdkButton.h"
#include "wxUtil.h"

#include <wx/artprov.h>
#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

#include "Images/checkbox_border.xpm"
#ifdef __WXMSW__
#include "Images/search_ctrl_button.h"
#endif

IMPLEMENT_DYNAMIC_VOBJECT( VdkSearchCtrl );

//////////////////////////////////////////////////////////////////////////

VdkSearchCtrl::VdkSearchCtrl()
	: m_edit( NULL ),
	  m_button( NULL ),
	  m_bmpBorder( wxBitmap( CheckBox_xpm ), 4, 1 ),
	  m_borderWeight( 1 ),
	  m_radius( 3 ),
	  m_borderPen( *wxGREY_PEN ),
	  m_hilightPen( *wxBLUE_PEN )
{

}

void VdkSearchCtrl::Create(wxXmlNode* node)
{
	VdkControl::DoXrcCreate( node );

	Create( VdkSearchCtrlInitializer().
		    style( VCS_BORDER_NONE ).
		    align( ALIGN_BOTTOM | ALIGN_SYNC_X ) );
}

void VdkSearchCtrl::Create(const VdkSearchCtrlInitializer& init_data)
{
	VdkControl::Create( init_data );

	//////////////////////////////////////////////////////////////////////////
	// 按钮

#   ifdef __WXMSW__
	int nExpl = 3;
	wxBitmap bmpButton( LoadPngFromRawData( SearchButton_png,
                                        sizeof( SearchButton_png ) ) );
#   else
    int nExpl = 1;
    wxBitmap bmpButton( wxArtProvider::GetBitmap( wxART_FIND ) );
#   endif // __WXMSW__

	int w = bmpButton.GetWidth() / nExpl;
	int h = bmpButton.GetHeight();
	wxRect rcButton( m_Rect.width - w,
		( m_Rect.height - m_borderWeight * 2 - h ) / 2, w, h );

	m_button = new VdkButton;
	m_button->Create( VdkButtonInitializer().
					  window( m_Window ).
					  addToWindow( true ).
					  name( m_strName + L"_Button" ).
					  id( m_id ).
					  parent( this ).
					  align( ALIGN_RIGHT ).
					  bitmap( bmpButton ).
					  explode( nExpl ).
					  rect( rcButton ) );

	//////////////////////////////////////////////////////////////////////////
	// 文本框

	wxRect rcEdit( m_Rect );
	rcEdit.x = m_borderWeight + m_radius;
	rcEdit.y = m_borderWeight;
	rcEdit.width -= m_borderWeight * 2 + m_radius + w;
	rcEdit.height -= m_borderWeight * 2;

	long style0 = GetStyle() & ~( VCS_HANDLER | VCS_ONESHOT_HOVERING );
	style0 |= VCS_ERASE_BG;

	m_edit = new VdkEdit;
	m_edit->Create( VdkEditInitializer().
					window( m_Window ).
					addToWindow( true ).
					name( m_strName + L"_Edit" ).
					parent( this ).
					id( VdkGetUniqueId() ).
					style( style0 ).
					align( m_align ).
					rect( rcEdit ) );

	//--------------------------------------------------

	// 我们不需要焦点转移，需要的是 Edit
	RemoveStyle( VCS_TAB_TRAVERSAL );
}

void VdkSearchCtrl::DoHandleNotify(const VdkNotify& notice)
{
	switch( notice.GetNotifyCode() )
	{
	case VCN_FOCUS:

		m_Window->FocusCtrl( m_edit, notice.GetVObjDC() );
		break;

	case VCN_ID_CHANGED:

		m_button->SetID( m_id );
		break;

	default:
		break;
	}
}

void VdkSearchCtrl::DoDraw(wxDC& dc)
{
	dc.SetPen( m_borderPen );
	dc.SetBrush( *wxWHITE_BRUSH );
	dc.DrawRoundedRectangle( m_Rect, m_radius );
}

void VdkSearchCtrl::DoEraseBackground(wxDC& dc, const wxRect& rc)
{

}

VdkCtrlId VdkSearchCtrl::GetEditID() const
{
	return m_edit->GetID();
}
