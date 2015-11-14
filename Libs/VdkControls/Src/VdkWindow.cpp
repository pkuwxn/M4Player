/***************************************************************
 * Name:      VdkWindow.cpp
 * Purpose:   VdkWindow ��ʵ���ļ�
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-02-13
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkWindow.h"

#include "VdkControl.h"
#include "VdkCtrlParserInfo.h"
#include "VdkMenu.h"
#include "VdkEvent.h"
#include "VdkDC.h"

#include "wxUtil.h"

#ifdef __WXMSW__
#	include "msw/TrayIcon.h"
#else
/// \brief ԭ�� wxTrayIcon ����չ�࣬�����ͼ����˸������
class TrayIcon {};
#endif // __WXMSW__

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkWindowInitializer::VdkWindowInitializer()
   : BgColor( *wxWHITE ),
     bResizeable( true ),
	 ResizeType( RESIZE_TYPE_TILE )
{

}

//////////////////////////////////////////////////////////////////////////
// TAB order ����

typedef unsigned int tab_order_t;

class TabOrderIterator
{
public:

	// ���һ���ؼ�
	void AddCtrl(VdkControl* pCtrl);

	// ɾ��һ���ؼ�
	bool RemoveCtrl(VdkControl* pCtrl);

	/// ���������ע��Ŀؼ�
	void Clear();

	// ���� TAB order ���е���һ���ؼ���ת�ƽ��㣩
	//
	// @param current ��Ϊ�գ���ἤ��ؼ����еĵ�һ��
	// @return ���ɹ���������е���һ���ؼ��򷵻��档
	bool FocusNext(VdkControl* current);

private:

	// ����ָ������Ŀؼ�
	VdkCtrlIter FindCtrl(VdkControl* pCtrl);

private:

	VdkCtrlList m_ctrlsOfTabTraversal;
};

void TabOrderIterator::AddCtrl(VdkControl* pCtrl)
{
	wxASSERT( pCtrl->TestStyle( VCS_TAB_TRAVERSAL ) );

	if( FindCtrl( pCtrl ) == m_ctrlsOfTabTraversal.end() )
		m_ctrlsOfTabTraversal.push_back( pCtrl );
}

bool TabOrderIterator::RemoveCtrl(VdkControl* pCtrl)
{
	return m_ctrlsOfTabTraversal.DeleteObject( pCtrl );
}

void TabOrderIterator::Clear()
{
	m_ctrlsOfTabTraversal.clear();
}

bool TabOrderIterator::FocusNext(VdkControl* current)
{
	if( m_ctrlsOfTabTraversal.empty() )
		return false;

	VdkCtrlIter beg( m_ctrlsOfTabTraversal.begin() );
	VdkCtrlIter end( m_ctrlsOfTabTraversal.end() );
	VdkCtrlIter iter( end );

	if( current )
	{
		iter = FindCtrl( current );
		if( iter != end )
		{
			++iter;
		}

		// TODO: ���統ǰ�ؼ�û�����ÿ�ת�ƽ������δ���
	}

	if( iter == end )
	{
		iter = beg;
	}

	VdkCtrlList::size_type numCtrls = m_ctrlsOfTabTraversal.size();
	size_t tries = 0;

	while( tries++ < numCtrls )
	{
		VdkControl* pCtrl = *iter;

		if( pCtrl == current )
			continue;

		if( pCtrl->IsShown() )
		{
			VdkWindow* win = pCtrl->GetVdkWindow();
			VdkDC dc( win, win->Rect00(), NULL );

			win->FocusCtrl( pCtrl, &dc );
			return true;
		}

		//-----------------------------------------------------
		// ��������ȥ

		++iter;

		// ѭ��
		if( iter == end )
		{
			iter = beg;
		}
	}

	return false;
}

VdkCtrlIter TabOrderIterator::FindCtrl(VdkControl* pCtrl)
{
	VdkCtrlIter iter( m_ctrlsOfTabTraversal.begin() );
	for( ; iter != m_ctrlsOfTabTraversal.end(); ++iter )
	{
		if( *iter == pCtrl )
			break;
	}

	return iter;
}

//////////////////////////////////////////////////////////////////////////

VDK_DEFINE_WX_PTR_LIST( VdkCtrlList );
VDK_DEFINE_WX_PTR_LIST( VdkWindowList );
VDK_DEFINE_WX_PTR_LIST( VdkEventFilterList );

enum {
	// ���ָ���Ƴ���ǰ����ʱ���͵����ڵ�α���λ�õ�����ֵ
	MEANNINGLESS_MOUSE_COORD = 9999999,
};

// ȫ�־�̬��������ǰ�������ָ��֮�µĿؼ�
VdkControl* VdkWindow::ms_hovering = NULL;

//////////////////////////////////////////////////////////////////////////

VdkWindow::VdkWindow(wxWindow* handle, long style)
		 : VdkStyleAndStateOwner( style, VWST_INITING ),
		   m_this( handle ),
		   m_panel( m_this ),
		   m_hiddenCtrl( NULL ),
		   m_focus( NULL ),
		   m_menu( NULL ),
		   m_trayIcon( NULL ),
		   m_dragBySpace( false ),
		   m_tabOrderIter( NULL ),
		   m_cachedDC( NULL ),
		   m_postListeners( NULL )
		   // ������Ա������ Init() �г�ʼ��
		   // TODO: ��ЩӦ���� Init() �г�ʼ����
{
	wxASSERT( m_this );
	Init();

#ifdef __WXMSW__
	m_this->SetFont( wxEasyCreatFont( L"����" ) );
#endif

	//-----------------------------------------------------------------

	// TODO: �û��ı�����������ô�죿
	m_dragBySpace = TestStyle( VWIS_DRAG_BY_SPACE );
}

void VdkWindow::Reset()
{
	m_mouseOn.x = m_mouseOn.y = 0;

	//-----------------------------------------------------------------

	if( m_pMouseHoldTimer )
	{
		m_pMouseHoldTimer->Stop();
		delete m_pMouseHoldTimer;
	}

	if( ms_hovering && ms_hovering->GetVdkWindow() == this )
		ResetCtrlOnHover();

	ResetCtrlOnFocus();

	VdkMenu* menuOnShow = GetMenuOnShow();
	if( menuOnShow && menuOnShow->GetVdkWindow() == this )
		HideMenu();

	SetAddinState( VWST_RESET );

	//-----------------------------------------------------------------

	m_Ctrls.DeleteContents( true );
	m_Ctrls.Clear();
	m_Ctrls.DeleteContents( false );

	if( m_tabOrderIter )
		m_tabOrderIter->Clear();

	m_EventFilters.Clear();

	//-----------------------------------------------------------------

	Init();
}

void VdkWindow::Init()
{
	m_nLastCtrlState = NORMAL;
	m_mouseEventNotForMe = false;
	m_nLastMouseEvent = NORMAL;
	m_pMouseHoldTimer = NULL;

	m_nMinWidth = -1;
	m_nMinHeight = -1;
	m_xDcOrigin = 0;
	m_yDcOrigin = 0;
}

#define BindVObjEvent( fx, mid ) m_panel->Bind( wxEVT_VOBJ, &VdkWindow::fx, this, mid )
#define BindToPanel( type, fx ) m_panel->Bind( type, &VdkWindow::fx, this )
#define BindToWindow( type, fx ) m_this->Bind( type, &VdkWindow::fx, this )

void VdkWindow::BindHandlers()
{
	// ���細�廹û��ʼ����ɣ���δ���Ƶ�һ�Σ���
	// �Ҳ��Ǵ������ô���Ĺ���֮��
	// ���Բ��ܰ�������ͬ���¼�������
	if( IsInitializing() && !TestState( VWST_RESET ) )
	{
		// Windows �� GTK �Լ����¼��Ĵ���������ͬ��
		// Windows ����ͨ�� base panel ֱ�ӽ��ռ����¼��ܲ����ף���˲�ʹ��
		// base panel ���ػ񰴼��¼�������ʹ��һ�����ص� wxControl ��ʵ����
		// GTK ��ʹ�����ؿؼ��ķ������ػ񰴼��¼���������Ҫ�ı����뷨״̬����
		// ��λ��ʱȴ�ֲ������ˣ��޷������ֻ��ʹ�� base panel ���������

#ifndef __WXMSW__
		if( TestStyle( VWS_BASE_PANEL ) )
		{
			m_panel = new wxPanel( m_this, wxID_ANY,
								   wxDefaultPosition, wxDefaultSize,
								   wxTAB_TRAVERSAL |
								   wxNO_BORDER |
								   wxWANTS_CHARS );

			BindToPanel( wxEVT_KEY_DOWN, OnKeyDown );
			BindToPanel( wxEVT_KEY_UP, OnKeyUp );
			BindToPanel( wxEVT_CHAR, OnChars );

			// �������Ĭ�ϱ���
            m_panel->SetBackgroundStyle( wxBG_STYLE_CUSTOM );

			// ������˸
			BindToWindow( wxEVT_ERASE_BACKGROUND, OnEraseBackground );
			BindToWindow( wxEVT_PAINT, OnWindowPaintDummy );
		}
#endif

		//-----------------------------------------------------------------

        m_this->SetBackgroundStyle( wxBG_STYLE_CUSTOM ); // �������Ĭ�ϱ���

		BindToPanel( wxEVT_LEFT_DOWN, OnLeftDown );
		BindToPanel( wxEVT_LEFT_DCLICK, OnDLeftDown );
		BindToPanel( wxEVT_LEFT_UP, OnLeftUp );
		BindToPanel( wxEVT_RIGHT_UP, OnRightUp );
		BindToPanel( wxEVT_MOTION, OnMouseMove );
		BindToPanel( wxEVT_MOUSE_CAPTURE_LOST, OnMouseLost );
		BindToPanel( wxEVT_LEAVE_WINDOW, OnLeaveWindow );

#ifdef __WXMSW__
		// �ܶ�ʱ����ͨ����Ҳ��Ҫ�����¼�����ȡ�������Ĳ˵�����ʾ
		BindToPanel( wxEVT_KEY_DOWN, OnKeyDown );
		BindToPanel( wxEVT_KEY_UP, OnKeyUp );
#endif

		BindToPanel( wxEVT_ERASE_BACKGROUND, OnEraseBackground );
		BindToPanel( wxEVT_PAINT, OnPaint );

		BindToWindow( wxEVT_MOUSEWHEEL, OnMouseWheel ); // TODO: Why here?
		BindToWindow( wxEVT_SHOW, OnShow );
		BindToWindow( wxEVT_ACTIVATE, OnWindowFocus );
		if( TestStyle( VWS_CLOSE_TO_DESTORY ) )
			BindToWindow( wxEVT_CLOSE_WINDOW, OnClose );

		// �ӳ���ʾ�˵�
		BindVObjEvent( OnDelayShowContextMenu, VEMC_SHOW_CONTEXT_MENU );

		// ֪���˵�����
		BindVObjEvent( OnMenuHid, VEMC_MENU_HID );
	}
}

#undef BindVObjEvent
#undef BindToPanel
#undef BindToWindow

void VdkWindow::Create(const VdkWindowInitializer& init_data)
{
	BindHandlers();

	//-----------------------------------------------------------------

	wxBitmap bkCanvas;
	if( !init_data.bmBkGnd.IsOk() )
	{
		if( !init_data.strFileName.IsEmpty() )
		{
			// λͼĬ�ϲ�����͸��ɫ
			VdkUtil::ImRead( bkCanvas, init_data.strFileName );
		}
		else
		{
			m_this->SetBackgroundColour( init_data.BgColor );
		}
	}
	else
	{
		bkCanvas = init_data.bmBkGnd;
	}

	// ��ͨ�߿�û�б���λͼ
	if( !bkCanvas.IsOk() )
	{
		m_nMinWidth = init_data.Rect.width;
		m_nMinHeight = init_data.Rect.height;

		if( IsDoubleBuffering() )
		{
			m_bmBuffered = wxBitmap( init_data.Rect.width,
									 init_data.Rect.height );
		}

		if( !init_data.Rect.IsEmpty() )
			m_this->SetSize( init_data.Rect );
	}
	else
	{
		m_nMinWidth = bkCanvas.GetWidth();
		m_nMinHeight = bkCanvas.GetHeight();

		m_bkCanvas.Set( bkCanvas, init_data.ResizeType, init_data.TileArea );
		bkCanvas = wxNullBitmap;

		if( !init_data.TileArea.IsEmpty() )
		{
			if( init_data.bResizeable ) // nBorder > 0 ʱ�Ż����ÿɸı䴰�ڴ�С
				SetAddinStyle( VWS_RESIZEABLE );

			int w = init_data.Rect.width;
			int h = init_data.Rect.height;

			if( w < m_nMinWidth ) w = m_nMinWidth;
			if( h < m_nMinHeight ) h = m_nMinHeight;

			Resize( w, h );
		}
		else
		{
			RemoveStyle( VWS_RESIZEABLE );
			Resize( m_nMinWidth, m_nMinHeight );
		}

		// ���� init_data.Rect Ϊ�վͲ�Ҫ�ƶ�������
		if( !init_data.Rect.IsEmpty() && 
				init_data.Rect.GetPosition() != wxDefaultPosition )
		{
			m_this->Move( init_data.Rect.x, init_data.Rect.y );
		}

	}
}

VdkWindow* VdkWindow::Create(wxXmlNode* node, MapOfCtrlIdInfo* ids)
{
	wxXmlNode* chd = NULL;
	bool resizeable = true;
	wxRect ResizeRect;
	// TODO: ǧǧ����Ĭ��������
	VdkResizeableBitmapType nResizeType = RESIZE_TYPE_STRETCH;

	// ��������
	chd = FindChildNode( node, L"resize_rect" );
	if( chd )
	{
		wxString strTmp( chd->GetNodeContent() );
		ResizeRect = VdkControl::ParseRect( strTmp, PRT_RECTANGLE );

		if( ResizeRect.width == 0 || ResizeRect.height == 0 )
		{
			resizeable = false;
		}
		else
		{
			chd = FindChildNode( node, L"resize_tile" );
			if( XmlGetContentOfBoolean( chd, false ) )
				nResizeType = RESIZE_TYPE_TILE;

			chd = FindChildNode( node, L"resizeable" );
			if( !XmlGetContentOfBoolean( chd, true ) )
				resizeable = false;
		}
	}

	// ��������
	TextInfo info( VdkUtil::GetXrcTextInfo( node, m_this ) );
	m_this->SetFont( info.font );
	if( m_panel != m_this )
	{
		m_panel->SetFont( info.font );
	}

	//-----------------------------------------------------------------
	// ��ʼ������

	wxString bgFile( XmlGetChildContent( node, L"image" ) );
	Create(  VdkWindowInitializer().
			 fileName( VdkControl::GetFilePath( bgFile ) ).
			 rect( VdkControl::GetXrcRect( node ) ).
			 resizeable( resizeable ).
			 tileType( nResizeType ).
			 tileArea( ResizeRect ) );

	if( TestStyle( VWS_RESIZEABLE ) )
	{
		wxString strTmp( XmlGetChildContent( node, L"min-size" ) );
		if( !strTmp.IsEmpty() )
		{
			wxString::size_type pos( strTmp.find( ',' ) );
			if( pos != wxString::npos )
			{
				int w, h;
				w = wxAtoi( strTmp.Mid( 0, pos ) );
				h = wxAtoi( strTmp.Mid( ++pos ) );

				if( w > m_nMinWidth || h > m_nMinHeight )
					SetMinSize( w, h );
			}
		}
	}

	//-----------------------------------------------------------------
	// �������пؼ�

	VdkControl::ParseObjects( VdkCtrlParserInfo().
							  window( this ).
							  node( node ).
							  ids( ids ) );

	// ����ؼ���ַ
	if( ids )
	{
		VdkControl** addr = NULL;
		VdkControl* pCtrl = NULL;

		MapOfCtrlIdInfo::iterator i( ids->begin() );
		for( ; i != ids->end(); ++i )
		{
			pCtrl = FindCtrl( i->first );

			if( pCtrl )
			{
				addr = i->second->ptr();
				if( addr && !*addr )
					*addr = pCtrl;

				pCtrl->SetID( i->second->id() );
			}
		}
	}

	return this;
}

void VdkWindow::BeginExit()
{
	// ���绹û��ʼ���������Զ��˳�
	SetAddinState( VWST_EXITING );

	// TODO: FindMenu
	if( FindCtrl( ms_hovering ) )
		ResetCtrlOnHover();
}

VdkWindow::~VdkWindow()
{
	Reset();

	wxDELETE( m_tabOrderIter );
	wxDELETE( m_trayIcon );
	wxDELETE( m_menu );
	wxDELETE( m_postListeners );
}

void VdkWindow::RecoverCtrl(VdkControl* pCtrl)
{
	if( pCtrl )
	{
		wxPoint mousePos( MEANNINGLESS_MOUSE_COORD, MEANNINGLESS_MOUSE_COORD );
		VdkDC vdc( this, pCtrl->GetAbsoluteRect(), NULL );
		VdkMouseEvent fakeEvent( NORMAL, mousePos, vdc );

		pCtrl->HandleMouseEvent( fakeEvent );

		if( ms_hovering == pCtrl )
		{
			ResetCtrlOnHover();
		}
	}
}

void VdkWindow::HandleMouseHoldOn(int evtCode,
								  const wxPoint& Point,
								  VdkControl* pCtrl )
{
	if( evtCode != NORMAL )
	{
		SetAddinState( VWST_MOUSE_HOLD_ON );

		if( !m_pMouseHoldTimer )
		{
			m_pMouseHoldTimer = new MouseHoldTimer;
		}

		m_pMouseHoldTimer->Start( pCtrl, evtCode, Point );
	}
	else if( m_pMouseHoldTimer && m_pMouseHoldTimer->IsRunning() )
	{
		RemoveState( VWST_MOUSE_HOLD_ON );
		m_pMouseHoldTimer->Stop();

		wxPoint mousePos( wxDefaultPosition );
		VdkDC vdc( this, Rect00(), NULL );
		VdkMouseEvent fakeEvent( MOUSE_HOLD_ON_RELEASED, mousePos, vdc );

		m_pMouseHoldTimer->GetCtrl()->HandleMouseEvent( fakeEvent );
	}
}

void VdkWindow::OnWindowPaintDummy(wxPaintEvent&)
{
	wxASSERT( m_panel != m_this );
	wxPaintDC dc( m_this );
}

void VdkWindow::OnPaint(wxPaintEvent&)
{
	wxPaintDC pdc( m_panel );
	Draw( pdc );
}

void VdkWindow::Draw(wxDC& pdc)
{
	if( TestState( VWST_EXITING ) )
		return;

	// TODO: �Ƿ���룿
	bool isFirstDraw = false;

	//-----------------------------------------------------------------

	if( IsDoubleBuffering() )
	{
		if( !IsBufferedBitmapOk() )
		{
			if( m_bkCanvas.IsOk() )
				m_bkCanvas.Blit( pdc, 0, 0 );

			return;
		}

		// ע�⣺VdkDC vdc ������� VWST_INITING ��־λ��ȡ����
		// �Ż᷵����ȷ�� DC ���
		if( TestState( VWST_INITING ) )
		{
			isFirstDraw = true;
			RemoveState( VWST_INITING );
		}

		// TODO: ����ΪʲôҪ��ô�ɣ�
		VdkDC vdc( this, Rect00(), &pdc );

		// ����ֻ�ǲ��ǵ�һ�λ��ƣ�ֱ�Ӵ�˫����λͼ�и���
		// ��Ϊ�������еĻ�ͼ������������˫����λͼ�н���
		if( !TestStyle( VWS_ALWAYS_REFRESH ) && // ����������Ҫˢ��
			!TestState( VWST_REDRAW_ALL ) && // ���粻�Ǳ�����Ҫˢ��
			!TestState( VWST_RESET ) &&  // ���粻�������˴���
			!isFirstDraw )
		{
			return;
		}

		DoDraw( vdc );
	}
	else
	{
		DoDraw( pdc );
	}

	if( isFirstDraw )
	{
		if( TestStyle( VWS_FIRST_SHOWN_EVT ) )
		{
			VdkWindowFirstShownEvent* e =
				new VdkWindowFirstShownEvent( this );

			wxQueueEvent( m_this, e );
		}
	}

	RemoveState( VWST_INITING | VWST_RESET | VWST_REDRAW_ALL );
}

void VdkWindow::DoDraw(wxDC& dc)
{
	EraseBackground( dc, Rect00() );

	// ��������Զ��ػ��¼�������Ӧ
	DoPaint( dc );

	// �����ٸı� VdkWindow �Ĵ�Сʱ���������ֱ����ܻḲ�ǵ������¹�������ť��
	// ���ԴӺ���ǰ�����Ȼ��ֱ����ٻ���ť�����
	VdkControl* pCtrl( NULL );
	VdkCtrlList::reverse_iterator i, rEnd( m_Ctrls.rend() );
	for( i = m_Ctrls.rbegin(); i != rEnd; ++i )
	{
		pCtrl = *i;

		if( !pCtrl->IsShown() || pCtrl->GetParent() )
			continue;

		pCtrl->Draw( dc );
	}
}

void VdkWindow::QueueRedrawEvent()
{
	SetAddinState( VWST_REDRAW_ALL );
	m_this->Refresh( false );
	m_this->Update();
}

void VdkWindow::OnShow(wxShowEvent& e)
{
#if 0
	VdkNotify notice( e.IsShown() ? VCN_PARENT_SHOW : VCN_PARENT_HID );

	VdkCtrlIter it( m_Ctrls.begin() ), itEnd( m_Ctrls.end() );
	for( ; it != itEnd; ++it )
	{
		(*it)->HandleNotify( notice );
	}
#endif

	e.Skip( true );
}

void VdkWindow::OnIconize(wxIconizeEvent&)
{
	// ������Ҫ��ԭ��С����ť������״̬
	wxMouseEvent e( wxEVT_LEAVE_WINDOW );
	e.SetX( -1 );
	e.SetY( -1 );

	OnLeaveWindow( e );
}

void VdkWindow::OnMaximize(wxMaximizeEvent&)
{
	SetAddinState( VWST_MAXIMIZING );
}
#if 0
void VdkWindow::OnSize(wxSizeEvent& e)
{
	wxSize size( e.GetSize() );

	// �����Ǵ����С�뱳��λͼ��С��ͬ����Ҫ���� Resize
	if( !TestState( VWST_INITING ) &&
		m_bkCanvas.IsOk() && // ���粻���ڱ���λͼ����������Ӧ�ı��С�¼�
		size != m_bkCanvas.GetSize() ) // Ŀ�Ĵ�С�뵱ǰ����λͼ��С����
	{
		if( !Resize( size.x, size.y ) )
		{
			if( IsZoomed() )
			{
				// �ύ����
				wxRect rc( wxDisplay().GetClientArea() );
				DoResize( rc.width, rc.height );

				m_this->SetMaxClientSize( rc.GetSize() );

				RemoveState( VWST_MAXIMIZING );
			}
		}
	}
	else
	{
		if( !IsBufferedBitmapOk() )
		{
			m_bmBuffered = wxBitmap( size.x, size.y );
			SetAddinState( VWST_REDRAW_ALL );
		}
	}
}
#lse
void VdkWindow::OnSize(wxSizeEvent& e)
{

}
#endif

void VdkWindow::OnWindowFocus(wxActivateEvent& e)
{
	if( !e.GetActive() )
	{
		if( GetMenuOnShow() )
		{
			// �˵����ڱ���Ӧ�ý��� LOSTFOCUS �¼�
			wxASSERT( !IsMenuImpl() );
			// �õ�����Ĵ��ڲ��ܾ��ǵ�ǰ������ʾ�Ĳ˵�ʵ��ʵ�ִ�������
			wxASSERT( wxWindow::FindFocus() !=
				(wxWindow* )GetMenuOnShow()->GetImpl() );

			HideMenu();
		}
	}
#ifdef __WXMSW__
#if 0
	else
	{
		// ���ڴ�ʧ��״̬���뼤��״̬�����Զ�������壬
		// ��������ϵ� VdkEdit �޷�������뽹��
		if( m_this != m_panel )
			m_panel->SetFocus();
	}
#endif
#endif // __WXMSW__
}

void VdkWindow::OnClose(wxCloseEvent&)
{
	// �����ǲ˵����壬ȡ������
	if( IsMenuImpl() )
		return;

	BeginExit();
	m_this->Destroy();
}

void VdkWindow::OnLeftDown(wxMouseEvent& e)
{
	HandleMouseEvent( e, LEFT_DOWN );

	// ������Դ������Ͻǵ�λ�ã�����ģ���϶�����
	SetAddinState( VWST_LEFT_DOWN_SKIPPED );
	m_mouseOn = e.GetPosition();

	SetLastMouseEvent( LEFT_DOWN );

#	ifdef __WXMSW__
	if( !m_panel->HasCapture() )
		m_panel->CaptureMouse();
#	endif // __WXMSW__
}

inline void VdkWindow::OnLeftUp(wxMouseEvent& e)
{
	HandleMouseEvent( e, LEFT_UP );
	SetLastMouseEvent( LEFT_UP );

#	ifdef __WXMSW__
	if( m_panel->HasCapture() )
		m_panel->ReleaseMouse();
#	endif // __WXMSW__
}

inline void VdkWindow::OnDLeftDown(wxMouseEvent& e)
{
	HandleMouseEvent( e, DLEFT_DOWN );
	SetLastMouseEvent( DLEFT_DOWN );
}

inline void VdkWindow::OnRightUp(wxMouseEvent& e)
{
	HandleMouseEvent( e, RIGHT_UP );
	SetLastMouseEvent( RIGHT_UP );
}

void VdkWindow::OnMouseMove(wxMouseEvent& e)
{
	// ��֪��Ϊʲô���ڴ����е��һ�¼����ʱ�ᷢ��
	// �ϴ�ʧ��ʱ��һ�� MOVING ����¼�
	if( e.GetPosition() != m_this->ScreenToClient( wxGetMousePosition() ) )
		return;

	if( e.Dragging() && e.LeftIsDown() )
	{
		// ��󻯺��������϶�
		if( !HandleMouseEvent( e, DRAGGING ) &&
			 TestStyle( VWS_DRAGGABLE ) &&
			 (m_nLastMouseEvent == LEFT_DOWN ||
			  m_nLastMouseEvent == DRAGGING) )
        {
            EmulateDragAndMove( m_this->ClientToScreen( e.GetPosition() ) );
        }

		SetLastMouseEvent( DRAGGING );
		return;
	}

	HandleMouseEvent( e, HOVERING );
	SetLastMouseEvent( HOVERING );
}

void VdkWindow::OnMouseWheel(wxMouseEvent& e)
{
	if( e.m_wheelRotation < 0 )
	{
		HandleMouseEvent( e, WHEEL_DOWN );
		SetLastMouseEvent( WHEEL_DOWN );
	}
	else
	{
		HandleMouseEvent( e, WHEEL_UP );
		SetLastMouseEvent( WHEEL_UP );
	}
}

void VdkWindow::OnLeaveWindow(wxMouseEvent& e)
{
	// ����ʲô�����ֻҪ���ָ���뿪���ڣ��ͻ�ԭ���ָ��ΪĬ�ϼ�ͷ��
	// ����ʱ�������á����������߿�ı䴰�ڴ�Сʱ��Ҫ���á�
	if( !wxGetMouseState().LeftIsDown() )
		ResetCursor();

	if( (m_nLastCtrlState == DRAGGING) || // �϶��¼�ʱָ���Ƴ�����
		// ���յ��ġ�����뿪���ڡ��¼����ڡ������������ڡ��¼�����������
		(ms_hovering && (ms_hovering->GetVdkWindow() != this)) )
	{
		return;
	}

	HandleMouseEvent( e, MOUSE_LEAVE_WINDOW );
	SetLastMouseEvent( MOUSE_LEAVE_WINDOW );
}

void VdkWindow::OnMouseLost(wxMouseCaptureLostEvent&) {}

void VdkWindow::OnKeyDown(wxKeyEvent& e)
{
	HandleKeys( KEY_DOWN, e );
}

void VdkWindow::OnKeyUp(wxKeyEvent& e)
{
	HandleKeys( KEY_UP, e );
}

void VdkWindow::OnChars(wxKeyEvent& e)
{
	HandleKeys( KEY_CHAR, e );
}

void VdkWindow::HandleKeys(VdkKeyEventType type, wxKeyEvent& e)
{
	// �ȷ��ɸ������ദ��
	if( DoHandleKeyEvent( e ) )
		return;

	bool isMenuOnShow = (GetMenuOnShow() != NULL);
	int keyCode = e.GetKeyCode();

	// ����Windows context menu��������ʵ���Ǵ��Ҽ��˵�
	if( !isMenuOnShow && (keyCode == WXK_WINDOWS_MENU) && (type == KEY_UP) )
	{
		wxMouseEvent fakeEvent( wxEVT_RIGHT_UP );
		fakeEvent.SetRightDown( true );

		// �����н���ؼ����ɽ���ؼ������������ɴ���������
		wxPoint pos( 0, 0 );
		if( m_focus )
		{
			pos = m_focus->GetAbsoluteRect().GetPosition();
		}

		fakeEvent.SetPosition( pos );

		//----------------------------------------------

		OnRightUp( fakeEvent );

		// Comsume it.
		return;
	}

	// ������̵���
	if( !isMenuOnShow && m_tabOrderIter &&
		(type == KEY_DOWN) &&  (keyCode == WXK_TAB) &&
		(!m_focus || !m_focus->TestStyle( VCS_WANTS_ALL_CHARS )) )
	{
		if( m_tabOrderIter->FocusNext( m_focus ) )
			return;
	}

	bool skip = (type == KEY_DOWN);
	if( !isMenuOnShow && m_focus && m_focus->TestStyle( VCS_KEY_EVENT ) )
	{
		VdkDC vdc( this, Rect00(), NULL );
		VdkKeyEvent vke( type, e, vdc );

		m_focus->HandleKeyEvent( vke );
		skip = (vke.GetSkipped() || e.GetSkipped());
	}

	if( skip )
	{
		VdkMenu* menuOnShow = GetMenuOnShow();
		VdkWindow* win = menuOnShow ? menuOnShow->GetVdkWindowImpl() : this;

		// ʹ�� ESC �رմ���/�˵�
		if( (type == KEY_DOWN) && keyCode == WXK_ESCAPE &&
			win->TestStyle( VWS_DISMISS_BY_ESC ) &&
			// ʹ�� ESC ȡ�����뷨״̬����
			!IsImeWindowVisiable( win->m_this ) )
		{
			// Hide() or Close() ?
			// ������ Close() �ȽϺ��ʣ��Ͼ��ܶ�ʱ��Ի����ǡ�һ���ԡ���
			if( !menuOnShow )
			{
				m_this->Close();
			}
			else
			{
				menuOnShow->Return();
			}

			// Eat this event.
			return;
		}

		e.Skip( true );
	}
}

void VdkWindow::OnWindowMenuKeyUp(wxMouseEvent& e)
{
	// ��Ҫ�ظ���ʾ�˵�
	if( GetMenuOnShow() )
		return;

	// �����н���ؼ����ɽ���ؼ������������ɴ���������
	wxPoint pos( 0, 0 );
	if( m_focus )
	{
		pos = m_focus->GetAbsoluteRect().GetPosition();
	}

	e.SetPosition( pos );

	//----------------------------------------------

	OnRightUp( e );
}

void VdkWindow::PopEventFilter(VdkEventFilter* filter, bool del)
{
	wxASSERT( filter );

	m_EventFilters.remove( filter );
	if( del )
	{
		delete filter;
	}
}

bool VdkWindow::FilterEvent(int evtCode, const wxMouseEvent& e)
{
	VdkEventFilter* filter;
	EventFilterIter i( m_EventFilters.begin() );
	for( ; i != m_EventFilters.end(); ++i )
	{
		filter = *i;

		if( filter->IsActive() )
		{
			if( filter->FilterEvent( VdkEventFilter::EventForFiltering
					( this, evtCode ).mouseEvent( e ) ) )
			{
				return true;
			}
		}
	}

	return false;
}

#if 0
#define DC_INSTANCE_HERE() \
	wxLogDebug( L"[%s:%03d]new VdkDC instance.", \
			    m_debugCaption, __LINE__ );\
	VdkDC dc( this, Rect00(), NULL )
#else
#define DC_INSTANCE_HERE() \
	VdkDC dc( this, Rect00(), NULL )
#endif

#if 0
#define RETURN( ret ) \
{\
	if( evtCode != HOVERING ) \
	{\
		wxLogDebug( L"[%s]HandleMouseEvent: %s(#%d)", m_debugCaption, \
					GetEventName( evtCode ), __LINE__ );\
	}\
	\
	return ret;\
}
#else
#define RETURN( ret ) \
	return ret
#endif

bool VdkWindow::HandleMouseEvent(wxMouseEvent& evt, VdkMouseEventType evtCode)
{
	// ��ʱ����ֵ�����ж��϶�����ʱʹ��
	if( TestState( VWST_INITING ) || TestState( VWST_EXITING ) )
	{
		RETURN( true );
	}

	// �����ڲ��¼��������������ࣩ
	if( FilterEventBefore( evt, evtCode ) )
	{
		RETURN( true );
	}

	// �����ⲿ�¼����������ؼ�,etc.��
	if( !m_EventFilters.empty() )
	{
		if( FilterEvent( evtCode, evt ) )
		{
			RETURN( true );
		}
	}

	// �Ҽ��˵��Ƿ�����ʾ
	if( GetMenuOnShow() && !IsMenuImpl() )
	{
#ifdef __WXMSW__
		// �� wxMSW ���棬��һ���˵�����ʾʱ�������Ĺ����¼��ᱻ
		// ���͵��˵�ʵ�崰��ĸ����ڵ���Ϣ������
		if( evtCode == WHEEL_UP || evtCode == WHEEL_DOWN )
		{
			VdkMenu* menu = GetMenuOnShow()->GetLastShownMenu();
			VdkWindow* menuImpl = menu->GetVdkWindowImpl();

			wxASSERT( menuImpl != this );
			if( menuImpl )
			{
				wxGetMousePosition( &evt.m_x, &evt.m_y );
				menuImpl->GetHandle()->ScreenToClient( &evt.m_x, &evt.m_y );

				// ��Ȼ�����ָ����Ҫ�������˵�ʵ�崰����
				if( menuImpl->Rect00().Contains( evt.m_x, evt.m_y ) )
				{
					m_mouseEventNotForMe = true;
					menuImpl->HandleMouseEvent( evt, evtCode );

					RETURN( true );
				}
			}
		}
#endif //__WXMSW__

		// ��갴���¼�������һ���������¼��Ļ��������ֻ��Ҫ
		// ��ȡ���Ҽ����µ��¼���������ȡ���˵�����ʾ���ɡ�
		if( IsActivatableEvent( evtCode ) )
		{
			HideMenu();
		}
		else // ��������һ���¼�
		{
			RETURN( true );
		}
	}

	//----------------------------------------------------------

	wxPoint mousePos( evt.GetPosition() );

	//----------------------------------------------------------
	// �϶��ı䴰�ڴ�С

	switch( evtCode )
	{
	case HOVERING:
		{
			if( TestStyle( VWS_RESIZEABLE ) )
			{
				// ��֪��Ϊʲôϵͳ�����϶���ʱ��������ƶ��¼���
				// ƫƫ�ҵĳ����ⲻ�������λ�ڴ��ڱ߽��ϵ�
				if( TestState( VWST_DRAG_AND_RESIZING ) &&
					wxGetMouseState().LeftIsDown() )
				{
					RETURN( false );
				}

				HandleRzCursor( mousePos );
			}

			break;
		}

	case DRAGGING:
		{
			if( TestState( VWST_DRAG_AND_RESIZING ) )
			{
				if( GetMenuOnShow() )
					HideMenu();

				EmulateDragAndResize( mousePos );
				RETURN( true );
			}
			else if( TestState( VWST_DRAG_AND_MOVING ) )
			{
				RETURN( false );
			}

			break;
		}

	case LEFT_UP:
		{
			if( TestState( VWST_DRAG_AND_RESIZING ) )
			{
				ResetRzCursor();

				// ���ﲻ��ֱ�� RETURN(true) ��������һ��������Ե
				// �Ŀؼ��ϰ�����꣬�ؼ��ı���������״̬��ȴ�޷�
				// ��ȡ�����ͷŵ��¼���
			}

			RemoveState( VWST_DRAG_AND_MOVING );

			break;
		}

	default:

		break;
	}

	//----------------------------------------------------------
	// ���� VdkControl

	VdkCtrlList::reverse_iterator iContainer( m_Ctrls.rend() ), // ���ܸı�
								  rEnd( iContainer );
	wxRect rect;
	VdkControl*	pCtrl;

	// ���粻��������ʧȥ������߽�����Ƴ�������
	if( mousePos.x != MEANNINGLESS_MOUSE_COORD )
	{
		// �����ϴ��¼�ʱ�����϶� VdkSlider ���������͵��Ǹ� VdkSlider
		if( evtCode == DRAGGING &&
			ms_hovering &&
		   !TestState( VWST_MOUSE_HOLD_ON ) )
		{
			pCtrl = ms_hovering;
			VdkWindow* win( pCtrl->GetVdkWindow() );

			if( this != win )
			{
				wxPoint Another( win->m_this->GetScreenPosition() );
				wxPoint This( m_this->GetScreenPosition() );

				mousePos.x += This.x - Another.x;
				mousePos.y += This.y - Another.y;

				m_mouseEventNotForMe = true;
				win->HandleMouseEvent( evt, evtCode );

				RETURN( true ); // ����¼��ǲ��������ǵ�
			}
		}
		else
		{
			bool bContains(false);

			VdkCtrlList::reverse_iterator i;
			for( i = m_Ctrls.rbegin(); i != rEnd; ++i )
			{
				pCtrl = *i;

				if( !pCtrl->CanHandleEvent() ||
					pCtrl->IsCtrlHandler() )
				{
					continue;
				}

				bContains = pCtrl->HitTest( mousePos );
				if( bContains )
				{
					// ������ container���������������ؼ��������� container �Ա���
					if( pCtrl->TestStyle( VCS_CTRL_CONTAINER ) )
					{
						// ֻ������һ���ҵ����Ǹ� container
						if( iContainer == rEnd )
							iContainer = i;

						continue;
					}
					else if( pCtrl->TestStyle( VCS_IGNORE_ALL_EVENTS ) )
					{
						continue;
					}
					else
					{
						break;
					}
				}
			} // end for

			// ��ֱ�ӷ�������Ϊ����Ҫ����û�л�ԭ��
			// ǰһ�� VdkControl
			if( i == rEnd )
			{
				pCtrl = NULL;
			}
		}

		//----------------------------------------------------------

		// ������ס����¼�
		// <---��Ҫ���߳�������������ֹͣһ�� Timer ��---->
		if( TestState( VWST_MOUSE_HOLD_ON ) )
		{
			// �ڳ�����ס��ͬһ�� VdkControl �������϶�ָ��
			if( evtCode == DRAGGING && pCtrl == ms_hovering )
			{
				RETURN( true );
			}

			HandleMouseHoldOn( NORMAL, mousePos, NULL );
			// �����Ǳ����ٴ��ƻ�״̬
			RETURN( true );
		}
	}

	// ����ͬʱ���� Container ���͵Ŀؼ���
	// ��Ϊ����Ҳ���ܱ���Ϊ��ǰ��������״̬�Ŀؼ�
	VdkControl* pCtrl2 = pCtrl;
	if( !pCtrl2 && iContainer != rEnd )
		pCtrl2 = *iContainer;

	// �Ƿ�����ͬ�� VdkControl
	if( pCtrl2 &&
		evtCode == HOVERING &&
		pCtrl2 == ms_hovering &&
		pCtrl2->TestStyle( VCS_ONESHOT_HOVERING ) )
	{
		RETURN( true );
	}

	//----------------------------------------------------------

	// ��ԭ��һ�� VdkControl
	if( ms_hovering && pCtrl2 != ms_hovering &&
		ms_hovering->CanHandleEvent() ) // ����ؼ������ã���Ҫ���ͻ�ԭ�¼�
	{
		// ���� pCtrl ������һ�����ڵ����
		VdkWindow* lastwindow( ms_hovering->GetVdkWindow() );

		if( lastwindow != this )
		{
			lastwindow->RecoverCtrl( ms_hovering );
			goto PROCESSING_EVENTS;
		}

		VdkControl* hovering = ms_hovering;
		ResetCtrlOnHover();
		m_nLastCtrlState = NORMAL;

		DC_INSTANCE_HERE();
		VdkMouseEvent e( NORMAL, mousePos, dc );
		hovering->HandleMouseEvent( e );
	}

	// ����������뽹��Ŀؼ�
	if( IsActivatableEvent( evtCode ) &&
		pCtrl2 && // �ڴ��ڿհ״������ʧ�ǰ����ؼ�
		m_focus != pCtrl2 )
	{
		DC_INSTANCE_HERE();
		FocusCtrl( pCtrl2, &dc, mousePos.x, mousePos.y );
	}

	//----------------------------------------------------------
	// ������ʼ�����¼�

PROCESSING_EVENTS:

	if( pCtrl ) // ����� VdkControl ����������Ҳ����ڵ������ڴ�С
	{
		m_nLastCtrlState = evtCode;
		SetCtrlOnHover( pCtrl );

		DC_INSTANCE_HERE();
		VdkMouseEvent e( evt, evtCode, dc );

		pCtrl->HandleMouseEvent( e );

		// TODO: �Ƿ�Ӧ����������һ������Ҫ��� VdkControl �أ�
		if( e.GetSkipped() )
			goto FILTEREVENTAFTER;

		// ִ�е���һ���ر�/���ô��ڵ�����
		if( TestState( VWST_EXITING ) || TestState( VWST_RESET ) )
			RETURN( true );
	}
	else if( evtCode == RIGHT_UP && m_menu ) // ��ʾ�뱾 VdkWindow ������Ĳ˵�
	{
		if( GetMenuOnShow() )
			HideMenu();

		ShowContextMenu( NULL, mousePos.x, mousePos.y );

		RETURN( true );
	}
	// �����ڴ��ڿհ״����ƶ�
	else if( iContainer == rEnd )
	{
		ResetCtrlOnHover();
		m_nLastCtrlState = NORMAL;
	}

	if( pCtrl2 )
	{
		// ��δ�����Ϊ�� container ����Ƶ�
		if((evtCode == LEFT_DOWN || evtCode == DLEFT_DOWN) &&
			pCtrl2->TestStyle( VCS_HONLD_ON ) )
		{
			// ������ס��ť
			HandleMouseHoldOn( LEFT_DOWN, mousePos, pCtrl2 );
		}
		// ���¼����� container
		else if( pCtrl2->TestStyle( VCS_CTRL_CONTAINER ) && iContainer != rEnd )
		{
			DC_INSTANCE_HERE();
			VdkMouseEvent e( evt, evtCode, dc );

			pCtrl2->HandleMouseEvent( e );

			if( e.GetSkipped() )
			{
				goto FILTEREVENTAFTER;
			}
			else
			{
				// ��ʹ�� container ҲҪ���ֺ�״̬
				// ����ֱ���϶� VdkSlider ���������϶��ֱ���
				// ��һ��Ҫ�ܳ����϶�������뿪���������������
				// ȡ��״̬��
				SetCtrlOnHover( pCtrl2 );
				m_nLastCtrlState = evtCode;
			}
		}

	}
	else if( !GetMenuOnShow() ) // �����ڴ��ڿհ״����ƶ�
	{
FILTEREVENTAFTER:
		if( !FilterEventAfter( evt, evtCode ) ) // �����ڴ���հ����϶�����
		{
			RETURN( !CanDragBySpace() );
		}
	}

	RETURN( true );
}

void VdkWindow::EmulateDragAndMove(const wxPoint& screen)
{
	SetAddinState( VWST_DRAG_AND_MOVING );

	wxCoord x = screen.x - m_mouseOn.x;
	wxCoord y = screen.y - m_mouseOn.y;

#ifdef __WXMSW__
	m_this->Move( x, y );
#elif defined( __WXGTK__ )
    MoveWindowSynchronously( m_this, x, y );
#endif
}

void VdkWindow::OnDelayShowContextMenu(VdkVObjEvent& e)
{
	ShowContextMenu( e.GetCtrl(), e.GetInt(), e.GetExtraLong(), false );
}

void VdkWindow::ShowContextMenu(VdkControl* pCtrl, int x, int y, bool delay)
{
	if( delay )
	{
		VdkVObjEvent e( VEMC_SHOW_CONTEXT_MENU );
		e.SetCtrl( pCtrl );
		e.SetInt( x );
		e.SetExtraLong( y );

		wxPostEvent( m_panel, e );
		return;
	}

	VdkMenu* menu(NULL);

	if( pCtrl )
	{
		menu = pCtrl->GetMenu();
		pCtrl->SetMenuState( true );
	}
	else if( !pCtrl )
	{
		menu = m_menu;
		if( !menu )
			return;
	}

	// ��֪��Ϊʲô�����Ƶ� ShowContext ����
	SetMenuOnShow( menu );

	m_this->ClientToScreen( &x, &y );
	menu->ShowContext( x, y, pCtrl );
}

/*static*/
void VdkWindow::HideMenu()
{
	VdkMenu* menuOnShow = GetMenuOnShow();
	if( menuOnShow )
	{
		// m_MenuOnShow = NULL �� OnMenuHid �����
		menuOnShow->Return();
	}
}

