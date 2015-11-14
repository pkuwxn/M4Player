#pragma once
#include "VdkControl.h"

class OOPStopWatch;

/// \brief ��ʾ�Ѳ���ʱ���ģ�� LED ���ȱ�
class OOPLed : public VdkControl, public wxTimer
{
public:

	/// \brief Ĭ�Ϲ��캯��
	OOPLed();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief 
	void Create(VdkWindow* Window, 
			    const wxString& strName, 
			    const wxRect& rc,
				const wxBitmap& bmLed,
				const align_type& align);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ǰ�����ļ�ʱ��
	void SetStopWatch(OOPStopWatch* sw) { m_stopWatch = sw; }

	/// \brief ��ʼ��ʾ
	void StartLed();

	/// \brief ��ͣ��ʾ
	void PauseLed();

	/// \brief ������ʾ
	void ResumeLed();

	/// \brief ֹͣ��ʾ
	void StopLed(wxDC* pDC);

	/// \brief ��ʾĳ��ȷ����ʱ��
	/// \param nTimeInMS Ҫ��ʾ��ʱ�䣬�Ժ���(MS)����
	void Combine(long nTimeInMS, wxDC& pDC);

private:

	// ��ʼ���ؼ�
	void Init();

	// ��ʼ��ʾ
	void DoStart();

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	// �̳��� wxTimer
	virtual void Notify();

	// ��ȡ��ǰʱ����� gs_Internal ������
	//
	// ��Ϊ OOPLed ���� 1s Ϊ����������ʾ���ʵ��� 1s �м������ͣ
	// ��ֱ�ӻָ������� < 1s ���֣��ͻ���ɽϴ���
	int GetRestTimeForThisSecond();

private:

	wxBitmap			m_bmAll;
	
	int					m_nElWidth;		// Ԫ�ؿ��
	int					m_nElHeight;	// Ԫ�ظ߶�

	int					m_nPos[4];		// �ĸ�Ԫ������� m_Rect.x ��ƫ��

	OOPStopWatch*		m_stopWatch;	// �� OOPLyric::m_stopWatch

	DECLARE_DYNAMIC_VOBJECT
};
