/***************************************************************
 * Name:      LyricHost.cpp
 * Purpose:   歌词服务器
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-03-25
 **************************************************************/
#include "StdAfx.h"
#include "LyricHost.h"

#include "LyricGrabber.h"
#include "wxCharsetHelper.h" // for wxCharsetHelper::Escape()

#include "wxUtil.h" // for XML parsing

#include <wx/xml/xml.h>
#include <wx/sstream.h> // for XML parsing
#include <wx/tokenzr.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

namespace LyricGrabber {

LyricHost::LyricHost(const wxString &listUrlTemplate,
                     wxFontEncoding urlEncoding,
                     wxFontEncoding lyricEncoding)
    : m_listUrlTemplate(listUrlTemplate),
      m_urlConv(urlEncoding),
      m_lyricEncoding(lyricEncoding) {

}

LyricHost::~LyricHost() {

}

void LyricHost::SetUrlEscapeDestCharset(const wxCSConv &conv) {
    m_urlConv = conv;
}

wxString LyricHost::PrepareForTask(const Task &task) {
    wxString url(m_listUrlTemplate);
    wxString escaped;

    m_artist.assign(task.GetArtist());
    escaped.assign(wxCharsetHelper::Escape(m_artist, m_urlConv));
    url.Replace(L"{artist}", escaped);

    m_title.assign(task.GetTitle());
    escaped.assign(wxCharsetHelper::Escape(m_title, m_urlConv));
    url.Replace(L"{title}", escaped);

    return url;
}

//////////////////////////////////////////////////////////////////////////

/*! 可选的歌词下载服务器 */
enum HostId {
    LH_M4PLAYER, /*! 自建歌词服务器 */
    LH_BAIDU_ZHANGMEN, /*! 百度音乐搜索 */
    LH_QQ_MUSIC, /*! QQ 音乐 */
    LH_TTPLAYER, /*! 千千静听 -- 未实现 */
};

wxString Gb2312XmlHack(const wxString &xml) {
    wxString ret(xml);
    ret.Replace(L"gb2312", L"utf-8");   // wxXml 只支持 UTF-8 编码

    return ret;
}

//////////////////////////////////////////////////////////////////////////
// QQ 音乐

class QQMusic : public LyricHost {
public:

    // 构造函数
    QQMusic();

private:

    // 解析下载到的歌词列表
    virtual bool ParseList(const wxString &list, ResultSet &lyrics) const;

    // 解析下载到的歌词
    virtual bool ParseLyric(wxString &lyric) const;
};

QQMusic::QQMusic()
    : LyricHost(L"http://qqmusic.qq.com/fcgi-bin/qm_getLyricId.fcg?"
                L"name={title}&singer={artist}&from=qqplayer",
                wxFONTENCODING_GB2312,
                wxFONTENCODING_GB2312)
{}

bool QQMusic::ParseList(const wxString &list, ResultSet &lyrics) const {
    lyrics.clear();

    //-----------------------------------------

    if (!list.EndsWith(L"-qq-music>")) {
        return false;
    }

    wxStringInputStream stream(Gb2312XmlHack(list));
    wxXmlDocument doc(stream);

    if (!doc.IsOk()) {
        wxLogDebug(L"Error parsing the lyric list.");
        return false;
    }

    wxXmlNode *cmd(doc.GetRoot()->GetChildren());
    wxXmlNode *numResultsNode(FindChildNode(cmd, L"songcount"));
    int numResults = XmlGetContentOfNum(numResultsNode, 0);

    if (numResults == 0) {
        return false;
    }

    //-----------------------------------------

    wxXmlNode *songInfoNode = FindChildNode(cmd, L"songinfo");
    SearchResult res;

    do {
        res.artist = XmlGetChildContent(songInfoNode, L"singername");
        res.title = XmlGetChildContent(songInfoNode, L"name");

        wxString lyricId(songInfoNode->GetAttribute(L"id"));
        res.url.Printf(L"http://music.qq.com/miniportal/static/lyric/%s/%s.xml",
                       lyricId.Mid(lyricId.length() - 2),
                       lyricId);

        lyrics.push_back(res);

        //-------------------------------------

        songInfoNode = FindSibling(songInfoNode);
    } while (songInfoNode);

    return !lyrics.empty();
}

bool QQMusic::ParseLyric(wxString &lyric) const {
    if (lyric.Trim(true).Trim(false).empty() ||
            !lyric.EndsWith(L"</lyric>")) {
        wxLogDebug(L"[%s:%d]下载的内容不完整(长度:%d)", __FILE__, __LINE__,
                   lyric.length());

        lyric.clear();
        return false;
    }

    wxStringInputStream stream(Gb2312XmlHack(lyric));
    wxXmlDocument doc(stream);

    if (doc.IsOk()) {
        lyric = doc.GetRoot()->GetNodeContent();
        return true;
    } else {
        lyric.clear();
        return false;
    }
}

//////////////////////////////////////////////////////////////////////////
// 百度音乐盒

class BaiduZhangmen : public LyricHost {
public:

