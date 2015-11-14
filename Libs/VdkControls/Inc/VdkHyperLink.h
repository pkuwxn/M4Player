#pragma once
#include "VdkHotArea.h"

class VdkWindow;
/// \brief ��ʾһ��������
class VdkHyperLink : public VdkHotArea
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkHyperLink();

	/// \brief ִ��ʵ�ʹ�������
	void Create(VdkWindow* Window,
                const wxString& strName,
                const wxRect& rc,
                const wxString& strUrl,
                const wxString& strCaption);

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

private:

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	wxString					m_strCaption;
	wxPen						m_pen;
	int							m_yFix;

	DECLARE_DYNAMIC_VOBJECT
};
