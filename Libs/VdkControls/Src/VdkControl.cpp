#include "StdAfx.h"
#include "VdkControl.h"

#include "VdkMenu.h"
#include "VdkWindow.h"
#include "VdkDC.h"
#include "VdkCtrlParserInfo.h"

#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

#define EMPTY_BLOCK

VdkControl* VdkControl::ms_pToolTipOnShow = NULL;
wxColour VdkControl::ms_maskColor;
wxString VdkControl::ms_rootPath;

void VdkControl::Initialize(const wxColour& maskColor, const wxString& rootPath)
{
	ms_maskColor = maskColor;
	ms_rootPath = rootPath;
}

VdkControl* VdkControl::ParseObjects(const VdkCtrlParserInfo& info)
{
	wxASSERT_MSG( info.Node, L"XML 节点为空。" );

	wxString className;
	VdkControl *pCtrl = NULL, *firstCtrl = NULL;
	wxXmlNode* objNode = info.Node->GetChildren();
	wxXmlNode* tabTraversalNode = NULL;

	while( objNode )
	{
		if( objNode->GetName().Cmp( L"object" ) == 0 && 
			objNode->GetType() != wxXML_COMMENT_NODE )
		{
			className = objNode->GetAttribute( L"class", wxEmptyString );
			pCtrl = (VdkControl *) VdkObject::CreateDynamicVObject( className );

			if( !pCtrl )
			{
				wxLogDebug(L"无法实例化 `%s` 类型。", className);
			}
			else
			{
				if( !firstCtrl )
					firstCtrl = pCtrl;

				// 基本信息
				pCtrl->m_Window = info.Window;
				pCtrl->m_WindowImpl = info.Window->GetHandle();

				if( info.Parent )
				{
					pCtrl->SetParent( info.Parent );
					pCtrl->m_Font = info.Parent->m_Font;
				}
				else
				{
					pCtrl->m_Font = pCtrl->m_WindowImpl->GetFont();
				}

				// TAB order
				tabTraversalNode = FindChildNode( objNode, L"tab-traversal" );
				if( tabTraversalNode )
				{
					if( XmlGetContentOfBoolean( tabTraversalNode ) )
						pCtrl->SetAddinStyle( VCS_TAB_TRAVERSAL );
				}

				pCtrl->Create( objNode );

				//-------------------------------------

				// 查找控件的 ID
				if( info.IDs )
				{
					IdInfoIter iter( info.IDs->find( pCtrl->m_strName ) );
					if( iter != info.IDs->end() )
					{
						pCtrl->m_id = (*iter).second->Id;

						VdkControl** ptr = (*iter).second->Ptr;
						if( ptr )
						{
							*ptr = pCtrl;
						}
					}
				}

				info.Window->AddCtrl( pCtrl );

				//-------------------------------------

				// 回调函数
				if( info.Callback )
					info.Callback->Notify( info.Window, className, pCtrl );
			}
		}

		objNode = objNode->GetNext();
	}

	return firstCtrl;
}

VdkControl::VdkControl()
		  : VdkStyleAndStateOwner( VCS_ONESHOT_HOVERING ),
			m_id( wxID_ANY ),
			m_parent( NULL ),
		    m_firstChild( NULL ),
		    m_prev( NULL ),
		    m_next( NULL ),
			m_align( 0 ),
		    m_Window( NULL ),
			m_WindowImpl( NULL ),
		    m_menu( NULL ),
			m_shown( true ),
			m_enabled( true ),
			m_focused( false ),
			m_freezed( false )
{
	wxASSERT( ms_maskColor.IsOk() );
}

VdkControl::~VdkControl()
{
	wxASSERT( m_Window );

	// 假如是正常的 RESET ，不许这么麻烦
	if( !m_Window->TestState( VWST_RESET ) )
	{
		VdkControl* chd = m_firstChild;
		while( chd )
			delete chd;

		m_Window->RemoveCtrl( this );
	}

	wxDELETE( m_menu );
}

