/***************************************************************
 * Name:      VdkSidePanel.cpp
 * Purpose:   Code for VdkSidePanel implementation
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-04-08
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkSidePanel.h"
#include "VdkCtrlParserInfo.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT( VdkSidePanel )

VdkSidePanel::VdkSidePanel()
{
	SetAddinStyle( VCS_CTRL_CONTAINER );
}

void VdkSidePanel::Create(wxXmlNode* node)
{
	m_strCaption = XmlGetChildContent( node, L"cpation" );
	DoXrcCreate( node );

	ParseObjects( VdkCtrlParserInfo().
				  window( m_Window ).
				  parent( this ).
				  node( node ) );

	int dy = m_Font.GetPixelSize().y * 1.4;
	wxPoint pos;
	ChildIterator chd;
	for( chd = begin(); chd != end(); ++chd )
	{
		pos = chd->GetPosition();
		pos.y += dy;
		chd->Move( pos );
	}
}

void VdkSidePanel::Clone(VdkControl* o, VdkControl* parent)
{

}

void VdkSidePanel::DoDraw(wxDC& dc)
{
	if( !m_strCaption.empty() )
	{
		dc.SetFont( m_Font );

		int addin = m_Font.GetPixelSize().y * 0.2;
		dc.DrawText( m_strCaption, m_Rect.x + addin, 
			m_Rect.y + addin );
	}
}
