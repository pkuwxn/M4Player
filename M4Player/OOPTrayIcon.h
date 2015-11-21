/***************************************************************
 * Name:      OOPTrayIcon.cpp
 * Purpose:   OOPlayer 的托盘图标
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-16
 **************************************************************/
#pragma once
#include <wx/taskbar.h>

/// OOPlayer 的托盘图标
class OOPTrayIcon : public wxTaskBarIcon {
public:

private:

    // 创建菜单
    virtual wxMenu *CreatePopupMenu();
    // 响应菜单事件
    void OnMenu(wxCommandEvent &e);
    // 响应在托盘图标上按下鼠标右键事件
    void OnRightDown(wxTaskBarIconEvent &);

private:

    DECLARE_EVENT_TABLE()
};
