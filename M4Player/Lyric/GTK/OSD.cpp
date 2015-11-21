/***************************************************************
 * Name:      OSD.cpp
 * Purpose:   桌面歌词平台相关部分的代码
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-06
 **************************************************************/
#include "StdAfx.h"
#include "../OOPDesktopLyric.h"

#include <gtk/gtk.h>
#include <wx/graphics.h>

//////////////////////////////////////////////////////////////////////////

void OOPDesktopLyric::Initialize() {

}

void OOPDesktopLyric::Finalize() {

}

bool OOPDesktopLyric::UpdateAtOnce() {
    wxClientDC dc(this);
    wxGraphicsContext *gc = wxGraphicsContext::Create(dc);

    if (!gc) {
        wxLogError(L"wxGraphicsContext::Create() failed!");
        return false;
    }

    if (m_showBackgound) {
        ShowBackgound(gc);
    }

    const wxString textToDraw(IsCurrLineValid() ?
                              (*m_currLine)->GetLyric() :
                              GetInteractiveOutput());

    //========================================
    // 添加文本

    cairo_t *cr = (cairo_t *) gc->GetNativeContext();

    wxCoord winHeight;
    GetSize(NULL, &winHeight);
    wxASSERT(size_t(winHeight) >= m_style.pxFontSize);

    PangoLayout *layout = pango_cairo_create_layout(cr);

    PangoFontDescription *font_desc = NULL;
    font_desc = pango_font_description_from_string(m_style.fontFace);

    pango_layout_set_text(layout, textToDraw, -1);
    pango_layout_set_font_description(layout, font_desc);

    pango_font_description_free(font_desc);

    int lrc_width, lrc_height;
    pango_layout_get_size(layout, &lrc_width, &lrc_height);
    lrc_width = lrc_width / PANGO_SCALE/* + 20*/;
    lrc_height /= PANGO_SCALE;

    m_textPathBounds.m_x = 0;
    m_textPathBounds.m_y = 0;
    m_textPathBounds.m_width  = lrc_width;
    m_textPathBounds.m_height = lrc_height;

    pango_cairo_update_layout(cr, layout);
    pango_cairo_layout_path(cr, layout);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1);
    cairo_stroke_preserve(cr);

    cairo_clip(cr);
    g_object_unref(layout);

    wxGraphicsBrush normalBrush =
        gc->CreateLinearGradientBrush(0, (winHeight - m_style.pxFontSize) / 2,
                                      0, (winHeight + m_style.pxFontSize) / 2,
                                      wxColour(255,255,255,255),
                                      wxColour(30,120,195,255));
    gc->SetBrush(normalBrush);

    cairo_rectangle(cr, 0, 0, lrc_width, lrc_height);
    cairo_fill(cr);

    delete gc;
    gc = NULL;

    //===========================================

    return Present(dc);
}

wxGraphicsPath OOPDesktopLyric::InitLineTextPath(wxGraphicsContext *gc) {
    wxASSERT(gc);

    // 文本轮廓
    wxGraphicsPen pen = gc->CreatePen(wxPen(wxColour(0,0,0,255), 3));

    const wxString textToDraw(IsCurrLineValid() ?
                              (*m_currLine)->GetLyric() :
                              GetInteractiveOutput());

    //========================================
    // 添加文本

    cairo_t *cr = (cairo_t *) gc->GetNativeContext();

    wxCoord winHeight;
    GetSize(NULL, &winHeight);
    wxASSERT(size_t(winHeight) >= m_style.pxFontSize);

    PangoLayout *layout = pango_cairo_create_layout(cr);

    PangoFontDescription *font_desc = NULL;
    font_desc = pango_font_description_from_string(m_style.fontFace);

    //printf("%s\n", (const char*) textToDraw);

    pango_layout_set_text(layout, textToDraw, -1);
    pango_layout_set_font_description(layout, font_desc);

    pango_font_description_free(font_desc);

    int lrc_width, lrc_height;
    pango_layout_get_size(layout, &lrc_width, &lrc_height);
    lrc_width = lrc_width / PANGO_SCALE/* + 20*/;
    lrc_height /= PANGO_SCALE;

    //printf("w: %d, h: %d\n", lrc_width, lrc_height);

    m_textPathBounds.m_x = 0;
    m_textPathBounds.m_y = 0;
    m_textPathBounds.m_width  = lrc_width;
    m_textPathBounds.m_height = lrc_height;

    //path.UnGetNativePath( cr );

    //========================================
    // 添加到路径中

    wxGraphicsPath path = gc->CreatePath();
    gc->SetPen(pen);
    gc->StrokePath(path);

    return path;
}

bool OOPDesktopLyric::Present(wxDC &drawings) {
    Update();
    return true;
}

void OOPDesktopLyric::OnLeftDown(wxMouseEvent &) {
    // 移动窗口

}

