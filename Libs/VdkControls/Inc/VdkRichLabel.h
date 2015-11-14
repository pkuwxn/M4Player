#pragma once
#include "VdkLabel.h"
#include <wx/vector.h>

/// \brief VdkRichLabel ������� URL
struct Url {

	wxString	strUrl; ///< URL ��ַ
	wxString	strUrlDisplay; ///< ��ʾ��ʵ�� URL �ı������ܽض̣�
	wxString	strDummy; ///< VdkRichLabel ��������� URL λ�õ������ı�

	wxRect		Rect; ///< ���⻭���ϵ�λ��
};

/// \brief ���ı���ǩ
///
/// ��ǰ���еĹ��ܣ�
/// ** ������ʾ��
/// ** URL ���͡�
class VdkRichLabel : public VdkLabel
{
public:

	/// \brief ���캯��
	VdkRichLabel();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ȷ�� VdkLabel ��ʵ��������
	virtual void PrepareRect();

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���� URL ��ʾ����ı���ɫ���Ƿ���ʾ�»��ߵȣ�
	void SetUrlStyle(wxString strStyle);

	/// \brief �õ�ָ����ŵ� URL
	wxString GetUrl(unsigned id = 0);

private:

	/// \brief һ�β�ɫ�ı�
	struct ColorTextNode {

		wxString	strText; ///< �ı�
		int			width; ///< �ı����
		int			x, y; ///< �ı������λ��
		wxColour	color; ///< �ı���ɫ
	};

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

	/// \brief ���� UBB ���룬�ֽ�����ֵ
	bool GetUbbNode( wxString& strUbb, 
					 wxString& strName, 
					 wxString& strAttriValue,
					 wxString& strText );

	/// \brief ���� URL
	void DrawUrl(int& UrlIndex, const wxString& strUrlAll, wxDC& dc, int x, int y);
	/// \brief ��������� URL ͬ�������������λ�õ������ı�
	void GetRandomString(wxString& strOut, int len);

	//////////////////////////////////////////////////////////////////////////

	wxColour				m_UrlColor;

	_WX_DECLARE_VECTOR(Url, VectorOfUrl, );
	_WX_DECLARE_VECTOR(ColorTextNode, VectorOfColorTextNode, );

	VectorOfUrl				m_Urls;				// �ı��а���������URL
	wxArrayString			m_strBreaks;		// �����ı����Է��С��ִ�
	VectorOfColorTextNode	m_colorTexts;		// ��ɫ�ı�

	DECLARE_DYNAMIC_VOBJECT
};
