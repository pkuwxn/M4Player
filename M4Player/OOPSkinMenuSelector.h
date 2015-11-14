/***************************************************************
 * Name:      OOPSkinMenuSelector.h
 * Purpose:   �ṷ2011 ����Ƥ��ѡ����
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-18
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkScrolledPanel.h"

/// \brief �ṷ2011 ����Ƥ��ѡ����
class OOPSkinMenuSelector : public VdkScrolledPanel
{
public:

	/// \brief ���캯��
	OOPSkinMenuSelector();

	/// \brief ��������
	~OOPSkinMenuSelector();

private:

	// �����Լ��Ĺ�����
	virtual VdkScrollBar* ScrollBarExists();

	//////////////////////////////////////////////////////////////////////////

	DECLARE_DYNAMIC_VOBJECT
};

#include "OOPSkin.h"
typedef OOPSkin::SkinInfo OOPSkinInfo;

/// \brief Ƥ������ͼ
class SkinAvatar : public VdkControl
{
public:

	/// \brief Ĭ�Ϲ��캯��
	SkinAvatar();

	/// \brief �����ؼ�
	void Create(VdkWindow* win,
				const wxRect& rc,
				const OOPSkinInfo& info);

	/// \brief Ƥ����Ϣ
	void GetSkinInfo()
	{

	}

private:

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	// ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	// �ӳ�����Ƥ������ͼ(��һ����ʾ��)
	void OnLoadAvatar(wxCommandEvent& e);

	//////////////////////////////////////////////////////////////////////////

	const OOPSkinInfo* m_skinInfo;
	VdkMouseEventType m_state; // �ؼ���ǰ������״̬������/����

	const static int ms_tnwidth = 80; // ����ͼ���
	const static int ms_tnheight = 80; // ����ͼ�߶�

	const static int ms_yPadding = 2; // ����ͼ������������ľ���
	int m_xText; // �ı��� X ����
	wxString m_labelOverflow; // �ض��˵��ı�

	wxPen m_borderNomal; // ����״̬�µı߿�
	wxBrush m_hilight; // ����״̬�µı���
};
