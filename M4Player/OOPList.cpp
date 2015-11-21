/***************************************************************
 * Name:      OOPList.cpp
 * Purpose:   OOPList 实现文件
 * Author:    Ning (vanxining@139.com)
 * Created:   2010
 * Copyright: Wang Xiao Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPList.h"

#include "OOPApp.h"
#include "OOPSong.h"
#include "Playlist.h"

#include "VdkDC.h"
#include "wxUtil.h"

#include <wx/wfstream.h>
#include <wx/mstream.h> // for wxMemoryInputStream

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLONEABLE_VOBJECT(OOPList, VdkListCtrl);
extern OOPlayerApp *g_app;

enum {
    CNST_FILE_VERSION = 6, // 当前已保存播放列表文件格式的版本号
    // 播放列表文件保存字符串使用的单个字符大小
    // 所谓“单个字符”不一定指单个 Unicode 字符，也有可能是多个 char 组合成
    // 一个有意义的 Unicode 字符
#if wxUSE_UNICODE_WCHAR
    CNST_SIZE_OF_CHAR_T = sizeof(wchar_t),
#elif wxUSE_UNICODE_UTF8
    CNST_SIZE_OF_CHAR_T = sizeof(char),
#endif

    // 播放列表时间列的右侧内边距
    CNST_TIME_COLUMN_RIGHT_PADDINGS = 2,
};

//////////////////////////////////////////////////////////////////////////

OOPList::OOPList()
    : VdkListCtrl(VLCS_MULTI_SELECT) {

}

OOPList::~OOPList() {

}

void OOPList::Clone(OOPList *) {

}

void OOPList::OnXrcCreate(wxXmlNode *node) {
    VdkListCtrl::DoXrcCreate(node);
    SetRowHeight(15);

    //----------------------------------------------------------

    wxString strColor;
    wxColour color;

    // 序列号
    strColor = XmlGetChildContent(node, L"Color_Number");
    m_snColor.Set(strColor);

    // 曲目名的文本颜色
    strColor = XmlGetChildContent(node, L"Color_Text");
    m_textColor.Set(strColor);

    // 时间
    strColor = XmlGetChildContent(node, L"Color_Duration");
    m_timeColor.Set(strColor);

    // 交错颜色
    strColor = XmlGetChildContent(node, L"Color_Bkgnd");
    color.Set(strColor);
    m_crossBrush1.SetColour(color);

    strColor = XmlGetChildContent(node, L"Color_Bkgnd2");
    color.Set(strColor);
    m_crossBrush2.SetColour(color);

    // 正在播放的曲目与其他项目的区分
    strColor = XmlGetChildContent(node, L"Color_Hilight");
    m_lockedColor.Set(strColor);

    // 渐变色条(单击选中一个曲目后的加亮)
    strColor = XmlGetChildContent(node, L"Color_Select");
    m_selectedColor.Set(strColor);

    // 表示拖动项目目标位置的线
    unsigned char r = color.Red(),
                  g = color.Green(),
                  b = color.Blue();

    m_draggTarget.SetColour(wxColour(255 - r, 255 - g, 255 - b));

    //----------------------------------------------------------

    bool selectedBarInit = false;
    wxString strSelectedImage
    (GetFilePath(XmlGetChildContent(node, L"selected_image")));
    if (wxFileExists(strSelectedImage)) {
        // 不要为 stippleBitmap 创建掩码位图
        wxBitmap stippleBitmap(strSelectedImage, wxBITMAP_TYPE_BMP);
        if (stippleBitmap.IsOk()) {
            selectedBarInit = true;

            stippleBitmap = stippleBitmap.GetSubBitmap
                            (wxRect(0, 0, 1, stippleBitmap.GetHeight()));

            m_selected.SetStipple(stippleBitmap);
        }
    }

    if (!selectedBarInit) {
        CreateGradientBrush();
    }
}

void OOPList::OnNotify(const VdkNotify &notice) {
    VdkListCtrl::OnNotify(notice);

    if (notice.GetNotifyCode() == VCN_CREATE) {
        Playlist &playList = Playlist::GetInstance();
        Attach(&playList);

        //-----------------------------------------------

        int numRows = GetRowCount();

        wxString maxSN;
        maxSN.Printf(L"%d.", numRows);

        //-----------------------------------------------

        int width0, width1, width2;
        m_WindowImpl->GetTextExtent(maxSN, &width0, NULL, 0, 0, &m_Font);

        m_WindowImpl->GetTextExtent(L"99:99", &width2, NULL, 0, 0, &m_Font);
        width2 += CNST_TIME_COLUMN_RIGHT_PADDINGS;

        width1 = m_Rect.width - width0 - width2;

        //-----------------------------------------------

        // 换肤后以及克隆得到的列表不要重复插入列
        if (GetColumnCount() == 0) {
            // 插入序号列
            InsertColumn(VdkLcColumnInitializer(this).
                         width(width0).
                         textAlign(TEXT_ALIGN_RIGHT).
                         textColor(m_snColor));

            // 插入标题列
            InsertColumn(VdkLcColumnInitializer(this).
                         index(1).
                         percentage(100).
                         width(width1).
                         textColor(m_textColor));

            // 插入歌曲时间列
            InsertColumn(VdkLcColumnInitializer(this).
                         width(width2).
                         textColor(m_timeColor));
        } else {
            LcColIter it(playList.cols.begin());
            VdkLcColumn *colSN = *it;
            VdkLcColumn *colTitle = *(++it);
            VdkLcColumn *colDuration = *(++it);

            // 序号列
            colSN->x = 0;
            colTitle->x = width0;
            colDuration->x = width0 + width1;

            colSN->UpdateWidth(width0);
            colTitle->UpdateWidth(width1);
            colDuration->UpdateWidth(width2);

            colSN->textColor = m_snColor;
            colTitle->textColor = m_textColor;
            colDuration->textColor = m_timeColor;

            colSN->font = m_Font;
            colTitle->font = m_Font;
            colDuration->font = m_Font;
        }

        m_locked = end();
    }
}

void OOPList::UpdateSnColumnWidth(int numRowsOld, int numRowsCurr) {
    if (numRowsCurr == wxNOT_FOUND) {
        numRowsCurr = GetRowCount();
    }

    wxString sn, sn0;

    sn0.Printf(L"%d.", numRowsOld);
    sn.Printf(L"%d.", numRowsCurr);

    if (sn.length() > sn0.length()) {
        int w;
        m_WindowImpl->GetTextExtent(sn, &w, NULL, NULL, NULL, &m_Font);

        AjustCollumn(w, 0, 1);
    }
}

void OOPList::AddToList(OOPSong &song, size_t numRows) {
    wxString title(song.BuildTitle(g_app->GetPlayListTitleFormat()));
    AddToList(*(song.GetListEntry()), title, numRows);
}

void OOPList::AddToList(OOPListEntry &entry, const wxString &title, size_t numRows) {
    VdkLcCell *head = Append(wxEmptyString, false);
    VdkLcCell *midCell = (VdkLcCell *) head->GetClientData();

    midCell->SetLabel(title);
    UpdateTime((VdkLcCell *) midCell->GetClientData(), entry.length());
    // 注意上面用到了 midCell 的 ClientData
    midCell->SetClientData(&entry);

    entry.addTime(wxGetLocalTime());
}

void OOPList::UpdateTime(int row, unsigned int len) {
    SetCellLabel(row, 2, OOPSong::GetStdTimeStr(len));
}

void OOPList::UpdateTime(VdkLcCell *cell, unsigned int len) {
    cell->SetLabel(OOPSong::GetStdTimeStr(len));
}

void OOPList::CreateGradientBrush() {
    wxMemoryDC memDC;
    wxBitmap stippleBitmap(1, GetRowHeight());
    wxRect rc(0, 0, 1, GetRowHeight());

    memDC.SelectObject(stippleBitmap);
    memDC.GradientFillLinear(rc,
                             m_selectedColor,
                             m_crossBrush1.GetColour(),
                             wxSOUTH);
    memDC.SelectObject(wxNullBitmap);

    m_selected.SetStipple(stippleBitmap);
}

VdkCusdrawReturnFlag OOPList::DoDrawCellText(const VdkLcCell *cell,
        int col_index,
        int index,
        wxDC &dc,
        VdkLcHilightState state) {
    switch (state) {
    case VDKLC_HS_SELECTED:

        dc.SetTextForeground(GetSelectedColour());
        break;

    case VDKLC_HS_NORMAL: {
        if (m_locked != end()) {
            if (cell == *m_locked) {
                dc.SetTextForeground(m_lockedColor);
            }
        }

        break;
    }

    default:

        break;
    }

    // 动态绘制序号列
    VdkLcColumn *col = cell->GetColumn();
    int y = index * GetRowHeight();
    y += (col->height - dc.GetCharHeight()) / 2;

    switch (col_index) {
    case 0: {
        int w, tens = -1;
        wxString sn;
        sn.Printf(L"%d.", index + 1);

        if (tens < index / 10) {
            tens = index / 10;
            dc.GetTextExtent(sn, &w, NULL);
        }

        int x = col->x + (col->width - w);
        dc.DrawText(sn, x, y);

        return VCCDRF_SKIPDEFAULT;
    }

    default:
        break;
    }

    return VCCDRF_DODEFAULT;
}

VdkCusdrawReturnFlag OOPList::DoEraseRow
(const LcCellIter &it, int index, wxDC &dc) {
    LcCellIter lastSel = GetLastSel();
    if (it == lastSel) {
        dc.SetPen(wxPen(GetSelectedColour()));
    }

    return VCCDRF_DODEFAULT;
}

inline wxColour OOPList::GetSelectedColour() const {
    return (m_selectedColor != *wxWHITE) ? *wxWHITE : m_lockedColor;
}

void OOPList::RedrawRows(const wxArrayInt &rows, wxDC &dc) {
    bool atLeastOne = false;
    int count = rows.size();
    for (int i = 0; i < count; i++) {
        if (rows[i] != wxNOT_FOUND) {
            atLeastOne = true;
            break;
        }
    }

    if (!atLeastOne) {
        return;
    }

    VdkDcDeviceOriginSaver saver(dc);
    PrepareDC(dc);

    for (int i = 0; i < count; i++) {
        if (rows[i] != wxNOT_FOUND) {
            UpdateRow(rows[i], dc);
        }
    }
}

void OOPList::OnMouseEvent(VdkMouseEvent &e) {
    if (IsEmpty()) {
        return;
    }

    // 撤销所画代表最后选中的白框
    LcCellIter lastSelOld = GetLastSel();

    VdkListCtrl::OnMouseEvent(e);

    // 重新设置当前 DC 绘图状态(设备坐标和裁剪区域)，后面紧跟的两次 UpdateRow()
    // 调用要求 DC 是已经按照滚动窗口的当前可视区域适配过了的。
    PrepareDC(e.dc);

    UpdateRow(GetLastSel(), e.dc);
    UpdateRow(lastSelOld, e.dc);
}

void OOPList::OnKeyEvent(VdkKeyEvent &e) {
    if (e.evtCode != KEY_DOWN) {
        return;
    }

    int lastSelOld = GetLastSelIndex();
    VdkListCtrl::OnKeyEvent(e);

    //----------------------------------------------

    wxArrayInt rows;
    rows.Add(GetLastSelIndex());
    rows.Add(lastSelOld);

    RedrawRows(rows, e.dc);
}

void OOPList::SetLocked(int index, wxDC *pDC) {
    LcCellIter locked(GetCellIterator(1, index));
    SetLocked(locked, pDC);
}

void OOPList::SetLocked(const LcCellIter &locked, wxDC *pDC) {
    LcCellIter old = m_locked;
    m_locked = locked;

    int index = wxNOT_FOUND;
    if (m_locked != end()) {
        index = IndexOf(*m_locked);

        int ystart;
        GetViewStart(NULL, &ystart);
        int lastVisuable = ystart + GetShownItems() - 1;

        if (index == lastVisuable) { // 当前屏的最后一个，下滑一行
            SetViewStart(0, ++ystart, pDC);
        } else if (index == ystart - 1) { // 当前屏的第一个，上滑一行
            SetViewStart(0, --ystart, pDC);
        } else if ((index < ystart) || (index > lastVisuable)) { // 不在可视范围内
            SetViewStart(0, index, pDC);
        }
    }

    if (pDC) {
        VdkDcDeviceOriginSaver saver(*pDC);
        PrepareDC(*pDC);

        if (index != wxNOT_FOUND) {
            UpdateRow(index, *pDC);
        }

        if (old != end()) {
            int oldindex = IndexOf(*old);
            if (oldindex != wxNOT_FOUND) {
                UpdateRow(oldindex, *pDC);
            }
        }

        RestoreDC(*pDC);
    }
}

OOPSongPtr OOPList::GetSongPtr(int index) {
    return GetSongPtr(GetCellIterator(1, index));
}

OOPSongPtr OOPList::GetSongPtr(const LcCellIter &i) {
    wxASSERT(i != end());
    return OOPSongPtr(new OOPSong(GetListEntry(i)));
}

OOPListEntry *OOPList::GetListEntry(const LcCellIter &i) {
    wxASSERT(i != end());
    return (OOPListEntry *)(*i)->GetClientData();
}

void OOPList::DoClear(wxDC *pDC) {
    RemoveAllSongPtrs();
}

void OOPList::RemoveSongPointer(const LcCellIter &i) {
    wxASSERT(i != end());
    wxASSERT(i != m_locked);

    VdkLcCell *cell = *i;
    delete(OOPListEntry *) cell->GetClientData();

    cell->SetClientData(NULL);
}

void OOPList::RemoveSongPointer(int i) {
    RemoveSongPointer(GetCellIterator(1, i));
}

void OOPList::RemoveAllSongPtrs() {
    // 必须保证序号不能使被锁住的行
    wxASSERT(!HasLocked());

    LcCellIter i(begin()), e(end());
    for (; i != e; ++i) {
        RemoveSongPointer(i);
    }
}

void OOPList::RemoveSong(int index) {
    // 必须保证序号不能使被锁住的行
    wxASSERT(index != GetLockedIndex());

    int count0 = GetRowCount(), count;

    RemoveSongPointer(index);
    RemoveRow(index, false, NULL);

    count = count0 - 1;
    count0 = GetNumBitCount(count0) + 1;
    count = GetNumBitCount(count) + 1;

    // 缩小序号列
    if (count < count0) {
        AjustCollumn(m_WindowImpl->GetCharWidth() * count, 0, 1);
    }
}

void OOPList::RemoveSong(const LcCellIter &i) {
    RemoveSong(IndexOf(*i));
}

int OOPList::GetLockedIndex() {
    return (m_locked != end()) ? IndexOf(*m_locked) : wxNOT_FOUND;
}

OOPSongPtr OOPList::GetLockedSong() {
    return HasLocked() ? GetSongPtr(m_locked) : OOPSongPtr(NULL);
}

void OOPList::Serialize(wxInputStream &fin) {
    wxASSERT(fin.IsOk());

    wxMemoryInputStream meminp(fin);

    // 读入版本号
    int version = ReadVar<int>(meminp);
    if (version != CNST_FILE_VERSION) {
        wxLogDebug(L"列表文件的版本不符。\n文件版本: %d，"
                   L"程序支持的版本: %d",
                   version, (int) CNST_FILE_VERSION);

        return;
    }

    // 读入这个列表使用的宽字符的大小
    int szOfWch = ReadVar<int>(meminp);
    if (szOfWch != CNST_SIZE_OF_CHAR_T) {
        wxLogDebug(L"列表文件的不支持跨平台使用。\n"
                   L"创建文件的时使用的宽字符大小为 %d 字节，"
                   L"当前程序支持的宽字符大小为 %d 字节。",
                   szOfWch, (int) CNST_SIZE_OF_CHAR_T);
        return;
    }

    // 读入歌曲总数
    int count = ReadVar<int>(meminp);

    //----------------------------------------------------------

    int len;
    wxString title, songPath;
    OOPListEntry *entry;

    // 循环读入歌曲列表
    for (int i = 0; i < count; i++) {
        try {
            // 读入列表标题
            title = ReadString(meminp);
            // 读入歌曲长度
            len = ReadVar<int>(meminp);
            // 读入歌曲路径
            songPath = ReadString(meminp);
        } catch (serialize_bad_format) {
            wxLogDebug(L"播放列表格式不正确。");
            break;
        }

        entry = new OOPListEntry(songPath);

        //======================================================

        entry->length(len);
        // 读入歌曲播放次数
        entry->playCount(ReadVar<int>(meminp));
        // 读入歌曲加入播放列表的时间
        entry->addTime(ReadVar<long>(meminp));

        AddToList(*entry, title, i);
    }

    UpdateSnColumnWidth(0, count);
    UpdateSize(NULL);
}

void OOPList::Serialize(wxOutputStream &fout) {
    wxASSERT(fout.IsOk());

    // 写入版本号
    WriteVar(fout, (int) CNST_FILE_VERSION);
    // 写入创建这个列表的操作系统
    WriteVar(fout, (int) CNST_SIZE_OF_CHAR_T);
    // 写入歌曲总数
    WriteVar(fout, GetRowCount());

    //----------------------------------------------------------

    wxString title;

    LcCellIter e(end());
    for (LcCellIter i(begin()); i != e; ++i) {
        VdkLcCell *cell = *i;
        OOPListEntry *entry = (OOPListEntry *) cell->GetClientData();
        title = cell->GetLabel();

        // 写入列表项标题
        WriteString(fout, title);
        // 写入歌曲长度
        WriteVar(fout, entry->length());
        // 写入歌曲路径
        WriteString(fout, entry->path());

        // 写入歌曲播放次数
        WriteVar(fout, entry->playCount());
        // 写入歌曲加入播放列表的时间
        WriteVar(fout, entry->addTime());
    }
}

//////////////////////////////////////////////////////////////////////////

bool CompareTitle(const VdkLcCell *c1, const VdkLcCell *c2) {
    return CompareUnicodeStrings(c1->GetLabel(), c2->GetLabel()) < 0;
}

bool ComparePath(const VdkLcCell *c1, const VdkLcCell *c2) {
    OOPListEntry *e1 = (OOPListEntry *) c1->GetClientData(),
                  *e2 = (OOPListEntry *) c2->GetClientData();

    return CompareUnicodeStrings(e1->path(), e2->path()) < 0;
}

bool CompareAlbum(const VdkLcCell *c1, const VdkLcCell *c2) {
    OOPListEntry *e1 = (OOPListEntry *) c1->GetClientData(),
                  *e2 = (OOPListEntry *) c2->GetClientData();

    OOPSong s1(e1);
    OOPSong s2(e2);

    s1.ParseTag();
    s2.ParseTag();

    return CompareUnicodeStrings(s1.album(), s2.album()) < 0;
}

bool CompareLength(const VdkLcCell *c1, const VdkLcCell *c2) {
    OOPListEntry *e1 = (OOPListEntry *) c1->GetClientData(),
                  *e2 = (OOPListEntry *) c2->GetClientData();

    return e1->length() < e2->length();
}

bool CompareAddTime(const VdkLcCell *c1, const VdkLcCell *c2) {
    OOPListEntry *e1 = (OOPListEntry *) c1->GetClientData(),
                  *e2 = (OOPListEntry *) c2->GetClientData();

    return e1->addTime() < e2->addTime();
}

bool ComparePlayCount(const VdkLcCell *c1, const VdkLcCell *c2) {
    OOPListEntry *e1 = (OOPListEntry *) c1->GetClientData(),
                  *e2 = (OOPListEntry *) c2->GetClientData();

    return e1->playCount() < e2->playCount();
}

void OOPList::SortList(VdkCtrlId cmd) {
    // wxList 和 std::list 至少有两点不同：
    // ① 前者不支持 splice()
    // ② 前者调用 sort() 后迭代器全部指向不同的值(另一种意义上的失效)

    switch (cmd) {
    case OTBM_SORT_BY_TITLE:

        Sort(1, CompareTitle);
        break;

    case OTBM_SORT_BY_ALBUM:

        Sort(1, CompareAlbum);
        break;

    case OTBM_SORT_BY_PATH:

        Sort(1, ComparePath);
        break;

    case OTBM_SORT_BY_LENGTH:

        Sort(1, CompareLength);
        break;

    case OTBM_SORT_BY_ADD_TIME:

        Sort(1, CompareAddTime);
        break;

    case OTBM_SORT_BY_PLAY_COUNT:

        Sort(1, ComparePlayCount);
        break;

    default:

        break;
    }

    //------------------------------------------------------------

    // 更新时间列的显示、锁定项
    int row = 0;
    LcCellIter i1(begin());
    LcCellIter i2(GetCellIterator(2, 0));
    LcCellIter e(end());

    for (; i1 != e; ++i2, ++i1, ++row) {
        OOPListEntry *entry = (OOPListEntry *)(*i1)->GetClientData();
        UpdateTime(*i2, entry->length());
    }

    m_Window->QueueRedrawEvent();
}

LcCellIter OOPList::begin() {
    return GetCellIterator(1, 0);
}

LcCellIter OOPList::end() {
    return GetColumnEnd(1);
}