void VdkControl::Clone(VdkControl* o, VdkControl* parent)
{
	m_Window = o->m_Window;
	m_WindowImpl = o->m_WindowImpl;

	if( !o->m_strName.IsEmpty() )
		m_strName = o->m_strName + L"_Cloned";

	m_align = o->m_align;
	m_strToolTip = o->m_strToolTip;
	m_Rect = o->m_Rect;
	m_Font = o->m_Font;

	SetState( o->GetState() | VCST_CLONING );
	SetStyle( o->GetStyle() );

	SetParent( parent );
	ChildIterator i;
	VdkControl* obj = NULL;
	for( i = o->begin(); i != o->end(); ++i )
	{
		obj = i->GetImitation();
		obj->Clone( i.get(), this );
		m_Window->AddCtrl( obj );
	}
}

void VdkControl::FireEvent(wxDC* pDC, void* extraData)
{
	if( IsReadyForEvent() )
	{
		VdkVObjEvent* e = new VdkVObjEvent( m_id );
		e->SetCtrl( this );
		e->SetClientData( extraData );

		// 必须保证 pDC 的有效性
		if( pDC && !TestStyle( VCS_DELAY_EVENT ) &&
			!m_Window->TestStyle( VWS_MENU_IMPL ) )
		{
			e->SetVObjDC( pDC );
			m_WindowImpl->ProcessWindowEvent( *e );

			delete e;
			e = NULL;
		}
		else
		{
			wxQueueEvent( m_WindowImpl, e );
		}
	}
}

void VdkControl::HandleNotify(const VdkNotify& notice)
{
	switch( notice.GetNotifyCode() )
	{
	case VCN_FOCUS:

		UpdateFocus( true, NULL );
		break;

	case VCN_LOST_FOCUS:

		UpdateFocus( false, NULL );
		break;

	default:

		break;
	}

	DoHandleNotify( notice );
}

void VdkControl::SetToolTip(const wxString& strToolTip, bool updateOnce)
{
	m_strToolTip = strToolTip;
	if( updateOnce )
		m_WindowImpl->SetToolTip( strToolTip );
}

void VdkControl::ResetToolTip()
{
	if( ms_pToolTipOnShow )
	{
		ms_pToolTipOnShow = NULL;
		m_WindowImpl->UnsetToolTip();
	}
}

void VdkControl::Show(bool bShow, wxDC* pDC)
{
	if( IsOnShow() == bShow )
		return;

#ifdef __WXDEBUG__
	if( bShow && m_parent && !m_parent->IsOnShow() )
	{
		wxASSERT( false );
	}
#endif // __WXDEBUG__

	m_shown = bShow;

	VdkNotify notice( m_shown ? VCN_SHOW : VCN_HIDE );
	notice.SetWparam( m_shown ? 1 : 0 );
	notice.SetLparam( long( pDC ) );
	HandleNotify( notice );

	VdkControl* child( m_firstChild );
	while( child )
	{
		child->Show( m_shown, pDC );
		child = child->GetNext();
	}

	if( pDC )
	{
		m_shown ? Draw( *pDC ) : // 擦除残留的背景
			EraseBackground( *pDC, m_Rect );
	}
}

void VdkControl::Freeze()
{
	if( IsFreezed() )
		return;

	m_freezed = true;

	//----------------------------------------------------------

	HandleNotify( VdkNotify( VCN_FREEZE ) );

	VdkControl* child( m_firstChild );
	while( child )
	{
		child->Freeze();
		child = child->GetNext();
	}
}

void VdkControl::Thaw(wxDC* pDC)
{
	if( !IsFreezed() )
		return;

	m_freezed = false;

	//----------------------------------------------------------

	VdkNotify n( VCN_THAW );
	n.SetVObjDC( pDC );
	HandleNotify( n );

	VdkControl* child( m_firstChild );
	while( child )
	{
		child->Thaw( pDC );
		child = child->GetNext();
	}

	if( pDC && IsOnShow() )
		Draw( *pDC );
}

void VdkControl::UpdateFocus(bool focus, wxDC* pDC)
{
	m_focused = focus;

	if( pDC )
	{
		Draw( *pDC );
	}
}

