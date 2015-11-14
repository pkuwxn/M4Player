/***************************************************************
 * Name:      OOPLyricSchDlg.h
 * Purpose:   “搜索歌词”对话框
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-08-20
 **************************************************************/
#pragma once
#include "VdkWindowImpl.h"
#include "ListView/LvpClick.h"

class OOPSongPtr;

class VdkChoiceCtrl;
class VdkListView;
class VdkLabel;
class VdkButton;
class VdkEdit;

namespace LyricGrabber {
	class Task;
}

class LyricListAdapter;

/// \brief “搜索歌词”对话框
class OOPLyricSchDlg : public VdkDialog, public LvpClick::OnItemClickListener
{
public:

	/// \brief 构造函数
	OOPLyricSchDlg(wxWindow* parent);

	/// \brief 析构函数
	~OOPLyricSchDlg();

	/// \brief 弹出模式对话框
	void Popup(OOPSongPtr song);

private:

	// 查找所有需要管理的控件
	void InitCtrls();

	// 使用当前歌曲信息填充界面上的控件
	void Fill(OOPSongPtr song);

	// 清空当前状态，主要是歌词列表框和错误提示 VdkLabel
	void ClearState(wxDC* pDC);

	// 用户提交了搜索请求
	void OnSearch(VdkVObjEvent& e);

	// 用户提交了下载指定歌词的请求
	void OnDownload(VdkVObjEvent& e);

	// 用户点击了某项，要求下载
	virtual void OnItemClick(LvpClick* parent, int index, wxDC& dc);

private:

	// 歌词可选列表下载完毕
	void OnListLoaded(wxCommandEvent&);

	// 下载歌词
	void Download(int index, wxDC* pDC);

	// 指定歌词下载完毕
	void OnLyricLoaded(wxCommandEvent& e);

private:

	// 响应 VdkControl 按键事件
	virtual bool DoHandleKeyEvent(wxKeyEvent& e);

	// 原生关闭事件
	void OnClose(wxCloseEvent&);

	// 按下了界面上的 VdkButton
	void CloseDlg(VdkVObjEvent&);

	// 按下了界面上的“取消”按钮
	void OnCancel(VdkVObjEvent&);

	// 关闭并设置父窗口焦点
	void HideAndFocusParent();

private:

	LyricListAdapter* m_listAdapter;
	LvpClick* m_clickPlugin;

	// 不能对 OOPSongPtr 的有效性做任何假设，只能缓存它对应的文件路径
	wxString m_songPath;

	VdkChoiceCtrl* m_lyricServer;
	VdkListView* m_lyricList;
	VdkLabel* m_resultLabel;
	VdkButton* m_download;
	VdkButton* m_search;

	VdkEdit* m_artist;
	VdkEdit* m_title;
};
