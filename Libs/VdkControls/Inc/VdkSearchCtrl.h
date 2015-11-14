/***************************************************************
 * Name:      VdkSearchCtrl.h
 * Purpose:   Code for VdkSearchCtrl declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-03-13
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkControl.h"
#include "VdkBitmapArray.h"

/// \brief Ϊ VdkSearchCtrl ���ó�ʼ����Ϣ
class VdkSearchCtrlInitializer : 
	public VdkCtrlInitializer< VdkSearchCtrlInitializer >
{
public:
private:

	friend class VdkSearchCtrl;
};

//////////////////////////////////////////////////////////////////////////

/// \brief VdkSearchCtrl �ķ�����Լ�
enum VdkSearchCtrlStyle {
};

class VdkEdit;
class VdkButton;
/// \brief ��׼����������
class VdkSearchCtrl : public VdkCtrlHandler
{
public:

	/// \brief ���캯��
	VdkSearchCtrl();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ʵ�ʹ�������
	void Create(const VdkSearchCtrlInitializer& init_data);

	/// \brief ��ȡ�ı���
	VdkEdit* GetEditCtrl() { return m_edit; }

	/// \brief ��ȡ�ı���Ŀؼ���ʶ��
	VdkCtrlId GetEditID() const;

private:

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	/// \brief ��������
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

	//////////////////////////////////////////////////////////////////////////

	VdkEdit* m_edit;
	VdkButton* m_button;

	VdkBitmapArray m_bmpBorder; // �߿�λͼ

	int m_borderWeight; // �߿���
	int m_radius; // Բ�Ǿ���С��Բ�뾶

	wxPen m_borderPen; // ����ģʽ�±߿�
	wxPen m_hilightPen; // �����߿�

	DECLARE_DYNAMIC_VOBJECT
};