void VdkWindow::OnMenuHid(VdkVObjEvent& e)
{
	// һ��Ҫ�����жϣ�
	if( GetMenuOnShow() == e.GetMenu() )
		SetMenuOnShow( NULL );

	//----------------------------------------------------------

	VdkControl* pCtrl = e.GetCtrl();
	if( pCtrl )
	{
		VdkDC dc( this, Rect00(), NULL );
		pCtrl->SetMenuState( false, &dc );
	}
}

void VdkWindow::AssignCursor(const wxCursor& cursor)
{
#ifdef __WXMSW__
	if( m_cursor.GetHCURSOR() == cursor.GetHCURSOR() )
		return;
#endif

	m_cursor = cursor;
	m_panel->SetCursor( m_cursor );
}

void VdkWindow::ResetCursor()
{
#ifdef __WXMSW__
	if( m_cursor.GetHCURSOR() == wxSTANDARD_CURSOR->GetHCURSOR() )
		return;
#endif

	m_cursor = *wxSTANDARD_CURSOR;
	m_panel->SetCursor( m_cursor );
}

void VdkWindow::SetTrayIcon(TrayIcon* trayIcon)
{
	if( m_trayIcon ) delete m_trayIcon;
	m_trayIcon = trayIcon;
}

wxTopLevelWindow* VdkWindow::GetTopLevelWindowHandle() const
{
	if( m_this->IsTopLevel() )
		return dynamic_cast< wxTopLevelWindow* >( m_this );

	return NULL;
}

