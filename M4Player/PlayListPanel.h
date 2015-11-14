/***************************************************************
 * Name:      PlayListPanel.h
 * Purpose:   ���� PlayListPanel
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-2-27
 **************************************************************/
#pragma once
#include "OOPWindow.h"
#include "VdkBitmapArray.h"

class OOPSongPtr;
class OOPList;
class OOPFilePropDlg;
class OOPPlaylistSearchDlg;
class VdkLcCell;

/// \brief �����б���
class PlayListPanel : public OOPWindow
{
public:

	/// \brief ���캯��
	PlayListPanel(wxWindow* parent, bool showAtOnce);

	/// \brief ���û�ѯ�ʲ���ӵõ��ģ�������ļ�
	/// \param parent �����ļ���ģ̬�Ի���ĸ�����
	void AddFiles(wxWindow* parent);

	/// \brief ִ��ʵ������ļ��Ĳ���
	void AddFiles(wxArrayString* files);

	//! ���Ŀ¼�ļ���ģʽ
	enum RecursiveMode {
		RM_RECURSIVE, ///< �������ļ���
		RM_ONLY_TOP_LEVEL_FILES, ///< ֻ���������ļ�
	};

	/// \brief ���û�ѯ�ʲ���ӵõ���һ���ļ���
	void AddFolder(RecursiveMode rm);

	/// \brief ���û�ѯ�ʲ���ӵõ���һ���ļ���
	void AddFolder(const wxString& dirPath, RecursiveMode rm, wxArrayString* files);

private:

	// ���ô������õ�Ƥ��
	virtual void DoSetSkin();

	// ��Ӧ�����¼�
	virtual bool DoHandleKeyEvent(wxKeyEvent& e);

	// �ػ�����¼�(ʵ�֡�αģ̬�Ի���)
	virtual bool FilterEventBefore(wxMouseEvent&, int);

	// ��⵱ǰ״̬���Ƿ���Ҫʵ�֡�αģ̬�Ի���
	bool TestPsuedoModal();

	// �ػ񴰿ڹر��¼�
	void OnClose(wxCloseEvent&);

	// �ļ�ɨ�����
	void OnScanDone(wxCommandEvent& e);

	// �򿪡��ļ����ԡ��Ի���
	void OnFileProperties(VdkVObjEvent&);

	// �û������˸����� TAG
	void OnSongInfoUpdated(wxCommandEvent& e);

    // �����������ļ�
	void OnRename(VdkVObjEvent& e);

	// �û��ύ��һ������
	void OnSearch(wxCommandEvent& e);

	// ִ�о�����������
	void Search();

	// ���ɲ˵�
	void GenerateMenu();

	// �˵���İ�����ʾ
	void OnPreShowMenu(VdkVObjEvent&);

	// ��ȡ�б��е�ǰ��ѡ�еĵ�һ����Ŀ
	OOPSongPtr GetCurrSel(VdkLcCell** pCurr, int* pIndex) const;

private:

	OOPList* m_playList;
	OOPFilePropDlg* m_filePropDlg; // ���ļ����ԡ��Ի���
	// �Ѵ����ļ����ԡ��Ի���رպ󷢹����ĸ���Ԫ��Ϣ�Ѹ����¼�
	bool m_songInfoUpdated;

	OOPPlaylistSearchDlg* m_searchDlg; // �����ٶ�λ�ļ����Ի���
	wxString m_lastKeyword; // �ϴ�ִ�в��ҵĹؼ���
	int m_lastFound; // �ϴ��ҵ�����Ŀ���к�

	int m_menuId; // �˵� ID

	VdkBitmapArray m_playListMenuBitmaps;
};
