/***************************************************************
 * Name:      StickyWindows.cpp
 * Purpose:   WinAMP 风格粘附窗口实现
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2011-03-07
 * Copyright: Wang Xiaoning
 **************************************************************/
#include "StdAfx.h"
#include "StickyWindows.h"

#include "VdkWindow.h"
#include "VdkDefs.h" // for vdkSOUTH, vdkALIGN_TOP, etc.

#include "wxUtil.h" // for RightOf()、BottomOf()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT(wxEVT_STICKY_WINDOW_ATTACHED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_STICKY_WINDOW_DETACHED, wxCommandEvent);

StickyWindows::StickyWindows(int criticalDistance)
    : m_criticalDistance(criticalDistance) {

}

void StickyWindows::AddWindow(VdkWindow *win) {
#ifdef __WXGTK__
    VdkWindowList::iterator i;
    for (i = m_windows.begin(); i != m_windows.end(); ++i) {
        if (*i == win) {
            return;
        }
    }
#else
    if (m_windows.Member(win)) {
        return;
    }
#endif // __WXGTK__

    win->PushEventFilter(this);
    m_windows.push_back(win);
}

void StickyWindows::SetMainWindow(VdkWindow *win) {
    wxASSERT(m_main == m_windows.end());

    for (m_main = m_windows.begin();
         m_main != m_windows.end();
         ++m_main) {
        if (*m_main == win) {
            // 拦截左键按下事件还是不能少，否则无法在最开始移动主窗口之前就
            // 得知有哪些子窗口粘附到它，因为 VDK 提供的模拟移动状态在开始
            // 第一次移动之后才会有效
            win->GetWindowHandle()->Bind(wxEVT_LEFT_DOWN,
                                        &StickyWindows::OnMainLeftDown,
                                         this);

            win->GetWindowHandle()->Bind(wxEVT_MOVE,
                                        &StickyWindows::LinklyMove,
                                         this);

            break;
        }
    }
}

void StickyWindows::RemoveWindow(VdkWindow *win) {
    if (m_main != m_windows.end() && *m_main == win) {
        win->GetWindowHandle()->Unbind(wxEVT_LEFT_DOWN,
                                      &StickyWindows::OnMainLeftDown,
                                       this);

        win->GetWindowHandle()->Unbind(wxEVT_MOVE,
                                      &StickyWindows::LinklyMove,
                                       this);

        m_main = m_windows.end();
    }

    win->PopEventFilter(this, false);

    RemoveChild(win);
    m_windows.DeleteObject(win);
}

bool StickyWindows::FilterEvent(const EventForFiltering &e) {
    switch (e.evtCode()) {
    case DRAGGING: {
        VdkWindow *win = e.window();
        wxPoint mousePos(e.mouseEvent().GetPosition());

        // 主窗口移动事件的拦截由 wxWidgets 原生机制提供
        if (win != *m_main) {
            if (win->TestState(VWST_DRAG_AND_MOVING)) {
                wxASSERT(!win->TestState(VWST_DRAG_AND_RESIZING));

                return HandleChildMove(win, mousePos);
            }
        }

        if (win->TestState(VWST_DRAG_AND_RESIZING)) {
            wxASSERT(!win->TestState(VWST_DRAG_AND_MOVING));

            return HandleResize(win, mousePos);
        }

        break;
    }

    case LEFT_UP:

        m_stickyToMain.clear();

        break;

    default:

        break;
    }

    return false;
}

// rc1 和 rc2 是否相等或者一方包含着另一方
bool IsContains(const wxRect &rc1, const wxRect &rc2) {
    return (rc1 == rc2) || rc1.Contains(rc2) || rc2.Contains(rc1);
}

// 检测@a moving 是否可以粘附到 @a still 的某个方向上
// @param alignment 若不为空，可以获取两者边界对齐信息
unsigned FindDirection(const wxRect &moving, const wxRect &still,
                       unsigned *alignment,
                       int criticalDistance) {
    wxASSERT(criticalDistance >= 0);

    // 万分注意：不能对负数进行 |=
    unsigned direction = vdkDIRECTION_INVALID;

    // 查找左侧
    if (abs(RightOf(moving) - still.x) < criticalDistance) {
        direction |= vdkWEST;
    }

    // 查找右侧
    if (abs(moving.x - RightOf(still)) < criticalDistance) {
        direction |= vdkEAST;
    }

    // 查找上侧
    if (abs(BottomOf(moving) - still.y) < criticalDistance) {
        direction |= vdkNORTH;
    }

    // 查找下侧
    if (abs(moving.y - BottomOf(still)) < criticalDistance) {
        direction |= vdkSOUTH;
    }

    if (alignment) { // 边界对齐
        // 初始化
        *alignment = vdkALIGN_INVALID;

        if (abs(moving.x - still.x) < criticalDistance) {
            *alignment |= vdkALIGN_LEFT;
        }

        if (abs(RightOf(moving) - RightOf(still)) < criticalDistance) {
            *alignment |= vdkALIGN_RIGHT;
        }

        if (abs(moving.y - still.y) < criticalDistance) {
            *alignment |= vdkALIGN_TOP;
        }

        if (abs(BottomOf(moving) - BottomOf(still)) < criticalDistance) {
            *alignment |= vdkALIGN_BOTTOM;
        }
    }

    return direction;
}

