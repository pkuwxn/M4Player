/***************************************************************
 * Name:      OOPStopWatch.h
 * Purpose:   ���������ʱ��
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-09
 **************************************************************/
#pragma once

namespace sf {
	class Music;
}

class MusicEndingDetector;

/// ���������ʱ��
class OOPStopWatch
{
public:

	/// ���캯��
	OOPStopWatch(const sf::Music& music, MusicEndingDetector& endingDetector);

	/// Get elapsed time since the last Start() in milliseconds
	size_t Time();

	/// �Ƿ����ڼ�ʱ��û�б���ͣ��
	bool IsRunning() const;

private:

	const sf::Music& m_music;
	MusicEndingDetector& m_endingDectector;
};

/*! ����������˳������֪ͨ�¼� */
wxDECLARE_EVENT( MM_EVT_SONG_FINISHED, wxCommandEvent );

/// ������������¼�̽����
class MusicEndingDetector
{
public:

	/// ���캯��
	MusicEndingDetector(const sf::Music& music);

	/// ��ʼ����
	void Play();

	/// �����ڲ�״̬��ʹ֮����ȷ�����ص�����
	void Update();

	/// ����̽����(�ֶ�ֹͣ����)
	void Stop();

private:

	const sf::Music& m_music;

	// �Ƿ��ѷ��͡�����˳��������ϡ�֪ͨ�¼�
	bool m_finishEventSent;
};
