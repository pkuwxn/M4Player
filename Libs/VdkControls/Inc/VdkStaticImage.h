#pragma once

class VdkWindow;
#include "VdkControl.h"

/*!\brief һ����̬ͼ����
 * 
 * ����ʵ�����ƿɻ�������ġ����ơ����ܡ�\n
 * Ϊʵ�ֵ��ƹ��ܣ����齫 XRC �ļ��б��ඨ���λ�÷ŵ��ļ��ͷ��\n
 * �����㲻�Ḳ�Ǻ����ӿؼ��Ļ��ƺ��¼�����\n
 * ͼ���ļ������� .BMP��.PNG��.JPG/.JPEG��.GIF ���ָ�ʽ֮һ��
**/
class VdkStaticImage : public VdkControl
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkStaticImage() {}

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief �����ؼ�
	/// \param Window ������
	/// \param strName �ؼ���ʶ��
	/// \param rc �ؼ�������
	/// \param bm Ҫ��ʾ��λͼ
	/// \param rescaleBitmap �Ƿ�����λͼ����Ӧ�ؼ�ԭ��������
	void Create(VdkWindow* Window, 
				const wxString& strName, 
				const wxRect& rc, 
				const wxBitmap& bm,
				bool rescaleBitmap);

protected:

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	/// \brief ��ȡҪ��ʾ��ͼ��
	const wxBitmap& GetImage() const { return m_image; }

	/// \brief ����Ҫ��ʾ��ͼ��
	void SetImage(const wxBitmap& image) { m_image = image; }

private:

    // Ҫ��ʾ��ͼ��
	wxBitmap		m_image;

	DECLARE_DYNAMIC_VOBJECT
};
