#pragma once
#include "VdkNativeCtrl.h"
#include <wx/richtext/richtextctrl.h>

/// \brief ��װ���ı��༭��
class VdkRichTextCtrl : public wxRichTextCtrl, 
						public VdkNativeCtrl< VdkRichTextCtrl >
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkRichTextCtrl(){ Init(); }

	/// \brief ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief �����ؼ�
	void Create( VdkWindow* win, 
				 const wxString& strName, 
				 const wxRect& rc, 
				 long style );

private:

	/// \brief ��ʼ���ؼ�
	void Init();

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	long				m_nativeStyle;
	int					m_borderWeight;
	wxPen				m_borderPen;

	DECLARE_DYNAMIC_VOBJECT
};
