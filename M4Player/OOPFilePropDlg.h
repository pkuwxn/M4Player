/***************************************************************
 * Name:      OOPFilePropDlg.cpp
 * Purpose:   “文件属性”对话框
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-8-17
 **************************************************************/
#pragma once
#include "VdkWindowImpl.h"

class OOPSongPtr;

/// \brief “文件属性”对话框
class OOPFilePropDlg : public VdkDialog
{
public:

	/// \brief 构造函数
	OOPFilePropDlg(wxWindow* parent);

	/// \brief 析构函数
	~OOPFilePropDlg();

	/// \brief 显示模式对话框
	void Popup(OOPSongPtr song);

private:

	// 发送歌曲信息已更新的消息，更新会发生在用户进行了修改，或延迟修改不成功，
	// 歌曲信息仍然是原样。
	void SendInfoUpdatedEvent();

	// 关闭并设置父窗口焦点
	void HideAndFocusParent();

	// 保存更改到磁盘
	void SaveToFile(VdkVObjEvent&);

	// 原生关闭事件
	void OnClose(wxCloseEvent&);

	// 按下了界面上的 VdkButton
	void CloseDlg(VdkVObjEvent&);

	//-----------------------------------------------

	class PropDlgImpl;
	PropDlgImpl* m_impl;
};

//////////////////////////////////////////////////////////////////////////

wxDECLARE_EVENT( OOP_EVT_SONG_INFO_UPDATED, wxCommandEvent );
