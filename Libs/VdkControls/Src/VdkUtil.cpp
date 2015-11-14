/***************************************************************
 * Name:      VdkUtil.cpp
 * Purpose:   Code for VdkUtil implementation
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-02-15
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkUtil.h"

#include "wxUtil.h"
#include "VdkControl.h"

#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/xml/xml.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

extern const int gs_invalidPadding;

void VdkUtil::ClrBkGnd(wxDC& dc, const wxBrush& bgBrush, const wxRect& rc)
{
	dc.SetBrush( bgBrush);
	dc.SetPen( *wxTRANSPARENT_PEN );
	dc.DrawRectangle( rc );
}

void VdkUtil::SaveDC(wxDC& dc, const wxRect& rc, const wxChar* szSubfix)
{
	wxBitmap bmTmp( rc.width, rc.height );
	wxMemoryDC memDC( bmTmp );
	memDC.Blit( 0, 0, rc.height, rc.width, &dc, rc.x, rc.y );

	RandSave( bmTmp, szSubfix );
}

bool VdkUtil::LoadMaskBitmap(wxBitmap& bitmap, const wxString& path)
{
	bitmap = wxNullBitmap;

	if( path.IsEmpty() || !wxFileExists( path ) )
	{
		return false;
	}

	bitmap.LoadFile( path, wxBITMAP_TYPE_BMP );
	if( bitmap.IsOk() )
	{
		CreateMask( bitmap, NULL );
	}
	else
	{
		wxLogDebug( L"[VdkUtil::ImRead]无法载入%s。\n", path );
		return false;
	}

	// TODO:
#ifdef __WXMSW__
	bitmap.SetDepth( wxGetDisplayDepth() );
#endif

	return true;
}

void VdkUtil::CreateMask(wxBitmap& bitmap, const wxColour* pMaskColor)
{
	wxColour maskColor( VdkControl::GetMaskColour() );
	if( pMaskColor )
		maskColor = *pMaskColor;

	wxMask* mask = new wxMask;
	if( mask->Create( bitmap, maskColor ) )
		bitmap.SetMask( mask );
}

wxBitmap* VdkUtil::ExplodeBitmap(const wxBitmap& bm, int nExpl)
{
	if( !bm.IsOk() )
		return NULL;

	wxBitmap* bmArray = new wxBitmap[nExpl];
	wxRect rect( 0, 0, bm.GetWidth() / nExpl, bm.GetHeight() );

	for( int i=0; i < nExpl; ++i )
	{
		bmArray[i] = bm.GetSubBitmap( rect );
		rect.x += rect.width;
	}

	return bmArray;
}

wxBitmap VdkUtil::GetSubBitmapWithoutMask
	(const wxBitmap& bmp, const wxRect& subRegion)
{
	wxBitmap bmpDest( subRegion.GetWidth(), subRegion.GetHeight() );

	wxMemoryDC mdc( bmpDest ), mdcSrc;
	mdcSrc.SelectObjectAsSource( bmp );

	mdc.Blit( 0, 0, subRegion.width, subRegion.height, &mdcSrc,
			  subRegion.x, subRegion.y, wxCOPY );

	mdc.SelectObject( wxNullBitmap );
	return bmpDest;
}

wxBitmap VdkUtil::TileBitmap(wxBitmap& bm,
							 int w,
							 int h,
							 const wxRect& rcTile,
							 bool mask)
{
    if( !bm.IsOk() )
        return wxNullBitmap;

	int bw = bm.GetWidth(),
		bh = bm.GetHeight();

	if( w < bw ) w = bw;
	if( h < bh ) h = bh;

	wxBitmap bmResized( w, h );
	wxMemoryDC dcSrc( bm ), dcDest( bmResized );

	dcDest.SetBrush( VdkControl::GetMaskColour() );
	dcDest.Clear();

	//////////////////////////////////////////////////////////////////////////

	int left = rcTile.x,
		top = rcTile.y,
		bottom = bh - ( rcTile.y + rcTile.height ),
		right = bw - ( rcTile.x + rcTile.width );

	int bottomCoord = rcTile.y + rcTile.height,
		rightCoord = rcTile.x + rcTile.width;

	// 复制到四个角
	dcDest.Blit( 0, 0, left, top, &dcSrc, 0, 0, wxCOPY, mask ); // 左上角
	dcDest.Blit( 0, h - bottom, left, bottom, &dcSrc, 
				 0, bottomCoord, wxCOPY, mask ); // 左下角
	dcDest.Blit( w - right, 0, right, top, &dcSrc, 
				 rightCoord, 0, wxCOPY, mask ); // 右上角
	dcDest.Blit( w - right, h - bottom, right, bottom, &dcSrc,
				 rightCoord, bottomCoord, wxCOPY, mask ); // 右下角

	int newMidWidth = w - left - right,
		newMidHeight = h - top - bottom;

	// 顶部平铺
    TileDraw_X( wxRect( left, 0, rcTile.width, top ), dcSrc,
                wxRect( left, 0, newMidWidth, top ), dcDest, mask );
	// 底部平铺
    TileDraw_X( wxRect( left, bottomCoord, rcTile.width, bottom ), dcSrc,
				wxRect( left, h - bottom, newMidWidth, bottom ), dcDest, mask );
	// 左边平铺
    TileDraw_Y( wxRect( 0, top, left, rcTile.height ), dcSrc,
                wxRect( 0, top, left, newMidHeight ), dcDest, mask );
	// 右边平铺
    TileDraw_Y( wxRect( rightCoord, top, right, rcTile.height ), dcSrc,
                wxRect( w - right, top, right, newMidHeight ), dcDest, mask );

	// 中心平铺
	int yTile( static_cast< float > ( newMidHeight ) / rcTile.height );
	int Rest( newMidHeight - rcTile.height * yTile );

	// 执行 yTile 次水平平铺
	for( int i = 0; i < yTile ; i++ )
	{
		TileDraw_X
			( wxRect( left, top, rcTile.width, rcTile.height ), dcSrc, 
			  wxRect( left, top + rcTile.height * i, newMidWidth, rcTile.height ), 
			  dcDest, mask );
	}

	// y 轴方向上剩余一点点
	if( Rest )
	{
		TileDraw_X
			( wxRect( left, bh - bottom - Rest, rcTile.width, Rest ), dcSrc, 
			  wxRect( left, h - bottom - Rest, newMidWidth, Rest ), 
			  dcDest, mask );
	}

	//////////////////////////////////////////////////////////////////////////

	dcDest.SelectObject( wxNullBitmap );/*

	RandSave( bmResized );*/

	return bmResized;
}

