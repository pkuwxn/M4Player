/***************************************************************
 * Name:      VdkMenu.cpp
 * Purpose:   һ������ VdkWindow �ĸ���ʽ�Ի�˵�
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-02-26
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkMenu.h"
#include "VdkEvent.h"
#include "VdkWindowImpl.h"
#include "VdkCtrlParserInfo.h"
#include "VdkButton.h"
#include "VdkDC.h"
#include "wxUtil.h"

#include <wx/popupwin.h>
#include <wx/artprov.h>
#include <wx/sstream.h>

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

// TODO: �� wxGTK ���洰���������첽���ã����ܼ�ʱ����������
// ���ٴ���ʾʱ�����ϴε����ز�����û��ɣ���ɻ��ҡ�
#if !defined( __WXMSW__ )
#   define DESTORY_MENU_IMPL_AT_ONCE
#endif

#ifdef __WXMSW__
// Windows ר�õĵ��������࣬�����������ػ�ʱ�໥���ǵ�����
class PopupWindowMSW : public wxPopupWindow
{
public:

	// ���캯��
	PopupWindowMSW()
	{

	}

	// ʵ�ʹ�������
	bool Create(wxWindow* parent)
	{
		wxCHECK_MSG( parent, false, L"can't create wxWindow without parent" );

		// popup windows are created hidden by default
		Hide();

		//////////////////////////////////////////////////////////////////////////

		int style = wxFULL_REPAINT_ON_RESIZE | wxPOPUP_WINDOW;
		if( !CreateBase
			( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style ) )
		{
			return false;
		}

		parent->AddChild(this);

		WXDWORD exstyle;
		DWORD msflags = MSWGetCreateWindowFlags( &exstyle );

		// "#32768" �� Windows �²˵����ڵ���������
		if( !MSWCreate( L"#32768", NULL,
						wxDefaultPosition, wxDefaultSize, msflags, exstyle ) )
		{
			return false;
		}

		InheritAttributes();

		return true;
	}

	// return the style to be used for the popup windows
	virtual WXDWORD MSWGetStyle(long flags, WXDWORD *exstyle) const
	{
		// we only honour the border flags, the others don't make sense for us
		WXDWORD style = wxPopupWindow::MSWGetStyle(flags, exstyle);

		// WS_CLIPSIBLINGS ��һ��������ǽ���ػ����ǵĹؼ�
		return (style | WS_CLIPSIBLINGS);
	}
};
#endif // __WXMSW__

//////////////////////////////////////////////////////////////////////////
// ����ʽ�Ի�˵���ʵ�ֻ���
//
// �����ѹ�������ԼӶ���һ��̳С�ͬʱ�к���ֵ����⣬
// �� Windows �£�����ڵ����������ƶ�ʱ�����Զ������������ڵ�
// Z-Order�������ᵼ����ʾ���Ӵ��ڵ�һ�������ݱ��ƻ���
class VdkMenuImplBase :
#ifndef __WXMSW__
	public VdkWindowImpl< wxPopupWindow >
#else
	public VdkWindowImpl< PopupWindowMSW >
#endif // !__WXMSW__
{
public:

	// ���캯��
	VdkMenuImplBase(wxWindow* parent)
	{
#	ifndef __WXMSW__
		wxPopupWindow::Create( parent );
#	else
		PopupWindowMSW::Create( parent );
#	endif // !__WXMSW__

		// ���ò˵������϶�
		RemoveStyle( VWS_DRAGGABLE );
	}

private:

	// �ı䴰���С
	virtual void DoResize(int x, int y, int width, int height, 
						  int sizeFlags = wxSIZE_AUTO) {
		SetSize( x, y, width, height, sizeFlags );
	}

	// ��С��
	virtual void DoMinimize(){}
};

//////////////////////////////////////////////////////////////////////////
// �˵���ʵ�ʴ���
class VdkMenuImpl : public VdkMenuImplBase
{
public:

	// ���캯��
	VdkMenuImpl(VdkMenu* menu, wxWindow* parent)
		: VdkMenuImplBase( parent ), m_menu( menu ), m_currSubMenu( NULL )
	{
		SetAddinStyle( VWS_MENU_IMPL | VWS_DISMISS_BY_ESC );
#		ifdef __WXDEBUG__
		SetDebugCaption( L"VdkMenuImpl" );
#		endif // __WXDEBUG__

		// ���¼�������
		BindHandlers();

#		ifdef __WXMSW__
		if( m_menu->TestStyle( VMS_ERASE_ALL ) && IsDoubleBuffering() )
			m_panel->Bind( wxEVT_PAINT, &VdkMenuImpl::EraseAll, this );
#		endif // __WXMSW__
	}

	// ��ȡ�˵����
	VdkMenu* GetMenu() const { return m_menu; }

	// ���õ�ǰ������ʾ���Ӳ˵�
	void SetCurrSubMenu(VdkMenuItemImpl* subMenu) { m_currSubMenu = subMenu; }

	// ��ȡ��ǰ������ʾ���Ӳ˵�
	VdkMenuItemImpl* GetCurrSubMenu() const { return m_currSubMenu; }

private:

#	ifdef __WXMSW__
	void EraseAll(wxPaintEvent& e)
	{
		wxPaintDC dc( this );
		dc.SetBackground( GetBackgroundColour() );
		dc.Clear();

		Draw( dc );
	}
#	endif // __WXMSW__

	// ������߽���ɫ��
	virtual void EraseBackground(wxDC& dc, const wxRect& rc);

	//////////////////////////////////////////////////////////////////////////

	VdkMenu* m_menu;
	VdkMenuItemImpl* m_currSubMenu; //  ��ǰ������ʾ���Ӳ˵�
};

void VdkMenuImpl::EraseBackground(wxDC& dc, const wxRect& rc)
{
	// ��Ҫ��ձ������ɲ˵������
	if( rc == Rect00() )
	{
		VdkMenuImplBase::EraseBackground( dc, rc );

		// ���Ʋ˵��߿�
		dc.SetPen( m_menu->GetBorderPen() );
		DrawRectangle( dc, Rect00() );
		
		if( m_menu->TestStyle( VMS_BITMAP ) )
		{
			dc.SetPen( *wxTRANSPARENT_PEN );
			dc.SetBrush( m_menu->GetBitmapRegionBrush() );

			int border = m_menu->GetBorderWeight();
			dc.DrawRectangle( wxRect( border, border,
									  m_menu->GetBitmapRegionWidth(),
									  rc.height - border * 2 ) );
		}

		return;
	}

	// ʼ����Ҫ��ձ���
	VdkUtil::ClrBkGnd( dc, wxBrush( GetBackgroundColour() ), rc );
	
	if( m_menu->TestStyle( VMS_BITMAP ) )
	{
		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.SetBrush( m_menu->GetBitmapRegionBrush() );
		dc.DrawRectangle( wxRect( m_menu->GetBorderWeight(), rc.y,
								  m_menu->GetBitmapRegionWidth(),
								  rc.height ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// �˵����ʵ�ʿ���ʾ����
class VdkMenuItemImpl : public VdkButton
{
public:

	// ���캯��
	VdkMenuItemImpl(VdkMenu* menu, VdkMenuItem* item)
		: m_menu( menu ), m_item( item )
	{
		m_nPaddingX = m_menu->GetTextPaddingToLeft();
		SetAddinStyle( VCS_ERASE_BG );
	}

	// ��ȡ���˵����
	VdkMenu* GetParentMenu() const { return m_menu; }

	// ��ȡ�Ӳ˵����
	VdkMenu* GetSubMenu() const { return m_item->subMenu(); }

	// ��ȡ�˵�����
	VdkMenuItem* GetMenuItem() const { return m_item; }

private:

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	// ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	//////////////////////////////////////////////////////////////////////////

	VdkMenu* m_menu;
	VdkMenuItem* m_item;
};

void VdkMenuItemImpl::DoDraw(wxDC& dc)
{
	bool disabled = m_item->disabled();
	Enable( !disabled, NULL );

	//////////////////////////////////////////////////////////////////////////

	// ����ʱ�����Ƹ�������
	if( !disabled && GetButtonState() != NORMAL )
	{
		dc.SetBrush( m_menu->GetHilightBrush() );
		dc.SetPen( m_menu->GetHilightBorderPen() );

		wxRect rc( m_Rect );
		rc.Deflate( 1 ) ;
		if( m_menu->GetSubMainGap() < 0 ) // ��������
			rc.width += m_menu->GetSubMainGap();
		dc.DrawRectangle( rc );
	}

	//////////////////////////////////////////////////////////////////////////
	// �����ı�

	VdkButton::DoDraw( dc );
	DrawHotKeyCaption( m_item->caption(), dc,
		m_Rect.x + m_nPaddingX, m_Rect.y + m_nPaddingY );

	if( !m_item->accelString().IsEmpty() )
	{
		dc.DrawText( m_item->accelString(),
					 m_Rect.x + m_Rect.width -
								m_menu->GetRightArrowSize() -
								m_item->accelStringWidth(),
					 m_Rect.y + m_nPaddingY );
	}

	// �����Ҽ�ͷ
	if( m_item->subMenu() )
	{
		dc.DrawText( L"\x25B6",
					 m_Rect.x + m_Rect.width - m_menu->GetRightArrowSize(),
					 m_Rect.y + m_nPaddingY );
	}

	//////////////////////////////////////////////////////////////////////////
	// �������λͼ/�Թ�

	enum {
		CONST_LEFT_BITMAP_SIZE = 16, // ���Сλͼ�ĳߴ�
	};

	int bitmapWidth = m_menu->GetBitmapRegionWidth();
	int x = m_Rect.x + (bitmapWidth - CONST_LEFT_BITMAP_SIZE) / 2;
	int y = m_Rect.y + (m_menu->GetRowHeight() - CONST_LEFT_BITMAP_SIZE) / 2;

	if( m_item->checked() )
	{
		dc.SetPen( m_menu->GetHilightBorderPen() );

		if( GetButtonState() != NORMAL )
			dc.SetBrush( m_menu->GetCheckedHilightBrush() );
		else
			dc.SetBrush( m_menu->GetCheckedBrush() );

		int edgetSize = CONST_LEFT_BITMAP_SIZE + 4;
		dc.DrawRectangle( x - 2, y - 2, edgetSize, edgetSize );

		if( !m_item->isBitmapOk() )
			m_menu->DrawCheckMark( dc, x, y );
	}

	if( m_item->isBitmapOk() )
		m_item->bmpArrayID().BlitTo( dc, x, y );
}

void VdkMenuItemImpl::DoHandleMouseEvent(VdkMouseEvent& e)
{
	VdkMenu* subMenu = m_item->subMenu();
	VdkMenuImpl* menuImpl = (VdkMenuImpl *) m_WindowImpl;

	// �ڲ˵������϶�
	if( e.evtCode == DRAGGING )
	{
		if( HitTest( e.mousePos ) )
		{
			e.evtCode = HOVERING;
		}
		else
		{
			// ʧ�ǰ�˵���
			e.Skip( true );
			menuImpl->HilightCtrl( NULL, e.dc );

			return;
		}
	}

	VdkButton::DoHandleMouseEvent( e );

	//////////////////////////////////////////////////////////////////////////

	switch( e.evtCode )
	{
	case HOVERING:
		{
			VdkMenuItemImpl* subMenu0 = menuImpl->GetCurrSubMenu();
			if( subMenu0 && subMenu0 != this )
			{
				subMenu0->GetSubMenu()->Return();
				subMenu0->Update( NORMAL, &e.dc );
				menuImpl->SetCurrSubMenu( NULL );
			}

			if( subMenu )
			{
				if( !subMenu->IsShown() )
				{
					wxRect rc( m_WindowImpl->GetScreenRect() );
					subMenu->ShowContext( rc.x + rc.width + m_menu->GetSubMainGap(),
										  rc.y + GetAbsoluteRect().y,
										  NULL );

					menuImpl->SetCurrSubMenu( this );
				}
			}

		}

		break;

	case NORMAL:

		if( subMenu )
		{
			// ���ָ��������Ӳ˵���
			if( subMenu->ContainsPointerGlobally() )
			{
				Update( VdkButton::HOVERING, &e.dc );

				break;
			}

			// ����ָ���뿪�˵��������ֲ����Ӳ˵��
			// �ֻ����ƶ��������˵�����
			subMenu->Return();
			menuImpl->SetCurrSubMenu( NULL );
		}

		break;

	default:

		break;
	}
}

//////////////////////////////////////////////////////////////////////////

class VdkMenuItemSeperator : public VdkCtrlHandler
{
public:

	// ���캯��
	VdkMenuItemSeperator(VdkWindow* win, wxRect rc)
	{
		m_strName = L"VdkMenuItemSeperator";
		m_Rect = rc;
		SetVdkWindow( win );

		SetAddinStyle( VCS_IGNORE_ALL_EVENTS | VCS_ERASE_BG );
		m_Window->AddCtrl( this );
	}

private:

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc)
	{
		wxPen Pen( *wxGREY_PEN );
		Pen.SetWidth( 1 );
		dc.SetPen( Pen );

		int x = m_Rect.x + ((VdkMenuImpl* ) m_WindowImpl)->GetMenu()->
			GetTextPaddingToLeft();
		int y = m_Rect.y + (m_Rect.height - 2) / 2;
		dc.DrawLine( x, y, x + m_Rect.width, y );
	}
};

//////////////////////////////////////////////////////////////////////////

VdkCtrlId VdkMenu::m_fakeId( VdkGetUniqueId() );
const static int gs_invalidBestWidth = -1;
// ֪ͨ�����ڲ˵�������
NEW_EVENT( wxEVT_MENU_HID )

VdkMenu::VdkMenu(int mid, VdkWindow* win, VdkMenu* parent)
	   : m_window( win ),
	     m_parent( parent ),
		 m_id( mid ),
	     m_width( 0 ),
	     m_height( 0 ),
		 m_bestWidth( gs_invalidBestWidth ),
		 m_maxTextWidth( 0 ),
		 m_pRelativeCtrl( NULL ),
	     m_impl( this ),
		 m_sstyle( StyleFactory::GetStyle( MSOFFICE_2003 ) )
{
	wxASSERT( m_window );
	//////////////////////////////////////////////////////////////////////////

	// ����Ĭ�Ϸ��
	SetAddinStyle( VMS_BITMAP );
}

VdkMenu::~VdkMenu()
{
	m_items.DeleteContents( true );
	m_items.Clear();

	m_attached.DeleteContents( true );
	m_attached.Clear();

	m_window = NULL; // ָʾ�˵��ѱ�����
}

void VdkMenu::AppendItem(VdkMenuItem* pitem)
{
	int nCaptionWidth = 0;

	switch( pitem->type() )
	{
	case MIT_NORMAL:
		{
			// ��ȡ�����ı����
			wxMemoryDC mdc;
			if( !pitem->strAccel.IsEmpty() )
			{
				mdc.GetTextExtent( pitem->strAccel, &pitem->nAcceStrWidth,
					NULL, 0, 0, &m_sstyle->menuFont );

				// ��������Ҽ�ͷ֮��ļ�϶���
				pitem->nAcceStrWidth += m_sstyle->componentPaddingCount;
			}


			mdc.GetTextExtent( pitem->strCaption, &nCaptionWidth,
				NULL, 0, 0, &m_sstyle->menuFont );

			// ���ϱ����ı����Ҽ�ͷ֮��ļ�϶���
			nCaptionWidth += m_sstyle->componentPaddingCount +
							 pitem->nAcceStrWidth;

			break;
		}

	case MIT_USER_DEFINED:
		{

			break;
		}

	default:

		break;
	}

	if( nCaptionWidth > m_maxTextWidth )
		m_maxTextWidth = nCaptionWidth;

	//////////////////////////////////////////////////////////////////////////

	m_items.push_back( pitem );
}

void VdkMenu::AppendItem(const wxString& strCaption)
{
    VdkMenuItem* item = new VdkMenuItem();
    item->caption( strCaption );
    
	AppendItem( item );
}

void VdkMenu::AppendSeperator()
{
	m_items.push_back( new VdkMenuItem( MIT_SEPERATOR ) );
}

void VdkMenu::ShowContext(int x, int y, VdkControl* pCtrl)
{
	if( m_items.size() == 0 )
		return;

	m_impl.Implement();
	ImplementMenu();

	// ���Ͳ˵�Ԥ��ʾ�¼�
	SendPreOperateMessage( VMS_SEND_PRESHOW_MSG );

	//////////////////////////////////////////////////////////////////////////

	int w, h;
	wxDisplaySize( &w, &h );
	if( x + m_width > w ) // �ұ߷Ų���
	{
		if( m_parent )
		{
			int sx;
			VdkMenuImpl* rootmenu = GetRootMenu()->GetImpl();
			rootmenu->GetScreenPosition( &sx, NULL );

			x = sx - m_width + abs( GetSubMainGap() );
		}
		else
			x = w - m_width;
	}
	if( y + m_height > h )
		y = h - m_height;

	m_impl->Resize( m_width, m_height );
	m_impl->Move( x, y );

	//////////////////////////////////////////////////////////////////////////

	m_impl->Show();
	m_pRelativeCtrl = pCtrl;
}

void VdkMenu::ImplementMenu()
{
	VdkWindow* win = m_impl.GetImpl();
	if( IsInitialized() ) // ��Ȼ��ʼ��
		return;

	// ���Ͳ˵�Ԥ�����¼�
	SendPreOperateMessage( VMS_SEND_PRECREATE_MSG );

	// ������
	CalcMenuWidth();

	m_height = 0;
	int x = m_sstyle->borderWeight,
		y = 0,
		w = m_width - m_sstyle->borderWeight * 2,
		h = m_sstyle->rowHeight;

	wxString strDummy( L" " );
	VdkMenuItemImpl* impl = NULL;
	wxWindow* winimp = win->VdkWindow::GetHandle();
	VdkMenuItem* item = NULL;
	ItemIter i;
	for( i = m_items.begin(); i != m_items.end(); ++i )
	{
		item = *i;
		y = m_height + m_sstyle->borderWeight;

		switch( item->nType )
		{
			case MIT_NORMAL:

				impl = new VdkMenuItemImpl( this, item );
				impl->VdkButton::Create(  VdkButtonInitializer().
										  // α�ı��������Լ����ı�
										  caption( strDummy ).
										  textAlign( TEXT_ALIGN_LEFT ).
										  name( L"VdkMenuItemImpl" ).
										  id( m_fakeId ).
										  rect( wxRect( x, y, w, h ) ).
										  window( win ).
										  addToWindow( true )
										);

				if( item->disabled() )
					impl->Enable( false, NULL );

				winimp->Bind( wxEVT_VOBJ, &VdkMenu::OnSelect, m_fakeId );
				m_height += m_sstyle->rowHeight;

				break;

			case MIT_SEPERATOR:

				new VdkMenuItemSeperator
					( win, wxRect( x, y, w - GetTextPaddingToLeft() - 4, 4 ) );

				m_height += 4;

				break;

			case MIT_USER_DEFINED:
				{
					VdkMenuCtrlHandler* hdler = 
						reinterpret_cast< VdkMenuCtrlHandler* >
						( item->GetClientData() );

					wxASSERT( hdler );
					//////////////////////////////////////////////////////////

					VdkControl* pCtrl = NULL;
					pCtrl = hdler->Implement
						( win, wxPoint( x + GetTextPaddingToLeft(), y ) );

					if( pCtrl )
					{
						m_height = pCtrl->GetRect().GetBottom() + 1;

						int l, t, r, b;
						hdler->GetPaddings( &l, &t, &r, &b );

						m_height += b;
					}

					break;
				}

			default:

				break;
		}
	}

	m_height += m_sstyle->borderWeight * 2; // ��ӱ߿�

	// ���²˵�����
	if( !m_attached.empty() )
	{
		int dX = MeasureOnMenuCtrls();
		if( dX )
		{
			ItemIter i;
			VdkWindow::CtrlIter ci;
			VdkMenuItem* item;
			VdkControl* notUserDefined;

			for( i = begin(), ci = win->begin(); i != end(); ++i, ++ci )
			{
				// ����Ҫ���һ��ԭ��һ���˵���ֻ����һ��û�и��ؼ��� VdkControl
				while( (*ci)->GetParent() )
					++ci;

				item = *i;
				if( item->type() != MIT_USER_DEFINED )
				{
					notUserDefined = *ci;

					int w, h;
					notUserDefined->GetSize( &w, &h );
					notUserDefined->SetSize( w + dX, h );
				}
			}
		}
	}

	// Ĭ�ϱ���ɫ
	winimp->SetBackgroundColour( m_sstyle->bgColor );

	//////////////////////////////////////////////////////////////////////////

	SetAddinState( VMST_INITIALIZED );
}

void VdkMenu::OnSelect(VdkVObjEvent& e)
{
	VdkMenuItemImpl* itemImpl = e.GetCtrl< VdkMenuItemImpl >();
	if( !itemImpl->GetSubMenu() )
	{
		VdkMenu* menu = itemImpl->GetParentMenu();
		VdkMenuItem* item = itemImpl->GetMenuItem();
		if( item->checkable() )
			item->checked( !item->checked() );
        
		menu->FireEvent( item );
		// ȡ���˵�����ʾ
		menu->GetRootMenu()->Return();
	}
}

int VdkMenu::GetTextPaddingToLeft() const
{
	return GetTextPaddingToRight() +
		( TestStyle( VMS_BITMAP ) ? GetBitmapRegionWidth() : 0 );
}

int VdkMenu::GetTextPaddingToRight() const
{
	return wxNORMAL_FONT->GetPointSize() * m_sstyle->textPaddingCount;
}

bool VdkMenu::ContainsPointer(const wxPoint& mousePos) const
{
	if( mousePos.x > 0 && mousePos.x < m_width &&
		mousePos.y > 0 && mousePos.y < m_height )
	{
		return true;
	}

	return false;
}

bool VdkMenu::ContainsPointerGlobally() const
{
	VdkMenuImpl* implWin = m_impl.GetImpl();
	return implWin && implWin->GetScreenRect().
							   Contains( wxGetMousePosition() );
}

void VdkMenu::Return()
{
	// ��û��ʵ��
	if( !m_impl.GetImpl() )
		return;

	if( m_impl->GetCurrSubMenu() )
		m_impl->GetCurrSubMenu()->GetSubMenu()->Return();

	m_impl.Release( false );
	
	//////////////////////////////////////////////////////////////////////////

	// ����ѡ���¼����˵��������¼�
	VdkVObjEvent e( VEMC_MENU_HID );
	e.SetMenu( this );
	e.SetCtrl( m_pRelativeCtrl );
	m_pRelativeCtrl = NULL;
	
	// ����ȴ���˵��������¼��Ž��к�������
	// ����˵������ص�ǰ�˵�����ʾһ���µĲ˵���
	// ��ʱ VdkWindow �������ȷ��״̬���� VdkWindow ������Ϣʱ��
	// ���Բ��ܻḲ�ǵ���ǰ��״̬��ʹ��ǰ�˵����Ϊ NULL ��
	// Ҳ�ɣ�wxPostEvent( m_window->GetWindowHandle(), e );
	m_window->GetHandle()->ProcessWindowEvent( e );
}

VdkMenu* VdkMenu::GetRootMenu()
{
	if( m_parent )
	{
		VdkMenu* menu( m_parent );
		while( menu->GetParentMenu() )
			menu = menu->GetParentMenu();

		return menu;
	}

	return this;
}

void VdkMenu::CalcMenuWidth()
{
	/*   �ı��߾�				 �Ҽ�ͷ
	||__|_|___________________|_|_|| <-�߿�
	| Сλͼ	�ı�����	  �ı��߾�
	|__ �߿�
	*/

	m_width = m_maxTextWidth + m_sstyle->borderWeight * 2;

	if( !TestStyle( VMS_NO_EXTRA_SPACE ) )
	{	
		if( TestStyle( VMS_BITMAP ) )
			m_width += GetBitmapRegionWidth();

		m_width += GetTextPaddingToRight() * 2 +
				   m_sstyle->rightArrowSize;
	}

	if( m_bestWidth != gs_invalidBestWidth &&
		m_width < m_bestWidth )
	{
		m_width = m_bestWidth;
	}
}

