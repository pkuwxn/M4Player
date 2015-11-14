#include "StdAfx.h"
#include "VdkSlider.h"
#include "VdkDC.h"
#include "VdkButton.h"
#include "VdkWindow.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkSliderInitializer::VdkSliderInitializer()
   : BitmapArray( NULL ),
     HoldBitmapArrayPointer( false ),
     Vertical( false ),
	 Expl( 0 ),
	 ThumbTileLen( 0 ),
	 ThumbResizeType( RESIZE_TYPE_TILE )
{

}

VdkSliderInitializer::~VdkSliderInitializer()
{
	if( HoldBitmapArrayPointer )
	{
		wxDELETEA( BitmapArray );
	}
}

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLONEABLE_VOBJECT( VdkSlider, VdkControl );

// 用于回调函数（拖动的最小间隔40ms）
const static int gs_minCallbackInternal = 40;

void VdkSlider::Init()
{
	m_pThumb = NULL;
	m_nThumbTileType = RESIZE_TYPE_TILE;
	m_nThumbTile = 1;
	m_dragTimeStamp = 0;

	SetAddinStyle( VCS_ERASE_BG | VCS_CTRL_CONTAINER );
}

void VdkSlider::Create(wxXmlNode* node)
{
	wxString SliderFileNames[] = {
		XmlGetChildContent( node, L"bar_image" ),
		XmlGetChildContent( node, L"fill_image" ),
		XmlGetChildContent( node, L"thumb_image" ),
	};

	for( int i = 0; i < 3 ; i++ )
	{
		if( !SliderFileNames[i].Trim( true ).Trim( false ).IsEmpty() )
		{
			SliderFileNames[i] = GetFilePath( SliderFileNames[i] );
		}
	}

	bool bVertical = false;
	if( XmlGetChildContent( node, L"vertical" ).CmpNoCase( L"true" ) == 0 )
	{
		bVertical = true;
	}

	int nTile = 1;
	wxXmlNode* chd = FindChildNode( node, L"thumb_resize_center" );
	if( chd )
	{
		nTile = XmlGetContentOfNum( chd, nTile );
	}

	Create( VdkSliderInitializer().
		    bitmapArray( GetBitmaps( SliderFileNames ), true ).
			vertical( bVertical ).
			thumbExpl( 4 ). // TODO:
			thumbResizeType( RESIZE_TYPE_TILE ).
			thumbTileLen( nTile ).
			window( m_Window ).
			name( GetXrcName( node ) ).
			rect( GetXrcRect( node ) ) );
}

/*static*/
wxBitmap* VdkSlider::GetBitmaps(wxString* strFileNames)
{
	wxBitmap* bmArray = new wxBitmap[3];
	for( int i = 0; i < 3; i++ )
	{
		if( !strFileNames[i].empty() )
		{
			LoadMaskBitmap( bmArray[i], strFileNames[i] );
		}
	}

	return bmArray;
}