void VdkUtil::TileDrawBitmap(wxDC& dc,
							 wxBitmap& bm,
							 int x,
							 int y,
							 int w,
							 int h,
							 const wxRect& rcTile)
{
    if( !bm.IsOk() )
        return;

	if( h ) // 竖直平铺
	{
		int nBase = rcTile.y;
		int nTileHeight = rcTile.height;
		int bw = bm.GetWidth();
		int nBottom = bm.GetHeight() - nBase - nTileHeight;
		wxMemoryDC mdc( bm );

		int newMidHeight = h - nBase - nBottom;
		int nTileCount = float( newMidHeight ) / nTileHeight;
		int nRest = newMidHeight - nTileHeight * nTileCount;

		// 顶部
		dc.Blit( x, y, bw, nBase, &mdc, 0, 0 );
		// 底部
		dc.Blit( x, y + h - nBottom,
				 bw, nBottom,
				 &mdc,
				 0, rcTile.y + rcTile.height );
		// 中间
		for( int i = 0 ; i < nTileCount; i++ )
		{
			dc.Blit( x, y + nBase + i * nTileHeight,
					 bw, nTileHeight,
					 &mdc, 0, nBase );
		}

		// 剩余的、小于 nTileCount 的部分
		if( nRest )
		{
			dc.Blit( x, y + h - nBottom - nRest,
					 bw, nRest,
					 &mdc, 0, nBase );
		}
	}

	if( w )
	{
		int nBase = rcTile.x;
		int nTileWidth = rcTile.width;
		int bh = bm.GetHeight();
		int nRight = bm.GetWidth() - nBase - nTileWidth;
		wxMemoryDC mdc( bm );

		int newMidWidth = w - nBase - nRight;
		int nTileCount = float( newMidWidth )/ nTileWidth;
		int nRest = newMidWidth - nTileWidth * nTileCount;

		// 左边
		dc.Blit( x, y, nBase, bh, &mdc, 0, 0 );
		// 右边
		dc.Blit( x + w - nRight, y,
				 nRight, bh,
				 &mdc,
				 rcTile.x + rcTile.width, 0 );
		// 中间
		for( int i = 0; i < nTileCount; i++ )
		{
			dc.Blit( x + nBase + i * nTileWidth, y,
					 nTileWidth, bh,
					 &mdc, nBase, 0 );
		}

		// 剩余的、小于 nTileCount 的部分
		if( nRest )
		{
			dc.Blit( x + w - nRight - nRest, y,
					 nRest, bh,
					 &mdc, nBase, 0 );
		}
	}
}