void VdkControl::Enable(bool bEnabled, wxDC* pDC)
{
	if( IsEnabled() == bEnabled )
		return;

	m_enabled = bEnabled;

	VdkNotify notice( m_enabled ? VCN_ENABLED : VCN_DISABLED );
	notice.SetVObjDC( pDC );
	HandleNotify( notice );

	//----------------------------------------------------------

	VdkControl* child( m_firstChild );
	while( child )
	{
		child->Enable( m_enabled, pDC );
		child = child->GetNext();
	}

	if( pDC )
	{
		Draw( *pDC );
	}
}

void VdkControl::SetMenuState(bool bMenuOnShow, wxDC* pDC)
{
	if( !bMenuOnShow )
	{
		VdkNotify n( VCN_MENU_HID );
		n.SetVObjDC( pDC );
		HandleNotify( n );
	}
}

bool VdkControl::IsMenuOnShow() const
{
	return m_menu && m_menu->IsShown();
}

void VdkControl::AddChild(VdkControl* chd)
{
	wxASSERT( chd );

	chd->m_parent = this;
	if( m_firstChild )
	{
		VdkControl* last(m_firstChild);
		while( true )
		{
			if( last == chd )
				return;

			if( last->m_next )
				last = last->m_next;
			else
				break;
		}

		last->SetNext(chd);
		chd->m_prev = last;
	}
	else
	{
		m_firstChild = chd;
	}
}

void VdkControl::SetID(VdkCtrlId id)
{
	m_id = id;
	HandleNotify( VdkNotify( VCN_ID_CHANGED ) );
}

void VdkControl::SetVdkWindow(VdkWindow* win)
{
    wxASSERT_MSG( win, L"父窗口不可为空。" );
    if( m_Window && m_WindowImpl )
        return;

    m_Window = win;
    m_WindowImpl = win->GetHandle();
	m_Font = m_WindowImpl->GetFont();
}

bool VdkControl::ReAttachToWindow(VdkWindow* win)
{
    wxASSERT_MSG( win, L"父窗口不可为空。" );
	m_Window = NULL;
	m_WindowImpl = NULL;

	SetVdkWindow( win );
	bool ret = DoReAttachToWindow();

	ChildIterator i;
	for( i = begin(); i != end(); ++i )
	{
		ret = i->ReAttachToWindow( win );
	}

	return ret;
}

void VdkControl::SetParent(VdkControl* parent)
{
	m_parent = parent;
	if( m_parent )
	{
		m_parent->AddChild( this );
	}
}

VdkControl* VdkControl::GetRootParent() const
{
	if( m_parent )
	{
		VdkControl* root = m_parent;
		while( root->m_parent )
			root = root->m_parent;

		return root;
	}

	return NULL;
}

void VdkControl::SetFont(const wxFont& font, wxDC* pDC)
{
	if( m_Font == font )
		return;

	m_Font = font;

	VdkControl* child( m_firstChild );
	while( child )
	{
		if( child->GetFont() == font )
		{
			child->SetFont( font, pDC );
		}

		child = child->GetNext();
	}

	VdkNotify n( VCN_FONT_CHANGED );
	n.SetVObjDC( pDC );
	HandleNotify( n );

	if( pDC )
	{
		Draw( *pDC );
	}
}

bool VdkControl::HitTest(const wxPoint& mousePos) const
{
	return GetAbsoluteRect().Contains( mousePos );
}

bool VdkControl::ContainsPointerGlobally() const
{
    // GTK 下可能会引发一个 ASSERT(此时窗口尚未创建出来)
    if( m_Window->IsInitializing() )
        return false;

	int x, y;
	wxGetMousePosition( &x, &y );
	m_WindowImpl->ScreenToClient( &x, &y );

	return HitTest( wxPoint( x, y ) );
}

wxRect VdkControl::TranslateRect(const wxRect& rc) const
{
	const VdkControl* pCtrl = this;
	wxRect ret( rc );

	while( pCtrl )
	{
		// 传入来的 rc 是一个子控件的作用域
		if( pCtrl != this )
		{
			ret.x += pCtrl->m_Rect.x;
			ret.y += pCtrl->m_Rect.y;
		}

		pCtrl = pCtrl->m_parent;
	}

	return ret;
}

