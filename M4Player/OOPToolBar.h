#pragma once
#include "VdkMenuBar.h"

/// \brief OOPlayer �����б�������˵�������
class OOPToolBar : public VdkMenuBar
{
public:

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

private:

	/// \brief ���һ��
	virtual VdkMenuBarEntry* CreateNewEntry(int& w, int& h);

	/// \brief ���Ʋ˵��ǰ״̬
	void UpdateEntry(wxDC& dc, unsigned eid, int state);

private:

	VdkBitmapArray	m_normal;
	VdkBitmapArray	m_hot;

	friend class OOPToolBarEntry;

	DECLARE_DYNAMIC_VOBJECT
};

/// \brief OOPlayer �����б�������˵���������һ��
class OOPToolBarEntry : public VdkMenuBarEntry
{
public:

	/// \brief Ĭ�Ϲ��캯��
	OOPToolBarEntry();

private:

	/// \brief ���������ƣ�����
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);
};