void VdkUtil::TileDraw_X(const wxRect& rcSrc, wxDC& dcSrc, 
						 const wxRect& rcDest, wxDC& dcDest, 
						 bool mask)
{
	int bh = rcSrc.height;
	int nTileWidth = rcSrc.width;
	int w = rcDest.width;

	if( w == nTileWidth )
	{
		dcDest.Blit( rcDest.x, rcDest.y, rcDest.width, rcDest.height,
					 &dcSrc, rcSrc.x, rcSrc.y, wxCOPY, mask );

		return;
	}

	int x = rcDest.x;
	int y = rcDest.y;
	int newMidWidth = rcDest.width;
	int nTileCount = float( newMidWidth ) / nTileWidth;
	int nRest = newMidWidth - rcSrc.width * nTileCount;

	// 中间
	for( int i = 0; i < nTileCount; i++ )
	{
		dcDest.Blit( x + i * nTileWidth, y,
					 nTileWidth, bh,
					 &dcSrc, 
					 rcSrc.x, rcSrc.y,
					 wxCOPY, mask );
	}

	// 剩余的、小于 nTileWidth 的部分
	if( nRest )
	{
		dcDest.Blit( x + w - nRest, y,
					 nRest, bh,
					 &dcSrc, 
					 rcSrc.x, rcSrc.y,
					 wxCOPY, mask );
	}
}

void VdkUtil::TileDraw_Y(const wxRect& rcSrc, wxDC& dcSrc, 
						 const wxRect& rcDest, wxDC& dcDest, 
						 bool mask)
{
	int bw = rcSrc.width;
	int nTileHeight = rcSrc.height;
	int h = rcDest.height;

	if( h == nTileHeight )
	{
		dcDest.Blit( rcDest.x, rcDest.y, rcDest.width, rcDest.height,
					 &dcSrc, rcSrc.x, rcSrc.y, wxCOPY, mask );

		return;
	}

	int x = rcDest.x;
	int y = rcDest.y;
	int newMidHeight = rcDest.height;
	int nTileCount = float( newMidHeight ) / nTileHeight;
	int nRest = newMidHeight - nTileHeight * nTileCount;

	// 中间
	for( int i = 0 ; i < nTileCount; i++ )
	{
		dcDest.Blit( x, y + i * nTileHeight,
					 bw, nTileHeight,
					 &dcSrc,
					 rcSrc.x,
					 rcSrc.y,
					 wxCOPY, mask );
	}

	// 剩余的、小于 nTileHeight 的部分
	if( nRest )
	{
		dcDest.Blit( x, y + h - nRest,
					 bw,
					 nRest,
					 &dcSrc,
					 rcSrc.x,
					 rcSrc.y,
					 wxCOPY, mask );
	}
}

