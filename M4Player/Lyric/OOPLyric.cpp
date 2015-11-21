/***************************************************************
 * Name:      OOPLyric.h
 * Purpose:   集成在歌词秀窗口的内嵌歌词显示控件
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2010
 **************************************************************/
#include "StdAfx.h"
#include "OOPLyric.h"

#include "OOPLyricParser.h"
#include "../OOPStopWatch.h"
#include "../OOPDefs.h"
#include "../OOPApp.h"

#include "VdkDC.h"
#include "VdkWindow.h"
#include "wxUtil.h" // for XmlInsertChild()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

// OOPLyric 的运行时状态属性集
enum OOPLyricState {

    // 是否已经开始
    OLST_STARTED                    =   1 << (VLCST_USER + 0),
    // 是否已经停止
    OLST_STOPPED                    =   1 << (VLCST_USER + 1),
    // 是否已经暂停
    OLST_PAUSED                     =   1 << (VLCST_USER + 2),
    // 是否已经开始拖动歌词
    OLST_DRAGGING_STARTED           =   1 << (VLCST_USER + 3),
    // 是否正在拖动歌词
    OLST_DRAGGING                   =   1 << (VLCST_USER + 4),
    // 拖动歌词前歌词是否正在运行
    OLST_RUNNING_BEFORE_DRAGGING    =   1 << (VLCST_USER + 5),
};

enum {
    /*! 使用渐变色还原上一句歌词持续的时间
     */
    ALPHA_SHOW_LAST_LINE_MS = 1000,
};

IMPLEMENT_DYNAMIC_VOBJECT(OOPLyric);

//////////////////////////////////////////////////////////////////////////

OOPLyric::OOPLyric()
    : VdkListCtrl(0), m_parser(NULL),
      m_timerId(wxIdManager::ReserveId(1)),
      m_timer(wxTheApp, m_timerId) {
    // 直接绑定本地数据集
    Attach(&m_dataSet);
}

void OOPLyric::Init() {
    wxASSERT(m_parser);

    m_currLine = m_parser->end();
    m_blankLinesTop = 0;
    m_blankLinesBottom = 0;

    m_draggDistance = 0;
    m_draggHit = m_parser->end();

    //---------------------------------------------------------
    // 清空所有位状态
    // TODO: 写法是这样的吗？

    unsigned bitfieldsClean = unsigned(~0);
    bitfieldsClean >>= sizeof(bitfieldsClean) * 8 - VLCST_USER;

    SetState(GetState() & bitfieldsClean);
}

void OOPLyric::OnXrcCreate(wxXmlNode *node) {
    // 这是 VdkControl 的函数
    VdkControl::DoXrcCreate(node);

    XmlInsertChild(node, L"header", L"0");
    VdkListCtrl::OnXrcCreate(node);

    wxArrayString colorArray;
    colorArray.Add(XmlGetChildContent(node, L"TextColor"));
    colorArray.Add(XmlGetChildContent(node, L"HilightColor"));
    colorArray.Add(XmlGetChildContent(node, L"BkgndColor"));

    Create(colorArray);
}

void OOPLyric::Create(const wxArrayString &colorArray) {
    m_strName.assign(L"OOPLyric");
    m_align = ALIGN_SYNC_X_Y;

    SetRowHeight(15);   // 行高
    SetLinesUpDown(0);   // 滚轮失去了作用
    SetScrollRate(1, 1);   // 每行高 1px
    SetShownItemsAddIn(1);   // 每屏多显示1行
    CalcShownItems();

    SetAddinStyle(VSWS_NO_SCROLLBAR | VLCS_NO_SELECT);

    //======================================================

    m_TextColor.Set(colorArray[0]);
    m_HilightColor.Set(colorArray[1]);
    m_BgColor.Set(colorArray[2]);
    m_crossBrush1 = m_crossBrush2 = wxBrush(m_BgColor, wxSOLID);

    InsertColumn(VdkLcColumnInitializer(this).
                 percentage(100).
                 width(m_Rect.width).
                 textAlign(TEXT_ALIGN_CENTER_X_Y).
                 textColor(m_TextColor));

    //======================================================

    m_WindowImpl->Bind(wxEVT_SHOW, &OOPLyric::OnParentShow, this);
    wxTheApp->Bind(wxEVT_TIMER, &OOPLyric::OnTimerNotify, this, m_timerId);
}

OOPLyric::~OOPLyric() {
    wxTheApp->Unbind(wxEVT_TIMER, &OOPLyric::OnTimerNotify, this, m_timerId);
    m_WindowImpl->Unbind(wxEVT_SHOW, &OOPLyric::OnParentShow, this);

    //======================================================

    m_timer.Stop();
}

