#include "StdAfx.h"
#include "VdkCaptionBar.h"
#include "VdkWindow.h"
#include "VdkButton.h"
#include "VdkLabel.h"
#include "VdkCtrlParserInfo.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT(VdkCaptionBar);

//////////////////////////////////////////////////////////////////////////

void VdkCaptionBar::Create(wxXmlNode *node) {
    DoXrcCreate(node);

    SetAddinStyle(VCS_CTRL_CONTAINER);   // 优先级较低
    m_align |= ALIGN_SYNC_X;

    //================================================

    // 这里应该取定义 XRC 文件时窗体背景位图的最小值
    int minw = m_Window->GetMinSize().x;
    int x = minw - XmlGetContentOfNum(FindChildNode(node, L"padding-right"), 0);
    int y = XmlGetContentOfNum(FindChildNode(node, L"padding-top"), 0);
    int nExpl = XmlGetContentOfNum(FindChildNode(node, L"expl"), 3);
    int h = 0;

    wxString buttonNames[] = { L"close", L"max", L"min" };
    int ids[3] = { VdkGetUniqueId(3) };
    ids[1] = ids[0] + 1;
    ids[2] = ids[1] + 1;

    typedef void (*handler)(VdkVObjEvent &);
    handler handlers[] = { ClosePanel, MaximizePanel, IconizePanel };

    for (int i = 0; i < sizeof(ids) / sizeof(int); i++) {
        wxString picName(XmlGetChildContent(node, buttonNames[i]));
        if (!picName.IsEmpty()) {
            wxBitmap bm;
            if (ImRead(bm, GetFilePath(picName))) {
                wxMask *mask = new wxMask(bm, GetMaskColour());
                bm.SetMask(mask);

                int w = bm.GetWidth() / nExpl;
                wxRect rcButton(x - w, y, w, bm.GetHeight());

                if (h < rcButton.height) {
                    h = rcButton.height;
                }

                VdkButton *btn = new VdkButton;
                btn->Create(VdkButtonInitializer().
                            id(ids[i]).
                            window(m_Window).
                            addToWindow(true).
                            name(buttonNames[i]).
                            rect(rcButton).
                            align(ALIGN_RIGHT).
                            bitmap(bm).
                            explode(nExpl));

                m_WindowImpl->Bind(wxEVT_VOBJ, handlers[i], ids[i]);

                //======================================

                x = rcButton.x;
            }

        }

    }

    //===========================================

    m_Rect.x = m_Rect.y = 0;
    m_Rect.width = x;

    int specHeight = XmlGetContentOfNum(FindChildNode(node, L"height"), 0);
    m_Rect.height = (specHeight > 0) ? specHeight : h;

    // 标题由 VDK 默认处理（太多可定义的东西了）
    ParseObjects(VdkCtrlParserInfo().window(m_Window).node(node));
}

void VdkCaptionBar::DoHandleMouseEvent(VdkMouseEvent &e) {
    if (e.evtCode == DLEFT_DOWN) {
        if (m_Window->TestStyle(VWS_MAXIMIZABLE)) {
            wxTopLevelWindow *toplevel =
                m_Window->GetTopLevelWindowHandle();

            if (toplevel) {
                toplevel->Maximize(!toplevel->IsMaximized());
            }
        }
    } else if (e.evtCode == DRAGGING &&
               m_Window->TestStyle(VWS_DRAGGABLE)) {
        m_Window->EmulateDragAndMove
            (m_WindowImpl->ClientToScreen(e.mousePos));
    } else {
        e.Skip(true);
    }
}

//////////////////////////////////////////////////////////////////////////

/*static*/
void VdkCaptionBar::ClosePanel(VdkVObjEvent &e) {
    ((wxTopLevelWindow *)(e.GetCtrl<VdkButton>()->
                          GetVdkWindow()->
                          GetWindowHandle()))->
    Close();
}

/*static*/
void VdkCaptionBar::IconizePanel(VdkVObjEvent &e) {
    ((wxTopLevelWindow *)(e.GetCtrl<VdkButton>()->
                          GetVdkWindow()->
                          GetWindowHandle()))->
    Iconize(true);
}

/*static*/
void VdkCaptionBar::MaximizePanel(VdkVObjEvent &e) {

}
