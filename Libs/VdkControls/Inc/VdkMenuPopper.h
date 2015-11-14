/***************************************************************
 * Name:      VdkMenuPopper.h
 * Purpose:   Code for VdkMenuPopper declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2010-12-03
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkToggleButton.h"

class VdkMenu;
class VdkWindow;

/// \brief ���°�ť�������˵�
class VdkMenuPopper : public VdkToggleButton, public VdkEventFilter
{
public:

	/// \brief XRC ��̬�����Ĺؼ�����
	///
	/// һ�� VdkControl �����������Ҫʵ�ֶ�̬��������Ӧ��ʵ���������
	/// \todo ���ؼ�
	virtual void Create(wxXmlNode* node);

	/// \brief ����������Ĳ˵�
	void PopupMenu();

protected:

	/// \brief ������Ը����ڵ���깳��
	///
	/// һ�������˵��ѱ���ʾ�����������ť����ȡ���˵�����ֻ���ڰ�ť�ⲿ
	/// ����Żᡣ
	void HookMouseEvent();

	/// \brief ĳЩ׼������
	/// \warning �����๹��ʱ�������
	void Prepare(wxDC* pDC);

	/// \brief �����¼�
	virtual bool FilterEvent
		(const VdkEventFilter::EventForFiltering& e);

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

	DECLARE_DYNAMIC_VOBJECT
};
