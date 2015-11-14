/***************************************************************
 * Name:      OOPSkinSelector.h
 * Purpose:   ʹ�� VdkListCtrl ����ʾ��������Ƥ��
 * Author:    Ning (vanxining@139.com)
 * Created:   2011.04.02
 * Copyright: Wang Xiao Ning
 **************************************************************/
#pragma once
#include "VdkMenu.h"
#include "OOPSkin.h"

class OOPSkinMenuSelector;

/// \brief ʹ�ù�����士����ͼ����ʾ��������Ƥ��
class OOPSkinSelector : public VdkMenuXrcCtrlWrapper
{
public:

	/// \brief ���캯��
	OOPSkinSelector();

	/// \brief ��������
	~OOPSkinSelector();

	/// \brief ʵ����Ҫ���ӵ� VdkMenu �ϵĿؼ�
	virtual VdkControl* Implement(VdkWindow* menuWin, const wxPoint& pos);

private:

	// ����ؼ�״̬
	virtual void SaveState();

	// ���һ������ͼ
	void AddAvatar(const OOPSkin::SkinInfo& info);

	// ˫������ͼ������Ƥ��
	void OnAvatar(VdkVObjEvent& e);

private:

	OOPSkinMenuSelector* m_selector;
	int m_avatarCount; // ��ʾ��Ƥ����

	double m_progress; // ��ǰ�鿴�Ľ���
};