void VdkWindow::SetMinSize(int w, int h)
{
	wxASSERT( w >= m_bkCanvas.GetMinSize().x &&
			  h >= m_bkCanvas.GetMinSize().y );

	m_nMinWidth = w;
	m_nMinHeight = h;

	//----------------------------------------------------------

	int ww, wh;
	m_panel->GetSize( &ww, &wh );
	if( (ww < m_nMinWidth) || (wh < m_nMinHeight) )
	{
		Resize( wxMax( m_nMinWidth, ww ),
				wxMax( m_nMinHeight, wh ) );
	}
}

wxSize VdkWindow::GetMinSize() const
{
	wxSize size;
	GetMinSize( &size.x, &size.y );

	return size;
}

void VdkWindow::GetMinSize(int* w, int* h) const
{
	// ����ΪʲôҪŪ����ô�����أ���Ϊ����һЩ���屳��ͼƬ����Ȼ��ʵ�ʵĳ���
	// ҪС����������������Ĵ�С֮���ǲ���Ҫ�ٸı䴰���С���ˡ����Եõ���С
	// ��С�Ĵ�С�������塣��Ϊ XRC �ļ��еĿؼ���С��λ�ö����Ǹ�������֮��
	// ��Ŀ�괰�ڵĴ�С�����ÿؼ�λ�õģ��������Ƿ��ص�ǰ���ڴ�С��
	if( TestStyle( VWS_RESIZEABLE ) )
	{
		if( w ) *w = m_nMinWidth;
		if( h ) *h = m_nMinHeight;
	}
	else
		m_panel->GetSize( w, h );
}

