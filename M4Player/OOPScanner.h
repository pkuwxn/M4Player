#pragma once
#include <wx/vector.h>

class OOPSongPtr;
class OOPProgressDlg;
class VdkVObjEvent;

wxDECLARE_EVENT( OOP_EVT_SCAN_DONE, wxCommandEvent );

/// \brief ����ָ��Ŀ¼�����Ҹ���
class OOPScanner : public wxThread
{
public:

	/// \brief ���캯��
	/// \param songList Ӧ���ڶ��Ϸ��䣬�ᱻ�ӹ�
	OOPScanner(wxWindow* dlgParent, wxArrayString* songList);

	/// \brief  ��������
	~OOPScanner();

private:

	// ���ݹ������̷߳�������Ϣ���¶Ի���
	void OnUpdateUI(VdkVObjEvent& e);

	// �߳���ں���
	virtual ExitCode Entry();

	//--------------------------

	OOPProgressDlg* m_dlg;
	wxWindow* m_dlgParent;

	wxArrayString* m_songList;
	wxVector< OOPSongPtr >* m_songs;

	int m_idUpdateUi;
};
