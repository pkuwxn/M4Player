#include "StdAfx.h"
#include "VdkButton.h"
#include "VdkWindow.h"
#include "VdkSlider.h"
#include "VdkDC.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkButtonInitializer::VdkButtonInitializer()
   : Expl( 4 ), 
     TextAlign( 0 ), 
	 Resizeable(false),
	 TileType( RESIZE_TYPE_TILE ),
	 TileLen( 1 )
{

}

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLONEABLE_VOBJECT( VdkButton, VdkControl );
extern const int gs_invalidPadding = -999;

VdkButton::VdkButton()
		 : m_nTile( 0 ),
		   m_TextAlign( TEXT_ALIGN_CENTER_X_Y ),
		   m_nPaddingX( gs_invalidPadding ),
		   m_nPaddingY( gs_invalidPadding ),
		   m_nState( NORMAL )
{

}

void VdkButton::Create(wxXmlNode* node)
{
	wxBitmap bm;
	GetXrcImage( node, bm );

	int nExpl = XmlGetContentOfNum( FindChildNode( node, L"expl" ), 4 );
	Create( node, bm, nExpl );
}

VdkControl* VdkButton::Create(wxXmlNode* node,
							  const wxBitmap& bm,
							  int nExpl)
{
	// 按钮文本
	wxString strCaption( GetXrcTextBlock( node ) );
	if( !strCaption.IsEmpty() )
	{
		TextInfo ti = GetXrcTextInfo( node, m_WindowImpl );
		m_Font = ti.font;

		if( ti.xPadding != gs_invalidPadding )
			m_nPaddingX = ti.xPadding;

		if( ti.yPadding != gs_invalidPadding )
			m_nPaddingY = ti.yPadding;

		m_TextColor = ti.foreground;
	}

	// 提示信息
	wxXmlNode* chd = NULL;
	chd = FindChildNode( node, L"tip" );
	if( chd )
	{
		SetToolTip( chd->GetNodeContent() );
	}

	// 是否可以隐藏
	wxString val( XmlGetChildContent( node, L"transparent" ) );
	if( val.CmpNoCase( L"true" ) == 0 )
		SetAddinStyle( VCS_ERASE_BG );

	bool bFit = false;
	wxString strFit( XmlGetChildContent( node, L"fit" ) );
	if( strFit.CmpNoCase( L"true" ) == 0 )
		bFit = true;

	// TODO: TileLen 读入

	Create( VdkButtonInitializer().
		    window( m_Window ).
			name( GetXrcName( node ) ).
			caption( strCaption ).
			bitmap( bm ).
			rect( GetXrcRect( node ) ).
			align( GetXrcAlign( node ) ).
			resizeable( bFit == true ).
			explode( nExpl ) );

	if( bFit )
		Fit( NULL );

	return this;
}

void VdkButton::Create(const VdkButtonInitializer& init_data)
{
	VdkControl::Create( init_data );

	//===================================================

	m_strCaption = init_data.Caption;

	if( init_data.TextAlign )
		m_TextAlign = init_data.TextAlign;

	if( !m_TextColor.IsOk() )
		m_TextColor = *wxBLACK;

	//===================================================

	m_bmArray.Set( init_data.BkGnd, init_data.Expl, 1 );

	if( init_data.Resizeable )
	{
		SetAddinStyle( VBS_RESIZEABLE );

		if( m_bmArray.IsOk() )
		{
			m_nTile = init_data.TileLen;
			if( init_data.TileType == RESIZE_TYPE_TILE )
				SetAddinStyle( VBS_RESIZE_TYPE_TILE );

			// 这里感觉非常奇怪，不能在 Ubuntu/GTK 下用 GetSubBitmap
			wxBitmap bmPrimitive
				( init_data.BkGnd.GetWidth(), init_data.BkGnd.GetHeight() );
			wxMemoryDC mdc( bmPrimitive );
			mdc.DrawBitmap( init_data.BkGnd, 0, 0 );
			mdc.SelectObject( wxNullBitmap );

			m_bmPrimArray.Set( bmPrimitive, init_data.Expl, 1 );
		}
	}

	// 只处理第一次 HOVERING 事件
	SetAddinStyle( VCS_ONESHOT_HOVERING );
}

void VdkButton::Clone(VdkButton* o)
{
	m_nTile = o->m_nTile;
	m_bmArray = o->m_bmArray;
	m_bmPrimArray = o->m_bmPrimArray;

	m_nPaddingX = o->m_nPaddingX;
	m_nPaddingY = o->m_nPaddingY;

	m_strCaption = o->m_strCaption;
	m_TextAlign = o->m_TextAlign;

	m_TextColor = o->m_TextColor;
	m_Region = o->m_Region;

	m_nState = o->m_nState;
}