void VdkMenu::SetBestWidth(int w)
{
	m_bestWidth = w;

	VdkWindow* win = m_impl.GetImpl();
	// ʵ�ִ��������Ȼ��ʼ��
	if( win && !win->TestState( VWST_INITING ) )
		CalcMenuWidth();
}

//////////////////////////////////////////////////////////////////////////

enum DestoryDelayTimeout
{
	DESTORY_AT_ONCE = 50, // ����ɾ��
	DELAY_DESTORY = // �ӳ�ɾ��
#ifdef __WXDEBUG__
		1000,
#else
		600000,
#endif // __WXDEBUG__
};

VdkMenu::VdkMenuPtr::VdkMenuPtr(VdkMenu* menu)
		  : m_menu( menu ), m_impl( NULL )
{

}

VdkMenuImpl* VdkMenu::VdkMenuPtr::operator->()
{
	Implement();
	return m_impl;
}

VdkMenu::VdkMenuPtr::~VdkMenuPtr()
{
	Release( true );
}

void VdkMenu::VdkMenuPtr::Release(bool delAtOnce)
{
	if( m_impl )
	{
#ifdef DESTORY_MENU_IMPL_AT_ONCE
        delAtOnce = true;
#endif
		if( delAtOnce )
		{
			// TODO����ʱ���ڱ��벻�����ڴ�������¼�����������¼�������жϣ���
			m_impl->Destroy();
		}
		else
		{
			m_impl->Hide();

			// ȡ��������
			VdkMenuItemImpl* itemImpl = m_impl->GetCurrSubMenu();
			if( itemImpl )
			{
				// �����ṩ��ȷ�� DC ������˫����λͼ�б���������Ǿɵ�
				VdkDC vdc( m_impl, itemImpl->GetAbsoluteRect(), NULL );
				itemImpl->Update( VdkButton::NORMAL, &vdc );
			}

			// ֻ��Ҫһ�� Timer �¼�
			Start( DELAY_DESTORY, true );
		}

	}
}

