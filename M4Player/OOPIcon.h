#pragma once
#include "VdkStaticImage.h"

class VdkWindow;
/// \brief ���� OOPlayer ��ͼ�����ɡ��쳣����ȵ�
class OOPIcon : public VdkStaticImage
{
public:

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ����ʹ�õ�ͼ��
	void SetFrame(wxTopLevelWindow* frame);

private:

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

private:

	wxIcon m_icon;
	wxIconBundle m_icons;

	DECLARE_DYNAMIC_VOBJECT
};
