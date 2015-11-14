/***************************************************************
 * Name:      Playlist.h
 * Purpose:   �����б�ʵ����
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-04-19
 **************************************************************/
#pragma once
#include "VdkListCtrl.h" // for LcDataSet

/// �����б�ʵ����
class Playlist : public LcDataSet
{
public:

	/// ��ȡ�������
	static Playlist& GetInstance();

private:

	Playlist() {}
	~Playlist() {}
	Playlist(const Playlist&) {}
};
