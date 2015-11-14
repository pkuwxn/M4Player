/***************************************************************
 * Name:      VdkToolBar.h
 * Purpose:   Code for VdkToolBar declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-04-07
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkControl.h"
#include "VdkBitmapArray.h"

/// \brief VdkToolBar �ķ�����Լ�
enum VdkToolBarStyle {

};

/// \brief VdkToolBar ������ʱ״̬���Լ�
enum VdkToolBarState {

};

class VdkToolBarEntry;

//////////////////////////////////////////////////////////////////////////

/// \brief ������������Ŀ�����
class VdkToolBarEntryAdder
{
public:

	/// \brief ���캯��
	VdkToolBarEntryAdder()
		: Menu( NULL )
	{

	}

	/// \brief ���ñ����ı�
	VdkToolBarEntryAdder& caption(const wxString& c) {
		Caption = c;
		return *this;
	}

	/// \brief ���������˵�
	VdkToolBarEntryAdder& menu(VdkMenu* m) {
		Menu = m;
		return *this;
	}

	/// \brief �����������Сλͼ
	VdkToolBarEntryAdder& bid(const VdkBitmapArrayId& i) {
		BId = i;
		return *this;
	}

private:

	wxString Caption;
	VdkMenu* Menu;
	VdkBitmapArrayId BId;

	friend class VdkToolBar;
};

/// \brief �������ľ�̬���
class VdkToolBarStaticStyle
{
public:

	/// \brief ���캯��
	VdkToolBarStaticStyle();

	/// \brief ���ñ߿���ɫ
	VdkToolBarStaticStyle& borderColor(const wxColour& color) {
		m_border.SetColour( color );
		return *this;
	}

	/// \brief ��ȡ�߿򻭱�
	wxPen& borderPen() { return m_border; }

	/// \brief �������ָ����������ʱ��ť�ĸ�����ɫ
	VdkToolBarStaticStyle& hilightColor(const wxColour& color) {
		m_hilight.SetColour( color );
		return *this;
	}

	/// \brief ��ȡ������ˢ
	wxBrush& hilightBrush() { return m_hilight; }

	/// \brief �������ָ�밴��ʱ��ť�ĸ�����ɫ
	VdkToolBarStaticStyle& pushedColor(const wxColour& color) {
		m_pushed.SetColour( color );
		return *this;
	}

	/// \brief ��ȡ����ʱ�Ļ�ˢ
	wxBrush& pushedBrush() { return m_pushed; }

	/// \brief ���ù�����������ˢ
	VdkToolBarStaticStyle& bgBrush(wxBrush brush) {
		m_bg = brush;
		return *this;
	}

	/// \brief ��ȡ������������ˢ
	wxBrush& bgBrush() { return m_bg; }

	/// \brief ���ù�������ť�ĸ���߾�
	VdkToolBarStaticStyle& bitmapAddin(int addin) {
		m_bitmapAddin = addin;
		return *this;
	}

	/// \brief ��ȡ��������ť�ĸ���߾�
	int bitmapAddin() const { return m_bitmapAddin; }

private:

	wxPen m_border;
	wxBrush m_hilight;
	wxBrush m_pushed;
	wxBrush m_bg; // ������ˢ

	int m_bitmapAddin;
};

class VdkToolBarLeftSideHandle;
class VdkToolBarRightSideHandle;

/// \brief ���� Office 2003 ���Ĺ�����
class VdkToolBar : public VdkCtrlHandler
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkToolBar();

	/// \brief ��������
	~VdkToolBar();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ��������ĩβ���һ��
	VdkToolBar& Append(const VdkToolBarEntryAdder& adder);

	/// \brief ��������ĩβ���һ���ָ���
	VdkToolBar& AppendSeperator();

	/// \brief ��ȡ��̬���
	VdkToolBarStaticStyle* GetStaticStyle() const {
		return m_sstyle;
	}

private:

	// ��������
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	//////////////////////////////////////////////////////////////////////////
	// �ؼ�״̬����

	class VdkToolBarLeftSideHandle* m_leftHandle;
	class VdkToolBarRightSideHandle* m_rightHandle;

	//////////////////////////////////////////////////////////////////////////
	// �����
	VdkToolBarStaticStyle* m_sstyle;

	DECLARE_CLONEABLE_VOBJECT( VdkToolBar )
};
