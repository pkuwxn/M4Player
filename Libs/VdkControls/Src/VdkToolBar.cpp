/***************************************************************
 * Name:      VdkToolBar.cpp
 * Purpose:   Code for VdkToolBar implementation
 * Author:    vanxining (vanxining@139.com)
 * RCS-ID:    $Id: VdkToolBar.cpp 24 2011-05-03 08:58:46Z Administrator $
 * Created:   2011-04-07
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkToolBar.h"
#include "VdkButton.h"
#include "VdkDC.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT( VdkToolBar );

//////////////////////////////////////////////////////////////////////////

// 工具栏的一项
class VdkToolBarEntry : public VdkButton
{
public:

	// 构造函数
	VdkToolBarEntry(VdkToolBar* toolbar, const VdkBitmapArrayId& bid)
		: m_toolbar( toolbar ),
		  m_bid( bid )
	{
		SetAddinStyle( VCS_ERASE_BG );
	}

private:

	// 擦除背景
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc)
	{
		VdkToolBarStaticStyle* sstyle = m_toolbar->GetStaticStyle();

		switch( GetLastState() )
		{
		case HOVERING:
		case PUSHED:

			dc.SetPen( sstyle->borderPen() );
			dc.SetBrush( GetLastState() == PUSHED ?
						 sstyle->pushedBrush() :
						 sstyle->hilightBrush() );
			dc.DrawRectangle( m_Rect );

			dc.SetBrush( wxNullBrush );

			break;

		default:

			break;
		}
	}

	// 绘制控件
	virtual void DoDraw(wxDC& dc)
	{
		VdkToolBarStaticStyle* sstyle = m_toolbar->GetStaticStyle();

		if( m_bid.IsOk() )
		{
			m_bid.BlitTo( dc, m_Rect.x + sstyle->bitmapAddin(),
							  m_Rect.y + sstyle->bitmapAddin() );
		}
	}

	VdkToolBar* m_toolbar;
	VdkBitmapArrayId m_bid;
};

// 工具栏分隔符
class VdkToolBarSeperator : public VdkControl
{
public:

	// 构造函数
	VdkToolBarSeperator(VdkToolBar* toolbar, const wxRect& rc)
	{
		SetVdkWindow( toolbar->GetVdkWindow() );
		SetParent( toolbar );
		m_strName = L"VdkToolBarSeperator";
		m_Rect = rc;

		SetAddinStyle( VCS_IGNORE_ALL_EVENTS | VCS_ERASE_BG );
		m_Window->AddCtrl( this );
	}

private:

	// 绘制控件
	virtual void DoDraw(wxDC& dc)
	{
		dc.SetPen( *wxGREY_PEN );

		int x = m_Rect.x + m_Rect.width / 2;
		dc.DrawLine( x, m_Rect.y, x, m_Rect.GetBottom() );
	}
};

const static int gs_sideHandle = 8; // 可拖动的手柄区域宽度
class VdkToolBarLeftSideHandle : public VdkControl
{
public:

	// 默认构造函数
	VdkToolBarLeftSideHandle() : m_toolbar( NULL )
	{

	}

	// 构造函数
	VdkToolBarLeftSideHandle(VdkToolBar* toolbar)
		: m_toolbar( toolbar )
	{
		VdkToolBarStaticStyle* sstyle = m_toolbar->GetStaticStyle();
		int size = 16 + sstyle->bitmapAddin() * 2;

		SetVdkWindow( m_toolbar->GetVdkWindow() );
		SetParent( m_toolbar );
		m_strName = L"VdkToolBarLeftSideHandle";
		m_Rect.x = 0;
		m_Rect.y = 0;
		m_Rect.width = gs_sideHandle;
		m_Rect.height = size;

		m_Window->AddCtrl( this );
	}

private:

	// 绘制控件
	virtual void DoDraw(wxDC& dc)
	{
		VdkToolBarStaticStyle* sstyle = m_toolbar->GetStaticStyle();
		const static int s_points = 4;

		// 擦除背景
		VdkDcClippingRegionDestroyer destroyer( dc, m_Rect );
		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.SetBrush( sstyle->bgBrush() );
		dc.DrawRoundedRectangle( m_Rect.x, m_Rect.y,
			m_Rect.width + 4, m_Rect.height, 4 );

		int x = m_Rect.x + m_Rect.width / 2;
		int y = m_Rect.y + sstyle->bitmapAddin();

		dc.SetPen( *wxGREY_PEN );
		for( int i = 0; i < s_points ; i++, y += 4 )
			dc.DrawRectangle( x, y, 2, 2 );

		++x;
		y = m_Rect.y + sstyle->bitmapAddin() + 1;
		dc.SetPen( *wxWHITE_PEN );
		for( int i = 0; i < s_points ; i++, y += 4  )
			dc.DrawRectangle( x, y, 2, 2 );
	}

	// 处理鼠标事件
	virtual void DoHandleMouseEvent(VdkMouseEvent& e)
	{
		switch( e.evtCode )
		{
		case HOVERING:

			m_Window->AssignCursor( wxCursor( wxCURSOR_SIZING ) );

			break;

		case NORMAL:

			m_Window->AssignCursor( wxNullCursor );

			break;

		default:
			break;
		}

		return VdkControl::DoHandleMouseEvent( e );
	}

private:

	VdkToolBar* m_toolbar;
};

// 工具栏最右边的下拉手柄
class VdkToolBarRightSideHandle : public VdkControl
{
public:

	// 构造函数
	VdkToolBarRightSideHandle(VdkToolBar* toolbar)
		: m_toolbar( toolbar )
	{
		VdkToolBarStaticStyle* sstyle = m_toolbar->GetStaticStyle();
		int size = 16 + sstyle->bitmapAddin() * 2;

		SetVdkWindow( m_toolbar->GetVdkWindow() );
		SetParent( m_toolbar );
		m_strName = L"VdkToolBarRightSideHandle";
		m_Rect.x = gs_sideHandle;
		m_Rect.y = 0;
		m_Rect.width = gs_sideHandle;
		m_Rect.height = size;

		m_Window->AddCtrl( this );
	}

private:

	// 绘制控件
	virtual void DoDraw(wxDC& dc)
	{
		// 擦除背景
		VdkDcClippingRegionDestroyer destroyer( dc, m_Rect );
		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.SetBrush( m_toolbar->GetStaticStyle()->bgBrush() );
		dc.DrawRoundedRectangle( m_Rect.x - 4, m_Rect.y,
			m_Rect.width + 4, m_Rect.height, 4 );

		/*int x = m_Rect.x + 2;
		int y = m_Rect.GetBottom() * 0.7;
		int points = 10;*/
	}

