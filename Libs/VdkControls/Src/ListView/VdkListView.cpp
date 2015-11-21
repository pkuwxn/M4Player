/***************************************************************
 * Name:      VdkListView.cpp
 * Purpose:   类 Android 的列表控件，能动态获取数据
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-03
 **************************************************************/
#include "StdAfx.h"
#include "ListView/VdkListView.h"
#include "ListView/IListViewPlaugin.h"

#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT(VdkListView)

//////////////////////////////////////////////////////////////////////////

VdkListView::VdkListView(long style)
    : VdkScrolledWindow(style),
      //------------------Style------------------------
      m_background(*wxWHITE),
      //-----------------------------------------------
      m_adapter(NULL),
      m_held(false),
      m_rowHeight(20),
      m_shownItems(0),
      m_nShownItemsAddIn(0) {

}

VdkListView::~VdkListView() {
    DestroyAllPlugins();

    if (m_held) {
        wxDELETE(m_adapter);
    }
}

void VdkListView::OnXrcCreate(wxXmlNode *node) {
    Create(m_Window, GetXrcName(node), GetXrcRect(node));
    m_align = GetXrcAlign(node);
}

void VdkListView::Create
(VdkWindow *parent, const wxString &strName, const wxRect &rc) {
    m_strName = strName;
    m_Rect = rc;
    SetVdkWindow(parent);

    //--------------------------------------------------

    // TODO:
    CalcShownItems();
}

void VdkListView::SetAdapter(Adapter *adapter, AdapterLifeTimePolicy altp) {
    wxASSERT(adapter);

    m_adapter = adapter;
    m_held = altp == ALTP_HELD;

    UpdateSize(NULL);
}

void VdkListView::UpdateSize(wxDC *pDC) {
    SetVirtualSize(0, m_rowHeight * m_adapter->GetRowCount(), pDC);
}

void VdkListView::SetRowHeight(int height, wxDC *pDC) {
    if (m_rowHeight == height) {
        return;
    }

    int ch = m_WindowImpl->GetCharHeight();
    if (ch > height) {
        height = ch;
    }

    m_rowHeight = height;

    //==============================================

    //LcColIter i;
    //for( i = m_cols.begin(); i != m_cols.end(); ++i )
    //{
    //  (*i)->height = m_rowHeight;
    //}

    SetScrollRate(20, m_rowHeight);
    //UpdateSize();
    CalcShownItems();

    if (pDC) {
        Draw(*pDC);
    }
}

void VdkListView::CalcShownItems() {
    float shownItems(float(m_Rect.height) / m_rowHeight);
    m_shownItems = shownItems;
    if (shownItems > m_shownItems) {
        m_shownItems++;
    }

    m_shownItems += m_nShownItemsAddIn;
}

void VdkListView::OnNotify(const VdkNotify &notice) {
    switch (notice.GetNotifyCode()) {
    case VCN_SIZE_CHANGED:
    case VCN_WINDOW_RESIZED:

        CalcShownItems();

        break;

    default:

        break;
    }
}

enum {
    X_PADDINGS = 5,
};

/*static*/
size_t VdkListView::GetCellLeftPadding() {
    return X_PADDINGS;
}

void VdkListView::OnDraw(wxDC &dc) {
    if (!m_adapter) {
        return;
    }

    int viewStartY, maxViewStartY;
    GetViewStart(NULL, &viewStartY);
    GetMaxViewStart(NULL, &maxViewStartY);
    if (viewStartY > maxViewStartY) {
        viewStartY = maxViewStartY;
    }

    int numRows = m_adapter->GetRowCount();

    // 行数不足一屏时清空底部已画内容
    if (viewStartY == maxViewStartY) {
        int lh, yblank; // 空白区域的高度
        GetVirtualSize(NULL, &lh);
        yblank = m_Rect.height - (numRows - viewStartY) * GetRowHeight();

        wxRect rc(0, lh, m_Rect.width, yblank);
        dc.SetBrush(m_background);
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(rc);
    }

    int to = viewStartY + m_shownItems;
    if (to > numRows) {
        to = numRows;
    }

    //-----------------------------------------------

    for (int row = viewStartY; row < to; row++) {
        EraseRow(row, dc);
    }

    dc.SetFont(m_Font);
    dc.SetTextForeground(*wxBLACK);

    int x = 0, y;
    int numCols = m_adapter->GetColumnCount();
    for (int col = 0; col < numCols; col++) {
        Adapter::ColumnStyle &cs = m_adapter->GetColumnStyle(col);
        for (int row = viewStartY; row < to; row++) {
            y = m_rowHeight * row;
            DrawCellText(dc, row, col, x, y);
        }

        x += cs.width;
    }
}

void VdkListView::EraseRow(int row, wxDC &dc) {
    dc.SetBrush(m_background);
    dc.SetPen(*wxTRANSPARENT_PEN);

    // TODO:
    wxVector<PluginPtr>::iterator plugin_iter(m_plugins.begin());
    for (; plugin_iter != m_plugins.end(); ++plugin_iter) {
        (*plugin_iter)->OnEraseRow(row, dc);
    }

    if (DoEraseRow(row, dc) == VCCDRF_DODEFAULT) {
        dc.DrawRectangle(0, m_rowHeight * row, m_Rect.width, m_rowHeight);
    }
}

void VdkListView::UpdateRow(int row, wxDC &dc) {
    // TODO:
    EraseRow(row, dc);
    DrawRowText(row, dc);
}

