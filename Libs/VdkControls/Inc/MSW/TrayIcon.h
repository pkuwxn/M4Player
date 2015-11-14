#pragma once
#include <wx/taskbar.h>

class VdkWindow;

/// \brief ��Ҫ�˵������õ� ID
enum EssentialMenuItems {
	TI_MENU_EXIT = wxID_HIGHEST + 1000,
	TI_FLASH_TIMER
};

/// \brief ��ǰ����ͼ��������״̬
enum TrayIconState {
	TI_STATE_NOMAL, ///< ����
	TI_STATE_HID, ///< ����
	TI_STATE_FLASHING, ///< ��˸
};

/// \brief ԭ�� wxTrayIcon ����չ�࣬�����ͼ����˸������
class TrayIcon : public wxTaskBarIcon
{
public:

	/// \brief ���캯��
	TrayIcon( VdkWindow* frame, 
			  const wxIcon& icon,
			  wxString strTrunkTooltip );

	/// \brief ����������Ĵ���
	void SetMainFrame(VdkWindow* frame);

	/// \brief ���ɵ����˵�
	virtual wxMenu* CreatePopupMenu();

	/// \brief ��ȡͼ����ʾ��Ϣ
	wxString GetToolTip() { return m_strTooltip; }
	/// \brief ����ͼ����ʾ��Ϣ
	void SetToolTip(wxString strTooltip);

	/// \brief ��˸������ͼ��
	void Flash(bool bEnabled = true);
	
	/// \brief �õ���ǰ����ͼ��������״̬��Flashing,etc.��
	TrayIconState GetState() const { return m_state; }

	/// \brief ����/��ʾͼ��
	void Hide(bool bToHide = true);

private:

	void OnLeftDoubleClick(wxTaskBarIconEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnTimer(wxTimerEvent&);
	void OnTimerUser(wxCommandEvent& e);

	//////////////////////////////////////////////////////////////////////////

	VdkWindow*					m_mainFrame;

	NOTIFYICONDATA				m_nid;

	bool						m_bNull; ///<�Ƿ�������ʾ�հ�ͼ�꣨������˸��ԭ��
	TrayIconState				m_state; ///<��ǰ����ͼ��������״̬��Flashing,etc.��

	wxTimer						m_timer; ///<���ֿ�����Ҫ�õ� Timer ����ʾ�������� Timer
	wxString					m_strTrunkTooltip; ///<���ڵ�һ�е���ʾ�����ֶ������ͬʱ���е����

	DECLARE_EVENT_TABLE()
};