//////////////////////////////////////////////////////////////////////////
// ԭ VdkWindowResizer �Ĵ���
//////////////////////////////////////////////////////////////////////////

void VdkWindow::HandleRzCursor(const wxPoint& mousePos)
{
	wxASSERT_MSG( TestStyle( VWS_RESIZEABLE ), L"���ڱ������ VWS_RESIZEABLE ���" );

	wxStockCursor cursorType( wxCURSOR_NONE );
	m_rectCached = m_this->GetScreenRect();

	int x = mousePos.x, y = mousePos.y;
	int minX = 0,
		minY = 0,
		maxX = m_rectCached.width,
		maxY = m_rectCached.height;

	enum {
		GS_WINDOW_BORDER = 5, // ���ָ���ƶ�����Щ Border ��ɵ�����ʱ��ı�
	};

	m_dragAndResizeType = 0;

	if( (x <= minX + GS_WINDOW_BORDER) && (x >= minX) )
		m_dragAndResizeType |= vdkWEST;

	if( (x >= maxX - GS_WINDOW_BORDER) && (x <= maxX) )
		m_dragAndResizeType |= vdkEAST;

	if( (y <= minY + GS_WINDOW_BORDER) && (y >= minY) )
		m_dragAndResizeType |= vdkNORTH;

	if( (y >= maxY - GS_WINDOW_BORDER) && (y <= maxY) )
		m_dragAndResizeType |= vdkSOUTH;

	bool north = (m_dragAndResizeType & vdkNORTH) != 0;
	bool west = (m_dragAndResizeType & vdkWEST) != 0;
	bool east = (m_dragAndResizeType & vdkEAST) != 0;
	bool south = (m_dragAndResizeType & vdkSOUTH) != 0;

	if( (north && west) || (east && south) )
		cursorType = wxCURSOR_SIZENWSE;
	else if( (north && east) || (west && south) )
		cursorType = wxCURSOR_SIZENESW;
	else if( west || east )
		cursorType = wxCURSOR_SIZEWE;
	else if( north || south )
		cursorType = wxCURSOR_SIZENS;

	if( m_dragAndResizeType != 0 )
	{
		wxASSERT( cursorType != wxCURSOR_NONE );

		AssignCursor( wxCursor( cursorType ) );
		SetAddinState( VWST_DRAG_AND_RESIZING );
	}
	else
	{
		ResetRzCursor();
	}
}