void VdkSlider::Create(const VdkSliderInitializer& init_data)
{
	wxASSERT( init_data.BitmapArray );

	VdkControl::Create( init_data );

	m_nThumbTile = init_data.ThumbTileLen;
	m_nThumbTileType = init_data.ThumbResizeType;

	m_bmBar = init_data.BitmapArray[0];
	wxBitmap& bmFull = init_data.BitmapArray[1];
	wxBitmap& bmThumb = init_data.BitmapArray[2];

    // 假如手柄位图无效，生造一个给它
    // TODO
	bool thumbIsOk = bmThumb.IsOk();
	if( !thumbIsOk )
        bmThumb = wxBitmap( 5, m_Rect.height );

	m_nMinHeight = bmThumb.GetHeight();
	m_bVertical = init_data.Vertical;
	wxRect rcThumb;

	if( !m_bVertical )
	{
		int nThumbHeight = bmThumb.GetHeight();
		rcThumb.y = ( m_Rect.height - nThumbHeight ) / 2;

		// 使自己的作用域包裹手柄，否则 VdkSlider 不会重画背景
		if( nThumbHeight > m_Rect.height )
		{
			m_Rect.y -= - rcThumb.y;
			m_Rect.height = nThumbHeight;
		}

		rcThumb.width = bmThumb.GetWidth() / init_data.Expl;
		rcThumb.height = nThumbHeight;

		//////////////////////////////////////////////////////////////////////////

		Bitmap bmFullImp = { m_Rect.width, m_Rect.height };
		Bitmap bmHandleImp = 
			{ bmThumb.GetWidth() / init_data.Expl, bmThumb.GetHeight() };
		SliderImp imp = { 0,
						  INVALID_MOUSE_ON_THUMB,
						  SD_NONE,
						  0,
						  bmFullImp,
						  bmHandleImp
		};

		m_imp = imp;
	}
	else // END if( !m_bVertical )
	{
		int nThumbWidth = bmThumb.GetWidth() / init_data.Expl;
		rcThumb.x = ( m_Rect.width - nThumbWidth ) / 2;

		// 使自己的作用域包裹手柄，否则 VdkSlider 不会重画背景
		if( nThumbWidth > m_Rect.width )
		{
			m_Rect.x -= -rcThumb.x;
			m_Rect.width = nThumbWidth;
		}

		rcThumb.width = nThumbWidth;
		rcThumb.height = bmThumb.GetHeight();

		//////////////////////////////////////////////////////////////////////////

		Bitmap bmFullImp = { m_Rect.height, m_Rect.width };
		Bitmap bmHandleImp = 
			{ bmThumb.GetHeight(), bmThumb.GetWidth() / init_data.Expl };
		SliderImp imp = { 0,
						  INVALID_MOUSE_ON_THUMB,
						  SD_NONE,
						  0,
						  bmFullImp,
						  bmHandleImp
		};

		m_imp = imp;
	}

	//////////////////////////////////////////////////////////////////////////

	m_bmFull = bmFull;
	if( !thumbIsOk )
        bmThumb = wxNullBitmap;

	m_pThumb = new VdkButton;
	m_pThumb->Create( VdkButtonInitializer().
					  window( m_Window ).
					  parent( this ).
					  name( m_strName + L"_thumb" ).
					  addToWindow( true ).
					  bitmap( bmThumb ).
					  rect( rcThumb ).
					  explode( init_data.Expl ).
					  style( VCS_ERASE_BG | VCS_REDIRECT_TO_PARENT ).
					  resizeable( true ).
					  tileType( init_data.ThumbResizeType ).
					  tileLen( init_data.ThumbTileLen )	);
}

void VdkSlider::Clone(VdkSlider* o)
{
	wxASSERT( CheckSourceCtrl< VdkButton >( m_firstChild ) );

	m_imp = o->m_imp;
	m_bVertical = o->m_bVertical;
	m_nMinHeight = o->m_nMinHeight;
	m_nThumbTileType = o->m_nThumbTileType;
	m_nThumbTile = o->m_nThumbTile;
	m_bmBar = o->m_bmBar; // TODO:
	m_bmFull = o->m_bmFull;

	m_pThumb = (VdkButton *) m_firstChild;
	GoTo( 0, NULL, false );
}

void VdkSlider::DoEraseBackground(wxDC& dc, const wxRect& rc)
{
	if( m_bmBar.IsOk() )
	{
		int x, y, w, h, dw( m_Rect.width ), dh( m_Rect.height );
		if( m_bVertical )
		{
			int bh( m_bmBar.GetHeight() );

			x = 0;
			y = bh / 4;
			w = m_bmBar.GetWidth();
			h = (bh > 1) ? bh / 2 : bh;

			dw = 0;
		}
		else
		{
			int bw( m_bmBar.GetWidth() );

			x = bw / 4;
			y = 0;
			w = (bw > 1) ? bw / 2 : bw;
			h = m_bmBar.GetHeight();

			dh = 0;
		}

		wxRect rcTile( x, y, w, h );
		TileDrawBitmap( dc, m_bmBar, m_Rect.x, m_Rect.y, dw, dh, rcTile );
	}
	else if( m_bgBrush.IsOk() )
	{
		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.SetBrush( m_bgBrush );
		dc.DrawRectangle( m_Rect );
	}

	if( m_bmFull.IsOk() )
	{
		int w = 0, h = 0;
		if( GetProgress() == 1 )
			w = m_imp.bmFull.work_side;
		else if( m_imp.curr > 0 )
			w = m_imp.curr + m_imp.bmHandle.work_side / 2;

		h = m_imp.bmFull.non_work_side;

		if( m_bVertical )
			wxSwap( w, h );

		//////////////////////////////////////////////////////////////////////////

		if( w || h )
		{
			int dX = ( m_Rect.width - m_bmFull.GetWidth() ) / 2;
			int dY = ( m_Rect.height - m_bmFull.GetHeight() ) / 2;
			// dX < 0 时，色条比 VdkSlider 的作用域宽
			int srcX = dX < 0 ? -dX : 0;
			int srcY = dY < 0 ? -dY : 0;
			int destX = dX > 0 ? m_Rect.x + dX : m_Rect.x;
			int destY = dY > 0 ? m_Rect.y + dY : m_Rect.y;

			if( m_bVertical && TestStyle( VSS_VERTICAL_BOTTOM_UP ) )
			{
				srcY = h;

				h = m_imp.bmFull.work_side - h;
				destY = m_Rect.y + m_Rect.height - h;
			}

			wxMemoryDC mdc( m_bmFull );
			dc.Blit( destX, destY, w, h, &mdc, srcX, srcY, wxCOPY, true );
		}
	}
}

