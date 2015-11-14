#pragma once
#include "VdkControl.h"

/// \brief VdkLabel �ķ����
enum VdkLabelStyle {

	VLBS_USER				= VCS_USER + 10, ///< \brief ���������
	/// \brief ��������Ŀ�߶�С���ı�ʵ�ʳߴ�ʱ�Ƿ��Զ���չ������
	VLBS_AUTO_EXPAND		= 1 << ( VCS_USER + 0 ), 
};

/// \brief VdkLabel ������ʱ���Լ�λ����
enum VdkLabelState {

	VLBST_USER				= VCST_USER + 10, ///< \brief ���������
	/// \brief ��������Ŀ�߶�С���ı�ʵ�ʳߴ�ʱ�ü���ͼ����
	VLBST_CLIPPING_REGION	= 1 << ( VCST_USER + 0 ), 
};

//////////////////////////////////////////////////////////////////////////

/// \brief Ϊ VdkLabel ���ó�ʼ����Ϣ
class VdkLabelInitializer : 
	public VdkCtrlInitializer< VdkLabelInitializer >
{
public:

	typedef VdkLabelInitializer Label;

	/// \brief ���캯��
	VdkLabelInitializer();

	/// \brief ����
	Label& caption(const wxString& s) { Caption = s; return *this; }

	/// \brief �ı���������
	Label& textAlign(align_type a) { TextAlign = a; return *this; }

	/// \brief �ı���ɫ
	Label& textColor(const wxColour& color) {
		TextColor = color; return *this;
	}

protected:

	wxString Caption; ///< ����
	align_type TextAlign; ///< �ı���������
	wxColour TextColor; ///< �ı���ɫ

	friend class VdkLabel;
};

//////////////////////////////////////////////////////////////////////////

class VdkLabelGhost;
/// \brief ������� GUI �ࡪ����ʾһС�δ��ı�
class VdkLabel : public VdkControl
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkLabel();

	/// \brief ʵ�ʹ�������
	void Create(const VdkLabelInitializer& init_data);

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ��������
	~VdkLabel();

	/// \brief ȷ�� VdkLabel ��ʵ��������
	///
	/// ���ı��ı����������ƣ��û����������ֺ��Ѿ�ȷ��
	virtual void PrepareRect();

	//////////////////////////////////////////////////////////////////////////

	/// \brief ������֮������ VdkLabelGhost
	/// \attention ��Ҫ���û��˵��á�
	void SetGhost(VdkLabelGhost* ghost) { m_ghost = ghost; }

	/// \brief ��ȡ�����ı�
	wxString GetCaption() { return m_strCaption; }

	/// \brief ���ñ����ı�
	void SetCaption(const wxString& strCaption, wxDC* pDC);

private:

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	/// \brief ���ա��������֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

protected:

	wxString			m_strCaption; ///< Ҫ��ʾ���ı�

	int					m_xFix; ///< x���������ֵ����Ҫ�����Ҷ�������
	int					m_yFix; ///< y���������ֵ��ʹ�ı���ֱ����
	align_type			m_TextAlign; ///< �ı���������

	wxColour			m_TextColor; ///< �ı���ɫ
	VdkLabelGhost*		m_ghost; ///< �ӳ�ָ��ʱ��ı� VdkLabel �ı���

	DECLARE_DYNAMIC_VOBJECT
};

/// \brief �ӳ�ָ��ʱ��ı� VdkLabel �ı���
///
/// �����ڶ��ϴ���������Ϊ�����delete this;ɾ���Լ���\n
/// ʹ�÷���:
/// \code
/// wxString oldCaption(label->GetCaption());
/// label->SetCaption(newCaption);
/// new VdkLabelGhost(label, oldCaption, 1000); // �ӳ�1��ָ���ʾ
/// \endcode
/// ��Ҫ�� VdkLabelGhost ���ͷ�
class VdkLabelGhost : public wxTimer
{
public:

	VdkLabelGhost(VdkLabel* label, const wxString& strNewText, int ms);

	/// \brief �ڴ˻ָ� VdkLabel �ı���
	virtual void Notify();

private:

	VdkLabel*			m_label;
	wxString			m_strNewText;
};