void VdkWindow::ResetRzCursor()
{
	if( TestState( VWST_DRAG_AND_RESIZING ) )
	{
		RemoveState( VWST_DRAG_AND_RESIZING );
		m_dragAndResizeType = 0;

		ResetCursor();
	}
}

void VdkWindow::EmulateDragAndResize(const wxPoint& mousePosClient)
{
	// ������ͻ���������϶��¼�
	if( (m_nLastMouseEvent != LEFT_DOWN) &&
		(m_nLastMouseEvent != DRAGGING) )
	{
		return;
	}

	// ���� GTK �µ����ʹ��ڣ����ܽ������Ƴ���Ļ��Χ���������� GTK ���ڣ�
	// ������ Move() �����Ƴ���Ļ����ֵĹ涨

#ifdef __WXGTK__
	// ����̫Ƶ���ı䴰�ڴ�С������ȡ Windows ��ֵ����� 40 ms
	wxMilliClock_t nTimeStamp = wxGetLocalTimeMillis();
	if( (nTimeStamp - m_nLastTimeStamp) < 40 )
		return;
#endif

	wxRect rcNew( m_this->GetScreenRect() );
	wxPoint mousePosScreen( m_this->ClientToScreen( mousePosClient ) );

	if( m_dragAndResizeType & vdkEAST )
		rcNew.width = mousePosScreen.x - m_rectCached.x;

	if( m_dragAndResizeType & vdkWEST )
	{
		rcNew.x = mousePosScreen.x;
		rcNew.width = RightOf( m_rectCached ) - rcNew.x;
	}

	if( m_dragAndResizeType & vdkNORTH )
	{
		rcNew.y = mousePosScreen.y;
		rcNew.height = BottomOf( m_rectCached ) - rcNew.y;
	}

	if( m_dragAndResizeType & vdkSOUTH )
		rcNew.height = mousePosScreen.y - m_rectCached.y;

	if( rcNew.width < m_nMinWidth )
	{
		rcNew.width = m_nMinWidth;

		// ��ʱ�� rc.x ��ȻԽ�����ڵ���߿򣬴��ڲ���������
		if( m_dragAndResizeType & vdkWEST )
			rcNew.x = RightOf( m_rectCached ) - rcNew.width;
	}

	if(	rcNew.height < m_nMinHeight )
	{
		rcNew.height = m_nMinHeight;

		// ��ʱ�� rc.y ��ȻԽ�����ڵ��ϱ߿򣬴��ڲ���������
		if( m_dragAndResizeType & vdkNORTH )
			rcNew.y = BottomOf( m_rectCached ) - rcNew.height;
	}

	Resize( rcNew );
#ifdef __WXGTK__
	m_nLastTimeStamp = wxGetLocalTimeMillis();
#endif
}

