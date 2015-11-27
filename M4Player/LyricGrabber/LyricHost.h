/***************************************************************
 * Name:      LyricHost.cpp
 * Purpose:   歌词服务器
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-25
 **************************************************************/
#pragma once
#include "Types.h"

namespace LyricGrabber {
class Task;

/// 歌词服务器
class LyricHost {
public:

    /// 虚析构函数
    virtual ~LyricHost();

    /// 下载歌词列表
    /// @param task 下载任务信息
    /// @return 歌词列表 URL
    wxString PrepareForTask(const Task &task);

    /// 以异步的方式下载地址为@url 的歌词
    void GrabbLyric(const wxString &url);

    /// 获取歌词服务器对象（工厂函数）
    static LyricHost *Create(int hostId);

public:

    // 解析得到的歌词列表
    // @param list 获得的原始歌词列表字符串
    // @param rs 输出歌词搜索结果列表，容器会首先被清空
    virtual bool ParseList(const wxString &list, ResultSet &rs) const = 0;

    // 解析得到的原始歌词文本
    virtual bool ParseLyric(wxString &lyric) const {
        return true;
    }

protected:

    /// 获取下载歌词列表
    LyricHost(const wxString &listUrlTemplate);

    /// 设置查询 URL 转义的目的编码
    void SetUrlEscapeDestCharset(const wxCSConv &conv);

    //////////////////////////////////////////////////////////////////////////
    // 下面两个函数是不得已而为之的，因为有些歌词服务器返回的内容中没有
    // 这两项信息，所以还是要保留这些状态信息
    // TODO: 在等待数据返回的时候用户提交了一次新的搜索会怎么样？

    /// 获取与本次下载任务相关的艺术家
    wxString GetArtist() const {
        return m_artist;
    }

    /// 获取与本次下载任务相关的歌曲标题
    wxString GetTitle() const {
        return m_title;
    }

private:

    // 下载歌词列表时的 URL 模板
    wxString m_listUrlTemplate;
    // 查询 URL 转义的目的编码
    wxCSConv m_escapeTo;

    wxString m_artist;
    wxString m_title;
};

}