    /// 构造函数
    BaiduZhangmen();

private:

    virtual bool ParseList(const wxString &list, ResultSet &lyrics) const;
};

BaiduZhangmen::BaiduZhangmen()
    : LyricHost(L"http://box.zhangmen.baidu.com/x?op=12&count=1&"
                L"title={title}$${artist}$$$$",
                wxFONTENCODING_GB2312,
                wxFONTENCODING_GB2312)
{}

bool BaiduZhangmen::ParseList(const wxString &list, ResultSet &lyrics) const {
    lyrics.clear();

    //==================================

    if (!list.EndsWith(L"</result>")) {
        return false;
    }

    wxStringInputStream stream(Gb2312XmlHack(list));
    wxXmlDocument doc(stream);

    // TODO:
    if (!doc.IsOk()) {
        wxLogDebug(L"Error parsing the lyric list.");
        return false;
    }

    SearchResult item;
    wxXmlNode *resultNode(doc.GetRoot());
    wxXmlNode *urlNode = resultNode->GetChildren();

    while (urlNode) {
        if (urlNode->GetName() == L"url") {
            wxXmlNode *lrcIdNode(FindChildNode(urlNode, L"lrcid"));
            int lrcId = XmlGetContentOfNum(lrcIdNode, 0);

            item.artist = GetArtist();
            item.title = GetTitle();
            item.url.Printf(L"http://box.zhangmen.baidu.com/bdlrc/%d/%d.lrc",
                            lrcId / 100, lrcId);

            lyrics.push_back(item);
        }

        urlNode = urlNode->GetNext();
    }

    return !lyrics.empty();
}

//////////////////////////////////////////////////////////////////////////
// 自建歌词服务器

class MyLyricHost : public LyricHost {
public:

    /// 构造函数
    MyLyricHost();

private:

    virtual bool ParseList(const wxString &list, ResultSet &lyrics) const;
};

MyLyricHost::MyLyricHost()
    : LyricHost(L"http://119.29.107.246:16723/lyric?"
                L"title={title}&artist={artist}", // TODO: Load from the confing file
                wxFONTENCODING_UTF8,
                wxFONTENCODING_UTF8)
{}

bool MyLyricHost::ParseList(const wxString &list, ResultSet &lyrics) const {
    lyrics.clear();

    wxStringTokenizer tokenizer(list, L"\r\n");
    if (tokenizer.CountTokens() % 3 != 0) {
        return false;
    }

    SearchResult res;

    while (tokenizer.HasMoreTokens()) {
        res.artist = tokenizer.GetNextToken();
        res.title = tokenizer.GetNextToken();
        res.url = tokenizer.GetNextToken();

        lyrics.push_back(res);
    }

    return !lyrics.empty();
}

//////////////////////////////////////////////////////////////////////////

/*static*/
LyricHost *LyricHost::Create(int hostId) {
    switch (hostId) {
    case LH_QQ_MUSIC:
        return new QQMusic;

    case LH_BAIDU_ZHANGMEN:
        return new BaiduZhangmen;

    case LH_M4PLAYER:
        return new MyLyricHost;

    default:
        return NULL;
    }
}

void LyricHost::GrabbLyric(const wxString &url) {}

}
