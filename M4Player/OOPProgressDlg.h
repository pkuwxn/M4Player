/***************************************************************
 * Name:      OOPProgressDlg.cpp
 * Purpose:   ������ʾ�Ի���
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-8-19
 **************************************************************/
#pragma once
#include "VdkWindowImpl.h"

class VdkLabel;

/// \brief ������ʾ�Ի���
class OOPProgressDlg : public VdkDialog
{
public:

	/// \brief ���캯��
	OOPProgressDlg(wxWindow* parent, 
				   const wxString& title, 
				   const wxString& msg,
				   int maximum = 100);

	/// \brief ��������
	~OOPProgressDlg();

	/// \brief ���µ�ǰ����ֵ
	/// \param value ��ǰ����ֵ
	/// \param newmsg �µ���ʾ�ַ���������Ϊ�գ�����ԭ���Ĳ���
	/// \param skip �����û������ˡ���������ť����ô���������һ�εĵ��ô�ֵ
	/// ����Ϊ��
	/// \return ���²����Ƿ�ɹ������û������ˡ�ȡ������ť���������ʧ��
	virtual bool Update(int value, 
						const wxString& newmsg = wxEmptyString, 
						bool* skip = NULL);

	/// \brief �����Ƿ��ѱ��û�ȡ��
	bool IsCanceled() const { return m_canceled; }

	/// \brief ����Ի���״̬
	void ClearState();

	/// \brief ��ȡ���ֵ
	int GetMaximum() const;

	/// \brief ��ȡ��ǰ����ֵ
	int GetValue() const;

private:

	// ԭ���ر��¼�
	void OnClose(wxCloseEvent&);

	// �����˽����ϵġ�ȡ������ť
	void OnCancel(VdkVObjEvent&);

private:

	VdkLabel* m_label;

	int m_maximum;
	int m_value;
	bool m_skipped;
	bool m_canceled;
};
