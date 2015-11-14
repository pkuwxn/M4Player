#include "StdAfx.h"
#include "VdkHotArea.h"

#include "VdkWindow.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


IMPLEMENT_DYNAMIC_VOBJECT( VdkHotArea );

//////////////////////////////////////////////////////////////////////////

VdkHotArea::VdkHotArea() : m_cursorHand( wxCURSOR_HAND ) {}

void VdkHotArea::Create(wxXmlNode* node)
{
	wxXmlNode* chd( NULL );
	chd = FindChildNode( node, L"url" );
	if( chd )
		m_strUrl = chd->GetNodeContent();

	Create( m_Window, GetXrcName( node ), GetXrcRect( node ), m_strUrl );
}

void VdkHotArea::Create(VdkWindow* Window, 
						const wxString& strName, 
						const wxRect& rc, 
						const wxString& strUrl)
{
	m_strName = strName;
	m_Rect = rc;
	SetVdkWindow( Window );

	m_strUrl = strUrl;
	StartSense();
}

void VdkHotArea::StartSense()
{
	m_bHand = false;
}

void VdkHotArea::DoHandleMouseEvent(VdkMouseEvent& e)
{
	switch( e.evtCode )
	{
	case HOVERING:

		// TODO:
		m_bHand = true;
		m_Window->AssignCursor( m_cursorHand );

		break;

	case NORMAL:

		if( m_bHand )
		{
			m_bHand = false;
			m_Window->ResetCursor();
		}
		break;

	case LEFT_UP:

		if( !m_strUrl.empty() )
			wxLaunchDefaultBrowser( m_strUrl );

		break;

	default:

		break;
	}
}
