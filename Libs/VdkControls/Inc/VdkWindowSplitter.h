#pragma once
#include "VdkControl.h"

/// \brief �����������ʹ�����ؼ��ܻ�������
///
/// �϶������ؼ��м��ĳ������ʹ��ı��С���������⡣
class VdkWindowSplitter : public VdkControl
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkWindowSplitter(){ Init(); }

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ִ��ʵ�ʹ�������
	///
	/// һ�����õ�����ǽ������������Ϊ�����ؼ�֮һ���ӿؼ���
	/// ���������������������һ���ؼ��ı��� VdkWindow �ϵ�������
	/// ����ơ�
	/// \param parent ������ VdkWindow
	/// \param strName �ؼ���ʶ����
	/// \param rc �ؼ�������
	/// \param win1 ��һ���ؼ������/�ϱߣ�
	/// \param win2 �ڶ����ؼ����ұ�/�±ߣ�
	/// \param bVertical �����ؼ�������˳������/���£�
	/// \attention \a win1 �� \a win2 �ĸ��ؼ�������ͬ�Ҳ���Ϊ�ա�
	void Create( VdkWindow* parent, 
				 const wxString& strName, 
				 const wxRect& rc, 
				 VdkControl* win1, 
				 VdkControl* win2, 
				 bool bVertical = true );

	/// \brief �����ؼ�����ʱ��С����Ĵ�С
	void SetMinSize(int min){ m_min = min; }

private:

	// ��ʼ���ؼ�
	void Init();

	// ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	VdkControl*				m_win1;
	VdkControl*				m_win2;

	int						m_mousePos;
	int						m_lastX;
	int						m_lastY;

	int						m_min;

	bool					m_bVertical;

	DECLARE_DYNAMIC_VOBJECT
};