void OOPLyric::DoClear(wxDC *) {
    if (m_timer.IsRunning()) {
        Stop(NULL);    // VdkListCtrl::Clear() 会直接重画整个控件
    }
}

bool OOPLyric::IsOk() const {
    bool isParserOk = m_parser && m_parser->IsOk();

    // 必须提供用以同步的外部计时器
    return m_stopWatch && isParserOk && !VdkListCtrl::IsEmpty();
}

void OOPLyric::Start() {
    if (!IsOk()) {
        wxLogError(L"歌词控件尚未正确初始化。");
        return;
    }

    // 不能两次进入本函数
    if (TestState(OLST_STARTED)) {
        if (m_stopWatch->IsRunning()) {
            Resume();
        }

        return;
    }

    SetAddinState(OLST_STARTED);
    RemoveState(OLST_STOPPED | OLST_PAUSED);

    //======================================================

    UpdateProgress(NULL);
}

void OOPLyric::AttachParser(const OOPLyricParser &parser) {
    // 首先清空列表框中的内容
    Clear(NULL);

    m_parser = &parser;

    // 在这里初始化的确有点奇怪
    // 主要是我们用到了 m_parser，必须保证其有效性
    Init();

    //======================================================
    // 生成 VdkListCtrl 的各行

    LineIter i(m_parser->begin()), e(m_parser->end());
    for (int curr = 0; i != e; ++i, ++curr) {
        VdkLcCell *cell = Append((*i)->GetLyric(), false);
        cell->SetClientData((void *)(curr + 1));
    }

    InsertBlankLines();
}

void OOPLyric::UpdateProgress(wxDC *pDC) {
    wxASSERT(m_parser);

    double now = m_stopWatch->Time();
    GoTo(now / m_parser->GetTimeSum(), pDC, !m_stopWatch->IsRunning());
}

void OOPLyric::ClearLyric(wxDC *pDC) {
    Clear(pDC);
}

void OOPLyric::Stop(wxDC *pDC) {
    GoTo(0, pDC, true);
}

void OOPLyric::Pause() {
    SetAddinState(OLST_PAUSED);
    m_timer.Stop();
}

bool OOPLyric::Resume() {
    // 不能在停止状态下调用本函数
    wxASSERT(!TestState(OLST_STOPPED));

    // 必须先进入 StartLyric()
    if (!TestState(OLST_STARTED) || m_timer.IsRunning()) {
        return false;
    }

    RemoveState(OLST_PAUSED);

    //--------------------------------------------------------

    m_timer.Start(REFRESH_INTERVAL_MS);
    return true;
}

void OOPLyric::GoTo(double percentage, wxDC *pDC, bool bPaused) {
    wxASSERT(IsOk());

    if (percentage == 1) {
        MoveToEnd(pDC);
        return;
    }

    //==============================================

    m_currLine = m_parser->WhichLine(m_parser->GetTimeSum() * percentage);
    if (m_currLine == m_parser->end()) {
        wxLogDebug(L"Wrong position.");
        return;
    }

    //==============================================

    m_timer.Stop();
    RefreshLyric(pDC, bPaused);
}

void OOPLyric::MoveToEnd(wxDC *pDC) {
    // 停止歌词
    m_timer.Stop();
    SetAddinState(OLST_STOPPED);

    // 精准定位在最后可能位置
    int maxY;
    GetMaxViewStartCoord(NULL, &maxY);
    SetViewStart(0, maxY, pDC);
}

void OOPLyric::NextLine(wxDC *pDC, bool bPaused) {
    wxASSERT(m_parser);

#ifdef __WXDEBUG__
    {
        // 必须由外部保证不会越界
        LineIter next(m_currLine);
        ++next;
        wxASSERT(next != m_parser->end());
    }
#endif

    m_timer.Stop();

    ++m_currLine;

    //==============================================

    RefreshLyric(pDC, bPaused);
}

void OOPLyric::RefreshLyric(wxDC *pDC, bool bPaused) {
    CorrectViewStart(pDC);

    if (!bPaused && !m_timer.IsRunning()) {
        Resume();
    }
}

void OOPLyric::OnParentShow(wxShowEvent &e) {
    if (IsOk()) {
        if (!e.IsShown()) {
            m_timer.Stop();
        } else {
            UpdateProgress(NULL);
            Start();

            VdkDC dc(m_Window, GetAbsoluteRect(), NULL);
            Draw(dc);
        }
    }

    e.Skip(true);
}