wxRect VdkControl::DeTranslateRect(const wxRect& rc) const
{
	if( !m_parent )
		return rc;

	wxRect rcParent(m_parent->GetAbsoluteRect() );
	return wxRect( rc.x - rcParent.x, rc.y - rcParent.y, 
				   rc.width, rc.height);
}

wxPoint VdkControl::TranslatePoint(const wxPoint& p) const
{
	wxRect rc( p.x, p.y, 1, 1 );
	rc = TranslateRect( rc );

	return rc.GetPosition();
}

wxPoint VdkControl::DeTranslatePoint(const wxPoint& p) const
{
	wxRect rc( GetAbsoluteRect() );
	return wxPoint( p.x - rc.x, p.y - rc.y );
}

void VdkControl::TranslateDC(wxDC& dc) const
{
	if( m_parent )
	{
		int x, y;
		dc.GetDeviceOrigin( &x, &y );

		wxRect rc( m_parent->GetAbsoluteRect() );
		dc.SetDeviceOrigin( x + rc.x, y + rc.y );
	}
}

void VdkControl::EraseBackground(wxDC& dc, const wxRect& rc)
{
	if( m_parent )
	{
		wxRect rcParent( m_parent->GetRect() );
		wxRect rc2( rcParent.x + rc.x,
					rcParent.y + rc.y,
					rc.width,
					rc.height );

		m_parent->EraseBackground( dc, rc2 );
	}
	else
	{
		m_Window->EraseBackground(dc, rc);
	}

	if( m_Rect.Intersects( rc ) )
	{
		VdkDcDeviceOriginSaver saver( dc );
		if( m_parent )
			TranslateDC( dc );

		DoEraseBackground( dc, rc );
	}
}

void VdkControl::Draw(wxDC& dc)
{
	if( !IsShown() )
		return;

	EMPTY_BLOCK
	{
		VdkDcDeviceOriginSaver saver( dc );

		if( TestStyle( VCS_ERASE_BG ) )
			EraseBackground( dc, GetRect() );

		if( m_parent )
			TranslateDC( dc );
		/*
		// 绘制具有焦点时的边框
		if( TestState( VCST_ONFOCUS ) )
		{
			static wxPen dashPen( *wxBLACK_DASHED_PEN );
			dc.SetPen( dashPen );
			dc.SetBrush( *wxTRANSPARENT_BRUSH );

			wxRect rcFocus( m_Rect );
			dc.DrawRectangle( m_Rect );
		} //*/

		DoDraw( dc );
	}

	VdkControl* chd( m_firstChild );
	while( chd )
	{
		if( chd->IsShown() )
			chd->Draw( dc );

		chd = chd->GetNext();
	}
}

void VdkControl::HandleMouseEvent(VdkMouseEvent& e)
{
	switch( e.evtCode )
	{
	case HOVERING:

		if( !m_strToolTip.IsEmpty() && ms_pToolTipOnShow != this )
		{
			m_WindowImpl->SetToolTip( m_strToolTip );
			ms_pToolTipOnShow = this;
		}

		if( TestStyle( VCS_HAND_CURSOR ) )
			m_Window->AssignCursor( wxCursor( wxCURSOR_HAND ) );

		break;

	case NORMAL:

		if( TestStyle( VCS_HAND_CURSOR ) )
			m_Window->ResetCursor();

	case LEFT_DOWN:
	case LEFT_UP:

		ResetToolTip();
		break;

	case DRAGGING:
		{
			// 我们不需要所有突如其来而无任何来由的拖动事件
			VdkMouseEventType lastMouseEvt = m_Window->GetLastMouseEvent();
			if( lastMouseEvt != LEFT_DOWN && lastMouseEvt != DRAGGING )
			{
				return;
			}

			break;
		}

	default:

		break;
	}

	DoHandleMouseEvent( e );

	//----------------------------------------------------------

	if( e.evtCode == DRAGGING &&
		e.GetSkipped() &&
	   !IsEnabled() && // 不可拖动已禁用的控件
	   !m_Window->CanDragBySpace() )
	{
		// 只有在窗口空白处才能拖动窗口
		e.Skip(false);
	}
}

