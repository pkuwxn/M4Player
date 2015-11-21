/***************************************************************
 * Name:      OOPSkinMenuSelector.cpp
 * Purpose:   酷狗2011 风格的皮肤选择器
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-05-03
 * Copyright: Wang Xiaoning
 **************************************************************/
#include "StdAfx.h"
#include "OOPSkinMenuSelector.h"

#include "VdkDC.h"
#include "wxUtil.h"

#include <memory>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT(OOPSkinMenuSelector)

OOPSkinMenuSelector::OOPSkinMenuSelector() {
    // 因为控件位于菜单上，所以需要引入一个 HACK
    VdkWindow::SetAddinStyle(VWS_MENU_IMPL);
}

OOPSkinMenuSelector::~OOPSkinMenuSelector() {

}

VdkScrollBar *OOPSkinMenuSelector::ScrollBarExists() {
    int nExpl = 3;
    wxString bmPath(VdkControl::GetRootPath());
    bmPath += L"../../App/ScrollBar.bmp";

    wxBitmap bmAll(bmPath, wxBITMAP_TYPE_BMP);
    wxBitmap bmArray[3];

    wxRect sub(0, 0, 42, 28);
    bmArray[0] = bmAll.GetSubBitmap(sub);

    sub.y = 28;
    sub.height = 19;
    bmArray[1] = bmAll.GetSubBitmap(sub);

    VdkScrollBar *sb = new VdkScrollBar;
    sb->Create(VdkScrollBarInitializer().
               window(ParentWindow()).
               name(Name() + L"_scrollbar").
               rect(ScrollBarRect()).
               align(ALIGN_SYNC_Y | ALIGN_RIGHT).
               addToWindow(true).
               parent(ScrollBarParent()).
               bitmapArray(bmArray, false).
               thumbExpl(nExpl).
               thumbResizeType(RESIZE_TYPE_TILE).
               thumbTileLen(5).
               castTo<VdkScrollBarInitializer>());

    return sb;
}

//////////////////////////////////////////////////////////////////////////

SkinAvatar::SkinAvatar()
    : m_skinInfo(NULL), m_state(NORMAL), m_xText(0),
      m_borderNomal(wxColour(140, 178, 222)),
      m_hilight(wxColour(0, 133, 235)) {

}

void SkinAvatar::Create(VdkWindow *win, const wxRect &rc,
                        const OOPSkinInfo &info) {
    SetVdkWindow(win);
    m_strName = info.name();
    m_Rect = rc;
    m_skinInfo = &info;

    SetAddinStyle(VCS_ERASE_BG);

    //====================================================

    // ToolTip
    wxString tooltip;
    tooltip.Printf(L"%s\n作者: %s", m_skinInfo->name(), m_skinInfo->author());

    wxString email(m_skinInfo->email());
    if (!email.IsEmpty()) {
        tooltip += L"\n邮箱: " + email;
    }

    wxString url(m_skinInfo->url());
    if (!url.IsEmpty()) {
        tooltip += L"\n网站: " + url;
    }

    SetToolTip(tooltip);

    // 说明文本

    wxMemoryDC mdc;
    mdc.SetFont(m_Font);

    int w;
    m_labelOverflow = m_skinInfo->name();
    mdc.GetTextExtent(m_labelOverflow, &w, NULL);

    if (w > m_Rect.width) {
        wxArrayInt widths;
        mdc.GetPartialTextExtents(m_labelOverflow, widths);

        int i(0),
            suitable(m_Rect.width - mdc.GetCharWidth() * 3);

        while (widths[i++] <= suitable) ;
        i--;

        m_labelOverflow = m_labelOverflow.Mid(0, i);
        m_labelOverflow += L"...";

        m_xText = m_Rect.x;
    } else {
        m_xText = m_Rect.x + (m_Rect.width - w) / 2;
    }
}

wxDEFINE_EVENT(OOP_EVT_LOAD_SKIN_AVATAR, wxCommandEvent);

void SkinAvatar::OnLoadAvatar(wxCommandEvent &e) {
    if (!m_skinInfo->isThumbNailOk()) {
        m_skinInfo->thumbNail(wxSize(ms_tnwidth, ms_tnheight));

        VdkDC vdc(m_Window, GetAbsoluteRect(), NULL);
        Draw(vdc);
    }

    m_WindowImpl->Unbind(OOP_EVT_LOAD_SKIN_AVATAR,
                        &SkinAvatar::OnLoadAvatar,
                         this, m_id);
}

void SkinAvatar::DoDraw(wxDC &dc) {
    int x = m_Rect.x + (m_Rect.width - ms_tnwidth) / 2;
    wxRect rcAvatar(x - ms_yPadding, m_Rect.y,
                    ms_tnwidth + ms_yPadding * 2,
                    ms_tnheight + ms_yPadding * 2);
    rcAvatar.Inflate(1);

    if (m_state == NORMAL) {
        dc.SetPen(m_borderNomal);
        DrawRectangle(dc, rcAvatar);
    } else {
        dc.SetBrush(m_hilight);
        dc.DrawRectangle(rcAvatar);
    }

    //====================================================
    // 画缩略图

    if (m_skinInfo->isThumbNailOk()) {
        wxSize sz(ms_tnwidth, ms_tnheight);
        wxBitmap thumbNail(m_skinInfo->thumbNail(sz));

        wxRect rc(x,
                  m_Rect.y + ms_yPadding,
                  ms_tnwidth,
                  ms_tnheight);

        dc.SetBrush(m_WindowImpl->GetBackgroundColour());
        dc.DrawRectangle(rc);
        dc.DrawBitmap(thumbNail, x, m_Rect.y + ms_yPadding, true);
    } else {
        m_WindowImpl->Bind(OOP_EVT_LOAD_SKIN_AVATAR,
                          &SkinAvatar::OnLoadAvatar,
                           this, m_id);

        wxCommandEvent e(OOP_EVT_LOAD_SKIN_AVATAR);
        e.SetId(m_id);
        wxPostEvent(m_WindowImpl, e);
    }
}

void SkinAvatar::DoHandleMouseEvent(VdkMouseEvent &e) {
    switch (e.evtCode) {
    case HOVERING:
    case NORMAL:

        m_state = (VdkMouseEventType) e.evtCode;
        Draw(e.dc);

        break;

    case WHEEL_DOWN:
    case WHEEL_UP:

        e.Skip(true);
        break;

    case DLEFT_DOWN:

        if (IsReadyForEvent()) {
            // 在 GTK 下面不能首先取消菜单的显示，即调用 VdkWindow::HideMenu()

            // 不要提供绘图上下文，异步发送通知事件
            FireEvent(NULL, NULL);
        }

        break;

    default:

        break;
    }
}