#if 1
bool VdkWindow::Resize(int w, int h)
{
	return Resize( wxDefaultCoord, wxDefaultCoord, w, h, wxSIZE_USE_EXISTING );
}

bool VdkWindow::Resize(int x, int y, int w, int h, int sizeFlags)
{
	/*
		�����и��ǳ���ֵ��߼���
		SetSize �����������м������ͣ�
		1�����ɸ��Ĵ�С���ṩ��ͬ��С�ı���λͼ��
		2�����ɸ��Ĵ�С���ṩС��Ҫ���С�ı���λͼ��
		3���ɸ��Ĵ�С

		��һ��ԭ����ǣ�һ��Ҫ��֤ m_bmBuffered ����Ч�ġ�����������һ���жϡ�
	*/

	bool resizeable = TestStyle( VWS_RESIZEABLE );

	if( m_bkCanvas.CanResize() )
	{
		// ��������ڳ����ʼ��ʱҲ���ã� wxWidgets ���������Ĭ�ϴ�С����
		// ��Ϊ��ʱ��û�д����κοؼ����ʵ��� LayoutWidgets �Գ�����Ӱ��
		int ww, hh;
		m_this->GetSize( &ww, &hh );

		if( w != 0 && w < m_nMinWidth )
			w = m_nMinWidth;

		if( h != 0 && h < m_nMinHeight )
			h = m_nMinHeight;

		if( w == 0 ) w = ww;
		if( h == 0 ) h = hh;

		int dX, dY;
		dX = w - m_bkCanvas.GetWidth();
		dY = h - m_bkCanvas.GetHeight();

		// �Ƿ���Ҫ��������λͼ�Ĵ�С
		if( dX || dY )
		{
			if( !m_bkCanvas.Rescale( wxSize( w, h ) ) )
				return false;

			LayoutWidgets( dX, dY );
		}
	}
	else if( resizeable )
	{
		RemoveStyle( VWS_RESIZEABLE );
		resizeable = false;

		return false;
	}

	// �ύ����
	DoResize( x, y, w, h, sizeFlags );

	if( !IsBufferedBitmapOk() )
	{
		int buffw( resizeable ? w * 2 : w ),
			buffh( resizeable ? h * 2 : h ), disx, disy;

		wxDisplaySize( &disx, &disy );

		if( buffw > disx ) buffw = disx;
		if( buffh > disy ) buffh = disy;

		m_bmBuffered = wxBitmap( buffw, buffh );
	}

	RemoveState( VWST_MAXIMIZING ); // TODO:
	SetAddinState( VWST_REDRAW_ALL ); // ȫ���ػ�

	return true;
}
#else
void VdkWindow::OnSize(wxSizeEvent& e)
{
	wxSize newSize( e.GetSize() );

	/*
		�����и��ǳ���ֵ��߼���
		SetSize �����������м������ͣ�
		1�����ɸ��Ĵ�С���ṩ��ͬ��С�ı���λͼ��
		2�����ɸ��Ĵ�С���ṩС��Ҫ���С�ı���λͼ��
		3���ɸ��Ĵ�С

		��һ��ԭ����ǣ�һ��Ҫ��֤ m_bmBuffered ����Ч�ġ�����������һ���жϡ�
	*/

	bool resizeable = TestStyle( VWS_RESIZEABLE );

	if( m_bkCanvas.CanResize() )
	{
		// ��������ڳ����ʼ��ʱҲ���ã� wxWidgets ���������Ĭ�ϴ�С����
		// ��Ϊ��ʱ��û�д����κοؼ����ʵ��� LayoutWidgets �Գ�����Ӱ��
		int ww, hh;
		m_this->GetSize( &ww, &hh );

		if( (w != 0) && (w < m_nMinWidth) )
			w = m_nMinWidth;

		if( (h != 0) && h < m_nMinHeight )
			h = m_nMinHeight;

		if( w == 0 ) w = ww;
		if( h == 0 ) h = hh;

		int dX, dY;
		dX = w - m_bkCanvas.GetWidth();
		dY = h - m_bkCanvas.GetHeight();

		// �Ƿ���Ҫ��������λͼ�Ĵ�С
		if( dX || dY )
		{
			if( !m_bkCanvas.Rescale( wxSize( w, h ) ) )
				return false;

			LayoutWidgets( dX, dY );
		}
	}
	else if( resizeable )
	{
		RemoveStyle( VWS_RESIZEABLE );
		resizeable = false;

		return false;
	}

	// �ύ����
	DoResize( x, y, w, h, sizeFlags );

	if( !IsBufferedBitmapOk() )
	{
		int buffw( resizeable ? w * 2 : w ),
			buffh( resizeable ? h * 2 : h ), disx, disy;

		wxDisplaySize( &disx, &disy );

		if( buffw > disx ) buffw = disx;
		if( buffh > disy ) buffh = disy;

		m_bmBuffered = wxBitmap( buffw, buffh );
	}

	RemoveState( VWST_MAXIMIZING );
	SetAddinState( VWST_REDRAW_ALL ); // ȫ���ػ�
}
#endif