void VdkSlider::DoDraw(wxDC& dc)
{
	if( !m_pThumb->IsEnabled() )
		return;

	VdkButton::State state;
	if( m_imp.ptr != INVALID_MOUSE_ON_THUMB )
		state = VdkButton::PUSHED;
	else
		state = VdkButton::NORMAL;

    // 滚动条是可以自走的，故手柄可能不知不觉间走到了指针下
	if( state == VdkButton::NORMAL )
	{
		if( m_pThumb->ContainsPointerGlobally() )
			state = VdkButton::HOVERING;
	}

	// 只需更新按钮的状态，重画事件由 VDK 分发
	m_pThumb->Update( state, NULL );
}

void VdkSlider::DoHandleMouseEvent(VdkMouseEvent& e)
{
	int evtCode = e.evtCode;
	// 当在手柄上发生滚轮事件，将其重定向至 VdkScrollBar 。
	// 而发生在 VdkSlider 上的滚轮事件已被 VdkScrollBar 抢先处理。
	if( (evtCode == WHEEL_UP) || (evtCode == WHEEL_DOWN) )
	{
		if( m_parent )
			m_parent->HandleMouseEvent( e );

		return;
	}

	if( evtCode != DRAGGING )
	{
		switch( evtCode )
		{
		case NORMAL:

			// 这是由“即点即达”特性激发的，忽略它
			if( TestState( VSST_LAST_CLICK_TO_GO ) )
			{
				wxASSERT( TestStyle( VSS_CLICK_TO_GO ) );
				RemoveState( VSST_LAST_CLICK_TO_GO );

				return;
			}

			m_imp.Release();

            // 拖动手柄放开后，不再在手柄作用域内：
			// 手柄上次的状态不会是 DRAGGING ，因为它直接忽略 DRAGGING 事件，
			// 不处理因此也就不会维持这个状态，它维持的是 LEFT_DOWN
			if( m_pThumb && m_pThumb->GetLastState() == VdkButton::PUSHED )
			{
				if( IsReadyForEvent() )
					FireEvent( &e.dc, (void *) NORMAL );
			}

			m_pThumb->Update( VdkButton::NORMAL, &e.dc );

			break;

		case LEFT_UP:

			m_imp.Release();

			// 只是在手柄上点击一下，不拖动
			if( TestState( VSST_DONT_FIRE_EVENT ) )
			{
				RemoveState( VSST_DONT_FIRE_EVENT );
				break;
			}

			if( TestStyle( VSS_CLICK_TO_GO ) &&
				m_pThumb && (m_pThumb->GetLastState() == VdkButton::PUSHED) )
			{
				m_pThumb->Update( VdkButton::HOVERING, &e.dc );
			}

			// 直接在滚动条上单击，此时手柄没有接收到 HOVERING 事件，
			// 故鼠标指针不会变化
			if( TestStyle( VCS_HAND_CURSOR ) )
				m_Window->AssignCursor( wxCursor( wxCURSOR_HAND ) );

			// 在手柄上释放鼠标指针
            if( IsReadyForEvent() )
                FireEvent( &e.dc, (void *) NORMAL );

			break;

		case LEFT_DOWN:
			{
				// 保存鼠标的当前位置
				wxRect rcThumb( m_pThumb->GetAbsoluteRect() );
				m_imp.ptr = ( m_bVertical ? (e.mousePos.y - rcThumb.y) :
											(e.mousePos.x - rcThumb.x) );

				//==================================================

				// 铅直滚动条暂时不提供“即点即至”的特性
				if( m_bVertical || !TestStyle( VSS_CLICK_TO_GO ) )
				{
					break;
				}

				bool go = true;
				if( m_pThumb && m_pThumb->GetAbsoluteRect().Contains( e.mousePos ) )
				{
					go = false;
					SetAddinState( VSST_DONT_FIRE_EVENT );
				}

				// 确定需要移动手柄到指针所处位置
				if( go )
				{
					SetAddinState( VSST_LAST_CLICK_TO_GO );

					int thumbWidth;
					m_pThumb->GetSize( &thumbWidth, NULL );

					wxRect rcValid( GetAbsoluteRect() );
					rcValid.x += thumbWidth / 2;
					rcValid.width -= thumbWidth;

					double percentage = (e.mousePos.x - rcValid.x) /
						static_cast< double >( rcValid.width );

					if( percentage < 0 )
						percentage = 0;
					else if( percentage > 1 )
						percentage = 1;

					m_imp.ptr = m_imp.bmHandle.work_side / 2;
					GoTo( percentage, &e.dc );

					if( m_pThumb )
						m_pThumb->Update( VdkButton::PUSHED, &e.dc );

					if( IsReadyForEvent() )
						FireEvent( &e.dc, (void*) LEFT_DOWN );
				}

				break;
			}

		// 手柄保持状态必须，不能跳过
		case HOVERING:
			break;

		default:

			e.Skip(true);
			break;
		}

		return;
	}

	// 别人的拖动事件
	if( !m_imp.IsOk() )
		return;

	// 直接拖动 VdkSlider 的空白处，
	// 但即点即至的 VdkSlider 允许拖动空白处
	if( !m_pThumb->GetAbsoluteRect().Contains( e.mousePos ) &&
		!m_imp.IsOk() &&
		!TestStyle( VSS_CLICK_TO_GO ) )
	{
		return;
	}

	// 清除不要发送事件的标志位
	RemoveState( VSST_DONT_FIRE_EVENT );

	// 正式处理拖动事件
	wxRect rcHandle( m_pThumb->GetRect() ),
		   rcThis( GetAbsoluteRect() );

	if( !m_bVertical )
	{
		m_imp.HandleDrag( e.mousePos.x - rcThis.x );
		rcHandle.x = m_imp.curr;
	}
	else
	{
		m_imp.HandleDrag( e.mousePos.y - rcThis.y );
		rcHandle.y = m_imp.curr;
	}

	// 一定要设置 pDC， 还原原来位置的背景（ VdkSlider 不一定会完全覆盖手柄的作用域）
	m_pThumb->SetRect( rcHandle, &e.dc );
	Draw( e.dc );

	// 拖动的中途通知
	wxMilliClock_t curr( wxGetLocalTimeMillis() );
	if( curr - m_dragTimeStamp >= gs_minCallbackInternal )
	{
		m_dragTimeStamp = curr;
		FireEvent( &e.dc, (void* ) DRAGGING );
	}

	//////////////////////////////////////////////////////////////////////////

	if( m_parent )
	{
		evtCode = NORMAL;
		if( m_imp.direction == SD_LESS )
			evtCode = SCROLLING_UP;
		else if( m_imp.direction == SD_MORE )
			evtCode = SCROLLING_DOWN;

		if( evtCode != NORMAL )
		{
			e.evtCode = evtCode;
			m_parent->HandleMouseEvent( e );
		}
	}
}