private:

	VdkToolBar* m_toolbar;
};

//////////////////////////////////////////////////////////////////////////

VdkToolBar::VdkToolBar()
	: m_leftHandle(	NULL ),
	  m_rightHandle( NULL ),
	  m_sstyle( new VdkToolBarStaticStyle )
{

}

void VdkToolBar::Create(wxXmlNode* node)
{
	DoXrcCreate( node );
	//////////////////////////////////////////////////////////////////////////

	int h = 16 + m_sstyle->bitmapAddin() * 2;
	m_Rect.height = h;

	wxBitmap stipple( 1, h );
	wxMemoryDC mdc( stipple );
	mdc.GradientFillLinear( wxRect( 0, 0, 1, h ),
							wxColour( 221,236,254 ),
							wxColour( 114,162,238 ),
							wxSOUTH );
	mdc.SelectObject( wxNullBitmap );

	wxBrush brush( stipple );
	m_sstyle->bgBrush( brush );

	m_leftHandle = new class VdkToolBarLeftSideHandle( this );
	m_Rect.width = gs_sideHandle;

	m_rightHandle = new class VdkToolBarRightSideHandle( this );
}

void VdkToolBar::Clone(VdkControl* o, VdkControl* parent)
{

}

VdkToolBar::~VdkToolBar()
{
	delete m_sstyle;
	m_sstyle = NULL;
}

VdkToolBar& VdkToolBar::Append(const VdkToolBarEntryAdder& adder)
{
	int size = 0;

	if( adder.Menu == NULL )
	{
		size = 16 + m_sstyle->bitmapAddin() * 2;

		VdkToolBarEntry* entry = new VdkToolBarEntry( this, adder.BId );
		entry->Create( VdkButtonInitializer().
					   caption( adder.Caption ).
					   parent( this ).
					   window( m_Window ).
					   addToWindow( true ).
					   rect( wxRect( m_Rect.width, 0, size, size ) )
					 );
	}

	m_Rect.width += size;

	wxPoint pos = m_rightHandle->GetPosition();
	pos.x += size;
	m_rightHandle->Move( pos );

	return *this;
}

VdkToolBar& VdkToolBar::AppendSeperator()
{
	/*VdkToolBarSeperator* sep = */new VdkToolBarSeperator
		( this, wxRect( m_Rect.width, m_sstyle->bitmapAddin(),
						3, 16 ) );

	m_Rect.width += 3;
	return *this;
}

void VdkToolBar::DoEraseBackground(wxDC& dc, const wxRect& rc)
{
	//dc.SetBrush( m_sstyle->bgBrush() );
	//dc.DrawRectangle( rc.x, m_Rect.y, rc.width, m_Rect.height );

	wxRect rect( rc.x, m_Rect.y, rc.width, m_Rect.height );
	dc.GradientFillLinear( rect,
						   wxColour( 221,236,254 ),
						   wxColour( 114,162,238 ),
						   wxSOUTH );
}

//////////////////////////////////////////////////////////////////////////

VdkToolBarStaticStyle::VdkToolBarStaticStyle()
	: m_border( *wxBLUE_PEN ),
	  m_hilight( wxColour( 255,238,194 ) ),
	  m_pushed( wxColour( 237,144,101 ) ),
	  m_bitmapAddin( 3 )
{

}