bool VdkWindow::IsBufferedBitmapOk()
{
	if( !IsDoubleBuffering() )
		return true;

	wxCoord w, h;
	m_this->GetSize( &w, &h );

	return m_bmBuffered.IsOk() &&
		  (m_bmBuffered.GetWidth() >= w) &&
		  (m_bmBuffered.GetHeight() >= h);
}

void VdkWindow::LayoutWidgets(int dX, int dY)
{
	VdkControl* pCtrl;
	CtrlIter walker( m_Ctrls.begin() ), e( m_Ctrls.end() );

	for(; walker != e; ++walker )
	{
		pCtrl = *walker;
		if( pCtrl->GetParent() )
			continue;

		pCtrl->HandleResize( dX, dY );
	}
}

//////////////////////////////////////////////////////////////////////////
// ԭ VdkCtrlContainer �Ĵ���
//////////////////////////////////////////////////////////////////////////

bool VdkWindow::RemoveCtrl(VdkControl* pCtrl)
{
	if( m_Ctrls.DeleteObject( pCtrl ) )
	{
		if( m_tabOrderIter )
			m_tabOrderIter->RemoveCtrl( pCtrl );

		if( ms_hovering == pCtrl )
			ResetCtrlOnHover();

		if( m_focus == pCtrl )
			ResetCtrlOnFocus();

		return true;
	}

	return false;
}

void VdkWindow::AddCtrl(VdkControl* pCtrl)
{
	wxASSERT( pCtrl );

	m_Ctrls.push_back( pCtrl );

	// ���̵����¼�
	if( pCtrl->TestStyle( VCS_TAB_TRAVERSAL ) )
	{
		if( !m_tabOrderIter )
		{
			m_tabOrderIter = new TabOrderIterator;
		}

		m_tabOrderIter->AddCtrl( pCtrl );
	}

	//----------------------------------------------------------

	// һ�о��������͡�CREATE���¼�
	pCtrl->HandleNotify( VdkNotify( VCN_CREATE ) );

	// ��¡�����Ŀؼ�����֪�� VdkWindow ��ǰ��С���ʼ��С֮��Ĳ�ֵ��
	if( !pCtrl->TestState( VCST_CLONING ) )
	{
		LayoutCtrl( pCtrl );
	}
	else
	{
		pCtrl->RemoveState( VCST_CLONING );
	}
}

