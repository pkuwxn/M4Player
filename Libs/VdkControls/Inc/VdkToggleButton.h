#pragma once
#include "VdkButton.h"

/// \brief VdkToggleButton �ķ�����Լ�
enum VdkToggleButtonStyle {

	/// \brief ��������ʵ����λ���
	VTBS_USER				= VBS_USER + 5,
	/// \see VdkToggleButton::SetOwnerControl
	VTBS_OWNER_CONTROL		= 1 << ( VBS_USER + 0 ),
};

/// \brief VdkToggleButton ������ʱ״̬���Լ�
enum VdkToggleButtonState {

	/// \brief ��������ʼ����λ���
	VTBST_USER				= VBST_USER + 5,
	/// \brief �Ƿ��Ѿ�������
	VTBST_TOGGLED			= 1 << ( VBST_USER + 0 ),
	/// \brief �Ƿ��ڳ�������״̬������ĳ����ʱ������Ҫ��ռ��
	VTBST_PUSHED			= 1 << ( VBST_USER + 1 ),
	/// \brief ����Ҫ������
	VTBST_TO_BE_PUSHED		= 1 << ( VBST_USER + 2 ),
	/// \brief ��Ϊ VdkToggledButton ���ڰ���״̬ʱ��m_nState 
	/// ʼ��Ϊ VBS_PUSHED��LEFT_DOWN��������޷��������״̬��������¼���ɵģ�
	/// ���� VdkToggleButton ������״̬���������һ������λ��������
	VTBST_LAST_LEFT_DOWN	= 1 << ( VBST_USER + 3 ),
};

/// \brief �����Զ����𡢻ָ�ԭ��״̬�İ�ť
class VdkToggleButton : public VdkButton
{
public:

	/// \brief ���캯��
	VdkToggleButton();

	/////////////////////////////////////////////////////////////////

	/// \brief ʹ��ť���ְ���״̬������Ӧ�¼�
	///
	/// ����һ�֡�δ��̬��������һ�ε����ʹ��ť���ְ���״̬��\n
	/// ������Ӧ�κ��¼���ֱ���û��ٴε��ñ������������״̬\n
	/// ��ʹ\a bPushed Ϊ false ����
	void PushButton(bool bPushed, wxDC* pDC);

	/// \brief �л���ť״̬
	void Toggle(bool bToggled, wxDC* pDC = NULL, bool bFireCallback = false);

	/// \brief ��ť�Ƿ��Ѿ�������
	bool IsToggled() const { return TestState( VTBST_TOGGLED ); }

	/// \brief ��ť�Ƿ��ڳ�������״̬
	bool IsPushed() const { return TestState( VTBST_PUSHED ); }

	/// \brief ��ť�Ƿ񼴽�Ҫ������
	bool IsToBePushed() const { return TestState( VTBST_TO_BE_PUSHED ); }
	
	/// \brief ��ť�ĵ����Ƿ����û����Ƶ�״̬��
	///
	/// ��ʱ VDK ���԰�ť���յ����¼����κ�Ĭ�Ϸ�Ӧ��
	/// \see SetOwnerControl
	bool IsOwnerControl() const { return TestStyle( VTBS_OWNER_CONTROL ); }

	/// \brief ���ð�ť������ʱ�ٴε��ʱ�Ƿ����û����Ƶ������
	///
	/// ����ť���ڡ��������¡�(Toggleed)��״̬ʱ���趨�ٴε�����ʱ�ɵ����߾���\n
	/// �����Ƿ���Ҫȡ�������ڲ�״̬������
	void SetOwnerControl(bool bEnabled = true){
		SetAddinStyle( VTBS_OWNER_CONTROL );
	}
	
protected:

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	DECLARE_DYNAMIC_VOBJECT
};