bool StickyWindows::HandleResize(VdkWindow *win, const wxPoint &mousePosClient) {
    VdkWindow::MousePtrPos mousePtrPosOnEdge(win->GetDragAndResizeType());
    if (mousePtrPosOnEdge == 0) {
        return false;
    }

    wxWindow *winMoving = win->GetWindowHandle();
    wxPoint mousePosScreen(winMoving->ClientToScreen(mousePosClient));
    wxRect rcMovingUnamended(winMoving->GetScreenRect());
    // 已根据鼠标指针进行修正的窗口作用域
    wxRect rcMoving(rcMovingUnamended);

    // 根据鼠标指针在 VdkWindow 边缘的位置修正 rcMoving 的值，使之与指针同步
    if (mousePtrPosOnEdge & vdkWEST) {
        rcMoving.x = mousePosScreen.x;
        rcMoving.width = RightOf(rcMovingUnamended) - mousePosScreen.x;
    }

    if (mousePtrPosOnEdge & vdkEAST) {
        rcMoving.width = mousePosScreen.x - rcMovingUnamended.x;
    }

    if (mousePtrPosOnEdge & vdkNORTH) {
        rcMoving.y = mousePosScreen.y;
        rcMoving.height = BottomOf(rcMovingUnamended) - mousePosScreen.y;
    }

    if (mousePtrPosOnEdge & vdkSOUTH) {
        rcMoving.height = mousePosScreen.y - rcMovingUnamended.y;
    }

    int minWidth, minHeight;
    win->GetMinSize(&minWidth, &minHeight);

    if (rcMoving.width < minWidth || rcMoving.height < minHeight) {
        return false;
    }

    //-------------------------------------------------

    // 窗体新的大小
    wxRect rcNew(rcMoving);

    VdkWindowIter i(m_windows.begin());
    for (; i != m_windows.end(); ++i) {
        wxWindow *winStill = (*i)->GetWindowHandle();
        if (winMoving == winStill || !winStill->IsShown()) {
            continue;
        }

        wxRect rcStill(winStill->GetScreenRect());

        // 两者是包含关系，退出当次循环，避免破坏状态
        if (IsContains(rcMoving, rcStill)) {
            continue;
        }

        unsigned alignment = vdkALIGN_INVALID;
        unsigned direction =
            FindDirection(rcMoving, rcStill, &alignment, m_criticalDistance);

        if (direction != vdkDIRECTION_INVALID || alignment != vdkALIGN_INVALID) {
            if (mousePtrPosOnEdge & vdkWEST) {
                wxCoord x = rcNew.x;

                if (direction & vdkEAST) {
                    rcNew.x = RightOf(rcStill);
                } else if (alignment & vdkALIGN_LEFT) {
                    rcNew.x = rcStill.x;
                }

                // 必须保持窗口右边缘不变
                if (x != rcNew.x) {
                    rcNew.width = RightOf(rcMoving) - rcNew.x;
                }
            }

            if (mousePtrPosOnEdge & vdkEAST) {
                if (direction & vdkWEST) {
                    rcNew.width = rcStill.x - rcMoving.x;
                } else if (alignment & vdkALIGN_RIGHT) {
                    rcNew.width = RightOf(rcStill) - rcMoving.x;
                }

                // 这里没有高度/宽度改变量判断了，因为我们就是要改变它们
            }

            if (mousePtrPosOnEdge & vdkNORTH) {
                wxCoord y = rcNew.y;

                if (direction & vdkSOUTH) {
                    rcNew.y = BottomOf(rcStill);
                } else if (alignment & vdkALIGN_TOP) {
                    rcNew.y = rcStill.y;
                }

                // 必须保持窗口下边缘不变
                if (y != rcNew.y) {
                    rcNew.height = BottomOf(rcMoving) - rcNew.y;
                }
            }

            if (mousePtrPosOnEdge & vdkSOUTH) {
                if (direction & vdkNORTH) {
                    rcNew.height = rcStill.y - rcMoving.y;
                } else if (alignment & vdkALIGN_BOTTOM) {
                    rcNew.height = BottomOf(rcStill) - rcMoving.y;
                }
            }

        } // direction != vdkDIRECTION_INVALID

    }

    if (rcNew != rcMoving) {
        win->Resize(rcNew);
        // 这里没必要调用 UpdateStickyChildren() 更新粘附子窗口，
        // 因为每次在鼠标左键按下时都会进行更新

        return true;
    }

    return false;
}

