/***************************************************************
 * Name:      VdkHttpThread.h
 * Purpose:   一个在线程中异步执行 HTTP 操作的便利工作者线程类
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-21
 **************************************************************/
#pragma once
#include <wx/vector.h>

class VdkHTTP;

/// 一个在线程中异步执行 HTTP 操作的便利工作者线程类
class VdkHttpThread : public wxThread {
public:

    /// 一个异步 HTTP 任务
    class Task {
    public:

        /// 构造函数
        Task(const wxString &url, int id, wxEvtHandler *sinker);

        /// 获取关联的 URL
        wxString GetURL() const {
            return m_url;
        }

        /// 获取通知事件标识 ID
        int GetEventId() const {
            return m_id;
        }

        /// 获取关联的事件监听者
        wxEvtHandler *GetEventSinker() const {
            return m_sinker;
        }

    private:

        wxString m_url;
        int m_id;
        wxEvtHandler *m_sinker;
    };

    /// 异步下载任务完成通知事件
    class FinishEvent : public wxCommandEvent {
    public:

        /// 构造函数
        FinishEvent(int id, const wxString &result);

        /// wxWidgets RTTI 必须
        virtual wxEvent *Clone() const {
            return new FinishEvent(*this);
        }

        /// 获取 HTTP 结果字符串
        wxString GetResult() const {
            return m_result;
        }

        /// 设置(修改后的) HTTP 结果字符串
        void SetResult(wxString result) {
            m_result = result;
        }

    private:

        wxString m_result;
    };

    /// 构造函数
    ///
    /// 要开始监听任务，仍需调用 wxThread::Run() 。
    /// @a httpConn 指针会被接管
    VdkHttpThread(VdkHTTP *httpConn);

    /// 析构函数
    ~VdkHttpThread();

    /// 添加一个任务到队列中
    /// @a task 指针会被接管
    void AddTask(Task *task);

    /// 允许外部等待线程安全退出，避免任何内存泄露
    ///
    /// 调用者必须保证@a waiter 在线程的整个生命周期中都有效。
    void SetDestoryWaiter(wxSemaphore &waiter);

    /// 停止工作，退出线程
    void StopAndExit();

private:

    virtual ExitCode Entry();

    // 清除所有任务
    //
    // @attention 函数内部不作任何同步控制。
    void ClearTasks();

    /// 唤醒进程
    void WakeUp();

private:

    bool m_stop;
    wxSemaphore *m_destoryWaiter; // 从外部等待 VdkHttpThread 顺利终结

    //------------------------------------------------

    VdkHTTP *m_httpConn;

    wxMutex m_taskListMutex;
    // 等待生产者将任务放到任务列表中
    wxSemaphore m_listNotEmpty;

    typedef wxVector<Task *> TaskVec;
    TaskVec m_tasks;
};

wxDECLARE_EVENT(wxEVT_HTTP_FINISH, VdkHttpThread::FinishEvent);

//////////////////////////////////////////////////////////////////////////

typedef void (wxEvtHandler::* VdkHttpFinishEventFunction)(VdkHttpThread::FinishEvent &);

#define EVT_HTTP_RANGE( id0, id1, fn )  \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_HTTP_FINISH, id0, id1, \
        ( wxObjectEventFunction )( wxEventFunction )(VdkHttpFinishEventFunction) \
            &fn, (wxObject*) NULL ),

#define EVT_HTTP( id, fn ) EVT_HTTP_RANGE( id, -1, fn )
