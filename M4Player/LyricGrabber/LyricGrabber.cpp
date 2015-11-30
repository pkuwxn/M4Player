/***************************************************************
 * Name:      LyricGrabber.cpp
 * Purpose:   歌词下载器
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-03-25
 **************************************************************/
#include "StdAfx.h"
#include "LyricGrabber.h"

#include "LyricHost.h"
#include "../OOPSingleHttpThread.h"

#include "VdkCtrlId.h"
#include "wxUtil.h" // for WriteBOM()

#include <wx/file.h> // for saving lyric file

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

namespace LyricGrabber {
wxDEFINE_EVENT(OOP_EVT_LYRIC_LIST_LOADED, wxCommandEvent);
wxDEFINE_EVENT(OOP_EVT_LYRIC_LOADED, wxCommandEvent);

Task::Task(wxEvtHandler *sinker)
    : m_hostId(wxNOT_FOUND), m_host(NULL),
      m_sessionType(ST_RETURN_LIST), m_length(0),
      m_idListLoaded(VdkGetUniqueId(10)), // 多多益善
      m_idLyricLoaded(m_idListLoaded + 5),
      m_sinker(sinker) {
    wxTheApp->Bind(wxEVT_HTTP_FINISH, &Task::OnListLoaded, this,
                   m_idListLoaded);

    wxTheApp->Bind(wxEVT_HTTP_FINISH, &Task::OnLyricLoaded,
                   this, m_idLyricLoaded);
}

Task::~Task() {
    DestoryHost();

    if (wxTheApp) {
        wxTheApp->Unbind(wxEVT_HTTP_FINISH, &Task::OnLyricLoaded,
                         this, m_idLyricLoaded);

        wxTheApp->Unbind(OOP_EVT_LYRIC_LIST_LOADED, &Task::OnListLoaded,
                         this, m_idListLoaded);
    }
}

void Task::DestoryHost() {
    if (m_host) {
        delete m_host;
        m_host = NULL;
    }
}

void Task::SetHost(int host) {
    wxASSERT(host >= 0);

    if (m_hostId == host) {
        return;
    }

    DestoryHost();

    m_hostId = host;
    m_host = LyricHost::Create(m_hostId);
}

void Task::GrabbList(SessionType st) {
    wxASSERT(m_host);

    m_sessionType = st;

    //----------------------------------------------

    wxString url(m_host->PrepareForTask(*this));
    VdkHttpThread::Task *httpTask = new VdkHttpThread::Task
        (url, m_host->GetLyricEncoding(), m_idListLoaded, wxTheApp);

    OOPSingleHttpThread::Instance().AddTask(httpTask);
}

void Task::OnListLoaded(wxCommandEvent &e) {
    m_result.clear();

    //----------------------------------------------

    VdkHttpThread::FinishEvent &finishEvt = (VdkHttpThread::FinishEvent &) e;

    wxString list(finishEvt.GetResult());
    if (list.Trim(true).Trim(false).empty()) {
        wxLogDebug(L"[%s:%d] 下载的内容不完整(长度:%d)", __FILE__, __LINE__,
                   list.length());
    } else {
        m_host->ParseList(list, m_result);
    }

    //----------------------------------------------

    if (m_sessionType == ST_RETURN_LIST) {
        int id = GetListLoadedEventId();
        wxCommandEvent evt(OOP_EVT_LYRIC_LIST_LOADED, id);

        wxPostEvent(m_sinker, evt);
    } else {
        if (!m_result.empty()) {
            GrabbLyric(0);
        } else {
            // 发送歌词下载失败的消息
            int id = GetLyricLoadedEvtId();
            VdkHttpThread::FinishEvent fe(id, wxEmptyString);
            fe.SetEventType(OOP_EVT_LYRIC_LOADED);
            fe.SetString(m_path);

            wxPostEvent(m_sinker, fe);
        }
    }
}

void Task::GrabbLyric(int serialNumber) {
    wxString url(m_result.at(serialNumber).url);
    VdkHttpThread::Task *httpTask = new VdkHttpThread::Task
        (url, m_host->GetLyricEncoding(), m_idLyricLoaded, wxTheApp);

    OOPSingleHttpThread::Instance().AddTask(httpTask);
}

void SaveLrcFile(const wxString &lyric, const wxString &fileName) {
    wxASSERT(!lyric.empty());
    wxASSERT(!fileName.empty());

    wxFile f(fileName, wxFile::write);
    WriteUtf8BOM(f);

    f.Write(lyric);
    f.Close();
}

void Task::OnLyricLoaded(wxCommandEvent &e) {
    VdkHttpThread::FinishEvent &finishEvt = (VdkHttpThread::FinishEvent &) e;

    wxString lyric(finishEvt.GetResult());
    if (m_host->ParseLyric(lyric)) {
        finishEvt.SetResult(lyric);
    } else {
        // 清空不正确的下载结果
        finishEvt.SetResult(wxEmptyString);
    }

    e.SetEventType(OOP_EVT_LYRIC_LOADED);
    e.SetId(GetLyricLoadedEvtId());
    e.SetString(m_path);

    wxPostEvent(m_sinker, e);
}

int Task::GetListLoadedEventId() const {
    return m_idListLoaded + 1;
}

int Task::GetLyricLoadedEvtId() const {
    return m_idLyricLoaded + 1;
}

}
