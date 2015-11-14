#include "StdAfx.h"
#ifdef __WXMSW__

#include "MSW/ExtMenuItem.h"
#include "MSW/ExtMenu.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


//////////////////////////////////////////////////////////////////////////

#define MENU_HEIGHT				22
#define CHECKRECT_HEIGHT		20
#define LEFTBAR_WIDTH			24

wxPen		ExtMenuItem::m_borderPen(wxColour(0,0,128));
wxBrush		ExtMenuItem::m_selectedBrush(wxColour(255,238,194));
wxBrush		ExtMenuItem::m_checkedBrush(wxColour(255,192,111));
wxBrush		ExtMenuItem::m_selectedBrushChecked(wxColour(254,128,62));

bool		ExtMenuItem::m_bLeftBitmapInited(false);
wxBitmap	ExtMenuItem::m_leftBar(LEFTBAR_WIDTH, MENU_HEIGHT);
wxBitmap	ExtMenuItem::m_leftBarForSep;

//////////////////////////////////////////////////////////////////////////

ExtMenuItem::ExtMenuItem(ExtMenu* parentMenu,
						 int id,
						 const wxString& name,
						 const wxString& help,
						 wxItemKind kind,
						 ExtMenu* subMenu)
		   : wxMenuItem(parentMenu, id, name, help, kind, subMenu)
{
	SetOwnerDrawn(true);
	((wxMenu*)parentMenu)->Append(this);

	if( !m_bLeftBitmapInited )
	{
		wxMemoryDC mdc(m_leftBar);
		mdc.GradientFillLinear(wxRect(0, 0, LEFTBAR_WIDTH, MENU_HEIGHT), 
							   wxColour(227,239,255),
							   wxColour(136,174,228));
		mdc.SelectObject(wxNullBitmap);
		m_leftBarForSep = m_leftBar.GetSubBitmap(wxRect(0, 0, LEFTBAR_WIDTH, 5));

		m_bLeftBitmapInited = true;
	}
}

bool ExtMenuItem::OnMeasureItem(size_t* pwidth, size_t* pheight)
{
	if( IsSeparator() )
	{
		*pwidth  = 3;
		*pheight = 5;
	}
	else
	{
		wxOwnerDrawn::OnMeasureItem(pwidth, pheight);
		*pheight = MENU_HEIGHT;
	}

	return true;
}

