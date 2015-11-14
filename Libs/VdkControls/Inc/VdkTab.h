#pragma once
#include "VdkControl.h"

class VdkToggleButton;
class VdkPanel;

/// \brief VdkTab �������
enum VdkTabStyle {

	/// \brief ��������λ�������ʼλ���
	VTABS_USER					= ( VCS_USER + 5 ),

	/// \brief �Ƿ����ѡ��߿�
	VTABS_TAB_BORDER			= 1 << ( VCS_USER + 0 ),
};

/*!\brief ѡ�
 *
 * һ��ʱ��ֻ����һ��ҳ�汻��������������ص�״̬��
**/
class VdkTab : public VdkCtrlHandler, public VdkCtrlParserCallback
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkTab();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ��������
	~VdkTab();

	/// \brief ���һ���ؼ�����\a page ��ҳ��
	/// 
	/// ͬʱ�Ὣ�ؼ���ӵ�ҳ�����( \link VdkPanel \endlink )�Ŀؼ��б��С�
	void AddCtrlToPage(int page, VdkControl* pCtrl);

	/// \brief ���һ���µ�ҳ��
	/// 
	/// \attention ��ǰѡ���������ҳ����ڡ�
	/// \return ������ҳ��ľ����
	VdkPanel* AddPage(const wxString& caption);

	/// \brief �Ƴ���\a id ��ҳ��
	void RemovePage(int id, wxDC* pDC);

	/// \brief �л�����\a id ��ҳ��
	void Toggle(int id, wxDC* pDC);

	/// \brief �õ�ҳ����Ŀ
	int GetPageCount() const;

	/// \brief ��ȡ��ǰѡ�е�ҳ��
	int GetValue() const;

	/// \brief �ص�֪ͨ����
	///
	/// ���� \link VdkCtrlParserCallback \endlink
	virtual void Notify(VdkWindow* win, 
						const wxString& clName,
						VdkControl* pCtrl);

private:

	// ��������İ�ť�õ�ҳ�� ID
	int GetPageIdFromHandle(VdkToggleButton* pCtrl) const;

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	// ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	// ��������ť���¼�������
	void OnToolBarEvents(VdkVObjEvent&);

	// ���ͱ�׼ wx �¼�
	void SendWxEvent(int index, int lastindex) const;

	//////////////////////////////////////////////////////////////////////////

	// ѡ���Ϣ����
	struct PageInfo {

        VdkToggleButton* btn; // ������İ�ť
        VdkPanel* panel; // �������ҳ��
    };

	wxVector<PageInfo *> m_pages; // ����ҳ��

	//////////////////////////////////////////////////////////////////////////

	int m_padding; // ��ť֮��ļ�϶

	// ��ǰ����İ�ť
	VdkToggleButton* m_currToggled;
	
	DECLARE_DYNAMIC_VOBJECT
};

//////////////////////////////////////////////////////////////////////////
/// \brief VdkTab ר���¼�֪ͨ��
class VdkTabEvent : public VdkVObjEvent
{
public:

	/// \brief ���캯��
	VdkTabEvent(VdkTab* tab, int index, int lastindex);

private:

	VdkTab* m_tab;

	int m_index;
	int m_lastindex;
};
