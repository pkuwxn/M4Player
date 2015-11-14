/***************************************************************
 * Name:      OOPIcon.cpp
 * Purpose:   OOPlayer 的图标
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2010-12-27
 * Copyright: (C) 2010 Wang Xiaoning
 **************************************************************/
#include "StdAfx.h"
#include "OOPIcon.h"
#include "wxUtil.h"

#include <wx/mstream.h>
//#include "Icon_bin.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT( OOPIcon );

//////////////////////////////////////////////////////////////////////////

void OOPIcon::Create(wxXmlNode* node)
{
	VdkStaticImage::Create( node );

	if( !GetImage().IsOk() )
	{
#if wxUSE_ICO_CUR
		wxString strIconFile = GetFilePath( XmlGetChildContent( node, L"icon" ) );
		if( !strIconFile.IsEmpty() && wxFileExists( strIconFile ) )
		{
			if( !wxImage::FindHandler( wxBITMAP_TYPE_ICO ) )
				wxImage::AddHandler( new wxICOHandler );

			m_icons = wxIconBundle( strIconFile, wxBITMAP_TYPE_ICO );
			if( m_icons.IsOk() )
			{
				wxIcon icon( m_icons.GetIcon( 16 ) );
				if( icon.IsOk() )
					SetImage( wxBitmap( icon ) );
			}

		}
#endif
	}
}

void OOPIcon::SetFrame(wxTopLevelWindow* frame)
{
    // 还是不要设置允许改变主窗口的图标吧，这些图标大部分分辨率都很小(16*16)
	if( m_icons.IsOk() )
	{
		//frame->SetIcons( m_icons );
	}
	else if( m_icon.IsOk() )
	{
		//frame->SetIcon( m_icon );
	}
	else
	{
		m_icons = frame->GetIcons();
		m_icon = frame->GetIcon();
	}
}

void OOPIcon::DoDraw(wxDC& dc)
{
	// TODO: 有些皮肤会将图标的显示位置移出窗口可视区域，但不知道为什么
	// 仍然会画到屏幕上
	if( !m_Window->Rect00().Contains( m_Rect ) )
		return;

	if( !GetImage().IsOk() )
	{
		if( m_icons.IsOk() )
		{
			wxIcon icon( m_icons.GetIconOfExactSize( 16 ) );
			if( icon.IsOk() )
			{
				dc.DrawIcon( icon, m_Rect.x, m_Rect.y );
				return;
			}
		}

		if( m_icon.IsOk() )
		{
			if( m_icon.GetWidth() == 16 )
			{
				dc.DrawIcon( m_icon, m_Rect.x, m_Rect.y );
			}
			else
			{
				wxBitmap bmp( m_icon );
				bmp = wxBitmap( bmp.ConvertToImage().Rescale( 16, 16  ) );

				dc.DrawBitmap( bmp, m_Rect.x, m_Rect.y );
			}
		}
	}
	else
	{
		VdkStaticImage::DoDraw( dc );
	}
}
