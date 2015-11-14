/***************************************************************
 * Name:      VdkHttpThread.h
 * Purpose:   一个在线程中异步执行 HTTP 操作的便利工作者线程类
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-21
 **************************************************************/
#include "StdAfx.h"
#include "VdkHttpThread.h"
#include "VdkHTTP.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT( wxEVT_HTTP_FINISH, VdkHttpThread::FinishEvent );

VdkHttpThread::VdkHttpThread(VdkHTTP* httpConn)
	: wxThread( wxTHREAD_DETACHED ), 
	  m_stop( false ), m_destoryWaiter( NULL ),
	  m_httpConn( httpConn )
{
	wxASSERT( m_httpConn );

	//=====================================

	if( Create() != wxTHREAD_NO_ERROR )
	{
	    wxLogError( L"Can't create the thread!" );
	}
	else if( Run() != wxTHREAD_NO_ERROR )
	{
	    wxLogError( L"Can't run the thread!" );
	}
}

VdkHttpThread::~VdkHttpThread()
{
	{
		wxMutexLocker lock( m_taskListMutex );
		ClearTasks();
	}
	
	wxDELETE( m_httpConn );
	
	if( m_destoryWaiter )
	{
	    m_destoryWaiter->Post();
	}
}

void VdkHttpThread::ClearTasks()
{
	TaskVec::const_iterator iter( m_tasks.begin() );
	for( ; iter != m_tasks.end(); ++iter )
		delete *iter;

	m_tasks.clear();
}

void VdkHttpThread::AddTask(Task* task)
{
    {
	    wxMutexLocker lock( m_taskListMutex );
	    m_tasks.push_back( task );
	}
	
	m_listNotEmpty.Post();
}

void VdkHttpThread::SetDestoryWaiter(wxSemaphore& waiter)
{
    m_destoryWaiter = &waiter;
}

void VdkHttpThread::WakeUp()
{
    m_listNotEmpty.Post();
}

wxThread::ExitCode VdkHttpThread::Entry()
{
	wxASSERT( m_httpConn );

	while( !m_stop )
	{
		m_taskListMutex.Lock();
		if( m_tasks.empty() )
		{
			m_taskListMutex.Unlock();
			m_listNotEmpty.Wait();

			continue;
		}

		// 只需要最后一个任务，清空其它
		Task* task = m_tasks.back();
		m_tasks.pop_back();
		ClearTasks();

		m_taskListMutex.Unlock();

		//===================================

		// TODO: 设置操作成功与否？
		wxString result;
		m_httpConn->Get( task->GetURL(), result );

		FinishEvent* e = new FinishEvent( task->GetEventId(), result );
		wxQueueEvent( task->GetEventSinker(), e );

		delete task;
	}

	return 0;
}

void VdkHttpThread::StopAndExit()
{
	{
		wxMutexLocker lock( m_taskListMutex );
		ClearTasks();
	}
	
	m_stop = true;
	WakeUp();
}

//////////////////////////////////////////////////////////////////////////

VdkHttpThread::Task::Task(const wxString& url, int id, wxEvtHandler* sinker)
	: m_url( url ), m_id( id ), m_sinker( sinker )
{

}

VdkHttpThread::FinishEvent::FinishEvent(int id, const wxString& result)
	: wxCommandEvent( wxEVT_HTTP_FINISH, id ),
	  m_result( result )
{

}
