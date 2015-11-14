/***************************************************************
 * Name:      OOPLyricSchDlg.h
 * Purpose:   ��������ʡ��Ի���
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

/// \brief ��������ʡ��Ի���
class OOPLyricSchDlg : public VdkDialog, public LvpClick::OnItemClickListener
{
public:

	/// \brief ���캯��
	OOPLyricSchDlg(wxWindow* parent);

	/// \brief ��������
	~OOPLyricSchDlg();

	/// \brief ����ģʽ�Ի���
	void Popup(OOPSongPtr song);

private:

	// ����������Ҫ����Ŀؼ�
	void InitCtrls();

	// ʹ�õ�ǰ������Ϣ�������ϵĿؼ�
	void Fill(OOPSongPtr song);

	// ��յ�ǰ״̬����Ҫ�Ǹ���б��ʹ�����ʾ VdkLabel
	void ClearState(wxDC* pDC);

	// �û��ύ����������
	void OnSearch(VdkVObjEvent& e);

	// �û��ύ������ָ����ʵ�����
	void OnDownload(VdkVObjEvent& e);

	// �û������ĳ�Ҫ������
	virtual void OnItemClick(LvpClick* parent, int index, wxDC& dc);

private:

	// ��ʿ�ѡ�б��������
	void OnListLoaded(wxCommandEvent&);

	// ���ظ��
	void Download(int index, wxDC* pDC);

	// ָ������������
	void OnLyricLoaded(wxCommandEvent& e);

private:

	// ��Ӧ VdkControl �����¼�
	virtual bool DoHandleKeyEvent(wxKeyEvent& e);

	// ԭ���ر��¼�
	void OnClose(wxCloseEvent&);

	// �����˽����ϵ� VdkButton
	void CloseDlg(VdkVObjEvent&);

	// �����˽����ϵġ�ȡ������ť
	void OnCancel(VdkVObjEvent&);

	// �رղ����ø����ڽ���
	void HideAndFocusParent();

private:

	LyricListAdapter* m_listAdapter;
	LvpClick* m_clickPlugin;

	// ���ܶ� OOPSongPtr ����Ч�����κμ��裬ֻ�ܻ�������Ӧ���ļ�·��
	wxString m_songPath;

	VdkChoiceCtrl* m_lyricServer;
	VdkListView* m_lyricList;
	VdkLabel* m_resultLabel;
	VdkButton* m_download;
	VdkButton* m_search;

	VdkEdit* m_artist;
	VdkEdit* m_title;
};
