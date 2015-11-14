/***************************************************************
 * Name:      OSD.cpp
 * Purpose:   桌面歌词平台相关部分的代码
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-06
 **************************************************************/
#include "StdAfx.h"
#include "../OOPDesktopLyric.h"

// GDI+ cries for them
template<class T> T min(T a, T b) { return a < b ? a : b; }
template<class T> T max(T a, T b) { return a < b ? b : a; }

#include <comdef.h> // for GDI+ Image COM declarations
#include <gdiplus.h>
#pragma comment( lib, "gdiplus.lib" )

#include <wx/graphics.h>
#include <wx/private/graphics.h> // for wxGraphicsObjectRefData

using namespace Gdiplus;

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

enum {
	WIDTH = 755,
	HEIGHT = 80,
};

//////////////////////////////////////////////////////////////////////////

void OOPDesktopLyric::Initialize()
{
	DWORD dwExStyle = GetWindowLong( m_hWnd, GWL_EXSTYLE );
	if( (dwExStyle & WS_EX_LAYERED) != WS_EX_LAYERED )
		SetWindowLong( m_hWnd, GWL_EXSTYLE, dwExStyle ^ WS_EX_LAYERED );
}

void OOPDesktopLyric::Finalize()
{

}

bool OOPDesktopLyric::UpdateAtOnce()
{
	wxBitmap canvas( WIDTH, HEIGHT );
	wxMemoryDC mdc( canvas );

	// Create graphics context from the memory DC
	wxGraphicsContext* gc = wxGraphicsContext::Create( mdc );

	if( !gc )
	{
		wxLogError( L"wxGraphicsContext::Create() failed!" );
		return false;
	}

	if( m_showBackgound )
		ShowBackgound( gc );

	wxGraphicsPath path = InitLineTextPath( gc );

	//===========================================

	wxGraphicsBrush normalBrush = 
		gc->CreateLinearGradientBrush( 0, (HEIGHT - m_style.pxFontSize) / 2, 
									   0, (HEIGHT + m_style.pxFontSize) / 2,
									   wxColour(255,255,255,255),
									   wxColour(30,120,195,255) );
	gc->SetBrush( normalBrush );
	gc->FillPath( path );

	if( IsCurrLineValid() )
	{
		// 已播放的部分
		wxGraphicsBrush playedBrush = 
			gc->CreateLinearGradientBrush( 0, (HEIGHT - m_style.pxFontSize) / 2, 
										   0, (HEIGHT + m_style.pxFontSize) / 2,
										   *wxYELLOW,
										   *wxYELLOW );

		gc->Clip( 0, 0, m_textPathBounds.GetRight() * GetLineProgress(), HEIGHT );
		gc->SetBrush( playedBrush );
		gc->FillPath( path );
	}

	delete gc;
	gc = NULL;

	//===========================================

	return Present( mdc );
}

struct WXGDIPlusPenData : public wxGraphicsObjectRefData
{
	Pen* m_pen;
	Image* m_penImage;
	Brush* m_penBrush;

	wxDouble m_width;
};

wxGraphicsPath OOPDesktopLyric::InitLineTextPath(wxGraphicsContext* gc)
{
	wxASSERT( gc );

	// 文本轮廓
	wxGraphicsPen pen = gc->CreatePen( wxPen( wxColour(0,0,0,255), 3 ) );

	const wxString textToDraw( IsCurrLineValid() ? 
								(*m_currLine)->GetLyric() : 
								GetInteractiveOutput() );

	wxGraphicsPath path = gc->CreatePath();

	//========================================
	// 添加文本

	FontFamily fontFamily( m_style.fontFace );
	StringFormat stringFormat;

	wxCoord winHeight;
	GetSize( NULL, &winHeight );
	wxASSERT( size_t( winHeight ) >= m_style.pxFontSize );

	GraphicsPath* nativePath = (GraphicsPath *) path.GetNativePath();
	nativePath->AddString( textToDraw,
						   -1,
						   &fontFamily,
						   m_style.bold ? FontStyleBold : FontStyleRegular,
						   m_style.pxFontSize,
						   Point( 0, (winHeight - m_style.pxFontSize) / 2 ),
						   &stringFormat );

	WXGDIPlusPenData* penImpl = (WXGDIPlusPenData *) pen.GetGraphicsData();

	RectF bounds;
	nativePath->GetBounds( &bounds, NULL, penImpl->m_pen );

	m_textPathBounds.m_x = bounds.X;
	m_textPathBounds.m_y = bounds.Y;
	m_textPathBounds.m_width  = bounds.Width;
	m_textPathBounds.m_height = bounds.Height;

	path.UnGetNativePath( nativePath );

	//========================================
	// 添加到路径中

	gc->SetPen( pen );
	gc->StrokePath( path );

	return path;
}

bool OOPDesktopLyric::Present(wxDC& drawings)
{
	HDC hdcScreen = ::GetDC( NULL );

	RECT rct;
	GetWindowRect( m_hWnd, &rct );

	POINT ptWinPos = { rct.left, rct.top };
	SIZE sizeWindow = { rct.right - rct.left, rct.bottom - rct.top };
	POINT ptSrc = { 0, 0 };

	BLENDFUNCTION blendFunc;
	// Initialize blend function
	blendFunc.BlendOp = 0; // the only BlendOp defined in Windows2000
	blendFunc.BlendFlags = 0; // nothing else is special...
	blendFunc.AlphaFormat = 1; // ...
	blendFunc.SourceConstantAlpha = m_style.alpha;

	BOOL bRet;
	bRet = UpdateLayeredWindow( m_hWnd, hdcScreen, 
								&ptWinPos, &sizeWindow, 
								drawings.GetHDC(), 
								&ptSrc, 
								0, &blendFunc, 2 );

	::ReleaseDC( NULL, hdcScreen );
	hdcScreen = NULL;

	return (bRet == TRUE);
}

void OOPDesktopLyric::OnLeftDown(wxMouseEvent&)
{
	// 移动窗口
	::SendMessage( m_hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0 );
}