bool ExtMenuItem::OnDrawItem(wxDC& dc, 
							 const wxRect& rc, 
							 wxODAction/* act*/, 
							 wxODStatus st)
{
	wxRect rt(rc);
	rt.x += 2;
	rt.y += 2;

	// *2, as in wxSYS_EDGE_Y
	int margin = GetMarginWidth() + 2 * wxSystemSettings::GetMetric(wxSYS_EDGE_X);

	// determine where to draw and leave space for a check-mark.
	// + 4 pixel to separate the edge from the highlight rectangle
	// + 3 pixel to separate text from the highlight rectangle
	int xText = rt.x + margin + 4 + 3;

	if( rc.GetY() == 0 )
	{
		HWND win = ExtMenu::GetActiveExtMenu();

		if( win )
		{
			RECT rect;
			GetClientRect(win, &rect);
			wxRect rt(0, 0, rect.right-rect.left, rect.bottom-rect.top);

			dc.SetPen(m_borderPen);
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.DrawRectangle(rt);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	if( IsSeparator() )
	{
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(*wxWHITE_BRUSH);
		dc.DrawBitmap(m_leftBarForSep, 1, rt.GetTop());
		dc.DrawRectangle(LEFTBAR_WIDTH, rt.y, rt.GetRight()-LEFTBAR_WIDTH, rt.GetHeight());

		dc.SetPen(*wxGREY_PEN);
		int y(rt.GetTop()+rt.GetHeight()/2);
		dc.DrawLine(xText, y, rt.GetRight(), y);

		return true;
	}

	dc.DrawBitmap(m_leftBar, 1, rt.GetTop());

	wxColour& m_colBack(GetBackgroundColour());
	wxColour& m_colText(GetTextColour());
	const wxBitmap& bmpChecked(GetBitmap(true));

	int m_nHeight(MENU_HEIGHT);
	static bool s_showCues(false);
	if( SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &s_showCues, 0) == 0 )
		s_showCues = true;

    // set the colors
    // --------------
    DWORD colBack, colText;
    if( st & wxODSelected )
    {
        colBack = wxColourToPalRGB(m_selectedBrush.GetColour());

        if( !(st & wxODDisabled) )
			colText = wxColourToPalRGB(*wxBLACK);
        else
            colText = GetSysColor(COLOR_GRAYTEXT);
    }
    else
    {
        // fall back to default colors if none explicitly specified
        colBack = m_colBack.Ok() ? wxColourToPalRGB(m_colBack)
                                 : GetSysColor(COLOR_MENU);
        colText = m_colText.Ok() ? wxColourToPalRGB(m_colText)
                                 : GetSysColor(COLOR_MENUTEXT);
    }

    HDC hdc = GetHdcOf(dc);
    COLORREF colOldText = ::SetTextColor(hdc, colText),
             colOldBack = ::SetBkColor(hdc, colBack);

	// select the font and draw the text
	// ---------------------------------

    // using native API because it recognizes '&'
    if( IsOwnerDrawn() )
    {
        int nPrevMode = SetBkMode(hdc, TRANSPARENT);

		//////////////////////////////////////////////////////////////////////////		

		if( st & wxODSelected && !(st & wxODDisabled) )
		{
			dc.SetPen(m_borderPen);
			dc.SetBrush(m_selectedBrush);

			dc.DrawRectangle(rt);
		}
		else
		{
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(*wxWHITE_BRUSH);

			wxRect rect(rt);
			rect.x += LEFTBAR_WIDTH - 2;
			rect.width -= LEFTBAR_WIDTH - 2;

			dc.DrawRectangle(rect);
		}

		//////////////////////////////////////////////////////////////////////////

#if wxVERSION_NUMBER >= 2900

        // use default font if no font set
        wxFont fontToUse;
		GetFontToUse(fontToUse);
        SelectInHDC selFont(hdc, GetHfontOf(fontToUse));

		wxString m_strName(GetName());
		wxString m_strAccel(GetItemLabel().AfterFirst(wxT('\t')));
        wxString strMenuText = m_strName.BeforeFirst('\t');

#else

		// use default font if no font set
		wxFont fontToUse(GetFontToUse());
		SelectInHDC selFont(hdc, GetHfontOf(fontToUse));
		wxString strMenuText = m_strName.BeforeFirst('\t');

#endif // wxVERSION_NUMBER >= 2900

		WCHAR* szText(new WCHAR[strMenuText.Length()+1]);
		wcscpy(szText, strMenuText.c_str());

        SIZE sizeRect;
        ::GetTextExtentPoint32(hdc, strMenuText.c_str(), strMenuText.length(), &sizeRect);
        ::DrawState(hdc, NULL, NULL,
                   (LPARAM)szText, strMenuText.length(),
                    xText, rt.y + (int)((rt.GetHeight()-sizeRect.cy)/2.0), // centre text vertically
                    rt.GetWidth()-margin, sizeRect.cy,
                    DST_PREFIXTEXT |
                    (((st & wxODDisabled) && !(st & wxODSelected)) ? DSS_DISABLED : 0) |
                    (((st & wxODHidePrefix) && !s_showCues) ? 512 : 0)); // 512 == DSS_HIDEPREFIX

        // ::SetTextAlign(hdc, TA_RIGHT) doesn't work with DSS_DISABLED or DSS_MONO
        // as the last parameter in DrawState() (at least with Windows98). So we have
        // to take care of right alignment ourselves.
        if( !m_strAccel.empty() )
        {
			delete [] szText;
			szText = NULL;
			szText = new WCHAR[m_strAccel.Length()+1];
			wcscpy(szText, m_strAccel.c_str());

            int accel_width, accel_height;
            dc.GetTextExtent(m_strAccel, &accel_width, &accel_height);

            // right align accel string with right edge of menu ( offset by the
            // margin width )
            ::DrawState(hdc, NULL, NULL,
					   (LPARAM)szText, m_strAccel.length(),
						rt.GetWidth()-16-accel_width, rt.y+(int) ((rt.GetHeight()-sizeRect.cy)/2.0),
						0, 0, 
						DST_TEXT |
						(((st & wxODDisabled) && !(st & wxODSelected)) ? DSS_DISABLED : 0));
        }

        (void)SetBkMode(hdc, nPrevMode);

		delete [] szText;
		szText = NULL;
    }
	
    // draw the bitmap
    // ---------------
    if( IsCheckable() && !bmpChecked.IsOk() )
    {
        if( st & wxODChecked )
        {
			DrawColorRect(dc, rt, st);

			//*
            // what goes on: DrawFrameControl creates a b/w mask,
            // then we copy it to screen to have right colors

			int cxCheck = GetSystemMetrics(SM_CXMENUCHECK);
			int cyCheck = GetSystemMetrics(SM_CYMENUCHECK);

            // first create a monochrome bitmap in a memory DC
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmpCheck = CreateBitmap(cxCheck, cyCheck, 1, 1, 0);
            SelectObject(hdcMem, hbmpCheck);

            // then draw a check mark into it
            RECT rect = { 0, 0, cxCheck, cyCheck };
            if( m_nHeight > 0 )
                ::DrawFrameControl(hdcMem, &rect, DFC_MENU, DFCS_MENUCHECK);

			int x(rt.x+(LEFTBAR_WIDTH-cxCheck)/2), 
				y(rt.y+(m_nHeight-cyCheck)/2);

			COLORREF clrTextPrev = SetTextColor(hdc, RGB(255,255,255));
			COLORREF clrBkPrev = SetBkColor(hdc, RGB(0,0,0));
			BitBlt(hdc, x, y, cxCheck, cyCheck, hdcMem, 0, 0, 0x00220326); // DSna
			SetTextColor(hdc, GetSysColor(COLOR_MENUTEXT));
			BitBlt(hdc, x, y, cxCheck, cyCheck, hdcMem, 0, 0, SRCPAINT);
			SetBkColor(hdc, clrBkPrev);
			SetTextColor(hdc, clrTextPrev);

            DeleteDC(hdcMem);
            DeleteObject(hbmpCheck);//*/
        }
    }
    else
    {
        wxBitmap bmp;

        if( st & wxODDisabled )
            bmp = GetDisabledBitmap();

        if( !bmp.IsOk() )
        {
            // for not checkable bitmaps we should always use unchecked one
            // because their checked bitmap is not set
            bmp = bmpChecked;/*GetBitmap(!IsCheckable() || (st & wxODChecked))*/;
			if( st & wxODChecked )
				DrawColorRect(dc, rt, st);

#if wxUSE_IMAGE
            if( bmp.Ok() && (st & wxODDisabled) )
            {
                // we need to grey out the bitmap as we don't have any specific
                // disabled bitmap
                wxImage imgGrey = bmp.ConvertToImage().ConvertToGreyscale();
                if( imgGrey.Ok() )
                    bmp = wxBitmap(imgGrey);
            }
#endif // wxUSE_IMAGE
        }

        if( bmp.Ok() )
        {
			int nBmpWidth  = bmp.GetWidth(),
				nBmpHeight = bmp.GetHeight();

			// there should be enough space!
			wxASSERT((nBmpWidth <= rt.GetWidth()) && (nBmpHeight <= rt.GetHeight()));

			dc.DrawBitmap(bmp, 1 + (LEFTBAR_WIDTH - nBmpWidth) / 2,
							   rt.y + (m_nHeight - nBmpHeight) / 2);
        }
    }

    ::SetTextColor(hdc, colOldText);
    ::SetBkColor(hdc, colOldBack);

    return true;
}

void ExtMenuItem::DrawColorRect(wxDC& dc, wxRect rt, wxODStatus st)
{
	// 画底部的色区

	int rx(1+(LEFTBAR_WIDTH-CHECKRECT_HEIGHT)/2), 
		ry(rt.y+(MENU_HEIGHT-CHECKRECT_HEIGHT)/2);

	dc.SetPen(m_borderPen);
	if( st & wxODSelected )
		dc.SetBrush(m_selectedBrushChecked);
	else
		dc.SetBrush(m_checkedBrush);

	dc.DrawRectangle(rx, ry, CHECKRECT_HEIGHT, CHECKRECT_HEIGHT);
}

#endif // __WXMSW__