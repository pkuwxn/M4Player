/***************************************************************
 * Name:      OOPWindow.h
 * Purpose:   ���� OOPWindow
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-2-27
 **************************************************************/
#pragma once
#include "VdkWindowImpl.h"

class OOPStickyWindowState;

/// \brief OOPlayer ר�õĿɻ�������
class OOPWindow : public VdkDialog
{
public:

    typedef VdkDialog Super;

	enum {
		/*! Ĭ�� VdkWindow ��� */
		DEFAULT_VSTYLE = VWS_DRAGGABLE | VWIS_DRAG_BY_SPACE,
	};

	/// \brief ���캯��
	OOPWindow(wxWindow* parent, const wxString& uuid, const wxString& title, 
			  long style = 0, long vstyle = DEFAULT_VSTYLE);

	/// \brief ��������
	~OOPWindow();

	/// \brief ��ȡ���ڵ� UUID
	wxString GetUUID() const { return m_uuid; }

	/// \brief ����Ƥ��
	void ResetSkin();

	/// \brief �����������ڵ�ճ��
	void NotifyShowHideAndStick(bool show);

	/// \brief ��ȡ��ǰճ��״̬��Ϣ�������ڳ־û�
	void UpdateStickyState(OOPStickyWindowState& sstate) const;

protected:
	
	/// \brief ���ô������õ�Ƥ��
	void SetSkin();

	/// \brief �󶨡��رա���ť���¼�������
	void BindCloseBtnEvent(const wxString& objName = L"close");

private:

	// �Ķ� XRC ���壬ʹ�ô��ڴ���ʱ��С��Ϊ�ϴιر�ʱ�Ĵ�С
	virtual void HackXrc(wxXmlNode* winRoot);

	// ����Ƥ��ǰ��׼������
	virtual void PreResetSkin() {}

	// ���ô������õ�Ƥ��
	// ���������ʵ�֡�
	virtual void DoSetSkin() = 0;

	// ����Ƥ���¼���Ӧ����
	void OnResetSkin(VdkVObjEvent&);

	//-----------------------------------------

	// �趨�Ƿ�ճ����������
	void SetStickyToMainState(bool sticky);

	// ��ȡ(����������ڵ�)�������������ڳ־û�
	wxRect GetRectToSerialize() const;

	// ճ����������
	void OnAttachToMain(wxCommandEvent&);
	// ������������
	void OnDetachFromMain(wxCommandEvent&);

	//-----------------------------------------

	// ��Ӧԭ�����ڹر��¼�(Alt + F4����)
	void OnCloseNative(wxCloseEvent&);

	// ��Ӧ�رհ�ť�����Ĵ��ڹر��¼�
	void OnClose(VdkVObjEvent& e);

	// �رմ��ڣ�����״̬
	void DoCloseWindow();

private:

	// ��С��(���ص�����)
	virtual void DoMinimize();

private:

	// ��ʶ���ڵ� UUID
	wxString m_uuid;

	// ����ǰ�Ƿ���Ȼճ����������
	bool m_stickyToMain;
	// ����ǰ��Ȼճ����������ʱ��������ڵ�λ��
	wxPoint m_relativePos;
};
