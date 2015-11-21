/***************************************************************
 * Name:      Playlist.h
 * Purpose:   指向 OOPSong 的智能指针
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-05-29
 **************************************************************/
#pragma once
#include <wx/sharedptr.h> // for wxSharedPtr
#include "OOPSong.h" // OOPSong dtor

/// 指向 OOPSong 的智能指针
///
/// 发觉扫描添加 1500+ 个文件时内存占用回去到 100M+，是不是 OOPSong 对象惹的祸呢？
/// 下面试试。
class OOPSongPtr {
public:

    /// (默认)构造函数
    explicit OOPSongPtr(OOPSong *song = NULL);

    /// 复制构造函数
    OOPSongPtr(const OOPSongPtr &other);

    /// 析构函数
    ~OOPSongPtr();

    /// 重载赋值操作符
    OOPSongPtr &operator=(const OOPSongPtr &other);

    /// 重载指针操作符
    OOPSong *operator->() const;

    /// 重载取值操作符
    OOPSong &operator*();

    /// 重载取值操作符
    const OOPSong &operator*() const;

    /// 有效性测试
    operator bool() const {
        return (m_song.get() != NULL);
    }

    /// 获取内部指针
    OOPSong *get() const {
        return m_song.get();
    }

    /// 将指针重置为@a ptr
    void reset(OOPSong *ptr = NULL) {
        m_song.reset(ptr);
    }

private:

    wxSharedPtr<OOPSong> m_song;
};

/// 特化 wxSharedPtr<OOPSong> 相等性测试
template <>
bool operator == (wxSharedPtr<OOPSong> const &a, wxSharedPtr<OOPSong> const &b);