void VdkControl::DoHandleMouseEvent(VdkMouseEvent& e)
{
	if( TestStyle( VCS_IGNORE_ALL_EVENTS ) )
		e.Skip( true );
}

void VdkControl::HandleKeyEvent(VdkKeyEvent& e)
{
	wxASSERT( TestStyle( VCS_KEY_EVENT ) );
	
	//----------------------------------------------------------

	DoHandleKeyEvent( e );
}

void VdkControl::HandleResize(int dX, int dY)
{
	if( !dX && !dY )
		return;

	wxRect rc( m_Rect );
	if( m_align != 0 )
	{
		if( dX && ( m_align & ALIGN_RIGHT ) )
			m_Rect.x += dX;

		if( dY && ( m_align & ALIGN_BOTTOM ) )
			m_Rect.y += dY;

		if( dX && ((m_align & ALIGN_CENTER_X) || (m_align & ALIGN_CENTER_X_Y)) )
			m_Rect.x += dX / 2;

		if( dY && ((m_align & ALIGN_CENTER_Y) || (m_align & ALIGN_CENTER_X_Y)) )
			m_Rect.y += dY / 2;

		if( dX && (( m_align & ALIGN_SYNC_X ) || ( m_align & ALIGN_SYNC_X_Y )) )
			m_Rect.width += dX;

		if( dY && (( m_align & ALIGN_SYNC_Y ) || ( m_align & ALIGN_SYNC_X_Y )) )
			m_Rect.height += dY;
	}

	if( m_firstChild )
	{
		// 修改为父控件的改变量
		int dX = m_Rect.width - rc.width;
		int dY = m_Rect.height - rc.height;

		if( dX || dY )
			HandleChildrenResize( dX, dY );
	}

	// 应该在一切就绪之后发送 VCN_WINDOW_RESIZED 事件
	// 例如滚动窗口在窗口大小改变之后更新滚动条高度，更新时滚动条应该应该要
	// 知悉窗口大小已经改变
	FireSizeNotify( dX, dY, false );

	int xPosChanged( m_Rect.x - rc.x );
	int yPosChanged( m_Rect.y - rc.y );
	if( xPosChanged || yPosChanged )
		FireMoveNotify( xPosChanged, yPosChanged );
}

void VdkControl::HandleChildrenResize(int dX, int dY)
{
	VdkControl* chd = m_firstChild;
	while( chd )
	{
		chd->HandleResize( dX, dY );
		chd = chd->GetNext();
	}
}

void VdkControl::HandleRelativeAlign(align_type align)
{
	if( align != vdkALIGN_INVALID )
	{
		m_align = align;

		int w, h;
		if( m_parent )
			m_parent->GetSize( &w, &h );
		else
			m_WindowImpl->GetSize( &w, &h );

		if( !w && !h )
			return;

		if( ( m_align & ALIGN_CENTER_X ) || ( m_align & ALIGN_CENTER_X_Y ) )
			m_Rect.x = ( w - m_Rect.width ) / 2;

		if( ( m_align & ALIGN_CENTER_Y ) || ( m_align & ALIGN_CENTER_X_Y ) )
			m_Rect.y = ( h - m_Rect.height ) / 2;
	}
}

void VdkControl::DoXrcCreate(wxXmlNode* node)
{
	m_Rect = GetXrcRect( node );
	m_strName = GetXrcName( node );
	HandleRelativeAlign( GetXrcAlign( node ) );

	TextInfo ti = GetXrcTextInfo( node, m_WindowImpl );
	m_Font = ti.font;
}

void VdkControl::GetPosition(int* x, int* y) const
{
	if( x ) *x = m_Rect.width;
	if( y ) *y = m_Rect.height;
}

void VdkControl::GetSize(int* w, int* h) const
{
	if( w ) *w = m_Rect.width;
	if( h ) *h = m_Rect.height;
}

