/***************************************************************
 * Name:      OOPLyricMgr.h
 * Purpose:   �����ʾ�ؼ�������
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-05
 **************************************************************/
#pragma once
#include <wx/vector.h>

class ILyric;
class OOPStopWatch;
class OOPLyricParser;

/// �����ʾ�ؼ�������
class OOPLyricMgr
{
public:

	/// ���캯��
	OOPLyricMgr();

	/// ��������
	~OOPLyricMgr();

	/// ���һ���µĿؼ�
	void Attach(ILyric* lyric);

	/// ɾȥһ��ԭ�пؼ�
	void Dettach(ILyric* lyric);

	/// ���ز�������ת��������
	operator bool() const;

	/// ����ָ�������
	OOPLyricMgr* operator->() { return this; }
	const OOPLyricMgr* operator->() const { return this; }

public:

	/// ���º����� ILyric �Ľӿ���ͬ

	bool IsOk() const;
	void SetStopWatch(OOPStopWatch& sw);
	void AttachParser(const OOPLyricParser& parser);
	void Start();
	void Pause();
	void Stop(wxDC* pDC);
	void ClearLyric(wxDC* pDC);
	void GoTo(double percentage, wxDC* pDC, bool bPaused);

	void SetDefualtInteractiveOutput(const wxString& msg);
	void SetInteractiveOutput(const wxString& msg, wxDC* pDC);
	void ResetInteractiveOutput(wxDC* pDC);

private:

	typedef wxVector< ILyric* > LyricVec;
	// ��ǰ���ڻ�ĸ����ؼ�
	LyricVec m_lyrics;
};
