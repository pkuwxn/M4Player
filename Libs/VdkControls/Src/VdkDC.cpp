#include "StdAfx.h"
#include "VdkDC.h"
#include "VdkWindow.h"
#include "VdkUtil.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

#ifndef __VDK_NOT_USE_DOUBLE_BUFFERING__

unsigned long VdkDC::ms_timeStamp( 0 );
VdkWindow* VdkDC::ms_window = NULL;

VdkDC::VdkDC(VdkWindow* Window, const wxRect& rc, wxDC* pdc)
{
	Construct( Window, rc, pdc );
}

#define TRACE_VDKDC_LIFETIME 0

VdkDC::VdkDC(VdkWindow* Window)
{
	Construct( Window, Window->Rect00(), NULL );
}

void VdkDC::Construct(VdkWindow* Window, const wxRect& rc, wxDC* pdc)
{
	m_Window = Window;
	m_Rect = rc;
	m_pdc = pdc;
	m_timeStamp = ++ms_timeStamp;

	// GTK 要求必须有一个有效的位图
	static wxBitmap psudo( 1, 1 );

	//////////////////////////////////////////////////////////////////////////

#if TRACE_VDKDC_LIFETIME
	wxLogDebug( L"[%s:%u]VdkDC::Construct", m_Window->GetDebugCaption(), 
				long( this ) );
#endif

	if( m_Window->IsInitializing() )
	{
		SelectObject( psudo );
	}
	else
	{
		wxBitmap& buffered = m_Window->GetBufferedBitmap();
		if( buffered.GetRefData()->GetRefCount() > 1 )
        {
			wxLogDebug( L"[%s]双缓冲位图不应该与人共享。",
					    m_Window->GetDebugCaption() );

			SelectObject( psudo );
        }
		else
		{
			SelectObject( Window->GetBufferedBitmap() );
		}
	}

	ms_timeStamp = m_timeStamp;
	ms_window = m_Window;

	m_Window->SetCachedDC( *this );
	m_Window->PrepareCachedDC();
}

VdkDC::~VdkDC()
{
#if TRACE_VDKDC_LIFETIME
	wxLogDebug( L"[%s:%u]VdkDC::~VdkDC", m_Window->GetDebugCaption(), 
				long( this ) );
#endif

	if( ms_timeStamp > m_timeStamp && ms_window == m_Window )
	{
		// 使用 VdkButton 的回调函数打开一个模态对话框，比如使用
		// 标准对话框打开一个文件，wxWidgets 首先会被冻结在 LEFT_UP 
		// 的事件处理上下文里，然后进入一个离开窗口的事件处理
		// 上下文里。模态对话框被关闭后，重新进入被冻结的 LEFT_UP 
		// 的事件处理上下文里，这样会造成绘制的内容不正确。
		// 故直接重画整个窗口。
		// 但以上发生的事件必须是属于同一窗口。否则事件处理过程
		// 中另外一个窗口新生成了一个新的 VdkDC 会破坏这个状态。
		// 不知道为什么，下面这两句还是需要的。
		m_Window->SetAddinState( VWST_REDRAW_ALL );
		m_Window->GetHandle()->Refresh( false );

		wxLogDebug( L"[%s:%u]同一窗口分配了两个 VdkDC ？",
				    m_Window->GetDebugCaption().t_str(),
				    int( this ) );

		return;
	}

	if( m_pdc )
	{
		Flush( *m_pdc );
	}
	else
	{
		wxClientDC dc( m_Window->GetHandle() );
		Flush( dc );
	}

	m_Window = NULL;
}

void VdkDC::Flush(wxDC& dc)
{
	SetDeviceOrigin( 0, 0 );

	dc.Blit( m_Rect.x, m_Rect.y, m_Rect.width, m_Rect.height,
			 this, m_Rect.x, m_Rect.y );

	if( m_Window->GetCachedDC() == this )
		m_Window->ResetCachedDC( dc );
}
#else
VdkDC::VdkDC(VdkWindow* Window, const wxRect& rc, wxDC* pdc)
	: wxClientDC( Window->GetHandle() )
{

}

VdkDC::VdkDC(VdkWindow* Window)
	: wxClientDC( Window->GetHandle() )
{

}
#endif // __VDK_NOT_USE_DOUBLE_BUFFERING__

//////////////////////////////////////////////////////////////////////////

VdkValidDC::VdkValidDC(wxDC* pDC, VdkWindow* window, wxRect* prc)
		  : m_pDC( pDC ), m_bOwnDC( false )
{
	if( !m_pDC )
	{
		m_bOwnDC = true;
		m_pDC = new VdkDC( window, prc ? *prc : window->Rect00() );
	}
}

VdkValidDC::~VdkValidDC()
{
	if( m_bOwnDC )
		delete m_pDC;
}

//////////////////////////////////////////////////////////////////////////

VdkDcDeviceOriginSaver::VdkDcDeviceOriginSaver(wxDC& dc) : m_dc( dc )
{
	m_dc.GetDeviceOrigin( &m_x, &m_y );
}

VdkDcDeviceOriginSaver::~VdkDcDeviceOriginSaver()
{
	m_dc.SetDeviceOrigin( m_x, m_y );
}

void VdkDcDeviceOriginSaver::Destroy()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////////

VdkDcClippingRegionDestroyer::VdkDcClippingRegionDestroyer
	(wxDC& dc,const wxRect& region) : m_saver( dc ), m_dc( dc )
{
	m_dc.GetClippingBox( m_clpbox0 );
	m_dc.DestroyClippingRegion();

	m_dc.SetClippingRegion( region );
}

VdkDcClippingRegionDestroyer::~VdkDcClippingRegionDestroyer()
{
	m_dc.DestroyClippingRegion();
	if( !m_clpbox0.IsEmpty() )
		m_dc.SetClippingRegion( m_clpbox0 );
}

//////////////////////////////////////////////////////////////////////////

void VdkDcDeviceOriginAdder(wxDC& dc, int dX, int dY)
{
	int x, y;
	dc.GetDeviceOrigin( &x, &y );

	dc.SetDeviceOrigin( x + dX, y + dY );
}

//////////////////////////////////////////////////////////////////////////

void VdkDcPostListener::RemoveFromWindow(VdkWindow* win)
{
	wxASSERT( win );
	win->RemovePostDrawListener( this );
}
