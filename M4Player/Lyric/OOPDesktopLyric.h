/***************************************************************
 * Name:      OOPDesktopLyric.cpp
 * Purpose:   ������(On Screen Display Lyric)
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-02
 **************************************************************/
#pragma once
#include <wx/popupwin.h>
#include <wx/geometry.h> // for wxRect2DDouble

#include "ILyric.h"
#include "OOPLyricParser.h"

typedef wxPopupWindow DeskLrcSuperClass;
class wxGraphicsContext;
class wxGraphicsPath;

/// ������(On Screen Display Lyric)
class OOPDesktopLyric : public DeskLrcSuperClass, public ILyric
{
public:

	/// �����
	struct Style
	{
		wxString fontFace; ///< ʹ�õ�����
		size_t pxFontSize; ///< �����С(����:����px)
		bool bold; ///< �����Ƿ���Ҫ�Ӵ���ʾ

		size_t borderSize; ///< �������ο�ı߿��С
		unsigned char alpha; ///< ����͸����

		/// Ĭ�Ϲ��캯��
		Style();
	};

	/// ���캯��
	OOPDesktopLyric(wxWindow* parent);

	/// ��������
	~OOPDesktopLyric();

	/// ����������ʾ
	bool UpdateAtOnce();

public:

	/// ���º�������ʵ�� ILyric �ӿ�
	virtual bool IsOk() const;
	virtual void AttachParser(const OOPLyricParser& parser);
	virtual void Start();
	virtual void Pause();
	virtual void Stop(wxDC* pDC);
	virtual void ClearLyric(wxDC* pDC);
	virtual void GoTo(double percentage, wxDC* pDC, bool bPaused);

private:

	// ��һЩ��ʼǰ�ĳ�ʼ������
	void Initialize();

	// ��һЩ�˳�ʱ��������
	void Finalize();

	// ��ʼ����ǰ�����ı��е� GDI ·��
	//
	// ������Ҫ�ǽ��ı������ӵ����С�
	wxGraphicsPath InitLineTextPath(wxGraphicsContext* gc);

	// ��ʾ�������ο�
	void ShowBackgound(wxGraphicsContext* gc);

	// ��������������ʾ����
	bool Present(wxDC& drawings);

private:

	// ��ʾ�������ı����
	virtual void DoSetInteractiveOutput(wxDC* pDC);

	// ��ֹ��ʾ��ǰ�У�ֱ����ʾ��һ��
	//
	// ���ڷ�ֹ��ǰ������ʱ�䳬�� LRC ָ����ʱ�䡣
	void NextLine();

	// ���ݶ�ʱ��������ǰ����
	void UpdateProgress();

	// ��ȡ��ǰ����еĽ���(�ٷֱȣ���Χ[0, 1])
	double GetLineProgress() const;

	// ��ǰ����ı����Ƿ���Ч��
	// 
	// ����Ч��Ӧ����ʾ��ǰ�������ı���
	bool IsCurrLineValid() const;

private:

	void OnEnterWindow(wxMouseEvent&);
	void OnLeaveWindow(wxMouseEvent&);

	void OnLeftDown(wxMouseEvent&);

	// wxTimer �ص�
	void OnTimerNotify(wxTimerEvent&);

private:

	Style m_style; // ��ǰ��ʹ�õķ��
	const OOPLyricParser* m_parser;

	LineIter m_currLine; // ��ǰ������
	bool m_showBackgound; // ��ǰ�Ƿ���Ҫ��ʾ�������ο�

	wxRect2DDouble m_textPathBounds; // �ı�·���İ�Χ��

	const wxWindowID m_timerId;
	wxTimer m_timer;

private:

	wxDECLARE_EVENT_TABLE();
};
