#include "StdAfx.h"
#include "VdkScrollBar.h"

#include "VdkWindow.h"
#include "VdkSlider.h"
#include "VdkButton.h"
#include "wxUtil.h"

#include <wx/ptr_scpd.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLONEABLE_VOBJECT( VdkScrollBar, VdkControl );

#include "Images/scrollbar_all.xpm"

void VdkScrollBar::Init()
{
	m_pSlider = NULL;
	m_pGoUp = NULL;
	m_pGoDown = NULL;
	m_bNotHoldOn = false;

	// VdkSlider �� container��ͬʱ VdkScrollBar �� VdkSlider �����
	// VdkWindow �Ŀռ��б���Ϊʲô VdkScrollBar ��Ҫ�� container �أ�
	// ��������ʹ�� VdkSlider ��Զ���޷����յ������������֮�ϵ���Ϣ����
	// ��Ϊ������ס����¼������� VdkSlider �������������Ͳ���Ӧ������������
	SetAddinStyle( VCS_CTRL_CONTAINER | VCS_HONLD_ON );
}

wxDECLARE_SCOPED_ARRAY( wxString, wxStringArrayPtr )
wxDEFINE_SCOPED_ARRAY( wxString, wxStringArrayPtr )

void VdkScrollBar::Create(wxXmlNode* node)
{
	wxString strFileNames[3];

	strFileNames[0] = XmlGetChildContent( node, L"buttons_image" );
	strFileNames[1] = XmlGetChildContent( node, L"thumb_image" );
	strFileNames[2] = XmlGetChildContent( node, L"bar_image" );

	for( int i = 0; i < 3; i++ )
	{
		if( !strFileNames[i].Trim().Trim( false ).IsEmpty() )
			strFileNames[i] = GetFilePath( strFileNames[i] );
	}

	//////////////////////////////////////////////////////////////////////////

	wxString strTmp;
	VdkResizeableBitmapType nThumbTileType( RESIZE_TYPE_TILE );
	int nThumbTile( 0 ), nExpl( 4 );

	strTmp = XmlGetChildContent( node, L"thumb_resize_tile" );
	if( strTmp.IsEmpty() || strTmp == L"0" )
		nThumbTileType = RESIZE_TYPE_STRETCH;

	nThumbTile = XmlGetContentOfNum( FindChildNode( node, L"thumb_resize_center" ) );
	nExpl = XmlGetContentOfNum( FindChildNode( node, L"thumb_expl" ) );

	//////////////////////////////////////////////////////////////////////////

	Create( VdkScrollBarInitializer().
			window( m_Window ).
			name( GetXrcName( node ) ).
			rect( GetXrcRect( node ) ).
			align( GetXrcAlign( node ) ).
			bitmapArray( GetBitmapArray( strFileNames, nExpl ), true ).
			thumbExpl( nExpl ).
			thumbResizeType( nThumbTileType ).
			thumbTileLen( nThumbTile ).
			castTo< VdkScrollBarInitializer >() );
}

