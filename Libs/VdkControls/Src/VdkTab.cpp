#include "StdAfx.h"
#include "VdkTab.h"

#include "VdkWindow.h"
#include "VdkPanel.h"
#include "VdkToggleButton.h"
#include "VdkCtrlParserInfo.h"
#include "wxUtil.h"

#include <wx/bookctrl.h>
#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT( VdkTab );

VdkTab::VdkTab() : m_padding( 2 ), m_currToggled( NULL )
{
	SetAddinStyle( VTABS_TAB_BORDER );
}

void VdkTab::Create(wxXmlNode* node)
{
	DoXrcCreate( node );
	ParseObjects( VdkCtrlParserInfo().window( m_Window )
									 .node( node ).parent( this )
									 .callback( this ) );

	m_padding = XmlGetContentOfNum
		( FindChildNode( node, L"button-padding" ), m_padding );

	//====================================================

	Toggle( 0, NULL );
}

VdkTab::~VdkTab()
{
	ClearPtrContainer( m_pages );
}

VdkPanel* VdkTab::AddPage(const wxString& cap)
{
	wxASSERT( !m_pages.empty() );

	VdkToggleButton* tglbtn = new VdkToggleButton;
	tglbtn->EasyClone( m_pages[0]->btn );
	tglbtn->SetCaption( cap, NULL );
	tglbtn->Toggle( false, NULL );

	if( tglbtn->TestStyle( VBS_RESIZEABLE ) )
		tglbtn->Fit( NULL );

	VdkPanel* npanel = new VdkPanel;
	npanel->Create( m_Window, wxEmptyString, wxRect() );
	npanel->SetParent( this );
	m_Window->AddCtrl( npanel ); // TODO: 是否优化一下这个流程？

	//====================================================

	Notify( m_Window, L"VdkToggleButton", tglbtn );
	Notify( m_Window, L"VdkPanel", npanel );

	return npanel;
}

void VdkTab::RemovePage(int id, wxDC* pDC)
{
	PageInfo* pi = m_pages[id];

	//====================================================

	RemoveChild( pi->btn );
	delete pi->btn;

	RemoveChild( pi->panel );
	delete pi->panel;

	//====================================================

	int btnw = pi->btn->GetRect().width;

	for( size_t i = id + 1; i < m_pages.size(); i++ )
	{
		wxRect rc( m_pages[i]->btn->GetRect() );
		rc.x -= btnw + m_padding;

		m_pages[i]->btn->SetRect( rc, NULL );
	}

	m_pages.erase( m_pages.begin() + id );

	//====================================================

	if( pDC )
		Draw( *pDC );
}

void VdkTab::AddCtrlToPage(int page, VdkControl* pCtrl)
{
	wxASSERT( page >= 0 && page < GetPageCount() );

	pCtrl->SetParent( m_pages[page]->panel );
	m_Window->AddCtrl( pCtrl );
}

void VdkTab::OnToolBarEvents(VdkVObjEvent& e)
{
	VdkToggleButton* curr = e.GetCtrl< VdkToggleButton >();
	if((m_currToggled != curr))
	{
		wxDC* pDC = e.GetVObjDC();

		int lastindex = wxNOT_FOUND;
		if( m_currToggled )
		{
			m_currToggled->Toggle( false, pDC );

			lastindex = GetPageIdFromHandle( m_currToggled );
			VdkPanel* panel( m_pages[lastindex]->panel );
			if( panel )
				panel->Freeze();
		}

		m_currToggled = curr;
		m_currToggled->Toggle( true, pDC );

		int index = GetPageIdFromHandle( curr );
		VdkPanel* panel( m_pages[index]->panel );
		if( panel )
			panel->Thaw( pDC );

		SendWxEvent( index, lastindex );
	}
}

void VdkTab::DoHandleMouseEvent(VdkMouseEvent& e)
{
	// TODO: !!!!!!!!!!!
	//if( e.evtCode == DLEFT_DOWN )
	//{
	//	VdkVObjEvent e( m_id );
	//	e.SetInt( GetPageIdFromHandle( m_currToggled ) );

	//	wxPostEvent( m_WindowImpl, e );
	//}

	// TODO:
	//VdkVObjEvent tabEvent( m_id );
	//tabEvent.SetCtrl( this );
	//
	//tabEvent.SetKeyCode( e.evtCode ); // Very bad!

	//tabEvent.SetInt( index );
	//tabEvent.SetExtraLong( lastindex );

	//wxPostEvent( m_WindowImpl, tabEvent );
}