void VdkControl::SetRect(int x, int y, int w, int h, wxDC* pDC)
{
	int dX( w - m_Rect.width );
	int dY( h - m_Rect.height );
	int xPosChanged( x - m_Rect.x );
	int yPosChanged( y - m_Rect.y );

	// 清除旧背景
	if( pDC )
		EraseBackground( *pDC, m_Rect );

	m_Rect.x = x;
	m_Rect.y = y;
	m_Rect.width = w;
	m_Rect.height = h;

	if( dX || dY )
	{
		// 子控件不会自动进行适配
		HandleChildrenResize( dX, dY );
		FireSizeNotify( dX, dY, true, pDC );
	}

	if( xPosChanged || yPosChanged )
		FireMoveNotify( xPosChanged, yPosChanged );

	// 绘制新的内容
	if( pDC )
		Draw( *pDC );
}

void VdkControl::FireSizeNotify(int dX, int dY,
	bool fromSetRect, wxDC* pDC)
{
	VdkNotify notice( fromSetRect ? VCN_SIZE_CHANGED :
		VCN_WINDOW_RESIZED );
	notice.SetWparam( dX );
	notice.SetLparam( dY );
	notice.SetVObjDC( pDC );

	HandleNotify( notice );
}

void VdkControl::FireMoveNotify(int xPosChanged, int yPosChanged)
{
	VdkNotify notice( VCN_POSITION_CHANGED );
	notice.SetWparam( xPosChanged );
	notice.SetLparam( yPosChanged );
	HandleNotify( notice );
}

void VdkControl::AttachMenu(VdkMenu* menu)
{
	if( m_menu )
		delete m_menu;

	m_menu =menu;
}

void VdkControl::RemoveChild(VdkControl* pCtrl)
{
	VdkControl* chd = m_firstChild;
	while( chd )
	{
		if( chd == pCtrl )
		{
			if( chd == m_firstChild )
			{
				m_firstChild = chd->m_next;
			}
			else
			{
				chd->m_prev->m_next = chd->m_next;
				if( chd->m_next )
					chd->m_next->m_prev = chd->m_prev;
			}

			pCtrl->m_prev = pCtrl->m_next = NULL;

			break;
		}

		chd = chd->m_next;
	}
}

bool VdkControl::IsShownOnScreen() const
{
	return IsShown() && m_WindowImpl->IsShownOnScreen();
}

bool VdkControl::IsShown() const
{
	// TODO:
	//wxLogDebug(L"IsOnShow(): %d, IsFreezed(): %d", IsOnShow(), IsFreezed());
	return IsOnShow() && !IsFreezed();
}

bool VdkControl::IsOnShow() const
{
	return m_shown;
}

//////////////////////////////////////////////////////////////////////////

bool operator == (const VdkControl::ChildIterator& lhs, const VdkControl::ChildIterator& rhs)
{
	return lhs.m_curr == rhs.m_curr;
}

bool operator != (const VdkControl::ChildIterator& lhs, const VdkControl::ChildIterator& rhs)
{
	return lhs.m_curr != rhs.m_curr;
}

void VdkControl::ChildIterator::Init()
{
	m_depot = m_curr = NULL;
}

VdkControl::ChildIterator::ChildIterator(VdkControl* pCtrl)
{
	Init();

	m_depot = pCtrl;
	if( m_depot )
		m_curr = m_depot->GetFirstChild();
}

VdkControl::ChildIterator& VdkControl::ChildIterator::operator ++ ()
{
	m_curr = m_curr->GetNext();
	return *this;
}

VdkControl::ChildIterator VdkControl::ChildIterator::operator ++(int)
{
	ChildIterator temp(*this);
	++*this;

	return temp;
}

VdkControl& VdkControl::ChildIterator::operator*()
{
	return *m_curr;
}

VdkControl* VdkControl::ChildIterator::operator->()
{
	return m_curr;
}

VdkControl::ChildIterator& VdkControl::ChildIterator::operator --()
{
	return *this;
}

VdkControl::ChildIterator VdkControl::ChildIterator::operator --(int)
{
	ChildIterator temp( *this );
	--*this;

	return temp;
}

//////////////////////////////////////////////////////////////////////////

VdkCtrlHandler::VdkCtrlHandler()
{
	SetAddinStyle( VCS_HANDLER );
}

//////////////////////////////////////////////////////////////////////////

wxBitmap* VdkBitmapArray::Explode() const
{
	return VdkControl::ExplodeBitmap( m_bmp, m_cols );
}