void VdkButton::GetSize(int* w, int* h) const
{
	// 当 VdkButton 作为其他控件的子控件时，
	// 其初始作用域（ XRC 文件中定义）可能为空

	if( w )
		*w = (m_Rect.width  == 0) ? m_bmArray.GetWidth()  : m_Rect.width;

	if( h )
		*h = (m_Rect.height == 0) ? m_bmArray.GetHeight() : m_Rect.height;
}

void VdkButton::GetMinSize(int* w, int* h) const
{
	if( TestStyle( VBS_RESIZEABLE ) )
	{
		if( w )
			*w = m_bmPrimArray.GetWidth();

		if( h )
			*h = m_bmPrimArray.GetHeight();
	}
	else
	{
		GetSize( w, h );
	}
}

void VdkButton::Update(VdkButton::State nState, wxDC* pDC)
{
	wxASSERT( !IsMenuOnShow() || (IsMenuOnShow() && nState == PUSHED) );
	
	//===================================================

	// 一个按钮要有“禁用”的状态，至少要有 4 个状态的图
	if( m_bmArray.GetExplNum() < 4 )
    {
        wxASSERT( m_nState != DISABLED );
    }

	// TODO: 补充为什么状态相同不能优化掉本次重画的理由
	SetButtonState( nState );

	if( pDC )
		Draw( *pDC );
}

void VdkButton::DoDraw(wxDC& dc)
{
	m_bmArray.BlitTo( dc, m_nState, 0, m_Rect.x, m_Rect.y );

	if( !m_strCaption.IsEmpty() )
	{
		// Expl == 4表明这是一个标准4状态按钮，假如
		// Expl < 4，表明这按钮不支持禁用状态；
		// Expl > 4，例如 VdkCheckBox ，4已经被其重新指定
		// 的状态参数占据，他们自己会处理文本颜色
		if( m_nState != DISABLED || m_bmArray.GetExplNum() != 4  )
			dc.SetTextForeground( m_TextColor );
		else
			dc.SetTextForeground( *wxLIGHT_GREY );

		dc.SetFont( m_Font );

		if( m_nPaddingX == gs_invalidPadding ||
			m_nPaddingY == gs_invalidPadding )
		{
			CalcTextPos();
		}

		dc.DrawText( m_strCaption, m_Rect.x + m_nPaddingX,
					 m_Rect.y + m_nPaddingY );
	}
}

void VdkButton::DoHandleMouseEvent(VdkMouseEvent& e)
{
	wxASSERT( !IsMenuOnShow() );
	
	//===================================================

	int evtCode = e.evtCode; // 防止 evtCode 会被父控件改变

	if( m_parent &&
		TestStyle( VCS_REDIRECT_TO_PARENT ) &&
		m_parent->CanHandleEvent() )
	{
		m_parent->HandleMouseEvent( e );
	}

	bool fireEvent = false;
	State state = (State) evtCode;
	switch( evtCode )
	{
		case NORMAL:
		case HOVERING:
		case LEFT_DOWN:

			break;

		case MOUSE_HOLD_ON_RELEASED:

			state = NORMAL;

			break;

		case DLEFT_DOWN:

			state = PUSHED;

			break;

		case LEFT_UP:
		{
			// 原先不是按下这个按钮的情况不能作默认处理
			// 例如原本按下的是另一个按钮，却不放起鼠标而是将指针拖到
			// 另一个按钮上
			if( m_nState == PUSHED )
				fireEvent = true;

			state = HOVERING;

			break;
		}

		case DRAGGING:

			return;

		default:

			// 注意是不符要求的事件类型会直接返回，
			// 所以不需要考虑父控件是否允许跳过本事件
			if( !m_parent )
				e.Skip( true );

			return;
	}

	Update( state, &e.dc );

	// 对回调函数的调用放到最后进行，因为用户可能在其代码中改变按钮的状态
	if( fireEvent && IsReadyForEvent() )
		FireEvent( &e.dc, NULL );
}

void VdkButton::EmuClick(wxDC& dc)
{
	wxPoint Point( GetAbsoluteRect().GetPosition() );
	Point.x++;
	Point.y++;

	VdkMouseEvent e( LEFT_DOWN, Point, dc );
	DoHandleMouseEvent( e );

	e.evtCode = LEFT_UP;
	DoHandleMouseEvent( e );
}

