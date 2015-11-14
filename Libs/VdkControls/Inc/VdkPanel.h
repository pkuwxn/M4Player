/***************************************************************
 * Name:      VdkPanel.h
 * Purpose:   Code for VListCtrl implementation
 * Author:    Ning (vanxining@139.com)
 * Created:   2010-09-11
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkControl.h"
class RedrawCallback;

/*!\brief ������ wxPanel ���ӿؼ���Ͽؼ�
 *
 * VdkPanel ����������֯һ���߼��Խ�ǿ�Ŀؼ�������ѡ��л���ʾ��һ�׿ؼ���
**/
class VdkPanel : public VdkCtrlHandler
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkPanel();
	/// \brief ��������
	~VdkPanel();

	/// \brief ִ��ʵ�ʹ�������
	void Create(VdkWindow* parent, const wxString& strName, const wxRect& rc);

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/////////////////////////////////////////////////////////////////////////

	/// \brief ���ûص����ƶ���
	void SetRedrawCallback(RedrawCallback* Redrawer) { m_redrawer = Redrawer; }

private:

	/// \brief ��������
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	RedrawCallback*				m_redrawer;

	wxBrush						m_bgBrush;
	GradientBgInfo*				m_gbi;
	BackgroundInfo*				m_bi;

	DECLARE_DYNAMIC_VOBJECT
};