void VdkTab::Notify(VdkWindow*, const wxString& clName, VdkControl* pCtrl)
{
	if( clName == L"VdkToggleButton" )
	{
		PageInfo* pi = new PageInfo;
		m_pages.push_back( pi );

		pi->btn = (VdkToggleButton *) pCtrl;

		//=============================================

		wxCoord w, h;
		pi->btn->GetSize( &w, &h );

		wxCoord x = m_padding;
		if( m_pages.size() > 1 )
		{
			VdkToggleButton* prevButton = m_pages[m_pages.size() - 2]->btn;
			wxRect rcPrev( prevButton->GetRect() );
			x += rcPrev.x + rcPrev.width;
		}

		wxRect rcButton( x, 0, w, h );
		pi->btn->SetRect( rcButton );

		//=============================================

		pi->btn->SetAddinStyle( VCS_REDIRECT_TO_PARENT );
		pi->btn->SetOwnerControl( true );

		VdkCtrlId id = VdkGetUniqueId();
		pi->btn->SetID( id );
		m_WindowImpl->Bind( wxEVT_VOBJ, &VdkTab::OnToolBarEvents, this, id );
	}
	else if( clName == L"VdkPanel" )
	{
		wxASSERT( !m_pages.empty() );

		//=============================================

		PageInfo* pi = m_pages[m_pages.size() - 1];
		pi->panel = (VdkPanel *) pCtrl;

		// TODO:处理竖排时的情况
		int dY = pi->btn->GetRect().height;

		wxRect rcPanel( 0, dY, m_Rect.width, m_Rect.height - dY );
		pi->panel->SetRect( rcPanel );

		pi->panel->Freeze();
	}
}

int VdkTab::GetPageIdFromHandle(VdkToggleButton* pCtrl) const
{
	for( size_t i = 0; i < m_pages.size() ; i++ )
	{
		if( m_pages[i]->btn == pCtrl )
			return i;
	}

	return -1;
}

void VdkTab::Toggle(int id, wxDC* pDC)
{
	wxASSERT( id >= 0 && id < GetPageCount() );

	int lastid = wxNOT_FOUND;
	VdkToggleButton* newbutton = m_pages[id]->btn;
	if( m_currToggled )
	{
		// 假如相同就不要切换了
		if( m_currToggled == newbutton )
			return;

		m_currToggled->Toggle( false, pDC );

		lastid = GetPageIdFromHandle( m_currToggled );
		if( m_pages[lastid]->panel )
			m_pages[lastid]->panel->Freeze();
	}

	newbutton->Toggle( true, pDC );
	if( m_pages[id]->panel )
		m_pages[id]->panel->Thaw( pDC );

	m_currToggled = newbutton;

	//====================================================

	SendWxEvent( id, lastid );
	//FireTabEvent( pDC, id, lastid ); // TODO:
}

void VdkTab::DoDraw(wxDC& dc)
{
	if( !m_pages.empty() && TestStyle( VTABS_TAB_BORDER ) )
	{
		dc.SetPen( *wxGREY_PEN );

		// 顶部
		wxCoord y = m_Rect.y + m_pages[0]->btn->GetRect().height - 1;
		dc.DrawLine( m_Rect.x, y, m_Rect.x + m_Rect.width, y );
		
		// 底部
		y = m_Rect.GetBottom(); // TODO: 为什么不行的？
		dc.DrawLine( m_Rect.x, y, m_Rect.GetRight(), y );
	}
}

int VdkTab::GetValue() const
{
	return GetPageIdFromHandle( m_currToggled );
}

int VdkTab::GetPageCount() const
{
	return m_pages.size();
}

void VdkTab::SendWxEvent(int index, int lastindex) const
{
	wxBookCtrlEvent bce
		( wxEVT_COMMAND_BOOKCTRL_PAGE_CHANGED, m_id, index, lastindex );

	wxPostEvent( m_WindowImpl, bce );
}

//////////////////////////////////////////////////////////////////////////

VdkTabEvent::VdkTabEvent(VdkTab* tab, int index, int lastindex)
	: VdkVObjEvent( tab->GetID() ), m_index( index ), m_lastindex( lastindex )
{

}
