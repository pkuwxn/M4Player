#include "StdAfx.h"
#include "VdkChoiceCtrl.h"

#include "VdkMenu.h"
#include "VdkButton.h"
#include "VdkWindow.h"
#include "VdkDC.h"
#include "wxUtil.h"
#include "Images/checkbox_border.xpm"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT( VdkChoiceCtrl );

enum {
	CNST_FIXED_HEIGHT = 22, // 控件的固定高度
	CNST_DROP_DOWN_EXPL = 4, // 右侧下拉按钮的切分小图数
};

VdkChoiceCtrl::VdkChoiceCtrl()
	: m_selected( wxNOT_FOUND ), m_lastSelected( wxNOT_FOUND )
{

}

void VdkChoiceCtrl::Create(wxXmlNode* node)
{
	Create( m_Window, GetXrcName( node ), GetXrcRect( node ), 
			GetXrcTextBlock( node ) );

	//-----------------------------------------
	// 初始值
	int defSel = XmlGetContentOfNum
		( FindChildNode( node, L"default" ), wxNOT_FOUND );

	if( defSel != wxNOT_FOUND )
	{
		Select( defSel, NULL );
	}
}

void VdkChoiceCtrl::Create(VdkWindow* Window,
						   const wxString& strName,
						   const wxRect& rc,
						   const wxString& strChoices)
{
	m_TextAlign = TEXT_ALIGN_LEFT;

	wxBitmap bg( CheckBox_xpm );
	VdkButton::Create( VdkButtonInitializer().
					   window( Window ).
					   name( strName ).
					   rect( rc ).
					   resizeable( true ).
					   tileLen( 15 ).
					   tileType( RESIZE_TYPE_TILE ).
					   bitmap( bg ).
					   explode( CNST_DROP_DOWN_EXPL ) );

	m_Rect.height = CNST_FIXED_HEIGHT;

	//------------------------------------------------

	VdkCtrlId menuId = VdkGetUniqueId(), id = wxID_ANY;
	if( !strChoices.IsEmpty() )
	{
		m_menu = GenerateMenu( menuId, m_Window, strChoices );
	}
	else
	{
		m_menu = new VdkMenu( menuId, m_Window, NULL );
	}

	VdkMenu::ItemIter_Const i( m_menu->begin() );
	for( ; i != m_menu->end(); ++i )
	{
		VdkMenuItem* item = *i;

		id = VdkGetUniqueId();
		item->id( id );
	}

	// 注意菜单事件发送到窗口，而不是其上的面板（假如有）
	m_Window->GetWindowHandle()->Bind( wxEVT_VOBJ, 
									   &VdkChoiceCtrl::OnDropDownMenu, 
									   this, menuId, id );

	//------------------------------------------------

	m_menu->SetBestWidth( m_Rect.width );
	m_menu->SelectStaticStyle( VdkMenu::TM2009 );
	m_menu->RemoveStyle( VMS_BITMAP );

	m_nPaddingX = m_menu->GetTextPaddingToLeft();

	//------------------------------------------------

	int w = m_Rect.width;
	if( w > (bg.GetWidth() / double( CNST_DROP_DOWN_EXPL )) )
	{
		Resize( w, 0 );
		m_bmPrimArray.Set( wxNullBitmap, CNST_DROP_DOWN_EXPL, 1 );
	}

	RemoveStyle( VBS_RESIZEABLE );

	//------------------------------------------------

	HookMouseEvent();
	Prepare( NULL );
}

void VdkChoiceCtrl::InsertItem(int index, const wxString& strLabel)
{
	m_menu->AppendItem( strLabel );
}

void VdkChoiceCtrl::Select(int index, wxDC* pDC)
{
	if( m_selected == index )
		return;

	if( index >= 0 )
	{
		VdkMenuItem* item = m_menu->GetItem( index );
		if( item )
		{
			m_strCaption = item->caption();
		}
		else
		{
			wxLogDebug( L"[VdkChoiceCtrl::Select]Invalid index: %d", index );
			return;
		}
	}
	else if( index == wxNOT_FOUND )
	{
		m_strCaption = wxEmptyString;
	}
	else
	{
		return;
	}

	if( pDC )
		Draw( *pDC );

	m_lastSelected = m_selected;
	m_selected = index;

	if( IsReadyForEvent() )
		FireEvent( pDC, NULL );
}

void VdkChoiceCtrl::OnDropDownMenu(VdkVObjEvent& e)
{
	VdkMenuItem* item = e.GetMenuItem();

	m_lastSelected = m_selected;
	m_selected = 0;

	VdkMenu::ItemIter i( m_menu->begin() );
	for( ; i != m_menu->end(); ++i, ++m_selected )
	{
		if( *i == item )
			break;
	}

	if( m_lastSelected == m_selected )
		return;

	m_strCaption = item->caption();
	VdkDC dc( m_Window, GetAbsoluteRect(), NULL );
	Draw( dc );

	if( IsReadyForEvent() )
		FireEvent( &dc, NULL );
}
