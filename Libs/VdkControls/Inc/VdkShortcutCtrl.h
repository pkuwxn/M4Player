/***************************************************************
 * Name:      VdkShortcutCtrl.h
 * Purpose:   �Կ��ӻ��ķ�ʽ������̿�ݼ�
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-18
 **************************************************************/
#pragma once
#include "VdkEdit.h"

/// �Կ��ӻ��ķ�ʽ������̿�ݼ�
class VdkShortcutCtrl : public VdkEdit
{
public:

	/// Ĭ�Ϲ��캯��
	VdkShortcutCtrl();

	/// XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// ���õ�ǰ��ʾ�Ŀ�ݼ�
	void SetShortcut(int modifiers, int keycode, wxDC* pDC);

	/// ��յ�ǰ��ݼ�
	void Clear(wxDC* pDC);

private:

	// ������̰����¼�
	virtual void DoHandleKeyEvent(VdkKeyEvent& e);

private:

	int m_modifiers;
	int m_keycode;

	DECLARE_DYNAMIC_VOBJECT
};