void VdkSlider::GoTo(double percentage, wxDC* pDC, bool fireCallback)
{
	m_imp.MoveTo( percentage );

	if( m_pThumb->IsOnShow() )
	{
		wxRect rc( m_pThumb->GetRect() );

		if( m_bVertical )
			rc.y = m_imp.curr;
		else
			rc.x = m_imp.curr;

		m_pThumb->SetRect( rc, NULL );
	}

	if( pDC )
		Draw( *pDC );

	if( fireCallback )
		FireEvent( pDC, NULL );
}

VdkSlider::SliderDirection VdkSlider::GetDirection
	(int curr, VdkSlider::SliderDirection& lastDirection) const
{
	lastDirection = m_imp.direction;
	curr -= GetAbsoluteRect().y;

	if( curr < m_imp.curr )
	{
		return SD_LESS;
	}
	else
	{
		if( (curr > m_imp.curr + m_imp.bmHandle.work_side) ||
			// 这种情况适用于手柄被隐藏后：我们决定此时不更新 
			// m_imp.bmHandle.work_side
			(curr > m_Rect.height) )
		{
			return SD_MORE;
		}
		else
		{
			return SD_NONE;
		}
	}
}

double VdkSlider::GetProgress(wxRect* pRect) const
{
	if( pRect )
	{
		*pRect = m_pThumb->GetAbsoluteRect();
	}

	return m_imp.percent;
}

