/***************************************************************
 * Name:      VdkMenuBar.cpp
 * Purpose:   Code for VdkMenuBar implementation
 * Author:    vanxining (vanxining@139.com)
 * Created:   2010-12-02
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkMenuBar.h"
#include "VdkWindow.h"
#include "VdkMenu.h"

#include "VdkDC.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkMenuBarInitializer::VdkMenuBarInitializer() 
	: menuArray( NULL )
{

}

VdkMenuBarEntryInitializer::VdkMenuBarEntryInitializer()
	: DrawStyle( NULL )
{

}

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT( VdkMenuBar );

#include <wx/listimpl.cpp>
WX_DEFINE_LIST( ListOfEntry );

const static int wAddIn = 4; // 左右各增添的长度
const static int hAddIn = 3; // 上下各增添的长度

//////////////////////////////////////////////////////////////////////////

VdkMenuBar::VdkMenuBar()
		  : m_total( 0 ),
		    m_drawStyle( GetMenuBarEntryStyle( VMBES_OFFICE2003 ) )
{

}

void VdkMenuBar::Create(wxXmlNode* node)
{
	m_Window->PushEventFilter( this );
	DoXrcCreate( node );
	SetAlign( GetAlign() | ALIGN_SYNC_X );
}

void VdkMenuBar::Create(const VdkMenuBarInitializer& init_data)
{

}

VdkMenuBar::~VdkMenuBar()
{
	delete m_drawStyle;
}

inline VdkMenuBarEntry* VdkMenuBar::CreateNewEntry(int& w, int& h)
{
	return new VdkMenuBarEntry;
}

VdkMenuBar& VdkMenuBar::Append(wxString strCaption, VdkMenu* menu, 
							   const wxString& strMenuTree)
{
	int w(0), h( m_Rect.height );

	if( !strCaption.IsEmpty() )
	{
		m_WindowImpl->GetTextExtent( strCaption, &w, &h, 0, 0, &m_Font );

		w += wAddIn * 2;
		h += hAddIn * 2;

		if( strCaption.find( '&' ) != wxString::npos )
			w -= m_WindowImpl->GetCharWidth();
	}

	//////////////////////////////////////////////////////////////////////////

	VdkMenuBarEntry* entry = CreateNewEntry( w, h );
	entry->SetClientData( (void *) m_entries.size() );

	entry->Create( VdkMenuBarEntryInitializer().
				   menu( menu ).
				   menuTree( strMenuTree ).
				   drawStyle( m_drawStyle ).
				   window( m_Window ).
				   addToWindow( true ).
				   show( IsOnShow() ).
				   thaw( !IsFreezed() ).
				   name( wxString::Format( L"%s_entry_%d", m_strName, 
														   m_entries.size() ) ).
				   rect( wxRect( m_total, ( m_Rect.height - h ) / 2, w, h ) ).
				   caption( strCaption ).
				   parent( this ).
				   castTo< VdkMenuBarEntryInitializer >() );

	m_entries.push_back( entry );

	m_total += w;
	if( m_total > static_cast< unsigned >( m_Rect.width ) )
		m_Rect.width = m_total;

	//////////////////////////////////////////////////////////////////////////

	return *this;
}

bool VdkMenuBar::FilterEvent(const EventForFiltering& e)
{
	wxPoint mousePos( e.mouseEvent().GetPosition() );

	// 不管什么事件，只要位于菜单栏作用域内我们都要拦截
	if( HitTest( mousePos ) )
	{
		VdkControl::ChildIterator i;
		for( i = begin(); i != end(); ++i )
		{
			if( i->HitTest( mousePos ) )
				break;
		}

		if( i != end() )
		{
			VdkControl::ChildIterator j;
			for( j = begin(); j != end(); ++j )
			{
				if( j->IsMenuOnShow() ) // 正在显示菜单的按钮
					break;
			}

			if( j != end() )
			{
				// 显示新的菜单
				if( i != j )
				{
					m_Window->HideMenu();

					VdkMenuBarEntry* entry = 
						static_cast< VdkMenuBarEntry* >( i.get() );
					VdkDC vdc( m_Window, entry->GetAbsoluteRect(), NULL );
					entry->EmuClick( vdc );
				}

				return true;
			}
			// 假如还没有显示菜单，直接返回（单击才显示菜单）
		}

	}

	return false;
}

void VdkMenuBar::DoEraseBackground(wxDC& dc, const wxRect& rc)
{
	if( !TestStyle( VCS_ERASE_BG ) )
		return;

	wxBrush brush( wxColour( 183,208,248 ) );
	ClrBkGnd( dc, brush, rc );
}

void VdkMenuBar::DoHandleNotify(const VdkNotify& notice)
{
	switch( notice.GetNotifyCode() )
	{
	case VCN_SHOW:
	case VCN_THAW:
	case VCN_ENABLED:

		Resume();

		break;

	case VCN_HIDE:
	case VCN_FREEZE:
	case VCN_DISABLED:

		Pause();

		break;

	case VCN_WINDOW_RESIZED:

		if( m_align & ALIGN_CENTER_X_Y )
		{
			ListOfEntry::iterator i;
			for( i = m_entries.begin(); i != m_entries.end(); ++i )
			{

			}
		}

		break;

	default:

		break;
	}
}

//////////////////////////////////////////////////////////////////////////

VdkMenuBarEntry::VdkMenuBarEntry()
{
	Init();
}

void VdkMenuBarEntry::DoEraseBackground(wxDC& dc, const wxRect& rc)
{
	if( m_strCaption.IsEmpty() )
	{
		m_strCaption = VdkMenuPopper::m_strCaption;
		VdkMenuPopper::m_strCaption = wxEmptyString;
	}

	switch( GetButtonState() )
	{
	case HOVERING:

		dc.SetPen( m_drawStyle->m_hilightBorderPen );
		dc.SetBrush( m_drawStyle->m_hilight );

		break;

	case PUSHED:

		dc.SetPen( m_drawStyle->m_selectedBorderPen );
		dc.SetBrush( m_drawStyle->m_selected );

		// 左边不要 GetRight() + 1 ，这样会与右边的矩形重合
		// 下边的 + 1 ，这样可以保证与菜单很好的接合

		// 左↓
		dc.DrawLine( m_Rect.x, m_Rect.y, m_Rect.x, m_Rect.GetBottom() + 1 );
		// 右↓
		dc.DrawLine( m_Rect.GetRight(), m_Rect.y,
					 m_Rect.GetRight(), m_Rect.GetBottom() + 1 );
		// 上→
		dc.DrawLine( m_Rect.x, m_Rect.y, m_Rect.GetRight(), m_Rect.y );

		break;

	default:

		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.SetBrush( m_drawStyle->m_background );

		break;
	}

	if( GetButtonState() != PUSHED )
		dc.DrawRectangle(m_Rect);

	dc.SetFont( m_Font );
	dc.SetTextForeground( *wxBLACK );
	DrawHotKeyCaption( m_strCaption, 
					   dc,
					   m_Rect.x + wAddIn,
					   m_Rect.y + hAddIn );
}

void VdkMenuBarEntry::Init()
{
	m_drawStyle = NULL;
	SetAddinStyle( VCS_ERASE_BG );
}

VdkMenuBarEntry::~VdkMenuBarEntry() {}

void VdkMenuBarEntry::Create(const VdkMenuBarEntryInitializer& init_data)
{
	VdkButton::Create( init_data );

	m_drawStyle = init_data.DrawStyle;
	m_menu = init_data.Menu;
	if( !m_menu && !init_data.MenuTree.IsEmpty() )
		m_menu = GenerateMenu( wxID_ANY, m_Window, init_data.MenuTree );

	Prepare( NULL );
}

//////////////////////////////////////////////////////////////////////////

VdkMenuBarEntryStyle* GetMenuBarEntryStyle(VdkMenuBarEntryStyleEnum sid)
{
	VdkMenuBarEntryStyle* style = new VdkMenuBarEntryStyle;

	switch( sid )
	{
	case VMBES_OFFICE2003:

		style->hilightBorderColour( wxColour( 0, 0, 128 ) );
		style->selectedBorderColour( wxColour( 0, 0, 128 ) );
		style->hilightColour( wxColour( 255, 192, 111 ) );
		style->bgBrush( *wxTRANSPARENT_BRUSH );

		break;

	default:

		break;
	}

	return style;
}
