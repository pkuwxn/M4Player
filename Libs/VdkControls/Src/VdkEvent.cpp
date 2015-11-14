#include "StdAfx.h"
#include "VdkEvent.h"

#include "VdkDC.h"
#include "VdkControl.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

////////////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT( wxEVT_REDIRECT_CMD, wxCommandEvent );

void MouseHoldTimer::Notify()
{
	VdkWindow* win( m_pCtrl->GetVdkWindow() );

	if( win->TestState( VWST_MOUSE_HOLD_ON ) )
	{
		VdkDC vdc( win, win->Rect00(), NULL );
		VdkMouseEvent e( m_evtCode, m_Point, vdc );
		m_pCtrl->HandleMouseEvent( e );
	}
	else
		Stop();
}

void MouseHoldTimer::Start(VdkControl* pCtrl, int evtCode, const wxPoint& Point)
{
	wxASSERT_MSG( pCtrl->TestStyle( VCS_HONLD_ON ), 
		L"指定控件不支持持续按住鼠标左键。" );

	m_Point	= Point;
	m_evtCode = evtCode;
	m_pCtrl = pCtrl;

	wxTimer::Start( 100 );
}
//////////////////////////////////////////////////////////////////////////
#if 1
VdkEvent::VdkEvent(int e, wxDC& d)
		: evtCode( e ),
		  dc( d ),
		  controlDown( false ),
		  shiftDown( false ),
		  m_bSkipped( false )
{

}

VdkMouseEvent::VdkMouseEvent(wxMouseEvent& e, int code, wxDC& d)
			 : VdkEvent( code, d ),
			   mousePos( e.GetPosition() ),
			   m_nativeEventObj( &e )

{
	controlDown = e.m_controlDown;
	shiftDown = e.m_shiftDown;
}

VdkMouseEvent::VdkMouseEvent(int e, const wxPoint& m, wxDC& d)
			 : VdkEvent( e, d ),
			   mousePos( m ),
			   m_nativeEventObj( NULL )
{

}

VdkKeyEvent::VdkKeyEvent(VdkKeyEventType type, wxKeyEvent& e, wxDC& dcref)
		   : VdkEvent( type, dcref ),
		     m_keyCode( e.GetKeyCode() ),
			 m_nativeEventObj( &e )
{
	controlDown = e.m_controlDown;
	shiftDown = e.m_shiftDown;
}

VdkKeyEvent::VdkKeyEvent(VdkKeyEventType type, int keyCode, wxDC& dcref)
		  : VdkEvent( type, dcref ),
		    m_keyCode( keyCode ),
			m_nativeEventObj( NULL )
{

}
#endif
//////////////////////////////////////////////////////////////////////////

VdkEventFilter::VdkEventFilter(VdkWindow* win)
			  : m_win(win), m_paused(false)
{
	if( m_win )
		m_win->PushEventFilter(this);
}

VdkEventFilter::~VdkEventFilter()
{
	if( m_win )
		m_win->PopEventFilter(this, false);
}

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT( wxEVT_VOBJ, VdkVObjEvent );

VdkVObjEvent::VdkVObjEvent(int id)
	: wxCommandEvent( wxEVT_VOBJ, id ),
	  m_pCtrl( NULL ),
	  m_menu( NULL ),
	  m_menuItem( NULL ),
	  m_pDC( NULL ),
	  m_keyCode( 0 )
{

}

VdkVObjEvent::VdkVObjEvent(const VdkVObjEvent& other)
	: wxCommandEvent( other ),
	  m_pCtrl( other.m_pCtrl ),
	  m_menu( other.m_menu ),
	  m_menuItem( other.m_menuItem ),
	  m_pDC( other.m_pDC ),
	  m_keyCode( other.m_keyCode )
{

}

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT( wxEVT_VW_FIRST_SHOWN, VdkWindowFirstShownEvent );

VdkWindowFirstShownEvent::VdkWindowFirstShownEvent(VdkWindow* win)
	: wxCommandEvent( wxEVT_VW_FIRST_SHOWN, win->GetHandle()->GetId() ),
	  m_window( win )
{
	wxASSERT( m_window );
}

//////////////////////////////////////////////////////////////////////////

VdkNotify::VdkNotify(VdkCtrlNotifyCode notice)
	: m_notice( notice ),
	  m_int( 0 ),
	  m_long( 0 ),
	  m_wParam( 0 ),
	  m_lParam( 0 ),
	  m_pDC( NULL )
{

}

//////////////////////////////////////////////////////////////////////////

VdkMouseEventType FromStd(const wxMouseEvent& e)
{
	return NORMAL;
}
