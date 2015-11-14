/***************************************************************
 * Name:      OOPFilePropDlg.cpp
 * Purpose:   ���ļ����ԡ��Ի���
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-8-17
 **************************************************************/
#pragma once
#include "VdkWindowImpl.h"

class OOPSongPtr;

/// \brief ���ļ����ԡ��Ի���
class OOPFilePropDlg : public VdkDialog
{
public:

	/// \brief ���캯��
	OOPFilePropDlg(wxWindow* parent);

	/// \brief ��������
	~OOPFilePropDlg();

	/// \brief ��ʾģʽ�Ի���
	void Popup(OOPSongPtr song);

private:

	// ���͸�����Ϣ�Ѹ��µ���Ϣ�����»ᷢ�����û��������޸ģ����ӳ��޸Ĳ��ɹ���
	// ������Ϣ��Ȼ��ԭ����
	void SendInfoUpdatedEvent();

	// �رղ����ø����ڽ���
	void HideAndFocusParent();

	// ������ĵ�����
	void SaveToFile(VdkVObjEvent&);

	// ԭ���ر��¼�
	void OnClose(wxCloseEvent&);

	// �����˽����ϵ� VdkButton
	void CloseDlg(VdkVObjEvent&);

	//-----------------------------------------------

	class PropDlgImpl;
	PropDlgImpl* m_impl;
};

//////////////////////////////////////////////////////////////////////////

wxDECLARE_EVENT( OOP_EVT_SONG_INFO_UPDATED, wxCommandEvent );
