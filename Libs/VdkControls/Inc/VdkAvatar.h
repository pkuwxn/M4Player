#pragma once
#include "VdkStaticImage.h"

/// \brief ��ʾһ������Բ��Ч��ͷ��
class VdkAvatar : public VdkStaticImage
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkAvatar();

	/// \brief ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ��������
	void SetRadius(int radius) { m_radius = radius; }

private:

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	int						m_radius;
	wxPen					m_borderPen;

	VdkMouseEventType		m_ptrState; // ��ǰ���ָ���״̬

	DECLARE_DYNAMIC_VOBJECT
};
