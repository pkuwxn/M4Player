/***************************************************************
 * Name:      VdkBitmapArray.cpp
 * Purpose:   Code for VdkBitmapArray implementation
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-01-10
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkBitmapArray.h"
#include "VdkUtil.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkBitmapArray VdkNullBitmapArray;

VdkBitmapArray::VdkBitmapArray() : m_cols( 1 ), m_rows( 1 )
{

}

VdkBitmapArray::VdkBitmapArray(const wxString& strPath, 
						 unsigned cols, 
						 unsigned rows)
{
	Set( strPath, cols, rows );
}

VdkBitmapArray::VdkBitmapArray(const VdkBitmapArray& other)
{
	Assign( other );
}

VdkBitmapArray& VdkBitmapArray::operator = (const VdkBitmapArray& rhs)
{
	Assign( rhs );
	return *this;
}

void VdkBitmapArray::Assign(const VdkBitmapArray& rhs)
{
	m_cols = rhs.m_cols;
	m_rows = rhs.m_rows;

	if( rhs.m_bmp.IsOk() )
	{
		m_bmp = rhs.m_bmp.GetSubBitmap
			( wxRect( 0, 0, rhs.m_bmp.GetWidth(), 
					  rhs.m_bmp.GetHeight() ) );
	}
}

void VdkBitmapArray::Set(const wxBitmap& bm, unsigned cols, unsigned rows)
{
	m_bmp = bm;
	m_cols = cols;
	m_rows = rows;
}

void VdkBitmapArray::Set(const wxString& strPath, unsigned cols, unsigned rows)
{
	wxBitmap bm;
	if( VdkUtil::ImRead( bm, strPath ) )
	{
		Set( bm, cols, rows );
	}
}

void VdkBitmapArray::BlitTo
	(wxDC& dc, unsigned col, unsigned row, wxCoord x, wxCoord y)
{
	if( !m_bmp.IsOk() )
		return;

	int w = GetWidth(), h = GetHeight();
	wxMemoryDC mdc( m_bmp );
	dc.Blit( x, y, w, h, &mdc, col * w, row * h, wxCOPY, true );
}

unsigned VdkBitmapArray::GetWidth() const
{
	return m_bmp.IsOk() ? m_bmp.GetWidth() / m_cols : 0;
}

unsigned VdkBitmapArray::GetHeight() const
{
	return m_bmp.IsOk() ? m_bmp.GetHeight() / m_rows : 0;
}

bool VdkBitmapArray::Resize(int newwidth, int newheight, 
							VdkResizeableBitmapType type, 
							const wxRect& rcTile)
{
	if( !IsOk() || m_rows != 1 )
		return false;

	int w = GetWidth();
	int h = GetHeight();

	if( newwidth == 0 ) newwidth = w;
	if( newheight == 0 ) newheight = h;

	//////////////////////////////////////////////////////////////////////////

	unsigned nExpl = GetExplNum();

    wxBitmap bmTmp0( w, h ), bmTmp = bmTmp0;
	wxBitmap bm( newwidth * nExpl, newheight );
	wxMemoryDC mdc( bmTmp ), mdcDest( bm );

	for( unsigned i = 0; i < nExpl; i++ )
	{
		BlitTo( mdc, i, 0, 0, 0 );
		mdc.SelectObject( wxNullBitmap );

		if( type == RESIZE_TYPE_TILE )
		{
			bmTmp = VdkUtil::TileBitmap( bmTmp, newwidth, newheight, rcTile );
		}
		else
		{
			bmTmp = VdkUtil::InflateBitmapByCenter
				( bmTmp, rcTile, wxSize( newwidth, newheight ) );
		}

		mdc.SelectObject( bmTmp );
        mdcDest.Blit( i * newwidth, 0, newwidth, newheight, 
					  &mdc, 0, 0, wxCOPY, true );

		mdc.SelectObject( wxNullBitmap );
		bmTmp = bmTmp0;
		mdc.SelectObject( bmTmp );
	}

	mdcDest.SelectObject( wxNullBitmap );
	Set( bm, nExpl, 1 );

	return true;
}

//////////////////////////////////////////////////////////////////////////

void VdkBitmapArrayId::BlitTo(wxDC& dc, wxCoord x, wxCoord y) const
{
	m_ba->BlitTo( dc, m_x, m_y, x, y );
}