wxBitmap VdkUtil::InflateBitmapByCenter(wxBitmap& bm,
										const wxRect& rc,
										const wxSize& newsize)
{
    if( !bm.IsOk() )
        return wxNullBitmap;

	wxBitmap bmDest( newsize.x, newsize.y );
	wxMemoryDC mdc( bmDest );

	InflateDrawByCenter( bm, mdc, newsize, rc );

	mdc.SelectObject( wxNullBitmap );
	return bmDest;
}

void VdkUtil::InflateDrawByCenter(wxBitmap& bm,
								  wxDC& dc,
								  const wxSize& newsize,
								  const wxRect& center)
{
	int left, right, top, bottom, width, height;
	width = bm.GetWidth();
	height = bm.GetHeight();

	left = center.x; // 左边不变的长度
	right = width - (center.x + center.width); // 右边

	top = center.y; // 上边
	bottom = height - (center.y + center.height); // 下边

	//---------------------------------------------------

	// 目标位图宽度和高度
	int xDest( newsize.x - left - right ),
		yDest( newsize.y - top - bottom );

	wxMemoryDC mdc( bm );

	//---------------------------------------------------

	// 坐标
	int bottom2( newsize.y - bottom ), right2( newsize.x - right );

	// 复制到四个角
	dc.Blit( 0, 0, left, top, &mdc, 0, 0 );
	dc.Blit( 0, bottom2, left, bottom, &mdc, 0, height - bottom );
	dc.Blit( right2, 0, right, top, &mdc, width - right, 0 );
	dc.Blit( right2, bottom2, right, bottom, &mdc,
			 width - right, height - bottom );

	wxBitmap bmTemp;

	StretchBlit( dc,
				 wxRect( 0, top, left, yDest ), // 左边
				 mdc,
				 wxRect( 0, top, left, center.height ),
				 bmTemp );

	StretchBlit( dc,
				 wxRect( left, 0, xDest, top ), // 上边
				 mdc,
				 wxRect( left, 0, center.width, top ),
				 bmTemp );

	StretchBlit( dc,
				 wxRect( right2, top, right, yDest ), // 右边
				 mdc,
				 wxRect( width - right, top, right, center.height ),
				 bmTemp );

	StretchBlit( dc,
				 wxRect( left, bottom2, xDest, bottom ), // 下边
				 mdc,
				 wxRect( left, height - bottom, center.width, bottom ),
				 bmTemp );

	StretchBlit( dc,
				 wxRect( left, top, xDest, yDest ), // 中间
				 mdc,
				 wxRect( left, top, center.width, center.height ),
				 bmTemp );
}

void VdkUtil::StretchBlit(wxDC& dcdest, 
						  const wxRect& dst,
						  wxDC& dcsrc, 
						  const wxRect& src, 
						  wxBitmap& bmTmp )
{
	if( !src.width || !src.height || !dst.width || !dst.height )
		return;

#	if defined( __WXMSW__ ) || defined( __WXMAC__ )

	dcdest.StretchBlit( dst.x, dst.y, dst.width, dst.height, &dcsrc, 
						src.x, src.y, src.width, src.height );

	return;

#	else

	if( !bmTmp.IsOk() ||
        bmTmp.GetWidth()  < dst.width ||
		bmTmp.GetHeight() < dst.height )
	{
	    bmTmp = wxBitmap( src.width, src.height );
	}
    else
    {
        bmTmp.SetWidth( src.width );
        bmTmp.SetHeight( src.height );
    }

	wxMemoryDC mdc( bmTmp );
	mdc.Blit( 0, 0, src.width, src.height, &dcsrc, src.x, src.y );
	mdc.SelectObject( wxNullBitmap );

	bmTmp = wxBitmap( bmTmp.ConvertToImage().Rescale( dst.width, dst.height ) );
	mdc.SelectObject( bmTmp );
	dcdest.Blit( dst.x, dst.y, dst.width, dst.height, &mdc, 0, 0 );

#	endif // defined( __WXMSW__ ) || defined( __WXMAC__ )
}