void VdkScrollBar::Create(const VdkScrollBarInitializer& init_data)
{
	bool addToWindow = init_data.AddToWindow;
	VdkScrollBarInitializer& init_data_ref = 
		const_cast< VdkScrollBarInitializer& >( init_data );

	// ����Ҫ�� VdkSlider �ټ��� VdkWindow �Ŀؼ��б�
	// ���Ǳ��뱣֤ VdkScrollBar �����ȼ��� VdkSlider �ߣ�
	// ���ȴ���ĳЩ�¼����������ס���� LEFT_DOWN �� MOUSE_HOLD_ON_RELEASED �ȵȡ�
	init_data_ref.addToWindow( false );
	VdkControl::Create( init_data_ref );

	//////////////////////////////////////////////////////////////////////////

	if( m_Rect.IsEmpty() )
	{
		if( m_parent )
		{
			m_Rect = m_parent->GetRect();
			m_Rect.x = m_Rect.y = 0;
		}
		else
		{
			wxASSERT( false );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// rc ���� VdkScrollBar ��ʵ��λ�ã�����һ���� VdkScrollBar ������ʾ������

	wxBitmap& bmBtns = init_data.BitmapArray[0];

	int btnWidth( bmBtns.GetWidth() / init_data.Expl );
	int btnHeight( bmBtns.GetHeight() / 2 );

	// ���� x ���꣬�μ���һ��ע��(���ﲻ���� rt.GetRight() ,������1)
	m_Rect.x = m_Rect.x + m_Rect.width - btnWidth;
	m_Rect.y = m_Rect.y + btnHeight;
	m_Rect.width = btnWidth;
	m_Rect.height = m_Rect.GetHeight() - btnHeight * 2;

	//////////////////////////////////////////////////////////////////////////

	// ������ȡ���ϡ����°�ť
	wxRect rcSubBitmap( 0, 0, btnWidth * init_data.Expl, btnHeight );
	wxBitmap bmSub( VdkUtil::GetSubBitmapWithoutMask( bmBtns, rcSubBitmap ) );

	//----------------------------------------------------
	// ������ϰ�ť

	wxRect btRect( 0, - btnHeight, btnWidth, btnHeight );

	m_pGoUp = new VdkButton;
	m_pGoUp->Create( VdkButtonInitializer(). 
					 parent( this ).
					 addToWindow( true ).
					 window( m_Window ).
					 style( VCS_HONLD_ON | VCS_REDIRECT_TO_PARENT ).
					 name( m_strName + L"_up" ).
					 bitmap( bmSub ).
					 rect( btRect ).
					 explode( init_data.Expl ) );
		
	//----------------------------------------------------
	// ������°�ť

	rcSubBitmap.y += btnHeight;
	bmSub = VdkUtil::GetSubBitmapWithoutMask( bmBtns, rcSubBitmap );
	btRect.y = m_Rect.height;

	m_pGoDown = new VdkButton;
	m_pGoDown->Create( VdkButtonInitializer(). 
					   parent( this ).
					   addToWindow( true ).
					   window( m_Window ).
					   name( m_strName + L"_down" ).
					   rect( btRect ).
					   align( ALIGN_BOTTOM ).
					   style( VCS_HONLD_ON | VCS_REDIRECT_TO_PARENT ).
					   bitmap( bmSub ).
					   explode( init_data.Expl ) );

	//----------------------------------------------------
	// ��� VdkSlider

	init_data_ref.BitmapArray[0] = init_data_ref.BitmapArray[2];
	init_data_ref.BitmapArray[2] = init_data_ref.BitmapArray[1];
	init_data_ref.BitmapArray[1] = wxNullBitmap;

	btRect.y = 0;
	btRect.height = m_Rect.height;

	m_pSlider = new VdkSlider;
	m_pSlider->Create( init_data_ref.
					   vertical( true ).
					   parent( this ).
					   window( m_Window ).
					   name( m_strName + L"_slider" ).
					   rect( btRect ).
					   align( ALIGN_SYNC_Y ).
					   style( VCS_HONLD_ON ).
					   addToWindow( true ) );

	m_pSlider->SetBackground( wxBrush( wxColour( 251,251,249 ) ) );

	//////////////////////////////////////////////////////////////////////////
	// ����һЩ��Ա����

	if( addToWindow )
		m_Window->AddCtrl( this );

	m_nWidth = btnWidth;
}

void VdkScrollBar::Clone(VdkScrollBar* o)
{
	m_nWidth = o->m_nWidth;
	
	m_pGoUp = (VdkButton *) m_firstChild;
	m_pGoDown = (VdkButton *) m_pGoUp->GetNext();
	m_pSlider = (VdkSlider *) m_pGoDown->GetNext();
}

void VdkScrollBar::DoHandleMouseEvent(VdkMouseEvent& e)
{
	if( m_parent && m_pSlider )
	{
		if( e.evtCode != LEFT_DOWN && e.evtCode != DLEFT_DOWN )
			m_bNotHoldOn = true;

		wxRect rcThumb( m_pSlider->GetThumb()->GetAbsoluteRect() );
		switch( e.evtCode )
		{
		// ������ס������
		case MOUSE_HOLD_ON_RELEASED:

			m_pSlider->ResetDirection();
			break;

		case LEFT_DOWN:
		case DLEFT_DOWN:
		{
			VdkSlider::SliderDirection nDirection, nLastDirection;
			nDirection = m_pSlider->GetDirection( e.mousePos.y, nLastDirection );

			// �ֱ������غ����հ״��޷��жϾ����û��������¹��������෴��
			// ��ֱ�Ӷ�������¼�����������
			if( nDirection == VdkSlider::SD_NONE )
				break;

			// �߼���Ҫ��϶����ΰ�ס��ť�¼�ʱ������ģ��������ϴι�������
			// ������������ģ������һ�ξ��ǰ�ס����ʱ������ȷ��Ӧ������ϴ�
			// ���������뱾�β�ͬ���ֱ�����������Ծʱ�ͻᵼ�³��ֲ�ͬ�������
			// �ϴι����ķ���������Դ�������֣�����ס������������������
			// ��ͬ���ߣ���������ɵ�״̬ת������Ӧ���������ס������
			if( nLastDirection && nDirection != nLastDirection && !m_bNotHoldOn )
				break;

			m_bNotHoldOn = false;

			// ���Ǹ������¼���ʶ�룬�����½�һ���¼�����
			VdkMouseEvent fakeEvent( e );
			if( fakeEvent.mousePos.y < rcThumb.y )
			{
				fakeEvent.evtCode = WHEEL_UP;
			}
			else
			{
				fakeEvent.evtCode = WHEEL_DOWN;
			}

			m_parent->HandleMouseEvent( fakeEvent );

			break;
		}

		case WHEEL_UP:
		case WHEEL_DOWN:
		case SCROLLING_UP:
		case SCROLLING_DOWN:
			
			m_parent->HandleMouseEvent( e );
			break;

		default:
			break;
		}
	}
}

int VdkScrollBar::GetThumbHeight()
{
	return m_pSlider->GetThumb()->GetRect().height;
}

void VdkScrollBar::SetThumbHeight(int height, wxDC* pDC)
{
	VdkButton* pThumb = const_cast<VdkButton *> ( m_pSlider->GetThumb() );

	wxRect rc( m_pSlider->GetRect() );
	m_Rect.width = rc.width;
	m_Rect.height = rc.height;

	int h;
	pThumb->GetSize( NULL, &h );
	if( h == height )
		return;

	int min;
	pThumb->GetMinSize( NULL, &min );
	if( min > height )
		height = min;

	// ���ø߶ȱ�Ҫ���С���ݲ����ֱ�
	if( height >= m_Rect.height )
	{
		if( pThumb->IsOnShow() )
			pThumb->Show( false, pDC );

		return;
	}
	else if( !pThumb->IsOnShow() && IsOnShow() )
		pThumb->Show( true, pDC );

	pThumb->Resize( 0, height );
	m_pSlider->UpdateThumbState( pDC );
}

bool VdkScrollBar::IsThumbOnShow() const
{
	return m_pSlider ? m_pSlider->GetThumb()->IsOnShow() : false;
}

void VdkScrollBar::ShowThumb(bool bShow, wxDC* pDC)
{
	if( m_pSlider )
	{
		VdkButton* thumb = 
			const_cast<VdkButton*>( m_pSlider->GetThumb() );

		thumb->Show( bShow, pDC );
	}
}

void VdkScrollBar::GoTo(double percentage, wxDC* pDC)
{
	if( m_pSlider )
		m_pSlider->GoTo( percentage, pDC );
}

double VdkScrollBar::GetProgress()
{
	if( m_pSlider )
		return m_pSlider->GetProgress();
	else
		return 0;
}

void VdkScrollBar::DoHandleNotify(const VdkNotify& notice)
{
	switch( notice.GetNotifyCode() )
	{
	case VCN_WINDOW_RESIZED:
	case VCN_SIZE_CHANGED:

		if( notice.GetLparam() && m_pSlider )
		{
			if( m_Rect.height < GetThumbHeight() )
				ShowThumb( false, NULL );
			else
				GoTo( GetProgress(), NULL );
		}

		break;

	case VCN_ENABLED:
	case VCN_DISABLED:
		{
			bool enable = IsEnabled();
			wxDC* pDC( notice.GetVObjDC() );

			// ��ʾ/�����ֱ�
			if( m_pSlider )
			{
				VdkButton* thumb = 
					const_cast< VdkButton* >( m_pSlider->GetThumb() );

				if( enable )
				{
					// �ֱ��ɼ��Ա����������ͬ��
					if( !thumb->IsOnShow() && IsOnShow() )
						thumb->Show( true, pDC );
				}
				else
					thumb->Show( false, pDC );
			}

			if( m_pGoUp )
				m_pGoUp->Enable( enable, pDC );

			if( m_pGoDown )
				m_pGoDown->Enable( enable, pDC );

			break;
		}

	default:

		break;
	}
}

wxBitmap* VdkScrollBar::GetBitmapArray(wxString strFileNames[3], int& nExpl)
{
	wxBitmap* bmArray = new wxBitmap[3];

	if(!( strFileNames && 
		  LoadMaskBitmap( bmArray[0], strFileNames[0] ) && 
		  LoadMaskBitmap( bmArray[1], strFileNames[1] ) && 
		  LoadMaskBitmap( bmArray[2], strFileNames[2] ) ) )
	{
		nExpl = 4;

		// ����Ĭ��Ƥ��
		wxBitmap bmAll( SB_xpm );

		// bmBtns
		wxRect rcSub( 0, 0, 64, 32 );
		bmArray[0] = bmAll.GetSubBitmap( rcSub );

		// bmThumb
		rcSub.y = 32;
		rcSub.height = 16;
		bmArray[1] = bmAll.GetSubBitmap( rcSub );

		// bmBar
		/*rcSub.y = 48;
		rcSub.width = 16;
		bmArray[2] = bmAll.GetSubBitmap( rcSub );*/
	}

	return bmArray;
}