void VdkMenu::VdkMenuPtr::Implement()
{
	if( !m_impl )
	{
		wxWindow* parent = NULL;
		VdkMenu* menu0 = m_menu->GetParentMenu();
		if( menu0 ) parent = menu0->GetImpl();
		if( !parent )
			parent = m_menu->GetVdkWindow()->GetWindowHandle();

		m_impl = new VdkMenuImpl( m_menu, parent );
		m_impl->Bind( wxEVT_DESTROY, &VdkMenuPtr::OnImplDestroy, this );
	}
	else
	{
		// ����ʹ�õ�ǰʵ�崰�壬ֹͣ Timer
		Stop();
	}
}

bool VdkMenu::VdkMenuPtr::IsShown() const
{
	return m_impl != NULL && m_impl->IsShown();
}

void VdkMenu::VdkMenuPtr::Notify()
{
	Release( true );
}

void VdkMenu::VdkMenuPtr::OnImplDestroy(wxWindowDestroyEvent&)
{
	wxASSERT( m_impl );

	m_impl = NULL;
	m_menu->OnMenuImplDestroy();
}

void VdkMenu::FireEvent(VdkMenuItem* item)
{
	VdkVObjEvent* e = NULL;

	if( item && item->id() != wxID_ANY ) // ��Ϣʱ���ڲ˵����
		e = new VdkVObjEvent( item->id() );
	else if( m_id != wxID_ANY ) // ��Ϣ�ǹ��ڲ˵���
		e = new VdkVObjEvent( m_id );

	if( e )
	{
		e->SetMenu( this );
		e->SetMenuItem( item );
		wxQueueEvent( m_window->GetWindowHandle(), e );
	}
}