void VdkWindow::LayoutCtrl(VdkControl* pCtrl)
{
	wxSize sz( m_this->GetSize() ), szMin( GetMinSize() );

	int dX( sz.x - szMin.x ),
		dY( sz.y - szMin.y );

	if( ( dX || dY ) &&
		pCtrl->GetAlign() &&
		!pCtrl->GetParent() )
	{
		pCtrl->HandleResize( dX, dY );
	}
}

VdkControl* VdkWindow::FindCtrl(const wxString& strName)
{
	CtrlIter i;
	for( i = m_Ctrls.begin(); i != m_Ctrls.end(); ++i )
	{
		if( strName == (*i)->GetName() )
			return *i;
	}

	return NULL;
}

bool VdkWindow::FindCtrl(VdkControl* pCtrl)
{
	CtrlIter i;
	for( i = m_Ctrls.begin(); i != m_Ctrls.end(); ++i )
	{
		if( *i == pCtrl )
			return true;
	}

	return false;
}

void VdkWindow::ShowCtrl(const wxString& strName, bool bShow, wxDC* pDC)
{
	VdkControl* pCtrl( FindCtrl( strName ) );
	if( pCtrl )
	{
		pCtrl->Show( bShow, pDC );
	}
}

void VdkWindow::HilightCtrl(VdkControl* pCtrl, wxDC& dc)
{
	if( pCtrl == ms_hovering )
		return;

	if( ms_hovering )
	{
		VdkWindow* lastwin = ms_hovering->GetVdkWindow();
		if( lastwin != this )
		{
			VdkDC vdc( lastwin, lastwin->Rect00(), NULL );
			lastwin->HilightCtrl( NULL, vdc );
		}
		else
		{
			wxPoint fakeMousePos( MEANNINGLESS_MOUSE_COORD,
								  MEANNINGLESS_MOUSE_COORD );

			VdkMouseEvent fakeEvent( NORMAL, fakeMousePos, dc );
			ms_hovering->HandleMouseEvent( fakeEvent );

			ResetCtrlOnHover();
			m_nLastCtrlState = NORMAL;
		}
	}

	//----------------------------------------------------

	if( pCtrl )
	{
		SetCtrlOnHover( pCtrl );
		m_nLastCtrlState = HOVERING;

		VdkMouseEvent e( HOVERING, pCtrl->GetPosition(), dc );
		pCtrl->HandleMouseEvent( e );
	}
}

void VdkWindow::FocusCtrl(VdkControl* pCtrl, wxDC* pDC)
{
	FocusCtrl( pCtrl, pDC, MEANNINGLESS_MOUSE_COORD, MEANNINGLESS_MOUSE_COORD );
}

void VdkWindow::FocusCtrl(VdkControl* pCtrl, wxDC* pDC, int x, int y)
{
	wxASSERT_MSG( pCtrl, L"�����ṩ��Ч�Ŀؼ������" );

	if( m_focus == pCtrl )
	{
		// ����һ�㣬���¼������ؿؼ�
		if( m_focus )
		{
			FocusHiddenCtrl();
		}

		return;
	}

	//----------------------------------------------------

	if( m_focus )
	{
		VdkControl* focus = m_focus;
		ResetCtrlOnFocus();

		VdkNotify n( VCN_LOST_FOCUS );
		n.SetVObjDC( pDC );
		focus->HandleNotify( n );
	}

	if( pCtrl )
	{
		SetCtrlOnFocus( pCtrl );
		// һ��Ҫ�ȼ������ؿؼ�
		FocusHiddenCtrl();

		VdkNotify n( VCN_FOCUS );
		n.SetWparam( x );
		n.SetLparam( y );
		n.SetVObjDC( pDC );

		pCtrl->HandleNotify( n );
	}
}

void VdkWindow::FocusHiddenCtrl()
{
#ifdef __WXMSW__
	// ���ܽ����������˵�������
	if( IsMenuImpl() )
		return;

	if( !m_hiddenCtrl )
	{
		m_hiddenCtrl = new wxControl( m_panel, wxIdManager::ReserveId(),
									  wxPoint( -100, -100 ),
									  wxSize( 1, 1 ),
									  wxWANTS_CHARS );

		// ���뱣֤�������������ؼ�
		new wxControl( m_panel, wxIdManager::ReserveId(),
					   wxPoint( -200, -200 ),
					   wxSize( 1, 1 ),
					   wxWANTS_CHARS );

		m_hiddenCtrl->Bind( wxEVT_KEY_DOWN, &VdkWindow::OnKeyDown, this );
		m_hiddenCtrl->Bind( wxEVT_KEY_UP, &VdkWindow::OnKeyUp, this );
		m_hiddenCtrl->Bind( wxEVT_CHAR, &VdkWindow::OnChars, this );

		// wxWidgets ��Ĭ�Ͻ���Windows context menu�������¼�ת������ͨ��
		// �Ҽ������¼�
		m_hiddenCtrl->Bind( wxEVT_RIGHT_UP,
							&VdkWindow::OnWindowMenuKeyUp,
							this );
	}

	m_hiddenCtrl->SetFocus();
#endif
}

void VdkWindow::EraseBackground(wxDC& dc, const wxRect& rc)
{
	if( m_bkCanvas.IsOk() )
	{
		m_bkCanvas.BlitRect( dc, rc );
	}
	else
	{
		wxBrush brush( m_this->GetBackgroundColour() );
		dc.SetBrush( brush );
		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.DrawRectangle( rc );
	}
}

bool VdkWindow::IsDoubleBuffering() const
{
#ifndef __VDK_NOT_USE_DOUBLE_BUFFERING__
	return true;
#else
	return false;
#endif // __VDK_NOT_USE_DOUBLE_BUFFERING__
}

void VdkWindow::SetCtrlOnHover(VdkControl* pCtrl)
{
	ms_hovering = pCtrl;
}

void VdkWindow::ResetCtrlOnHover()
{
	ms_hovering = NULL;
}

inline void VdkWindow::SetLastMouseEvent(VdkMouseEventType evtCode)
{
	if( !m_mouseEventNotForMe )
	{
		m_nLastMouseEvent = evtCode;
	}
	else
	{
		m_mouseEventNotForMe = false;
	}
}

inline void VdkWindow::SetCtrlOnFocus(VdkControl* pCtrl)
{
	m_focus = pCtrl;
}

inline void VdkWindow::ResetCtrlOnFocus()
{
	m_focus = NULL;
}

VdkMenu*& VdkWindow::GetMenuOnShow()
{
	static VdkMenu* s_menuOnShow = NULL;
	return s_menuOnShow;
}

void VdkWindow::SetMenuOnShow(VdkMenu* menu)
{
	VdkMenu*& menuOnShow = GetMenuOnShow();

	// ��ʾ֮ǰû�н�������ʾ������һ���˵����أ�
	if( menu && menuOnShow && menu != menuOnShow )
	{
#ifdef __WXDEBUG__
		wxASSERT( false );
#else
		menuOnShow->Return();
#endif // __WXDEBUG__
	}

	menuOnShow = menu;
}

VdkMenu* VdkWindow::CreateMenu(VdkMenu* parent, int mid)
{
	return new VdkMenu( mid, this, parent );
}

inline bool VdkWindow::IsActivatableEvent(int evtCode)
{
	return evtCode == LEFT_DOWN || evtCode == RIGHT_UP;
}

//////////////////////////////////////////////////////////////////////////

void VdkWindow::GetDcOrigin(int* x, int* y) const
{
	if( x )
		*x = m_xDcOrigin;

	if( y )
		*y = m_xDcOrigin;
}

void VdkWindow::SetDcOrigin(int x, int y)
{
	m_xDcOrigin = x;
	m_yDcOrigin = y;
}

void VdkWindow::ResetDcOrigin(wxDC& dc) const
{
	dc.SetDeviceOrigin( m_xDcOrigin, m_yDcOrigin );
}

//////////////////////////////////////////////////////////////////////////

void VdkWindow::SetCachedDC(VdkDC& vdc)
{
	m_cachedDC = &vdc;
}

void VdkWindow::PrepareCachedDC()
{

}

void VdkWindow::ResetCachedDC(wxDC& pdc)
{
	m_cachedDC = NULL;

	if( m_postListeners )
	{
		wxVector< VdkDcPostListener* >::iterator i;
		for( i = m_postListeners->begin(); i != m_postListeners->end(); ++i )
		{
			(*i)->OnPost( pdc );
		}
	}
}

void VdkWindow::AddPostDrawListener(VdkDcPostListener* lsner)
{
	if( !m_postListeners )
		m_postListeners = new wxVector< VdkDcPostListener* >;

	m_postListeners->push_back( lsner );
}

void VdkWindow::RemovePostDrawListener(VdkDcPostListener* lsner)
{
	if( m_postListeners )
	{
		wxVector< VdkDcPostListener* >::iterator i( m_postListeners->begin() );
		for( ; i != m_postListeners->end(); ++i )
		{
			if( *i == lsner )
			{
				m_postListeners->erase( i );
				return;
			}
		}
	}
}
