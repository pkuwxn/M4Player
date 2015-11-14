/***************************************************************
 * Name:      VdkMenuBar.h
 * Purpose:   Code for VdkMenuBar declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2010-12-02
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkControl.h"
#include "VdkButton.h"
#include "VdkMenuPopper.h"
#include "VdkEvent.h"

class VdkMenuBarEntryStyle;

/// \brief VdkMenuBarEntry ����ĳ�ʼ����Ϣ
class VdkMenuBarEntryInitializer : public VdkButtonInitializer
{
public:

	typedef VdkMenuBarEntryInitializer MenuBarEntry;

	/// \brief ���캯��
	VdkMenuBarEntryInitializer();

	/// \brief ���������Ĳ˵�
	MenuBarEntry& menu(VdkMenu* m) { Menu = m; return *this; }

	/// \brief ��\a s �ж�̬�����µĲ˵�
	MenuBarEntry& menuTree(const wxString& s) { MenuTree = s; return *this; }

	/// \brief ���û���ʱ���õķ��
	MenuBarEntry& drawStyle(VdkMenuBarEntryStyle* s) { DrawStyle = s; return *this; }

private:

	VdkMenu* Menu;
	wxString MenuTree;
	VdkMenuBarEntryStyle* DrawStyle;

	friend class VdkMenuBarEntry;
};

/// \brief Ϊ VdkMenuBar ����Ƥ��
class VdkMenuBarInitializer : 
	public VdkCtrlInitializer< VdkMenuBarInitializer >
{
public:

	typedef VdkMenuBarInitializer MenuBar;

	/// \brief ���캯��
	VdkMenuBarInitializer();

	/// \brief �������еĲ˵���
	MenuBar& menus(VdkMenuBarEntryInitializer menus[]) {
		menuArray = menus; return *this;
	}

private:

	VdkMenuBarEntryInitializer* menuArray;
	friend class VdkMenuBar;
};

//////////////////////////////////////////////////////////////////////////

class VdkMenuBarEntry;
WX_DECLARE_LIST_PTR( VdkMenuBarEntry, ListOfEntry );

/// \brief �˵���
class VdkMenuBar : public VdkCtrlHandler, public VdkEventFilter
{
public:

	/// \brief ���캯��
	VdkMenuBar();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief �����ؼ�
	void Create(const VdkMenuBarInitializer& init_data);

	/// \brief ��������
	~VdkMenuBar();
	
	/// \brief ���˵��������һ��
	/// \param strCaption �����ı�
	/// \param menu ��֮������Ĳ˵����
	/// \param strMenuTree ��\a menu Ϊ�գ����ָ�����������ж�̬����
	VdkMenuBar& Append(wxString strCaption, 
					   VdkMenu* menu, 
					   const wxString& strMenuTree = wxEmptyString);

public:

	/// \brief ���˲˵�����������ƶ��¼�
	virtual bool FilterEvent(const EventForFiltering& e);

private:

	// �Ӷ��ϴ���һ������� C++ ����
	virtual VdkMenuBarEntry* CreateNewEntry(int& w, int& h);

	// ��������
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	// ����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

	unsigned				m_total;
	ListOfEntry				m_entries;
	VdkMenuBarEntryStyle*	m_drawStyle;

	DECLARE_DYNAMIC_VOBJECT
};

//////////////////////////////////////////////////////////////////////////

enum VdkMenuBarEntryStyleEnum {

	VMBES_OFFICE2003,
};

/// \brief �˵�����Ŀ�Ĺ��÷�����
class VdkMenuBarEntryStyle
{
public:

	/// \brief ����һ�����ʱ�߿�������õ���ɫ
	VdkMenuBarEntryStyle& hilightBorderColour(const wxColour& color) {
		m_hilightBorderPen.SetColour(color);
		return *this;
	}

	/// \brief ����һ�ѡ��ʱ�߿�������õ���ɫ
	VdkMenuBarEntryStyle& selectedBorderColour(const wxColour& color) {
		m_selectedBorderPen.SetColour(color);
		return *this;
	}

	/// \brief ����һ������״̬�µı�����ɫ
	VdkMenuBarEntryStyle& bgBrush(const wxBrush& brush) {
		m_background = brush;
		return *this;
	}

	/// \brief ����һ�����ʱ�������õ���ɫ
	VdkMenuBarEntryStyle& hilightColour(const wxColour& color) {
		m_hilight.SetColour( color );
		return *this;
	}

	/// \brief ����һ�ѡ��ʱ�������õ���ɫ
	VdkMenuBarEntryStyle& selectedColour(const wxColour& color) {
		m_selected.SetColour(color);
		return *this;
	}

private:

	wxPen m_hilightBorderPen;
	wxPen m_selectedBorderPen;
	wxBrush m_background;
	wxBrush m_hilight;
	wxBrush m_selected;

	friend class VdkMenuBarEntry;
};

/// \brief �˵����е�һ�������˵���ť
class VdkMenuBarEntry : public VdkMenuPopper
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkMenuBarEntry();

	/// \brief �е�ʵ�ʹ�������
	void Create(const VdkMenuBarEntryInitializer& init_data);

	/// \brief ��������
	~VdkMenuBarEntry();

private:

	/// \brief �����ؼ�
	void Init();

	/// \brief ����\a rc ָ���ľ���
	/// \param rc �������Ϊ���ؼ������Ͻ�
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	//////////////////////////////////////////////////////////////////////////

	wxString m_strCaption;  // �˵�������ı�
	VdkMenuBarEntryStyle* m_drawStyle;
};

/// \brief ��ȡָ���˵����Ķ���
VdkMenuBarEntryStyle* GetMenuBarEntryStyle(VdkMenuBarEntryStyleEnum sid);
