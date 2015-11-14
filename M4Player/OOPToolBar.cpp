/***************************************************************
 * Name:      OOPToolBar.cpp
 * Purpose:   OOPToolBar ʵ���ļ�
 * Author:    Ning (vanxining@139.com)
 * Created:   2010
 * Copyright: Wang Xiao Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPToolBar.h"

#include "VdkMenu.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT( OOPToolBar );

//////////////////////////////////////////////////////////////////////////

void OOPToolBar::Create(wxXmlNode* node)
{
	VdkMenuBar::Create( node );

	//-------------------------------------------------------

	wxBitmap normal;
	GetXrcImage( node, normal );
	m_normal.Set( normal, 7, 1 );

	wxString strFileName( XmlGetChildContent( node, L"hot_image" ) );
	if( !strFileName.IsEmpty() )
	{
		wxBitmap hot;

		LoadMaskBitmap( hot, GetFilePath( strFileName ) );
		m_hot.Set( hot, 7, 1 );
	}
}

VdkMenuBarEntry* OOPToolBar::CreateNewEntry(int& w, int& h)
{
	w = m_normal.GetWidth();
	h = m_normal.GetHeight();

	return new OOPToolBarEntry;
}

void OOPToolBar::UpdateEntry(wxDC& dc, unsigned eid, int state)
{
	wxRect rc( m_firstChild->GetRect() );
	VdkBitmapArray* ba = &m_normal;

	// ���û�й���������״̬λͼ������ť������1һ������
	int yAaddIn = 0;

	switch( state )
	{
	case HOVERING:
	case LEFT_DOWN:

		ba = &m_hot;
		if( !ba->IsOk() )
		{
			ba = &m_normal;
			yAaddIn = -1;
		}

		break;

	default:

		break;
	}

	ba->BlitTo( dc, eid, 0, 
				rc.x + eid * ba->GetWidth(), 
				rc.y + yAaddIn );
}

//////////////////////////////////////////////////////////////////////////

OOPToolBarEntry::OOPToolBarEntry()
{
	// ���������ش���ı䴰�ڴ�С�����ָ��ָ�
	RemoveStyle( VCS_ONESHOT_HOVERING );
}

void OOPToolBarEntry::DoEraseBackground(wxDC& dc, const wxRect& rc)
{
	static_cast< OOPToolBar* >( GetParent() )->UpdateEntry
		( dc, ( unsigned ) GetClientData(), GetButtonState() );
}

void OOPToolBarEntry::DoHandleMouseEvent(VdkMouseEvent& e)
{
	if( m_Window->TestState( VWST_DRAG_AND_RESIZING ) )
		m_Window->ResetRzCursor();

	VdkMenuBarEntry::DoHandleMouseEvent( e );
}
