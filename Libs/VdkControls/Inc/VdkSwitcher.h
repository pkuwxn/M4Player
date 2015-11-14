#pragma once
#include "VdkControl.h"
#include <wx/hashmap.h>

class VdkVObjEvent;
class VdkToggleButton;
/// \brief �����ϰ� QQ ����ϵ�˷����л��б�
class VdkSwitcher : public VdkCtrlHandler
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkSwitcher(){ Init(); }

	/// \brief ���캯��
	VdkSwitcher( VdkWindow* parent,
			     const wxString& strName,
			     const wxString& strFileName,
			     const wxRect& rt,
				 int nExpl = 3 );

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ʵ�ʹ�������
	void Create( VdkWindow* win,
				 const wxString& strName,
				 const wxRect& rc,
				 const wxBitmap& bmp,
				 int nExpl );

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���� VdkSwitcher
	void Disable(bool bDisabled, wxDC* pDC);

	/// \brief �л���ָ����
	void Switch(int id, wxDC* pDC);

	/// \brief �л���ָ����
	void Switch(VdkToggleButton* btn, wxDC* pDC);

	/// \brief ��ȡ��ǰ���ѡ��ҳ
	int GetCurrentId() const { return m_curr; }

	/// \brief ���ѡ��ҳ
	void AddTab(const wxString& strCaption, VdkControl* win);

	/// \brief ��ȡ�л����Ĵ�С
	int GetTabHeight() const { return m_h; };

private:

	/// \brief ��ʼ���ؼ�
	void Init();

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

	/// \brief �л���ǩҳʱ��
	void OnSwitchTabs(VdkVObjEvent&);

	void WalkCoords(int id);
	void ResetCoords(int id);

	//////////////////////////////////////////////////////////////////////////

    struct TabInfo {

        int							id;
        wxString					strCaption;
        VdkToggleButton*			button;
        VdkControl*					win;
        wxRect						rc;
    };

	typedef VdkToggleButton* BtnPtr;
	WX_DECLARE_VOIDPTR_HASH_MAP( TabInfo, tab_map );

	wxBitmap						m_bmp;

	int								m_h;
	int								m_nExpl;

	tab_map							m_tabs;
	BtnPtr							m_last;

	int								m_curr;

	DECLARE_DYNAMIC_VOBJECT
};