void OOPLyric::OnTimerNotify(wxTimerEvent &) {
    wxASSERT(IsOk());

    if (m_Window->IsExiting()) {
        return;
    }

    if (m_stopWatch->Time() >= m_parser->GetTimeSum()) {
        m_timer.Stop();
        return;
    }

    //-----------------------------------------------------

    VdkDC dc(m_Window, GetAbsoluteRect(), NULL);
    LineIter next(m_currLine);
    ++next;

    if (next != m_parser->end()) {
        if ((*next)->GetStartTime() <= m_stopWatch->Time()) {
            NextLine(&dc, false);
            return;
        }
    }

    CorrectViewStart(&dc);
}

void OOPLyric::OnDraw(wxDC &dc) {
    // 即使没有歌词也要清空背景
    VdkListCtrl::OnDraw(dc);

    // 当没有歌词时，显示交互性文本
    if (!IsOk()) {
        wxString interactiveOutput(GetInteractiveOutput());

        if (!interactiveOutput.empty()) {
            int w, h;
            dc.SetFont(m_Font);
            dc.SetTextForeground(m_TextColor);

            dc.GetTextExtent(interactiveOutput, &w, &h);
            int xFix, yFix;
            xFix = (m_Rect.width - w) / 2;
            yFix = (m_Rect.height - h) / 2;

            dc.DrawText(interactiveOutput, xFix, yFix);
        }
    }
}

VdkCusdrawReturnFlag OOPLyric::DoDrawCellText(const VdkLcCell *cell,
        int col_index,
        int index0,
        wxDC &dc,
        VdkLcHilightState state) {
    wxASSERT(m_parser);

    // 注意：index 是不计算加入的空行的
    int index = (int)(cell->GetClientData()) - 1;
    if (index == -1) { // 此时 ClientData == NULL，是我们添加的空行
        return VCCDRF_DODEFAULT;
    }

    dc.SetTextForeground(m_TextColor);

    // 暂停时高亮当前行，情景见于用户正在拖动歌词。
    // 另外先暂停，然后拖动歌词完毕，此时假如歌词秀是以卡拉OK
    // 方式进行显示时，那么不会保持半高亮的状态，而是全高亮。
    if (TestState(OLST_PAUSED)) {
        int yStart;
        GetViewStartCoord(NULL, &yStart);

        int rowHeight = GetRowHeight();
        int dragRegion = yStart + rowHeight * m_blankLinesTop;
        int index2 = index + m_blankLinesTop;

        // 检测拖动歌词时中间线下面的一行
        if (rowHeight * index2 <= dragRegion &&
                rowHeight * (index2 + 1) > dragRegion) {
            m_draggHit = m_parser->GetLine(index);

            dc.SetTextForeground(m_HilightColor);
        }

        return VCCDRF_DODEFAULT;
    }

    LineInfo *currLine = *m_currLine;
    // TODO: 是否考虑优化？
    size_t currLineIndex = m_parser->IndexOf(m_currLine);
    int lineHasGone = m_stopWatch->Time() - currLine->GetStartTime();

    if ((index == currLineIndex - 1) && !cell->GetLabel().empty()) {
        // 使用渐变色还原上一句歌词
        if (lineHasGone < ALPHA_SHOW_LAST_LINE_MS) {
            unsigned char r, g, b;
            double alpha2 = double(lineHasGone) / ALPHA_SHOW_LAST_LINE_MS;
            double alpha1 = 1 - alpha2;

            r = m_HilightColor.Red() * alpha1 + m_TextColor.Red() * alpha2;
            g = m_HilightColor.Green() * alpha1 + m_TextColor.Green() * alpha2;
            b = m_HilightColor.Blue() * alpha1 + m_TextColor.Blue() * alpha2;

            dc.SetTextForeground(wxColour(r, g, b));
        }
    } else if (index == currLineIndex) { // 高亮当前文本行
        // 尽管这是一种很罕见的情况，但一旦出现了就会导致下面 (*) 表达式
        // 的除数为 0
        if (currLine->GetMilSeconds() == 0) {
            return VCCDRF_DODEFAULT;
        }

        if (!cell->IsEmpty()) {
            /* 经验教训：
            1. SetClippingRegiion 有叠加效应，因此在执行新的
               SetClippingRegiion 前别忘了销毁原来的 ClippingRegiion 。
            2. 关于表达式中整数与浮点数混用：注意中间运算结果会
               被强制转换成 int 然后参加下一步的运算，并不是对
               最终结果进行转换，使之成为一个浮点数。
            */

            const int rowHeight = GetRowHeight();
            int y = (currLineIndex + m_blankLinesTop) * rowHeight;
            cell->DrawLabel(dc, 0, y);

            // (*)
            double lineProgress = double(lineHasGone) / currLine->GetMilSeconds();

            // 要实现 KALA-OK 效果的文本宽度
            int w = (m_Rect.width - cell->GetX_Padding() * 2) * lineProgress;

            wxRect rc(GetAbsoluteRect());
            const int bottom = rc.y + rc.height;

            int yStart;
            VdkScrolledWindow::GetViewStartCoord(NULL, &yStart);
            rc.y += y - yStart;

            rc.width = cell->GetX_Padding() + w;
            rc.height = rowHeight;
            // 不能使 KALA-OK 效果的 ClippingRegion 超出列表窗口
            if ((rc.y + rc.height) > bottom) {
                rc.height = bottom - rc.y;
            }

            VdkDcDeviceOriginSaver saver(dc);
            dc.SetDeviceOrigin(0, 0);
            VdkDcClippingRegionDestroyer destroyer(dc, rc);

#       ifdef __WXGTK__
            dc.SetBrush(m_crossBrush1);
            dc.DrawRectangle(rc);
#       endif
            dc.SetTextForeground(m_HilightColor);

            cell->DrawLabel(dc, rc.x, rc.y);
        }

        return VCCDRF_SKIPDEFAULT;
    }

    return VCCDRF_DODEFAULT;
}

