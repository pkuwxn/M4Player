/***************************************************************
 * Name:      VdkSidePanel.h
 * Purpose:   Code for VdkSidePanel declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-04-08
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkControl.h"

/// \brief VdkSidePanel �ķ�����Լ�
enum VdkSidePanelStyle {

};

/// \brief VdkSidePanel ������ʱ״̬���Լ�
enum VdkSidePanelState {

};

/// \brief ���� Office 2003 �Ĳ����
class VdkSidePanel : public VdkControl
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkSidePanel();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

private:

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

private:

	wxString m_strCaption;

	DECLARE_CLONEABLE_VOBJECT( VdkSidePanel )
};
