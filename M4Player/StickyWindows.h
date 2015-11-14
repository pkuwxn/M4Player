/***************************************************************
 * Name:      StickyWindows.h
 * Purpose:   WinAMP ���ճ������ʵ��
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-03-07
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkEvent.h"
#include <wx/vector.h>

/*! �Ӵ���ճ���������� */
wxDECLARE_EVENT( wxEVT_STICKY_WINDOW_ATTACHED, wxCommandEvent );
/*! �Ӵ��ڴ����������� */
wxDECLARE_EVENT( wxEVT_STICKY_WINDOW_DETACHED, wxCommandEvent );

/// \brief WinAMP ���ճ������ʵ��
class StickyWindows : public VdkEventFilter
{
public:

	/// \brief ���캯��
	StickyWindows(int criticalDistance = 10);

	/// \brief ���һ���ܹ���Ĵ���
	void AddWindow(VdkWindow* win);

	/// \brief ����������
	///
	/// �Ƿ������ڣ��������϶�������ʱ��֮ճ�����Ӵ��ڻ�һ��������
	void SetMainWindow(VdkWindow* win);

	/// \brief �����Ӵ����Ƿ���Ȼ𤸽��������
	bool IsStickyToMain(VdkWindow* chd);

	/// \brief ���µ�ǰ����������ճ�����Ӵ���
	/// \param rcMain �����ڵ���Ļ������
	void UpdateStickyChildren(const wxRect& rcMain);

	/// \brief �Ƴ�һ���ܹ���Ĵ���
	void RemoveWindow(VdkWindow* win);

private:

	// ���ط��͸� VdkWindow ���¼�
	virtual bool FilterEvent(const EventForFiltering& e);

	// �����ƶ��Ӵ����¼����������ƶ��¼�������Ϊ LinklyMove() ��
	bool HandleChildMove(VdkWindow* win, const wxPoint& mousePosClient);

	// ����ı䴰�ڴ�С
	bool HandleResize(VdkWindow* win, const wxPoint& mousePosClient);

	// �������ƶ�֮ǰ����ճ���Ӵ���
	void OnMainLeftDown(wxMouseEvent&);

	// �����Ӵ�������
	void LinklyMove(wxMoveEvent& e);

private:

	// �����Ӵ����Ƿ�ֱ�ӻ��߼�ӵ�𤸽��������
	//
	// ��ν��ӣ���ͨ��һ����ճ���������ڵĴ����Ӵ��ڣ��ֵ��Ӵ��ڣ�\n
	// ��������ճ����
	// @param rcMain ����������Ļ�ϵ�������
	bool RecalcLinkageToMain(const wxRect& rcMain, VdkWindow* chd);

	// ���\a moving �Ƿ�ȷʵճ����\a still ��ĳ��������
	bool ReallySticky(const wxRect& moving, const wxRect& still);

	// ���¼����������ڵ�ճ������
	// 
	// ͬʱ�����ڲ�״̬��Ϣ��
	void UpdateStickyToMainState(const wxRect& rcMain, VdkWindow* win);

	// ����\a rcThis ��\a rcAgence �Ŀ�ճ��״̬����\a rcThis
	bool TryStickToAgent(wxRect& rcThis, const wxRect& rcAgent);

private:

	// ����ճ�����ٽ�ֵ
	// ���ﲻ���� unsigned ���ͣ���Ϊ std::abs() ���� int ������
	// �ᵼ�´�������
	int m_criticalDistance;

	VdkWindowList m_windows;
	// �����ڵ�����
	VdkWindowIter m_main;

	// ճ�����Ӵ��ڵ������Ϣ
	class StickyInfo {
	public:

		VdkWindow* win;

		wxCoord offsetX; // ���������� X ���ϵľ���
		wxCoord offsetY; // ���������� Y ���ϵľ���

		bool operator == (const StickyInfo& rhs) {
			return win == rhs.win;
		}
	};

	typedef wxVector< StickyInfo >::iterator InfoIter;
	typedef wxVector< StickyInfo >::const_iterator InfoIter_Const;
	wxVector< StickyInfo > m_stickyToMain; // �ĸ�������ճ���������ڵ��Ӵ���

	// �����Ӵ����Ƿ���Ȼ𤸽
	InfoIter FindStickyChild(VdkWindow* chd);

	// �Ƴ�𤸽���Ӵ���
	void RemoveChild(VdkWindow* chd);
};
