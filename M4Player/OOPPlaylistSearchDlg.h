/***************************************************************
 * Name:      OOPPlaylistSearchDlg.cpp
 * Purpose:   “快速定位文件”对话框
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-11
 **************************************************************/
#pragma once
#include "VdkWindowImpl.h"

class VdkEdit;

/// 进度显示对话框
class OOPPlaylistSearchDlg : public VdkDialog
{
public:

	/// 构造函数
	OOPPlaylistSearchDlg(VdkWindow* parent);

	/// 析构函数
	~OOPPlaylistSearchDlg();

	/// 获取用户输入的查找关键字
	wxString GetKeyword() const;

private:

	// 原生关闭事件处理器
	void OnClose(wxCloseEvent&);

	// 隐藏对话框自身，并激活父窗口
	void HideAndFocusParent();

	// 原生显示事件处理器
	void OnShow(wxShowEvent&);

	// 用户点击了“搜索按钮”
	void OnSearchButton(VdkVObjEvent&);

	// 响应 VdkControl 按键事件
	virtual bool DoHandleKeyEvent(wxKeyEvent& e);

	// 获取用户输入，发送搜索事件
	void PostSearchEvent();

private:

	VdkEdit* m_keyword;
};

wxDECLARE_EVENT( OOP_EVT_PLAYLIST_SEARCH, wxCommandEvent );
