/***************************************************************
 * Name:      OOPDesktopLyric.cpp
 * Purpose:   桌面歌词(On Screen Display Lyric)
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-02
 **************************************************************/
#include "StdAfx.h"
#include "OOPDesktopLyric.h"
#include "../OOPStopWatch.h"

#include <wx/graphics.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(OOPDesktopLyric, DeskLrcSuperClass)

    EVT_ENTER_WINDOW(OOPDesktopLyric::OnEnterWindow)
    EVT_LEAVE_WINDOW(OOPDesktopLyric::OnLeaveWindow)
    EVT_LEFT_DOWN(OOPDesktopLyric::OnLeftDown)

wxEND_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////

enum {
    WIDTH = 755,
    HEIGHT = 80,
};

OOPDesktopLyric::OOPDesktopLyric(wxWindow *parent)
    : DeskLrcSuperClass(parent),
      m_parser(NULL),
      m_showBackgound(false),
      m_timerId(wxIdManager::ReserveId(1)),
      m_timer(wxTheApp, m_timerId) {
    // TODO: 这样做貌似不够优雅
    SetSize(WIDTH, HEIGHT);

    Initialize();

    //======================================================

    wxTheApp->Bind(wxEVT_TIMER, &OOPDesktopLyric::OnTimerNotify, this, m_timerId);
}

OOPDesktopLyric::~OOPDesktopLyric() {
    wxTheApp->Unbind(wxEVT_TIMER, &OOPDesktopLyric::OnTimerNotify, this, m_timerId);

    //======================================================

    Finalize();
}

bool OOPDesktopLyric::IsOk() const {
    bool isParserOk = m_parser && m_parser->IsOk();
    return m_stopWatch && isParserOk;
}

void OOPDesktopLyric::AttachParser(const OOPLyricParser &parser) {
    m_parser = &parser;

    //==========================================

    m_currLine = m_parser->end();
}

void OOPDesktopLyric::UpdateProgress() {
    wxASSERT(m_parser);

    double now = m_stopWatch->Time();
    GoTo(now / m_parser->GetTimeSum(), NULL, !m_stopWatch->IsRunning());
}

double OOPDesktopLyric::GetLineProgress() const {
    LineInfo *currLine = *m_currLine;
    size_t lineHasGone = m_stopWatch->Time() - currLine->GetStartTime();
    if (lineHasGone >= currLine->GetMilSeconds()) {
        return 1;
    }

    // 假如除数为 0，不会来到这里
    return double(lineHasGone) / currLine->GetMilSeconds();
}

void OOPDesktopLyric::Start() {
    UpdateProgress();
}

void OOPDesktopLyric::Pause() {
    m_timer.Stop();
}

void OOPDesktopLyric::Stop(wxDC *pDC) {
    m_timer.Stop();
    m_currLine = m_parser->end();

    // 显示交互性文本输出
    if (pDC) {
        UpdateAtOnce();
    }
}

void OOPDesktopLyric::ClearLyric(wxDC *pDC) {
    Stop(pDC);
}

void OOPDesktopLyric::GoTo(double percentage, wxDC *, bool bPaused) {
    wxASSERT(IsOk());

    m_currLine = m_parser->WhichLine(m_parser->GetTimeSum() * percentage);
    if (m_currLine == m_parser->end()) {
        wxLogDebug(L"Wrong position.");
        return;
    }

    // 开始“渲染循环”
    if (!bPaused && !m_timer.IsRunning()) {
        m_timer.Start(REFRESH_INTERVAL_MS);
    }

    UpdateAtOnce();
}

void OOPDesktopLyric::DoSetInteractiveOutput(wxDC *pDC) {
    // 立即更新显示交互性文本输出
    if (pDC) {
        UpdateAtOnce();
    }
}

void OOPDesktopLyric::ShowBackgound(wxGraphicsContext *gc) {
    wxASSERT(m_showBackgound);

    wxGraphicsPath rect = gc->CreatePath();
    rect.AddRectangle(0, 0, WIDTH, HEIGHT);

    gc->SetBrush(wxBrush(wxColour(217,217,217,16)));
    gc->FillPath(rect);

    gc->SetPen(*wxGREY_PEN);
    gc->DrawRectangle(0, 0, WIDTH, HEIGHT);

    gc->SetPen(*wxGREY_PEN);
    gc->DrawRectangle(m_style.borderSize, m_style.borderSize,
                      WIDTH - m_style.borderSize * 2,
                      HEIGHT - m_style.borderSize * 2);
}

bool OOPDesktopLyric::IsCurrLineValid() const {
    return IsOk() && (m_currLine != m_parser->end());
}

void OOPDesktopLyric::OnEnterWindow(wxMouseEvent &) {
    m_showBackgound = true;
    UpdateAtOnce();
}

void OOPDesktopLyric::OnLeaveWindow(wxMouseEvent &) {
    m_showBackgound = false;
    UpdateAtOnce();
}

void OOPDesktopLyric::OnTimerNotify(wxTimerEvent &) {
    wxASSERT(IsOk());

    LineIter next(m_currLine);
    ++next;

    if (next != m_parser->end()) {
        if ((*next)->GetStartTime() <= m_stopWatch->Time()) {
            // 待上一句显示完整再跳到下一句
            UpdateAtOnce();
            NextLine();

            return;
        }
    }

    UpdateAtOnce();
}

inline void OOPDesktopLyric::NextLine() {
    wxASSERT(m_parser);

#ifdef __WXDEBUG__
    {
        // 必须由外部保证不会越界
        LineIter next(m_currLine);
        ++next;
        wxASSERT(next != m_parser->end());
    }
#endif

    ++m_currLine;
}

//////////////////////////////////////////////////////////////////////////

OOPDesktopLyric::Style::Style()
    : fontFace(L"黑体"), pxFontSize(40), bold(true),
      borderSize(1), alpha(196) {

}