void VdkMenu::DrawCheckMark(wxDC& dc, int x, int y) const
{
	if( m_sstyle->checkMark.IsOk() )
		dc.DrawBitmap( m_sstyle->checkMark, x, y, true );
	else
		dc.DrawBitmap( wxArtProvider::GetBitmap( wxART_TICK_MARK, wxART_MENU),
			x, y, true );
}

bool VdkMenu::SelectStaticStyle(MenuStyle style)
{
	StaticStyle* ss = StyleFactory::GetStyle(style);
	if( ss )
	{
		m_sstyle = ss;
		return true;
	}

	return false;
}

VdkMenuItem* VdkMenu::FindCheckedItem()
{
	ItemIter i;
	for( i = m_items.begin(); i != m_items.end(); ++i )
	{
		if( (*i)->checked() )
			return *i;
	}

	return NULL;
}

void VdkMenu::SendPreOperateMessage(VdkMenuStyle style)
{
	if( TestStyle( style ) && m_id != wxID_ANY )
	{
		VdkVObjEvent e( m_id );
		e.SetMenu( this );
		m_window->GetWindowHandle()->ProcessWindowEvent( e );
	}
}

void VdkMenu::AttachCtrl(VdkMenuCtrlHandler* hdler)
{
	wxASSERT( hdler );

	m_attached.push_back( hdler );
	VdkMenuItem* item = new VdkMenuItem( MIT_USER_DEFINED );
	item->SetClientData( hdler );

	AppendItem( item );
}