void VdkButton::DoHandleNotify(const VdkNotify& notice)
{
	switch( notice.GetNotifyCode() )
	{
	case VCN_DISABLED:

		// 更新内部值
		if( m_bmArray.GetExplNum() >= 4 )
			SetButtonState( DISABLED );

		break;

	case VCN_ENABLED:

		SetButtonState( NORMAL );

		break;

	default:

		break;
	}
}

bool VdkButton::Resize(int newwidth, int newheight)
{
	if( !TestStyle( VBS_RESIZEABLE ) )
		return false;

	// m_nTile 为0表明不能改变大小
	if( m_nTile == 0 )
		return false;

	if( (newheight != 0) && (unsigned( newheight ) != m_bmArray.GetHeight()) )
	{
		int w = m_bmPrimArray.GetWidth();
		int h = m_bmPrimArray.GetHeight();

		if( newheight >= h )
		{
			wxRect rc( 0, 0, w, h );
			wxRect rcTile( 0, double( h )/ 2 - m_nTile / 2, 
						   w, m_nTile );

			//===================================================

			unsigned nExpl = m_bmArray.GetExplNum();
            wxBitmap bmTmp( w, h );
 			wxBitmap bm( w * nExpl, newheight );
 			wxMemoryDC mdc( bmTmp ), mdcDest( bm );

			for( unsigned i = 0; i < nExpl; i++ )
			{
				m_bmPrimArray.BlitTo( mdc, i, 0, 0, 0 );
				mdc.SelectObject( wxNullBitmap );

				if( TestStyle( VBS_RESIZE_TYPE_TILE ) )
				{
					bmTmp = TileBitmap( bmTmp, 0, newheight, rcTile );
				}
				else
				{
					bmTmp = InflateBitmapByCenter
						( bmTmp, rcTile, wxSize( w, newheight ) );
				}

				// 不知道 wxBitmap::SetHeight 这个函数的初始设计思想是否这样，
				// 就是使高的 Bitmap 从 GetHeight() 体现得小一点。
				bmTmp.SetHeight( h );
                mdc.SelectObject( bmTmp );
                mdcDest.Blit( i * w, 0, w, newheight, &mdc, 0, 0, wxCOPY, true );
			}

			mdcDest.SelectObject( wxNullBitmap );
			m_bmArray.Set( bm, nExpl, 1 );

			//===================================================

			m_Rect.height = newheight;

			m_nPaddingY = gs_invalidPadding;
			CalcTextPos();

			return true;
		}

		return false;
	}

	if( (newwidth != 0) && (unsigned( newwidth ) != m_bmArray.GetWidth()) )
	{
		int w = m_bmPrimArray.GetWidth();
		int h = m_bmPrimArray.GetHeight();

		if( newwidth >= w )
		{
			int half( w / 2 );
			wxRect rc( 0, 0, w, h );
			wxRect rcTile( half / 2, 0, m_nTile, h );

			//===================================================

			unsigned nExpl = m_bmArray.GetExplNum();
			wxBitmap* bmArray = new wxBitmap[nExpl];
			wxBitmap* bmPrimArray = m_bmPrimArray.Explode();

			for( unsigned i = 0; i < nExpl; ++i )
			{
				bmArray[i] = bmPrimArray[i].GetSubBitmap( rc );
				if( newwidth == w )
					continue;

				// 边边角角单纯拉伸会导致变形、模糊化
				bmArray[i] = InflateBitmapByCenter
					( bmArray[i], rcTile, wxSize( newwidth, h ) );
			}

			//===================================================

			wxBitmap bm( newwidth * nExpl, m_bmArray.GetHeight() );
			wxMemoryDC mdc( bm );
			for( unsigned i = 0; i < nExpl; ++i )
				mdc.DrawBitmap( bmArray[i], i * newwidth, 0, true );

			mdc.SelectObject( wxNullBitmap );
			m_bmArray.Set( bm, nExpl, 1 );

			delete [] bmArray;
			delete [] bmPrimArray;

			//===================================================

			m_Rect.width = newwidth;

			if( m_TextAlign )
			{
				m_nPaddingX = gs_invalidPadding;
				CalcTextPos();
			}

			return true;
		}

		return false;
	}

	if( TestStyle( VBS_REGION ) )
		CreateRegion();

	return true;
}

