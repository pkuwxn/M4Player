/***************************************************************
 * Name:      LyricGrabber.cpp
 * Purpose:   歌词下载器
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-25
 **************************************************************/
#pragma once
#include "Types.h"

namespace LyricGrabber {
/*! 歌词（列表）下载操作的结果状态说明 */
enum Result {
    LGR_FAILED, /*! 连接服务器失败 */
    LGR_NO_MATCH, /*! 没有找到匹配的歌词 */
    LGR_OK, /*! 匹配成功 */
};

class LyricHost;

/*! 可选歌词列表已然下载/加载完毕 */
wxDECLARE_EVENT(OOP_EVT_LYRIC_LIST_LOADED, wxCommandEvent);

/*! 歌词已然下载/加载完毕 */
wxDECLARE_EVENT(OOP_EVT_LYRIC_LOADED, wxCommandEvent);

/// 从网络上下载歌词时需要提供的完整信息
class Task {
public:

    /// 构造函数
    Task(wxEvtHandler *sinker);

    /// 析构函数
    ~Task();

    /*! 新的歌词下载会话类型 */
    enum SessionType {
        /*! 自动下载搜索结果中的第一项，毋须用户干预 */
        ST_AUTO_SELECT_AND_DOWNLOAD,
        ST_RETURN_LIST, ///< 简单返回所有搜索结果，由用户手动选择要下载的项
    };

    /// 以异步的方式下载搜索结果列表
    void GrabbList(SessionType st);

    /// 以异步的方式下载序号为@a serialNumber 的项
    void GrabbLyric(int serialNumber);

    /// 获取下载成功的歌词列表
    const ResultSet &GetList() const {
        return m_result;
    }

public:

    /// 存取使用的歌词服务器
    int GetHost() const {
        return m_hostId;
    }
    void SetHost(int host);

    /// 存取歌词下载会话类型
    SessionType GetSessionType() const {
        return m_sessionType;
    }
    void SetSessionType(SessionType st) {
        m_sessionType = st;
    }

    /// 存取歌曲路径
    wxString GetPath() const {
        return m_path;
    }
    void SetPath(const wxString &path) {
        m_path = path;
    }

    /// 存取艺术家
    wxString GetArtist() const {
        return m_artist;
    }
    void SetArtist(const wxString &artist) {
        m_artist = artist;
    }

    /// 存取歌曲标题
    wxString GetTitle() const {
        return m_title;
    }
    void SetTitle(const wxString &title) {
        m_title = title;
    }

    /// 存取歌曲长度
    int GetLength() const {
        return m_length;
    }
    void SetLength(int length) {
        m_length = length;
    }

    /// 获取歌词异步下载完毕后发送消息时的接收者
    wxEvtHandler *GetEventSinker() const {
        return m_sinker;
    }

    /// 获取通知事件标识 ID
    int GetListLoadedEventId() const;

    /// 获取单首歌词下载完成通知事件 ID
    int GetLyricLoadedEvtId() const;

private:

    // 删除当前歌词服务器对象
    void DestoryHost();

    // 歌词列表下载完成
    void OnListLoaded(wxCommandEvent &e);

    // 歌词列表下载完成
    void OnLyricLoaded(wxCommandEvent &e);

private:

    int m_hostId; // 使用的歌词服务器
    LyricHost *m_host;

    SessionType m_sessionType;

    wxString m_path; // 歌曲路径
    wxString m_artist; // 艺术家
    wxString m_title; // 标题
    int m_length; // 歌曲长度

    int m_idListLoaded; // 本次下载任务的标识符
    int m_idLyricLoaded;

    wxEvtHandler *m_sinker; // 歌词异步下载完毕后发送消息时的接收者

    ResultSet m_result;
};

/// 将歌词内容保存到文件里(使用 UTF-8 BOM 编码)
void SaveLrcFile(const wxString &lyric, const wxString &fileName);

}