void VdkListView::DrawRowText(int row, wxDC &dc) {
    dc.SetFont(m_Font);
    dc.SetTextForeground(*wxBLACK);

    int x = 0, y = m_rowHeight * row;
    int numCols = m_adapter->GetColumnCount();
    for (int col = 0; col < numCols; col++) {
        DrawCellText(dc, row, col, x, y);
        x += m_adapter->GetColumnStyle(col).width;
    }
}

void VdkListView::DrawCellText(wxDC &dc, size_t row, size_t col, int x, int y) {
    wxString text(m_adapter->GetCell(row, col));

    int xFix = CalTextXAxisProp(dc, col, text);
    int yFix = (m_rowHeight - dc.GetCharHeight()) / 2;

    dc.DrawText(text, x + X_PADDINGS + xFix, y + yFix);
}

int VdkListView::CalTextXAxisProp(wxDC &dc, size_t col, wxString &text) {
    Adapter::ColumnStyle &cs = m_adapter->GetColumnStyle(col);

    //=====================================================

    int xFix = 0;
    int maxWidth = cs.width - X_PADDINGS;
    int w;

    dc.SetFont(m_Font);
    dc.GetTextExtent(text, &w, NULL);

    if (w > maxWidth) {
        wxArrayInt widths;
        dc.GetPartialTextExtents(text, widths);

        int dotsWidth;
        dc.GetTextExtent(L"...", &dotsWidth, NULL);

        int i(0), suitable(maxWidth - dotsWidth);
        while (widths[i] <= suitable) {
            ++i;
        }
        --i;

        text.erase(i, wxString::npos);
        text += L"...";

        xFix = 0;
    } else if (cs.textAlign & TEXT_ALIGN_CENTER_X_Y) {
        xFix = (maxWidth - w) / 2;
    } else if (cs.textAlign & TEXT_ALIGN_RIGHT) {
        xFix = maxWidth - w;
    }

    return xFix;
}

bool VdkListView::GetIndex(int y, int *rowAtPointer) {
    int numRows(m_adapter->GetRowCount());
    if (numRows == 0) {
        return true;
    }

    int base; // 正在显示的第一行
    GetViewStart(NULL, &base);

    int row = base + y / m_rowHeight;
    bool outOfRange = false;

    if (row >= numRows) {
        row = wxNOT_FOUND;
        outOfRange = true;
    } else if (rowAtPointer) {
        if (row < base) {
            row = base;
        } else if (row >= base + m_shownItems) {
            row = base + m_shownItems - 1;
        }
    }

    if (rowAtPointer) {
        *rowAtPointer = row;
    }

    return outOfRange;
}

void VdkListView::OnMouseEvent(VdkMouseEvent &e) {
    int numRows(m_adapter->GetRowCount());
    if (numRows == 0) {
        return;
    }

    int rowAtPointer;
    bool outOfRange = GetIndex(e.mousePos.y, &rowAtPointer);

    //-----------------------------------------

    // TODO:
    typedef void (IListViewPlaugin::*StdMouseEvtHandler)(int, VdkMouseEvent &);
    StdMouseEvtHandler handler = NULL;

    switch (e.evtCode) {
    case LEFT_DOWN:

        handler = &IListViewPlaugin::OnClickDown;
        break;

    case LEFT_UP:

        handler = &IListViewPlaugin::OnClickUp;
        break;

    case DLEFT_DOWN:

        handler = &IListViewPlaugin::OnDClick;
        break;

    case DRAGGING:

        handler = &IListViewPlaugin::OnDragg;
        break;

    default:

        break;
    }

    if (handler) {
        wxVector<PluginPtr>::iterator plugin_iter(m_plugins.begin());
        for (; plugin_iter != m_plugins.end(); ++plugin_iter) {
            ((*plugin_iter)->*handler)(rowAtPointer, e);
        }
    }
}

void VdkListView::OnKeyEvent(VdkKeyEvent &ke) {
    wxVector<PluginPtr>::iterator plugin_iter(m_plugins.begin());
    for (; plugin_iter != m_plugins.end(); ++plugin_iter) {
        (*plugin_iter)->OnKey(ke);
    }
}

void VdkListView::AddPlugin(PluginPtr plugin) {
    if (SearchContainer(m_plugins.begin(), m_plugins.end(), plugin)
            == m_plugins.end()) {
        m_plugins.push_back(plugin);
    }
}

void VdkListView::DestroyAllPlugins() {
    wxVector<PluginPtr>::const_iterator i(m_plugins.begin());
    for (; i != m_plugins.end(); ++i) {
        if ((*i)->CanDestoryByListView()) {
            delete *i;
        }
    }

    m_plugins.clear();
}

void VdkListView::NotifyItemAdd(size_t row) {
    OnDataSetChange(DSCA_ADD, row);
}

void VdkListView::NotifyItemRemove(size_t row) {
    OnDataSetChange(DSCA_REMOVE, row);
}

void VdkListView::NotifyClear() {
    OnDataSetChange(DSCA_CLEAR, wxNOT_FOUND);
}

void VdkListView::OnDataSetChange(DataSetChangeAction dsca, size_t row) {
    wxVector<PluginPtr>::const_iterator i(m_plugins.begin());
    for (; i != m_plugins.end(); ++i) {
        switch (dsca) {
        case DSCA_ADD:

            (*i)->OnItemAdd(row);
            break;

        case DSCA_REMOVE:

            (*i)->OnItemRemove(row);
            break;

        case DSCA_CLEAR:

            (*i)->OnClear();
            break;

        default:

            return;
        }
    }
}