void VdkButton::CalcTextPos()
{
	if( m_strCaption.IsEmpty() )
		return;

	int w, h;
	m_WindowImpl->GetTextExtent( m_strCaption, &w, &h, 0, 0, &m_Font );

	if( m_nPaddingX == gs_invalidPadding )
	{
		if( m_TextAlign & TEXT_ALIGN_CENTER_X_Y  )
			m_nPaddingX = ( m_Rect.width - w ) / 2;
		else if( m_TextAlign & TEXT_ALIGN_RIGHT  )
			m_nPaddingX = m_Rect.width - w - 5; // -5 是为了加多点容错
		else // 左对齐
			m_nPaddingX = m_WindowImpl->GetCharWidth();
	}

	if( m_nPaddingY == gs_invalidPadding )
		m_nPaddingY = (m_Rect.height - h) / 2;
}

void VdkButton::Fit(wxDC* pDC)
{
	SetCaption( m_strCaption, pDC );
}

void VdkButton::SetCaption(const wxString& cap, wxDC* pDC)
{
	m_strCaption.assign( cap );

	wxCoord w;
	m_WindowImpl->GetTextExtent( cap, &w, NULL, 0, 0, &m_Font );

	if( TestStyle( VBS_RESIZEABLE ) )
	{
		if( w >= m_Rect.GetWidth() * 0.8 || w * 1.2 <= m_Rect.GetWidth() )
		{
			wxRect rc( m_Rect );
			rc.width = w * 1.2;
			int minWidth( m_bmPrimArray.GetWidth() );
			if( rc.width < minWidth )
				rc.width = minWidth;

			if( rc.width != m_Rect.width )
			{
				if( pDC && rc.width < m_Rect.width )
					EraseBackground( *pDC, m_Rect );

				Resize( rc.GetWidth(), 0 );
			}
		}
	}

	m_nPaddingX = ( m_Rect.width - w ) / 2;

	if( pDC )
		Draw( *pDC );
}

bool VdkButton::HitTest(const wxPoint& mousePos) const
{
	if( TestStyle( VBS_REGION ) )
		return m_Region.Contains( DeTranslatePoint( mousePos ) ) == wxInRegion;
	else
		return VdkControl::HitTest( mousePos );
}

void VdkButton::CreateRegion()
{
	m_Region.Clear();
	RemoveStyle( VBS_REGION );

	if( !m_bmArray.IsOk() || !IsRegionCreationFailed() )
		return;

	wxBitmap bmAgent( m_bmArray.GetWidth(), m_bmArray.GetHeight() );
	wxMemoryDC mdc( bmAgent );
	mdc.SetBackground( *wxBLACK_BRUSH );
	mdc.Clear();
	m_bmArray.BlitTo( mdc, 0, 0, 0, 0 );
	mdc.SelectObject( wxNullBitmap );

	if( m_Region.Union( bmAgent, *wxBLACK ) && m_Region.IsOk() )
	{
		SetAddinStyle( VBS_REGION );
	}
	else
	{
		SetAddinState( VBST_REGION_FAILED );
	}
}

// TODO: 还需琢磨这里
const VdkBitmapArray& VdkButton::GetPrimaryBitmap() const
{
	return m_bmPrimArray.IsOk() ? m_bmPrimArray : m_bmArray;
}

//////////////////////////////////////////////////////////////////////////

void VdkWindow::InitRegionButtons()
{
	wxRect rc;

	VdkButton *vb, *vb2;
	VdkWindow::CtrlIter i, j;

	for( i = m_Ctrls.begin(); i != m_Ctrls.end(); ++i )
	{
		vb = dynamic_cast< VdkButton* >( *i );
		if( !vb )
			continue;

		// 不适合作为 RegionButton 或者已经准备好区域则退出当次循环
		if( !vb->IsRegionCreationFailed() || vb->IsRegionOk() )
			continue;

		rc = vb->GetAbsoluteRect();

		for( j = m_Ctrls.begin(); j != m_Ctrls.end(); ++j )
		{
			if( *i == *j )
				continue;

			vb2 = dynamic_cast<VdkButton*>( *j );
			if( !vb2 )
				continue;

			// 不适合作为 RegionButton 或者已经准备好区域则退出当次循环
			if( !vb2->IsRegionCreationFailed() || vb2->IsRegionOk() )
				continue;

			if( vb2->GetAbsoluteRect().Intersects( rc ) )
			{
				// 外部循环的按钮是否已然准备好循环
				if( vb->IsRegionCreationFailed() && !vb->IsRegionOk() )
					vb->CreateRegion();

				vb2->CreateRegion();
			}
		} // for j

	} // for i

}
