/***************************************************************
 * Name:      EqPanel.h
 * Purpose:   ����������
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-24
 **************************************************************/
#pragma once
#include "OOPWindow.h"

class VdkSlider;
class VdkToggleButton;
class VdkButton;

/// \brief ����������
class EqPanel : public OOPWindow
{
public:

	/// \brief ���캯��
	EqPanel(wxWindow* parent);

private:

	// ���ô������õ�Ƥ��
	virtual void DoSetSkin();

	// ���Ҿ�����������������׼�ֱ��� XRC ����
	wxXmlNode* FindEqfactor(wxXmlNode* windowRoot) const;

	// ����/�������пؼ�
	void EnableAll(bool enable, wxDC* pDC);

private:

	VdkToggleButton* m_enableAll;
	VdkButton* m_profile;
	VdkButton* m_reset;

	VdkSlider* m_balance;
	VdkSlider* m_surround;
	VdkSlider* m_preamp;

	enum {
		NUM_EQFACTORS = 10,
	};

	VdkSlider* m_eqfactors[NUM_EQFACTORS];
};