// 看看是否可以粘附到其它窗口
bool StickyWindows::HandleChildMove(VdkWindow *win, const wxPoint &mousePosClient) {
    wxWindow *winMoving = win->GetWindowHandle();

    wxPoint mousePosScreen(winMoving->ClientToScreen(mousePosClient));
    // 拖动窗口时，若窗口跟着指针一起移动，那么指针应该看起来会是
    // 一直在窗体上的同一位置
    wxPoint mousePosClientFixed(win->GetMouseOnForm());

    // 拖动后新的位置，可能会粘附到新的窗口而出现修正
    wxRect rcNew;
    winMoving->GetSize(&rcNew.width, &rcNew.height);
    rcNew.x = mousePosScreen.x - mousePosClientFixed.x;
    rcNew.y = mousePosScreen.y - mousePosClientFixed.y;

    //=======================================================
    // 移动子窗口

    // 窗体粘附后新的位置
    wxRect rcSticked(rcNew);

    VdkWindowIter i(m_windows.begin());
    for (; i != m_windows.end(); ++i) {
        wxWindow *winStill = (*i)->GetWindowHandle();
        if (winMoving == winStill || !winStill->IsShown()) {
            continue;
        }

        wxRect rcStill(winStill->GetScreenRect());
        // 可能粘附到不止一个窗口
        TryStickToAgent(rcSticked, rcStill);

    } // ends fori

    if (rcSticked != rcNew) {
        winMoving->Move(rcSticked.x, rcSticked.y);

        wxRect rcMain((*m_main)->GetWindowHandle()->GetScreenRect());
        UpdateStickyToMainState(rcMain, win);

        return true;
    }

    return false;
}

// TODO: Member() ?
StickyWindows::InfoIter StickyWindows::FindStickyChild(VdkWindow *chd) {
    InfoIter i;
    for (i = m_stickyToMain.begin(); i != m_stickyToMain.end(); ++i) {
        if (i->win == chd) {
            return i;
        }
    }

    return i;
}

void PostDetachEvent(wxWindow *win) {
    wxWindowID windowId = win->GetId();
    wxCommandEvent e(wxEVT_STICKY_WINDOW_DETACHED, windowId);

    wxPostEvent(win, e);
}

void StickyWindows::UpdateStickyChildren(const wxRect &rcMain) {
    // 必须首先清空当前所有粘附子窗口，否则移动一个子窗口，使之从主窗口
    // 脱离后，任何通过它间接粘附到主窗口的子窗口仍然会保留与主窗口的粘附
    // 状态，这是不正确的。那么此后移动主窗口，使得刚刚从主窗口脱离的子
    // 窗口再次粘附，那么它会首先粘附到主窗口，这不假，但它在接下来的迭代
    // 查找过程会不正确地粘附到粘附状态已失效的那些子窗口上面去！

    InfoIter_Const it(m_stickyToMain.begin());
    for (; it != m_stickyToMain.end(); ++it) {
        PostDetachEvent(it->win->GetWindowHandle());
    }

    m_stickyToMain.clear();

    //========================================================

    size_t numStickyWindows = 0;

    do {
        numStickyWindows = m_stickyToMain.size();

        // 只计算尚未粘附的子窗口，因为迭代开始时所有子窗口都处于未粘附
        // 的状态，不会出现粘附失效需要更新(使之脱离)的情况
        VdkWindowIter i(m_windows.begin());
        for (; i != m_windows.end(); ++i) {
            wxWindow *winstill = (*i)->GetWindowHandle();

            if (!winstill->IsShown() || *m_main == *i ||
                    IsStickyToMain(*i)) {
                continue;
            }

            UpdateStickyToMainState(rcMain, *i);
        }
    } while (numStickyWindows != m_stickyToMain.size());
}

void StickyWindows::UpdateStickyToMainState(const wxRect &rcMain, VdkWindow *win) {
    wxWindow *winNative = win->GetWindowHandle();
    InfoIter This(FindStickyChild(win));

    // 计算与主窗口的粘附属性
    if (RecalcLinkageToMain(rcMain, win)) {
        wxCoord x, y, offsetX, offsetY;
        winNative->GetPosition(&x, &y);

        offsetX = x - rcMain.x;
        offsetY = y - rcMain.y;

        if (This == m_stickyToMain.end()) {
            StickyInfo info = { win, offsetX, offsetY };
            m_stickyToMain.push_back(info);

            wxWindowID windowId = winNative->GetId();
            wxCommandEvent e(wxEVT_STICKY_WINDOW_ATTACHED, windowId);
            wxPostEvent(winNative, e);
        } else {
            This->offsetX = offsetX;
            This->offsetY = offsetY;
        }
    } else {
        if (This != m_stickyToMain.end()) {
            m_stickyToMain.erase(This);
            PostDetachEvent(winNative);
        }
    }
}

