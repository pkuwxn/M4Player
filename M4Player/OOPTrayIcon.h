/***************************************************************
 * Name:      OOPTrayIcon.cpp
 * Purpose:   OOPlayer ������ͼ��
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-16
 **************************************************************/
#pragma once
#include <wx/taskbar.h>

/// OOPlayer ������ͼ��
class OOPTrayIcon : public wxTaskBarIcon
{
public:

private:
    
    // �����˵�
    virtual wxMenu* CreatePopupMenu();
    // ��Ӧ�˵��¼�
    void OnMenu(wxCommandEvent& e);
    // ��Ӧ������ͼ���ϰ�������Ҽ��¼�
	void OnRightDown(wxTaskBarIconEvent&);
    
private:

    DECLARE_EVENT_TABLE()
};
