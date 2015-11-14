/***************************************************************
 * Name:      OOPPlaylistSearchDlg.cpp
 * Purpose:   �����ٶ�λ�ļ����Ի���
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-11
 **************************************************************/
#pragma once
#include "VdkWindowImpl.h"

class VdkEdit;

/// ������ʾ�Ի���
class OOPPlaylistSearchDlg : public VdkDialog
{
public:

	/// ���캯��
	OOPPlaylistSearchDlg(VdkWindow* parent);

	/// ��������
	~OOPPlaylistSearchDlg();

	/// ��ȡ�û�����Ĳ��ҹؼ���
	wxString GetKeyword() const;

private:

	// ԭ���ر��¼�������
	void OnClose(wxCloseEvent&);

	// ���ضԻ����������������
	void HideAndFocusParent();

	// ԭ����ʾ�¼�������
	void OnShow(wxShowEvent&);

	// �û�����ˡ�������ť��
	void OnSearchButton(VdkVObjEvent&);

	// ��Ӧ VdkControl �����¼�
	virtual bool DoHandleKeyEvent(wxKeyEvent& e);

	// ��ȡ�û����룬���������¼�
	void PostSearchEvent();

private:

	VdkEdit* m_keyword;
};

wxDECLARE_EVENT( OOP_EVT_PLAYLIST_SEARCH, wxCommandEvent );