wxString VdkUtil::GetFilePath(const wxString& strFileName)
{
	return VdkControl::GetRootPath() + strFileName;
}

wxRect VdkUtil::ParseRect(const wxString& src, RectType nType)
{
	wxRect rc;
	wxString strRect( src );

	if( !strRect.IsEmpty() )
	{
		int rtArray[4];
		int nPos = strRect.Find( L',' );
		int i = 0;

		while( (nPos != wxNOT_FOUND) && (i < 4) )
		{
			strRect.Mid( 0, nPos ).ToLong( (long *) &rtArray[i] );
			strRect = strRect.Mid( nPos + 1 );
			nPos = strRect.Find( L',' );
			i++;
		}

		if( i == 3 )
		{
			strRect.ToLong( (long *) &rtArray[i] );
		}
		else if( i < 3 )
		{
			wxLogDebug( L"Error parsing wxRect." );
			return wxRect( 0, 0, 1, 1 );
		}

		if( nType == PRT_POSITION )
		{
			rc.x = rtArray[0];
			rc.y = rtArray[1];
			rc.width = rtArray[2] - rc.x;
			rc.height = rtArray[3] - rc.y;
		}
		else
		{
			rc.x = rtArray[0];
			rc.y = rtArray[1];
			rc.width = rtArray[2];
			rc.height = rtArray[3];
		}
	}

	return rc;
}

wxRect VdkUtil::ParseRect(wxXmlNode* xmlNode)
{
	wxString strRect;
	xmlNode->GetAttribute( L"position", &strRect );
	return ParseRect( strRect, PRT_RECTANGLE );
}

wxRect VdkUtil::GetXrcRect(wxXmlNode* xmlNode)
{
	wxXmlNode* chd( FindChildNode( xmlNode, L"rect" ) );
	if( chd )
	{
		return ParseRect( chd->GetNodeContent(), PRT_RECTANGLE );
	}

	return wxRect( 0, 0, 0, 0 );
}

align_type VdkUtil::GetXrcAlign(wxXmlNode* xmlNode, wxString strNodeName)
{
	wxString strAlign;
	wxXmlNode* alignNode( NULL );

	if( strNodeName.IsEmpty() )
		strNodeName = L"align";

	align_type alignment = 0;
	alignNode = FindChildNode( xmlNode, strNodeName );
	if( alignNode )
	{
		strAlign = alignNode->GetNodeContent();

		if( !strAlign.IsEmpty() )
		{
			if( strAlign.Find( L"right" ) != wxNOT_FOUND )
				alignment |= ALIGN_RIGHT;

			if( strAlign.Find( L"top" ) != wxNOT_FOUND )
				alignment |= ALIGN_TOP;

			if( strAlign.Find( L"bottom" ) != wxNOT_FOUND )
				alignment |= ALIGN_BOTTOM;

			if( strAlign.Find( L"center_x_y" ) != wxNOT_FOUND )
				alignment |= ALIGN_CENTER_X_Y;

			if( strAlign.Find( L"center_x" ) != wxNOT_FOUND )
				alignment |= ALIGN_CENTER_X;

			if( strAlign.Find( L"center_y" ) != wxNOT_FOUND )
				alignment |= ALIGN_CENTER_Y;

			if( strAlign.Find( L"sync_x_y" ) != wxNOT_FOUND )
				alignment |= ALIGN_SYNC_X_Y;

			if( strAlign.Find( L"sync_x" ) != wxNOT_FOUND )
				alignment |= ALIGN_SYNC_X;

			if( strAlign.Find( L"sync_y" ) != wxNOT_FOUND )
				alignment |= ALIGN_SYNC_Y;
		}
	}

	return alignment;
}

wxString VdkUtil::GetXrcName(wxXmlNode* xmlNode)
{
	return xmlNode->GetAttribute( L"name", wxEmptyString );
}

wxString VdkUtil::GetXrcTextBlock(wxXmlNode* xmlNode)
{
	wxString strRet;
	if( xmlNode->GetChildren()->GetType() == wxXML_TEXT_NODE )
	{
		strRet = xmlNode->GetChildren()->GetContent();
		strRet.Trim( false ).Trim();
	}

	return strRet;
}