void VdkMenu::DeAttachCtrl(VdkMenuCtrlHandler* hdler)
{
	wxASSERT( hdler );


}

int VdkMenu::MeasureOnMenuCtrls()
{
	int w = 0, tmp;
	VdkMenuCtrlHandler* hdler;
	OnMenuCtrlIter i;
	for( i = m_attached.begin(); i != m_attached.end(); ++i )
	{
		hdler = *i;
		tmp = hdler->GetCtrl()->GetRect().width;

		int l, t, r, b;
		hdler->GetPaddings( &l, &t, &r, &b );

		tmp += r;

		if( tmp > w )
			w = tmp;
	}

	if( w > m_maxTextWidth )
	{
		int dX = w - m_maxTextWidth;
		m_maxTextWidth = w;

		CalcMenuWidth();

		return dX;
	}

	return 0;
}

void VdkMenu::OnMenuImplDestroy()
{
	RemoveState( VMST_INITIALIZED );

	VdkMenuCtrlHandler* hdler;
	OnMenuCtrlIter i;
	for( i = m_attached.begin(); i != m_attached.end(); ++i )
	{
		hdler = *i;
		hdler->Reset();
	}
}

VdkWindow* VdkMenu::GetVdkWindowImpl() const
{
	return m_impl.GetImpl();
}

