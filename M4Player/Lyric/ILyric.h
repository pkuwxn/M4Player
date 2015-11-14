/***************************************************************
 * Name:      ILyric.h
 * Purpose:   �����ĳ���ӿ�
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-05
 **************************************************************/
#pragma once

class OOPStopWatch;
class OOPLyricParser;

/// �����ʵ��
/// 
/// û��������������������ͨ�� ILyric ָ����ɾ������
class ILyric
{
public:

	/// Ĭ�Ϲ��캯��
	ILyric();

	/// �Ƿ���Խ�����ʾ
	virtual bool IsOk() const = 0;

	/// �趨��ǰ�����ļ�ʱ��
	virtual void SetStopWatch(OOPStopWatch& sw) { m_stopWatch = &sw; }

	/// �󶨸���ļ�����������ʼ���б�ؼ�
	virtual void AttachParser(const OOPLyricParser& parser) = 0;

	/// ��ʼ�����ʾ
	///
	/// @see SetStopWatch, AttachParser
	virtual void Start() = 0;

	/// ��ͣ�����ʾ
	virtual void Pause() = 0;

	/// ֹͣ�����ʾ
	virtual void Stop(wxDC* pDC) = 0;

	/// �����ǰ���
	virtual void ClearLyric(wxDC* pDC) = 0;

	/// ǰ��ĳ����(0 <= \a percentage <= 1)
	///
	/// ���������������Ϊ wxTimer::Notify() �в�����ֳ���һ�еĿ�Խ��
	/// ������ƶ�ʱ���ܻ���ֲ��Ϲ�������������Ҳ�����ȷ������ǰ��Ծ
	/// ʱ�������
	virtual void GoTo(double percentage, wxDC* pDC, bool bPaused) = 0;

	/// \brief ����Ĭ�ϵĵĽ����Ը�ʾ�ı�
	void SetDefualtInteractiveOutput(const wxString& msg);

	/// \brief ���õ����Ϊ��ʱ����ʾ��һС�ν����Ը�ʾ�ı�
	void SetInteractiveOutput(const wxString& msg, wxDC* pDC);

	/// \brief ����ΪĬ�ϵĽ����Ը�ʾ�ı�
	void ResetInteractiveOutput(wxDC* pDC);

protected:

	/// \brief ��ȡ��ǰ��Ч�Ľ����Ը�ʾ�ı�
	wxString GetInteractiveOutput() const;

	enum {
		/*! ���ˢ��Ƶ��(��λ��ms)������ʹ�ÿ��� OK ��ʽ��ʾ�ĸ�ʣ��Լ�ʹ�ý���ɫ
		 *  ��ԭ��һ����
		 */
		REFRESH_INTERVAL_MS = 40,
	};

private:

	/// \brief ֪Ϥ�����Ը�ʾ�ı��ѱ��ı�
	virtual void DoSetInteractiveOutput(wxDC* pDC) = 0;

	/// \brief ֪Ϥ�ⲿ���ż�ʱ���ѱ��趨
	virtual void DoSetStopWatch(OOPStopWatch& sw) {}

protected:

	// �ⲿ���ż�ʱ������ȷͬ�������ʾ
	OOPStopWatch* m_stopWatch;

private:

	// �����Ϊ��ʱ����ʾһС�ν����Ը�ʾ�ı�
	wxString m_defaultInteractiveOutput;
	wxString m_interactiveOutput;
};
