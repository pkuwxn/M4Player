#pragma once
#include "VdkMenuPopper.h"

class VdkMenu;
/// \brief ComboBox ��һ�ֱ��֣����ṩ�ı���༭ѡ��ֵ
class VdkChoiceCtrl : public VdkMenuPopper
{
public:

	/// \brief ���캯��
	VdkChoiceCtrl();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ������Ҫ��������
	/// \param Window ������
	/// \param strName �ؼ����֣���ʶ����
	/// \param rt �ؼ�������
	/// \param strChoirces ��ʼ���������ѡ�ѡ���ʹ�ð�Ƿֺš�;���ָ
	void Create(VdkWindow* Window, 
				const wxString& strName, 
				const wxRect& rc,
				const wxString& strChoices);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��\a index ָ����λ�ò���һ����Ŀ
	void InsertItem(int index, const wxString& strLabel);

	/// \brief ѡ��ָ����ŵ���
	void Select(int index, wxDC* pDC);

	/// \brief �õ���ѡ��� ID
	/// \return ���統ǰ��ѡ�������-1
	int GetSelected() const { return m_selected; }

	/// \brief �õ��ϴ�ѡ����� ID
	/// \return �����ϴ���ѡ�������-1
	int GetLastSelected() const { return m_lastSelected; }

private:

	/// \brief ��Ӧ�˵���ѡ��
	void OnDropDownMenu(VdkVObjEvent&);

	int m_selected;
	int m_lastSelected;

	DECLARE_DYNAMIC_VOBJECT
};
