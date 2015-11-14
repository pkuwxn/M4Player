#pragma once
#include "VdkControl.h"
#include <wx/vector.h>

//////////////////////////////////////////////////////////////////////////
/// \brief �����ı���ʾ
class OOPLabel : public VdkControl, public wxTimer
{
public:

	/// \brief Ĭ�Ϲ��캯��
	OOPLabel();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ʵ�ʹ�������
	void Create( VdkWindow* Window, 
				 const wxString& strName, 
				 const wxRect& Rect, 
				 const wxFont& font,
				 const wxColour& color,
				 const wxColour& bgColor,
				 const align_type& align );

	/// \brief ��������
	~OOPLabel();

	/// \brief �����ı���
	void SetItems(const wxArrayString& items, wxDC* pDC);

	/// \brief �Ƿ�����ı��ؼ���Ч
	bool IsOk() const { return !m_items.empty(); }

	/// \brief ��ʼ����
	void StartRolling();

	/// \brief ֹͣ���������ÿؼ�״̬��ɾ��һ����Ϣ
	void StopRolling(wxDC* pDC);

private:

	// ��ʼ���ؼ�
	void Init();

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	// ���Ʊ���
	void DrawLabel( const wxString& str, 
					int x, 
					int y, 
				    const wxString& str2 = wxEmptyString, 
				    int y2 = 0 );

	// �̳��� wxTimer
	virtual void Notify();

	//////////////////////////////////////////////////////////////////////////

	enum TimerNotityType {

		TNT_LINE_SWITCH, 
		TNT_LINE_ROLLING, 
		TNT_WAIT_FOR_SWITCHING
	};
	
	wxColour					m_TextColor;		// �ı���ɫ
	wxColour					m_BkGndColor;		// �ı���ɫ
	wxString					m_staticCaption;	// ֹͣ����ʱ��ʾ���ı�

	int							m_nOnShowId;		// ������ʾ����Ŀ ID
	TimerNotityType				m_nNotityType;		// Timer ֪ͨ������

	int							m_nTextInternal;	// �ı��� OOPLabel �����ľ��루ʹ֮���У�
	int							m_nRollingCount;	// ��ǰ�ı����ϻ������ҹ����Ĵ���
	wxDirection					m_Direction;		// ��ǰ�����ķ���
	wxStopWatch					m_StopWatch;		// �л����¾��ʱ��

	//////////////////////////////////////////////////////////////////////////

	struct ItemNode {

		int nTextWidth;			// �ı��ĳ���
		int nDelta;				// �ı����Ⱥ���ʾ���򳤶����Ĵ�С
		wxString strItem;		// Ҫ��ʾ���ı�
	};

	typedef wxVector< ItemNode > ArrayOfItemNodes;
	ArrayOfItemNodes			m_items;			// ������ʾ���ı�����

	DECLARE_DYNAMIC_VOBJECT
};
