/***************************************************************
 * Name:      MainPanel.h
 * Purpose:   ���� MainPanel
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-2-27
 **************************************************************/
#pragma once
#include "OOPWindow.h"

/// \brief �����ڣ���������
class MainPanel : public OOPWindow
{
public:

	/// \brief ���캯��
	MainPanel();

private:

	// ���ô������õ�Ƥ��
	virtual void DoSetSkin();
};