bool VdkUtil::GetXrcImage(wxXmlNode* xmlNode, wxBitmap& bm)
{
	wxString path( XmlGetChildContent( xmlNode, L"image" ) );
	if( path.IsEmpty() )
		return false;

	path = GetFilePath( path );

	//----------------------------------------------------

	bool ret = ImRead( bm, path );

	// TODO:
	if( ret && wxFileName( path ).GetExt().CmpNoCase( L"bmp" ) == 0 )
	{
		wxASSERT( bm.IsOk() );

		wxColour maskColor( VdkControl::GetMaskColour() );
		wxString s( XmlGetChildContent( xmlNode, L"transparent-color" ) );
		if( !s.IsEmpty() )
			maskColor.Set( s );

		CreateMask( bm, &maskColor );
	}

	return ret;
}

bool VdkUtil::ImRead(wxBitmap& bm, const wxString& path)
{
	if( path.IsEmpty() || !wxFileExists( path ) )
		return false;

	wxFileName filename( path );
	wxString ext( filename.GetExt() );
	// 目标图像格式
	wxBitmapType imageType;

	if( ext.CmpNoCase( L"bmp" ) == 0 )
	{
		imageType = wxBITMAP_TYPE_BMP;
	}
#if wxUSE_LIBJPEG
	else if( (ext.CmpNoCase( L"jpg" ) == 0) || (ext.CmpNoCase( L"jpeg" ) == 0) )
	{
		imageType = wxBITMAP_TYPE_JPEG;

		if( !wxImage::FindHandler( imageType ) )
			wxImage::AddHandler( new wxJPEGHandler );
	}
#endif
#if wxUSE_GIF
	else if( ext.CmpNoCase( L"gif" ) == 0 )
	{
		imageType = wxBITMAP_TYPE_GIF;

		if( !wxImage::FindHandler( imageType ) )
			wxImage::AddHandler( new wxGIFHandler );
	}
#endif
#if wxUSE_LIBPNG
	else if( ext.CmpNoCase( L"png" ) == 0 )
	{
		imageType = wxBITMAP_TYPE_PNG;

		if( !wxImage::FindHandler( imageType ) )
			wxImage::AddHandler( new wxPNGHandler );
	}
#endif
#if wxUSE_ICO_CUR
	else if( ext.CmpNoCase( L"ico" ) == 0 )
	{
		imageType = wxBITMAP_TYPE_ICO;

		if( !wxImage::FindHandler( imageType ) )
			wxImage::AddHandler( new wxICOHandler );

		wxIconBundle icons( path, imageType );
		if( icons.IsOk() )
		{
			wxIcon icon( icons.GetIconOfExactSize( 16 ) );
			if( icon.IsOk() )
			{
				bm = wxBitmap( icon );
				return true;
			}
		}

		return false;
	}
#endif
	else
	{
		wxLogDebug( L"[VdkUtil::ImRead]不支持的图像格式。\n"
					L"当前支持: .BMP;.ICO;.PNG;.JPG/.JPEG;.GIF。" );

		return false;
	}

	return bm.LoadFile( path, imageType );
}