void OOPLyric::InsertBlankLines() {
    wxASSERT(IsOk());

    int shownItems = GetShownItems() - GetShownItemsAddin();
    int numBlankLinesTop = shownItems / 2;
    int numBlankLinesBottom = shownItems - numBlankLinesTop;

    int delta = m_blankLinesTop + m_blankLinesBottom -
                numBlankLinesTop - numBlankLinesBottom;

    if (delta > 0) {
        int numLinesToDelete = m_blankLinesTop - numBlankLinesTop;
        for (int i = 0; i < numLinesToDelete; i++) {
            RemoveRow(0, false, NULL);
        }

        numLinesToDelete = m_blankLinesBottom - numBlankLinesBottom;
        for (int i = 0; i < numLinesToDelete; i++) {
            RemoveRow(GetRowCount() - 1, false, NULL);
        }
    } else if (delta < 0) {
        wxString blankLine(wxEmptyString);

        int numNewLines = numBlankLinesTop - m_blankLinesTop;
        for (int i = 0; i < numNewLines; i++) {
            InsertRow(0, blankLine, false);
        }

        numNewLines = numBlankLinesBottom - m_blankLinesBottom;
        for (int i = 0; i < numNewLines; i++) {
            Append(blankLine, false);
        }
    }

    m_blankLinesTop = numBlankLinesTop;
    m_blankLinesBottom = numBlankLinesBottom;

    UpdateVirtualHeight();
}

void OOPLyric::UpdateVirtualHeight() {
    wxASSERT(m_parser);

    int rowHeight = GetRowHeight();
    int topBlankLinesHeight = m_blankLinesTop * rowHeight;
    int bottomBlankLinesHeight = m_Rect.height - topBlankLinesHeight;
    int actualHeight = m_parser->GetLinesCount() * rowHeight;

    int ySize = topBlankLinesHeight + actualHeight + bottomBlankLinesHeight;

    // -1 是为了得到一个总是不超出有效作用域的大小
    SetVirtualSize(0, ySize - 1);
}

double OOPLyric::GetLineProgress() const {
    LineInfo *currLine = *m_currLine;
    size_t lineHasGone = m_stopWatch->Time() - currLine->GetStartTime();
    if (lineHasGone >= currLine->GetMilSeconds()) {
        return 1;
    }

    // 假如除数为 0，不会来到这里
    return double(lineHasGone) / currLine->GetMilSeconds();
}

void OOPLyric::CorrectViewStart(wxDC *pDC) {
    wxASSERT(m_parser);

    // TODO: 添加 m_currIndex 吧
    int rowHeight = GetRowHeight();
    int yStart = rowHeight * double
                 (m_parser->IndexOf(m_currLine) + GetLineProgress());

    SetViewStart(0, yStart, pDC);
}