void VdkSlider::UpdateThumbState(wxDC* pDC)
{
	/* 在这里吃了一个大亏，不能判断手柄是否显示：
	 * if( !m_pThumb->IsShown() )
	 *		return;
	 * 否则被暂时冻结而不是隐藏的手柄（如 VdkTab 中的手柄）不能如实反映
	 * 手柄作用域（ m_imp.bmHandle.work_side ）的更改
	 */
	// 这里必须是隐藏的而不是被冻结的
	if( !m_pThumb->IsOnShow() )
		return;

	wxRect rcThumb( m_pThumb->GetRect() );

	if( m_bVertical )
	{
		rcThumb.y = ( m_Rect.height - rcThumb.height ) * m_imp.percent;
		m_imp.curr = rcThumb.y;
		m_imp.bmHandle.work_side = rcThumb.height;
		m_imp.bmHandle.non_work_side = rcThumb.width;
	}
	else
	{

	}

	m_pThumb->SetRect( rcThumb, pDC );
	if( pDC )
		Draw( *pDC );
}

void VdkSlider::ResetDirection()
{
	m_imp.direction = SD_NONE;
}

void VdkSlider::DoHandleNotify(const VdkNotify& notice)
{
	switch( notice.GetNotifyCode() )
	{
	case VCN_WINDOW_RESIZED:
	case VCN_SIZE_CHANGED:

		if( notice.GetLparam() && m_bVertical &&
			(( m_align & ALIGN_SYNC_Y ) || (m_align & ALIGN_SYNC_X_Y)) )
		{
			m_imp.bmFull.work_side += notice.GetLparam();

			if( m_bmBar.IsOk() )
			{
				wxRect rcTile( 0, 0, m_bmBar.GetWidth(), m_bmBar.GetHeight() );
				TileBitmap( m_bmBar, 0, m_Rect.height, rcTile );
			}
		}

		break;

	default:

		break;
	}

}

//////////////////////////////////////////////////////////////////////////

void VdkSlider::SliderImp::HandleDrag(int mousePos)
{
	MoveTo( static_cast< double > ( mousePos - ptr ) /
				( bmFull.work_side - bmHandle.work_side ) );
}

void VdkSlider::SliderImp::Init()
{
	memset( this, 0, sizeof( SliderImp ) );
}

void VdkSlider::SliderImp::MoveTo(double p)
{
	if( p < 0 )
		p = 0;
	else if( p > 1 )
		p = 1;

	if( p > percent )
		direction = SD_MORE;
	else if( p < percent )
		direction = SD_LESS;
	else
		direction = SD_NONE;

	int end( bmFull.work_side - bmHandle.work_side );
	if( end < 0 )
		end = 0; // 手柄因为容纳不下被隐藏了

	percent = p;
	curr = end * percent;

	if( curr > end )
		curr = end;
	else if( curr < 0 )
		curr = 0;
}

VdkSlider::SliderImp& VdkSlider::SliderImp::operator=(const SliderImp& rhs)
{
	Init();

	ptr = rhs.ptr;
	bmHandle = rhs.bmHandle;
	bmFull = rhs.bmFull;

	return *this;
}