TextInfo VdkUtil::GetXrcTextInfo(wxXmlNode* xmlNode, wxWindow* window)
{
	TextInfo ti;
	wxXmlNode* chd;

	// 文本颜色
	chd = FindChildNode( xmlNode, L"font-color" );
	if( chd )
	{
		wxString strFontColor = chd->GetNodeContent();
		if( !strFontColor.IsEmpty() )
			ti.foreground.Set( strFontColor );
	}
	else
		ti.foreground = *wxBLACK;

	// 字号
	int nFontSize = 9;
	chd = FindChildNode( xmlNode, L"font-size" );
	if( chd )
	{
		wxString strFontSize = chd->GetNodeContent();
		if( !strFontSize.IsEmpty() )
			strFontSize.ToLong( (long *) &nFontSize );
	}

	// 是否粗体
	bool bBold = false;
	chd = FindChildNode( xmlNode, L"bold" );
	if( chd )
	{
		wxString strBold( chd->GetNodeContent() );
		if( strBold == L"true" )
			bBold = true;
	}

	// 字体名
	wxString strFontName;
	if( window )
	{
		strFontName = window->GetFont().GetFaceName();
	}

	chd = FindChildNode( xmlNode, L"font-family" );
	if( chd )
	{
		strFontName = chd->GetNodeContent();
		wxStringTokenizer tokenizer( strFontName, L",;\t" );
		while( tokenizer.HasMoreTokens() )
		{
			strFontName = tokenizer.GetNextToken();
			if( ti.font.SetFaceName( strFontName ) )
				break;
		}
	}

	ti.font = wxEasyCreatFont( strFontName, nFontSize, bBold );

	//////////////////////////////////////////////////////////////////////////

	// 文本边距
	ti.xPadding = gs_invalidPadding;
	ti.yPadding = gs_invalidPadding;
	ti.xPadding = XmlGetContentOfNum
		( FindChildNode( xmlNode, L"x-padding" ), gs_invalidPadding );
	ti.yPadding = XmlGetContentOfNum
		( FindChildNode( xmlNode, L"y-padding" ), gs_invalidPadding );

	return ti;
}

BackgroundInfo* VdkUtil::GetXrcBgInfo(wxXmlNode* xmlNode)
{
	BackgroundInfo* bi = NULL;
	wxXmlNode* chd = FindChildNode( xmlNode, L"bg" );
	if( chd )
	{
		bi = new BackgroundInfo;

		wxString strBgColor( XmlGetChildContent( chd, L"bg-color" ) ),
				 strBorderColor( XmlGetChildContent( chd, L"border-color" ) ),
				 strBorderWeight( XmlGetChildContent( chd, L"border-weight" ) );

		if( !strBgColor.IsEmpty() )
			bi->bgColor = wxColour( strBgColor );

		wxColour borderColor;
		if( !strBorderColor.IsEmpty() )
		{
			borderColor = wxColour( strBorderColor );

			int borderWeight = 1;
			if( !strBorderWeight.IsEmpty() )
				borderWeight = wxAtoi( strBorderWeight );

			bi->borderPen = wxPen( borderColor, borderWeight );
		}
	}

	return bi;
}

GradientBgInfo* VdkUtil::GetXrcGradientBgInfo(wxXmlNode* xmlNode)
{
	GradientBgInfo* gbi = NULL;
	wxXmlNode* chd = FindChildNode( xmlNode, L"gradient-bg" );
	if( chd )
	{
		gbi = new GradientBgInfo;
		gbi->beg = gbi->end = *wxWHITE;
		gbi->direction = wxSOUTH;

		wxString strBegColor(XmlGetChildContent( chd, L"grad-beg" ) ),
				 strEndColor( XmlGetChildContent( chd, L"grad-end" ) ),
				 strDirection( XmlGetChildContent( chd, L"direction" ) );

		wxDirection direction = wxSOUTH;
		if( strDirection.CmpNoCase( L"wxEAST" ) == 0 )
			direction = wxEAST;

		gbi->beg = wxColour( strBegColor );
		gbi->end = wxColour( strEndColor );
		gbi->direction = direction;
	}

	return gbi;
}

bool VdkUtil::IsLeftButtonEvent(int evtCode)
{
	return (evtCode == LEFT_DOWN) || (evtCode == DLEFT_DOWN);
}

wxBitmap VdkUtil::LoadPngFromRawData(const unsigned char* raw, int size)
{
#if wxUSE_LIBPNG
	wxMemoryInputStream istream( raw, size );

	if( !wxImage::FindHandler( wxBITMAP_TYPE_PNG ) )
		wxImage::AddHandler( new wxPNGHandler );

	return wxBitmap( istream, wxBITMAP_TYPE_PNG );
#else
	return wxNullBitmap;
#endif
}
