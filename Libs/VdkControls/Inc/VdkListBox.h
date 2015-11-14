#pragma once
#include "VdkListCtrl.h"

/// \brief Ϊ VdkListBox ���ó�ʼ����Ϣ
class VdkListBoxInitializer : public 
	VdkCtrlInitializer< VdkListBoxInitializer > {
public:

	typedef VdkListBoxInitializer ListBox;

	/// \brief �����ı����뷽ʽ
	ListBox& textAlign(align_type align) {
		TextAlign = align; return *this;
	}

private:

	align_type TextAlign;
	friend class VdkListBox;
};

//////////////////////////////////////////////////////////////////////////

/// \brief ֻ��һ�е� VdkListCtrl
///
/// ����һ�������ࡣ
class VdkListBox : public VdkListCtrl
{
public:

	/// \brief ���캯��
	/// \param style �б��ķ�����
	VdkListBox(long style = VCS_BORDER_SIMPLE | VLCS_HOVERING)
		: VdkListCtrl( style )
	{

	}

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ִ��ʵ�ʹ�������
	void Create(const VdkListBoxInitializer& init_data);

private:

	DECLARE_DYNAMIC_VOBJECT
};
