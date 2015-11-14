/***************************************************************
 * Name:      OOPLyric.h
 * Purpose:   �����ڸ���㴰�ڵ���Ƕ�����ʾ�ؼ�
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2010
 **************************************************************/
#pragma once
#include "VdkListCtrl.h"
#include "OOPLyricParser.h" // for LineIter
#include "ILyric.h"

class OOPStopWatch;
class VdkWindow;

/// \brief �����ʵ��
class OOPLyric : public VdkListCtrl, public ILyric
{
public:

	/// \brief ���캯��
	OOPLyric();

	/// \brief ��������
	~OOPLyric();

	/// \brief ���º�������ʵ�� ILyric �ӿ�
	virtual bool IsOk() const;
	virtual void AttachParser(const OOPLyricParser& parser);
	virtual void Start();
	virtual void Pause();
	virtual void Stop(wxDC* pDC);
	virtual void ClearLyric(wxDC* pDC);
	virtual void GoTo(double percentage, wxDC* pDC, bool bPaused);

private:

	// ��ʼ��
	void Init();

	// XRC ��̬����
	virtual void OnXrcCreate(wxXmlNode* node);

	// ��һЩ����Ĺ�������
	// @param colorArray ���ƿؼ�ʱ����Ԫ�ص���ɫ�����У�
	//    [0]: �����ı���ɫ
	//    [1]: ����ʱ���ı���ɫ
	//    [2]: ������ɫ
	void Create(const wxArrayString& colorArray);

private:

	// ����ͣ�лָ�
	// @return �Ƿ�ɹ��ָ���ʵ���ʾ��
	// @attention ��ʻ�û��ʼ��ʾ�������ʧ�ܵ������
	bool Resume();

	// ���ݶ�ʱ��������ǰ����
	void UpdateProgress(wxDC* pDC);

	// ��ֹ��ʾ��ǰ�У�ֱ����ʾ��һ��
	//
	// ���ڷ�ֹ��ǰ������ʱ�䳬�� LRC ָ����ʱ�䡣
	void NextLine(wxDC* pDC, bool bPaused = false);

	// ת�����
	void MoveToEnd(wxDC* pDC);

	// Ϊʹ��ǰ����ʾ�ڿؼ��м䣬����ڸ��ǰ������˿���
	// 
	// �������������⻭���Ĵ�С
	void InsertBlankLines();

	// ���������к��������⻭���ĸ߶�
	void UpdateVirtualHeight();

	// ���� m_nCurrIndex �����������ڵ���ʼ����λ��
	// ���Զ�����ʱ���ƶ���ָ���ٷֱ���Ҫʵ�ʸ��Ĺ������ڵ�
	// �������ԡ�
	// ��֮��һ��ԭ���µ�һ�п�ʼʱ�����ϸ��ֻ��ʾ��һ���С�
	void CorrectViewStart(wxDC* pDC);

	// ˢ�µ�ǰ��ʵ�״̬
	// 
	// ����¹������ڵ�������ʼ���꣬������\a bPaused ��ֵ�Ƿ�
	// ���µ�λ�ü�����ʾ��ʡ�
	void RefreshLyric(wxDC* pDC, bool bPaused);

	// ��ȡ��ǰ����еĽ���(�ٷֱȣ���Χ[0, 1])
	double GetLineProgress() const;

private:

	virtual void DoSetInteractiveOutput(wxDC* pDC);

	// ��յ�ǰ��ʾ�ĸ��
	virtual void DoClear(wxDC* pDC);

	// ��������¼�
	virtual void OnMouseEvent(VdkMouseEvent& e);

	// ��������¼�
	virtual void OnKeyEvent(VdkKeyEvent& vke);

	// ���ƶ����͵ײ�����ɫ�ı�
	virtual void OnDraw(wxDC& dc);

	// ���ա�����֪ͨ��Ϣ
	virtual void OnNotify(const VdkNotify& notice);

	// �ڴ˻���ÿһ����Ԫ��
	virtual VdkCusdrawReturnFlag DoDrawCellText(const VdkLcCell* cell, 
												int col_index, int index, 
												wxDC& dc, 
												VdkLcHilightState state);

	// ���ڲ��ɼ�ʱֹͣ�����ʾ
	void OnParentShow(wxShowEvent& e);

	// wxTimer �ص�
	void OnTimerNotify(wxTimerEvent&);

private:

	LcDataSet m_dataSet;
	const OOPLyricParser* m_parser;

	const wxWindowID m_timerId;
	wxTimer m_timer;

	wxColour m_TextColor;
	wxColour m_HilightColor;
	wxColour m_BgColor;

	LineIter m_currLine; // ��ǰ����

	int m_blankLinesTop; // �б�ͷ��ǰ�ӽ�ȥ�Ŀ�����
	int m_blankLinesBottom;

	// �������ʱ���ָ����ʶ����ľ��루״̬����
	int m_draggDistance;
	LineIter m_draggHit; // �������ʱ���ָ�����еĸ����

	DECLARE_DYNAMIC_VOBJECT
};
