#pragma once
#include "VdkButton.h"

/// \brief ��ѡ��
class VdkCheckBox : public VdkButton
{
public:

	/// \brief ���캯��
	VdkCheckBox();

	/// \brief ִ��ʵ�ʹ�������
	void Create(VdkWindow* Window, const wxString& strName, wxRect rc);

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	//////////////////////////////////////////////////////////////////////////

	/// \brief �л�ѡ�����
	void Toggle(bool bToggled, wxDC* pDC);

	/// \brief �Ƿ�ѡ��
	bool IsChecked() const { return m_bToggled; }

	/// \brief ʹ��/���ù����ؼ�
	void EnableRelatedCtrls(bool bEnabled, wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ӹ��� VdkControl �� CheckBox ��
	///
	/// ���� VdkCheckBox ʹ����һ���ı����� VdkCheckBox ѡ��ʱ�ı�����ã�
	/// ��ѡ��ʱʧ�
	void AddRelatedCtrl(VdkControl* relatedCtrl);

private:

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);
	
	/// \brief ��ѡ����Դ��ڵ�״̬
	enum State {

		NORMAL,
		HOVERING,
		PUSHED,
		CHECKED,
		CHECKED_HOVERING,
		NORMAL_DISABLED,
		THREE_STATE_DISABLED,
		CHECKED_DISABLED
	};

	/// \brief �� VdkCheckBox::State ǿ��ת��Ϊ VdkButton::State
	void SetButtonState(State state) {
		Update( (VdkButton::State )state, NULL );
	}

	//////////////////////////////////////////////////////////////////////////

	bool					m_bToggled;
	VdkCtrlList				m_relatedCtrls;

	DECLARE_DYNAMIC_VOBJECT
};
