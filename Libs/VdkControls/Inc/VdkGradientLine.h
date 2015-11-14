#pragma once
#include "VdkLabel.h"

/// \brief һ�ξ��н���Ч���ķָ���
class VdkGradientLine : public VdkLabel
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkGradientLine();

private:

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	int				m_dxLine; // ����ɫ�� x ���ϵĻ�ͼ��ʼ��
	int				m_dyLine;

	wxBitmap		m_gradientLine;

	DECLARE_DYNAMIC_VOBJECT
};
