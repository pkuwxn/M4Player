/***************************************************************
 * Name:      Playlist.h
 * Purpose:   播放列表实体类
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-04-19
 **************************************************************/
#pragma once
#include "VdkListCtrl.h" // for LcDataSet

/// 播放列表实体类
class Playlist : public LcDataSet
{
public:

	/// 获取单体对象
	static Playlist& GetInstance();

private:

	Playlist() {}
	~Playlist() {}
	Playlist(const Playlist&) {}
};
