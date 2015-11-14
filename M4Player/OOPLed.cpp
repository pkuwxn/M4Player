/***************************************************************
 * Name:      OOPLed.cpp
 * Purpose:   OOPLed 实现文件
 * Author:    Ning (vanxining@139.com)
 * Created:   2010
 * Copyright: Wang Xiao Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPLed.h"

#include "OOPStopWatch.h"
#include "VdkWindow.h"
#include "VdkDC.h"

#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT( OOPLed );
const static int gs_internal = 1000;

OOPLed::OOPLed()
	  : m_nElWidth( 0 ), m_nElHeight( 0 )
{
	memset( m_nPos, 0, sizeof( m_nPos ) );
}

void OOPLed::Create(wxXmlNode* node)
{
	GetXrcImage( node, m_bmAll );

	Create( m_Window,
		    GetXrcName( node ),
		    GetXrcRect( node ),
		    m_bmAll,
		    GetXrcAlign( node ) );
}

void OOPLed::Create(VdkWindow* Window,
				    const wxString& strName,
				    const wxRect& Rect,
					const wxBitmap& bmLed,
				    const align_type& align)
{
	m_Window = Window;
	m_WindowImpl = m_Window->GetHandle();
	m_strName = strName;
	m_Rect = Rect;
	m_align = align;

	SetAddinStyle( VCS_ERASE_BG );

	//////////////////////////////////////////////////////////////////////////

	m_bmAll = bmLed;
	m_nElHeight = m_bmAll.GetHeight();
	m_nElWidth = m_bmAll.GetWidth() / 12;

	int nWidth = m_nElWidth * 6,
		nHeight = m_bmAll.GetHeight();

	// 假如 m_Rext.width 小于实际需要的宽度，以右下角坐标为准
	if( m_Rect.width < nWidth || (m_align & ALIGN_RIGHT) )
	{
		m_Rect.x = m_Rect.x + m_Rect.width - nWidth;
		m_Rect.width = nWidth;
	}

	// 假如 m_Rext.height 小于实际需要的宽度，以右下角坐标为准
	if( m_Rect.height < nHeight )
	{
		m_Rect.y -= nHeight - m_Rect.height;
		m_Rect.height = nHeight;
	}
	else if( m_Rect.height > nHeight )
	{
		m_Rect.height = nHeight;
	}

	int xStart = m_Rect.x + m_nElWidth;

	// 不从 0 开始是因为前面可能有个“-”
	m_nPos[0] = xStart;
	m_nPos[1] = xStart + m_nElWidth;
	// 不 * 2 是因为中间有个“:”
	m_nPos[2] = xStart + m_nElWidth * 3;
	m_nPos[3] = xStart + m_nElWidth * 4;
}

void OOPLed::DoDraw(wxDC& dc)
{
	VdkDcDeviceOriginSaver saver( dc );
	Combine( m_stopWatch->Time(), dc );
}

void OOPLed::Notify()
{
	if( IsOneShot() )
		Start( gs_internal );

	VdkDC dc( m_Window, m_Window->Rect00(), NULL );
	Combine( m_stopWatch->Time(), dc );

	if( IsReadyForEvent() )
		FireEvent( &dc, NULL );
}

void OOPLed::Combine(long nTimeInMS, wxDC& dc)
{
	// 时间的数字
	unsigned char d[4] = { 0 };

	long curr = nTimeInMS / 1000;
	d[1] = curr / 60;
	d[3] = curr % 60;

	if( d[1] >= 10 )
	{
		d[0] = d[1] / 10;
		d[1] = d[1] % 10;
	}

	if( d[3] >= 10 )
	{
		d[2] = d[3] / 10;
		d[3] = d[3] % 10;
	}

	EraseBackground( dc, m_Rect );

	wxMemoryDC mdc( m_bmAll );
	wxRect rc( GetAbsoluteRect() );
	for( int i = 0; i < 4; i++ )
	{
		dc.Blit( m_nPos[i],
				 rc.y,
				 m_nElWidth,
				 m_nElHeight,
				 &mdc,
				 d[i] * m_nElWidth,
				 0,
				 wxCOPY,
				 true );
	}

	// 中间的“:”
	dc.Blit( m_nPos[1] + m_nElWidth,
		     rc.y,
			 m_nElWidth,
			 m_nElHeight,
			 &mdc,
			 m_nElWidth * 10,
			 0,
			 wxCOPY,
			 true );
}

void OOPLed::StartLed()
{
	wxASSERT( m_stopWatch );

	Stop();
	DoStart();
}

void OOPLed::ResumeLed()
{
	DoStart();
}

void OOPLed::DoStart()
{
	int restToUpdate = gs_internal - GetRestTimeForThisSecond();
	Start( restToUpdate, restToUpdate != gs_internal );
}

void OOPLed::PauseLed()
{
	Stop();
}

void OOPLed::StopLed(wxDC* pDC)
{
	Stop();

	if( pDC )
	{
		Draw( *pDC );
	}
}

int OOPLed::GetRestTimeForThisSecond()
{
	return gs_internal - m_stopWatch->Time() % gs_internal;
}