bool StickyWindows::IsStickyToMain(VdkWindow *chd) {
    wxASSERT(chd != *m_main);

    return FindStickyChild(chd) != m_stickyToMain.end();
}

bool StickyWindows::TryStickToAgent(wxRect &rcThis, const wxRect &rcAgent) {
    unsigned align;
    unsigned d = FindDirection(rcThis, rcAgent, &align, m_criticalDistance);

    if (d & vdkEAST) {
        rcThis.x = RightOf(rcAgent);
    } else if (d & vdkWEST) {
        rcThis.x = rcAgent.x - rcThis.width;
    }

    if (d & vdkNORTH) {
        rcThis.y = rcAgent.y - rcThis.height;
    } else if (d & vdkSOUTH) {
        rcThis.y = BottomOf(rcAgent);
    }

    //---------------------------------------------
    // 边界对齐

    if (align & vdkALIGN_TOP) {
        rcThis.y = rcAgent.y;
    } else if (align & vdkALIGN_BOTTOM) {
        rcThis.y = BottomOf(rcAgent) - rcThis.height;
    }

    if (align & vdkALIGN_LEFT) {
        rcThis.x = rcAgent.x;
    } else if (align & vdkALIGN_RIGHT) {
        rcThis.x = RightOf(rcAgent) - rcThis.width;
    }

    return (d != vdkDIRECTION_INVALID) || (align != vdkALIGN_INVALID);
}

bool StickyWindows::RecalcLinkageToMain(const wxRect &rcMain, VdkWindow *chd) {
    wxASSERT(chd != *m_main);

    wxRect rcChild(chd->GetWindowHandle()->GetScreenRect());
    wxRect rcNew(rcChild);

    bool sticky = ReallySticky(rcNew, rcMain) &&
                  TryStickToAgent(rcNew, rcMain);

    if (!sticky) {
        // 查找其它已经粘附到主窗口的子窗口
        VdkWindowIter i(m_windows.begin());
        for (; i != m_windows.end(); ++i) {
            if (i == m_main || *i == chd ||
             !(*i)->GetWindowHandle()->IsShown()) {
                continue;
            }

            wxRect rcAgent((*i)->GetWindowHandle()->GetScreenRect());
            // 找一个已经粘附到主窗口的窗口来间接粘附到主窗口
            if (IsStickyToMain(*i) || ReallySticky(rcAgent, rcMain)) {
                sticky = ReallySticky(rcNew, rcAgent) &&
                         TryStickToAgent(rcNew, rcAgent);

                if (sticky) {
                    break;
                }
            }
        } // END for i
    }

    if (rcNew != rcChild) {
        chd->GetWindowHandle()->Move(rcNew.x, rcNew.y);
    }

    return sticky;
}

void StickyWindows::RemoveChild(VdkWindow *chd) {
    InfoIter i = FindStickyChild(chd);
    if (i != m_stickyToMain.end()) {
        m_stickyToMain.erase(i);
    }
}

bool StickyWindows::ReallySticky(const wxRect &rc1, const wxRect &rc2) {
    // 两者必须有所接触
    wxRect rcInflated(rc1);
    return rcInflated.Inflate(m_criticalDistance).Intersects(rc2);
}

void StickyWindows::OnMainLeftDown(wxMouseEvent &e) {
    // 更新粘附子窗口
    UpdateStickyChildren((*m_main)->GetWindowHandle()->GetScreenRect());

    e.Skip(true);
}

void StickyWindows::LinklyMove(wxMoveEvent &e) {
    wxASSERT(m_main != m_windows.end());

    //------------------------------------------------

    wxRect rcMain((*m_main)->GetWindowHandle()->GetScreenRect());

    if (!m_stickyToMain.empty()) {
        wxWindow *chd = NULL;
        wxCoord newX, newY;

        InfoIter i(m_stickyToMain.begin());
        for (; i != m_stickyToMain.end(); ++i) {
            chd = i->win->GetWindowHandle();
            if (!chd->IsShown()) {
                continue;
            }

            newX = rcMain.x + i->offsetX;
            newY = rcMain.y + i->offsetY;

            chd->Move(newX, newY);
        }
    }

    // 移动主窗口时，可能会有未黏附至主窗口的子窗口接近主窗口，
    // 因此在这里需要即时更新粘附列表
    UpdateStickyChildren(rcMain);

    e.Skip(true);
}
