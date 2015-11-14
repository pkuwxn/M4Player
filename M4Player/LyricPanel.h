/***************************************************************
 * Name:      LyricPanel.h
 * Purpose:   声明 LyricPanel
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-2-27
 **************************************************************/
#pragma once
#include "OOPWindow.h"
#include "VdkBitmapArray.h"

class OOPSongPtr;
class OOPLyricSchDlg;
class OOPLyric;

/// \brief 歌词窗口
class LyricPanel : public OOPWindow
{
public:

	/// \brief 构造函数
	LyricPanel(wxWindow* parent);

	/// \brief 弹出歌词搜索对话框
	void PopupSchDlg(OOPSongPtr song);

private:

	// 设置窗口所用的皮肤
	virtual void DoSetSkin();

	// 生成菜单
	void GenerateMenu();

	// 菜单项的按需显示
	void OnPreShowMenu(VdkVObjEvent&);

public:

	OOPLyricSchDlg* m_schDlg;
	OOPLyric* m_lyric;
	int m_menuId; // 歌词秀控件菜单 ID

	VdkBitmapArray m_menuBitmaps;
};
