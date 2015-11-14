#pragma once
#include "VdkControl.h"

class VdkWindow;
/// \brief һ�����Ƴ����ӵ��ȵ����򣬿���ʹ�ñ���λͼ
class VdkHotArea : public VdkControl
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkHotArea();

	/// \brief ִ��ʵ�ʹ�������
	void Create(VdkWindow* Window, 
				const wxString& strName, 
				const wxRect& rc, 
				const wxString& strUrl);

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

protected:

	/// \brief ����������ĳ�ʼ��
	void StartSense();

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	bool			m_bHand; ///< ���ָ���Ƿ��Ѿ��ı�
	wxString		m_strUrl; ///< URL ��ַ

	wxCursor		m_cursorHand; ///< ��������������ָ����״

	DECLARE_DYNAMIC_VOBJECT
};
