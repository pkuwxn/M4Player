/***************************************************************
 * Name:      Playlist.h
 * Purpose:   ָ�� OOPSong ������ָ��
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-05-29
 **************************************************************/
#pragma once
#include <wx/sharedptr.h> // for wxSharedPtr
#include "OOPSong.h" // OOPSong dtor

/// ָ�� OOPSong ������ָ��
///
/// ����ɨ����� 1500+ ���ļ�ʱ�ڴ�ռ�û�ȥ�� 100M+���ǲ��� OOPSong �����ǵĻ��أ�
/// �������ԡ�
class OOPSongPtr
{
public:

	/// (Ĭ��)���캯��
	explicit OOPSongPtr(OOPSong* song = NULL);

	/// ���ƹ��캯��
	OOPSongPtr(const OOPSongPtr& other);

	/// ��������
	~OOPSongPtr();

	/// ���ظ�ֵ������
	OOPSongPtr& operator=(const OOPSongPtr& other);

	/// ����ָ�������
	OOPSong* operator->() const;

	/// ����ȡֵ������
	OOPSong& operator*();

	/// ����ȡֵ������
	const OOPSong& operator*() const;

	/// ��Ч�Բ���
	operator bool() const { return (m_song.get() != NULL); }

	/// ��ȡ�ڲ�ָ��
	OOPSong* get() const { return m_song.get(); }

	/// ��ָ������Ϊ@a ptr 
	void reset(OOPSong* ptr = NULL) { m_song.reset( ptr ); }

private:

	wxSharedPtr<OOPSong> m_song;
};

/// �ػ� wxSharedPtr<OOPSong> ����Բ���
template <>
bool operator == (wxSharedPtr<OOPSong> const &a, wxSharedPtr<OOPSong> const &b);