VdkMenu* VdkMenu::GetLastShownMenu() const
{
	VdkMenuImpl* winImpl = m_impl.GetImpl();
	if( winImpl )
	{
		VdkMenuItemImpl* itemImpl = winImpl->GetCurrSubMenu();
		if( itemImpl )
		{
			VdkMenu* shown = itemImpl->GetSubMenu();
			if( shown )
				return shown->GetLastShownMenu();
		}
		else
			return const_cast< VdkMenu* >( this );
	}

	return NULL;
}

size_t VdkMenu::GetItemCount() const
{
	return m_items.size();
}

bool VdkMenu::IsEmpty() const
{
	return m_items.empty();
}

VdkMenuItem* VdkMenu::GetItem(size_t i)
{
	return m_items[i];
}

VdkMenuItem* VdkMenu::FindItem(VdkCtrlId id)
{
	// TODO: δʵ��
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

void VdkMenuCtrlHandler::Reset()
{
	SaveState();
	m_pCtrl = NULL;
}

VdkControl* VdkMenuXrcCtrlWrapper::Implement(VdkWindow* menuWin, const wxPoint& pos)
{
	wxStringInputStream sstream( m_strXmlData );
	wxXmlDocument doc( sstream );
	wxXmlNode* root = doc.GetRoot();
	wxASSERT( root );

	VdkControl* pCtrl;
	m_pCtrl = pCtrl = VdkControl::ParseObjects( VdkCtrlParserInfo().
											    window( menuWin ).
											    node( root ) );

	while( pCtrl )
	{
		if( pos.x == 0 && pos.y == 0 )
			break;

		wxPoint pold( pCtrl->GetPosition() );
		pCtrl->Move( pold.x + pos.x + m_paddingLeft,
					 pold.y + pos.y + m_paddingRight );

		pCtrl = pCtrl->GetNext();
	}

	m_pCtrl->SetID( m_id );

	//////////////////////////////////////////////////////////////////////////

	RestoreState();
	return m_pCtrl;
}

VdkControl* VdkMenuCloneableCtrlWrapper::Implement
	(VdkWindow* menuWin, const wxPoint& pos)
{
	VdkControl* o = m_Window->FindCtrl( m_strName );
	wxASSERT( o );

	m_pCtrl = o->GetImitation();
	m_pCtrl->Clone( o, o->GetParent() );
	m_pCtrl->Move( pos );
	m_pCtrl->SetID( m_id );

	menuWin->AddCtrl( m_pCtrl );

	//////////////////////////////////////////////////////////////////////////

	RestoreState();
	return m_pCtrl;
}

//////////////////////////////////////////////////////////////////////////

VdkMenuItem::~VdkMenuItem()
{
	wxDELETE( pSubMenu );
}

VdkMenuItem& VdkMenuItem::checkable(bool b)
{
	if( b )
		SetAddinStyle( VMIS_CHECKABLE );
	else
		RemoveStyle( VMIS_CHECKABLE );

	return *this;
}

VdkMenuItem& VdkMenuItem::checked(bool b)
{
	if( !checkable() )
		checkable( true );

	if( b )
		SetAddinState( VMIST_CHECKED );
	else
		RemoveState( VMIST_CHECKED );

	return *this;
}

VdkMenuItem& VdkMenuItem::disabled(bool d)
{
	if( d != disabled() )
	{
		if( d )
			SetAddinState( VMIST_DISABLED );
		else
			RemoveState( VMIST_DISABLED );
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////////

VdkMenu::StaticStyle* VdkMenu::StyleFactory::GetStyle(MenuStyle style)
{
	switch( style )
	{
	case MSOFFICE_2003:
		return GetOffice2003Style();
	case TM2009:
		return GetTM2009Style();
	default:
		return NULL;
	}
}

void VdkMenu::StyleFactory::InitStyle(StaticStyle& sstyle)
{
	sstyle.rowHeight = 24;
	sstyle.borderWeight = 1; // �߿���
	sstyle.padding = 1; // ��������߿�Ŀ��
	sstyle.textPaddingCount = 1; // ��������߿�Ŀ��
	sstyle.bitmapRegion = 26;
	sstyle.rightArrowSize = 16; // �����Ӳ˵����ұ߼�ͷ���
	sstyle.subMainGap = -2; // �Ӳ˵��븸�˵�֮��ļ�϶���
	// �˵��ı�������֮��ļ�϶��ȣ����Ҽ�ͷ����ټ�֮��
	sstyle.componentPaddingCount = 2;

	sstyle.bgColor = *wxWHITE;
	sstyle.menuFont = *wxNORMAL_FONT;
	sstyle.bitmapRegionBrush.SetColour( *wxWHITE );
}

VdkMenu::StaticStyle* VdkMenu::StyleFactory::GetOffice2003Style()
{
	static VdkMenu::StaticStyle sstyle;
	if( !sstyle.bitmapRegionBrush.IsOk() )
	{
		InitStyle(sstyle);

		wxBitmap stipple( sstyle.bitmapRegion, 1 );
		wxMemoryDC mdc( stipple );
		mdc.GradientFillLinear( wxRect( 0, 0, stipple.GetWidth(), 1 ),
			wxColour(227,239,255), wxColour(142,179,231) );
		sstyle.bitmapRegionBrush.SetStipple( stipple );

		sstyle.borderPen.SetColour( wxColour(0,45,150) );
		sstyle.hilightBorderPen = *wxBLUE_PEN;

		sstyle.checkedBrush.SetColour( wxColour(255,197,118) );
		sstyle.checkedHilightBrush.SetColour( wxColour(254,128,62) );
		sstyle.hilightBrush.SetColour( wxColour(255,238,194) );
	}

	return &sstyle;
}

VdkMenu::StaticStyle* VdkMenu::StyleFactory::GetTM2009Style()
{
	static VdkMenu::StaticStyle sstyle;
	if( !sstyle.bitmapRegionBrush.IsOk() )
	{
		InitStyle(sstyle);

		sstyle.borderPen.SetColour( wxColour(131,165,187) );
		sstyle.hilightBorderPen = *wxTRANSPARENT_PEN;

		sstyle.checkedBrush.SetColour( wxColour(255,197,118) );
		sstyle.checkedHilightBrush.SetColour( wxColour(254,128,62) );
		sstyle.bitmapRegionBrush.SetColour( wxColour(213,226,236) );
		sstyle.hilightBrush.SetColour( wxColour(120,175,210) );
	}

	return &sstyle;
}

//////////////////////////////////////////////////////////////////////////

VdkMenu* GenerateMenu(int mid, VdkWindow* parent, const wxString& strMenuTree)
{
	VdkMenu* menu = new VdkMenu( mid, parent, NULL );
	wxString strItem;
	size_t nPos = strMenuTree.find( L';' );
	size_t nLastPos = 0;

	while( nPos != wxString::npos )
	{
		strItem = strMenuTree.Mid( nLastPos, nPos - nLastPos );
		if( strItem == L"<SEP>" )
		{
			menu->AppendSeperator();
	    }
		else
		{
		    menu->AppendItem( strItem );
		}

		nLastPos = nPos + 1;
		nPos = strMenuTree.find( L';', nLastPos );
	}

	strItem = strMenuTree.Mid( nLastPos, wxString::npos );
	if( !strItem.IsEmpty() )
		menu->AppendItem( strItem );

	return menu;
}

VdkMenu* GenerateMenu(VdkWindow* parent, const MenuGenerator& menuTree)
{
	VdkMenu* menu( new VdkMenu( menuTree.mid, parent, menuTree.parent ) );

	MenuGenItemMap::const_iterator i;
	for( i = menuTree.itemMap.begin(); i != menuTree.itemMap.end(); i++ )
	{
		if( i->second == L"<SEP>" )
			menu->AppendSeperator();
		else
			menu->AppendItem( &(new VdkMenuItem())->id( i->first ).
													caption(i->second) );
	}

	return menu;
}

void MenuGenerator::Clear()
{
	itemMap.clear();
	parent = NULL;
}

//////////////////////////////////////////////////////////////////////////

VDK_DEFINE_INTERNAL_WX_PTR_LIST( VdkMenu, item_list )
VDK_DEFINE_INTERNAL_WX_PTR_LIST( VdkMenu, OnMenuCtrls )
