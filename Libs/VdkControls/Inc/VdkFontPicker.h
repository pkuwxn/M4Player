#pragma once
#include "VdkButton.h"

/// \brief ����ѡ����
class VdkFontPicker : public VdkButton
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkFontPicker(){}

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ���õ�ǰ����
	const wxFont& SetSelectedFont(const wxString& strDesc, wxDC* pDC);

	/// \brief ���õ�ǰ����
	const wxFont& SetSelectedFont(const wxFont& font, wxDC* pDC);

	/// \brief ��ȡ�û���ѡ������
	const wxFont& GetSelectedFont() const { return m_selectedFont; }

	/// \brief �����û���ѡ������õ��ɶ��ı�
	wxString BuildString(wxFont* pFont = NULL);

	/// \brief ���ݿɶ��ı�����ָ��������
	static wxFont GetFontFromDescString(const wxString& strDesc);

private:

	// ������ѡ��Ի���
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	//////////////////////////////////////////////////////////////////////////

	wxFont m_selectedFont; // ѡ�е�����

	DECLARE_DYNAMIC_VOBJECT
};