void OOPLyric::OnMouseEvent(VdkMouseEvent &e) {
    switch (e.evtCode) {
    case RIGHT_UP: {
        // 拖动歌词时不要响应右键事件
        if (TestState(OLST_DRAGGING)) {
            return;
        }

        wxPoint menuPos(AbsoluteRect().GetPosition());
        menuPos.x += e.mousePos.x;
        menuPos.y += e.mousePos.y;

        m_Window->ShowContextMenu(this, menuPos);

        break;
    }

    case LEFT_DOWN: {
        m_draggDistance = e.mousePos.y;
        SetAddinState(OLST_DRAGGING_STARTED);

        break;
    }

    case DRAGGING: {
        // 不接受先在一首歌的会话中拖动歌词，然后在未释放鼠标的情况下
        // 另一首歌开始播放，继续前面的拖动事件
        if (!TestState(OLST_DRAGGING_STARTED)) {
            break;
        }

        if (!IsOk()) {
            break;
        }

        if (m_timer.IsRunning()) {
            Pause();
            SetAddinState(OLST_RUNNING_BEFORE_DRAGGING);
        }

        SetAddinState(OLST_DRAGGING);

        //======================================================

        int ystart;
        GetViewStartCoord(NULL, &ystart);

        int dY = e.mousePos.y - m_draggDistance;
        m_draggDistance = e.mousePos.y;

        // 这是一行我们手工加上去的空行
        int rowHeight = GetRowHeight();
        int upperBound =
            (*(m_parser->begin()))->GetLyric().empty() ? rowHeight : 0;

        // 无法继续将帘布向上卷(再拖下去就到下一首了)
        // 我们将拖到尽头的事件视为无效
        bool lastLine = false;

        // 情景：将虚拟画布像窗帘一样向下拖
        // 拖动尽头了，不能再把窗帘哪怕拖下一寸
        if (ystart - dY < upperBound) {
            dY = ystart - upperBound; // 加加减减的原因参照(*)
        } else {
            int maxy;
            GetMaxViewStartCoord(NULL, &maxy);

            // 将帘布向上卷，卷到尽头了，再卷下去就会导致
            // 无法完整遮住窗口
            if (ystart - dY > maxy) {
                dY = ystart - maxy; // 加加减减的原因参照(*)
                // 无效拖动事件
                lastLine = true;
            }
        }

        if (dY) {
            SetViewStart(0, ystart - dY, &e.dc);   //………………(*)
        }

        // 无效拖动事件
        if (lastLine) {
            m_draggHit = m_parser->end();
        }

        //===================================================
        // 绘制中间线段

        wxRect rc(GetAbsoluteRect());
        int y = rc.y + m_blankLinesTop * GetRowHeight();

        m_Window->ResetDcOrigin(e.dc);
        e.dc.SetPen(wxPen(m_TextColor));
        e.dc.DrawLine(rc.x, y, rc.GetRight(), y);

        break;
    }

    case NORMAL:
    case LEFT_UP: {
        if (!TestState(OLST_DRAGGING) ||
            !TestState(OLST_DRAGGING_STARTED)) {
            break;
        }

        wxASSERT(IsOk());

        //-----------------------------------------------------

        RemoveState(OLST_DRAGGING | OLST_DRAGGING_STARTED);

        // 拖到最下面了，尽头
        if (m_draggHit == m_parser->end()) {
            RefreshState(&e.dc);
        } else {
            int ystart; // 起始绘图坐标
            GetViewStartCoord(NULL, &ystart);

            LineInfo *lineDraggHit = *m_draggHit;
            size_t timeToGo = lineDraggHit->GetStartTime();

            int rowHeight = GetRowHeight();
            double linePercentage = double(ystart % rowHeight) / rowHeight ;
            timeToGo += lineDraggHit->GetMilSeconds() * linePercentage;

            // 歌词可能并不匹配正在播放的歌曲
            if (timeToGo < m_parser->GetTimeSum()) {
                m_currLine = m_draggHit;

                if (IsReadyForEvent()) {
                    FireEvent(&e.dc, (void *) timeToGo);
                }
            }
        }

        bool resume = TestState(OLST_RUNNING_BEFORE_DRAGGING);
        if (resume) {
            RemoveState(OLST_RUNNING_BEFORE_DRAGGING);
            Resume();
        }

        break;
    }

    default:

        break;
    }
}

void OOPLyric::OnKeyEvent(VdkKeyEvent &vke) {
    // 全部跳过
    vke.Skip(true);
}

void OOPLyric::OnNotify(const VdkNotify &notice) {
    VdkListCtrl::OnNotify(notice);

    switch (notice.GetNotifyCode()) {
    case VCN_WINDOW_RESIZED:

        if (IsOk() && (notice.GetLparam() > 0)) {
            InsertBlankLines();
            UpdateSize(notice.GetVObjDC());
        }

        break;

    default:

        break;
    }
}

void OOPLyric::DoSetInteractiveOutput(wxDC *pDC) {
    if (!IsOk() && pDC) {
        VdkUtil::ClrBkGnd(*pDC, m_crossBrush1, AbsoluteRect());
        Draw(*pDC);
    }
}
