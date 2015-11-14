/***************************************************************
 * Name:      LyricPanel.h
 * Purpose:   ���� LyricPanel
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-2-27
 **************************************************************/
#pragma once
#include "OOPWindow.h"
#include "VdkBitmapArray.h"

class OOPSongPtr;
class OOPLyricSchDlg;
class OOPLyric;

/// \brief ��ʴ���
class LyricPanel : public OOPWindow
{
public:

	/// \brief ���캯��
	LyricPanel(wxWindow* parent);

	/// \brief ������������Ի���
	void PopupSchDlg(OOPSongPtr song);

private:

	// ���ô������õ�Ƥ��
	virtual void DoSetSkin();

	// ���ɲ˵�
	void GenerateMenu();

	// �˵���İ�����ʾ
	void OnPreShowMenu(VdkVObjEvent&);

public:

	OOPLyricSchDlg* m_schDlg;
	OOPLyric* m_lyric;
	int m_menuId; // �����ؼ��˵� ID

	VdkBitmapArray m_menuBitmaps;
};
