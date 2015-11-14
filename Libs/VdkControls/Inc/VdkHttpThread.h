/***************************************************************
 * Name:      VdkHttpThread.h
 * Purpose:   һ�����߳����첽ִ�� HTTP �����ı����������߳���
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-21
 **************************************************************/
#pragma once
#include <wx/vector.h>

class VdkHTTP;

/// һ�����߳����첽ִ�� HTTP �����ı����������߳���
class VdkHttpThread : public wxThread
{
public:

	/// һ���첽 HTTP ����
	class Task
	{
	public:

		/// ���캯��
		Task(const wxString& url, int id, wxEvtHandler* sinker);

		/// ��ȡ������ URL
		wxString GetURL() const { return m_url; }

		/// ��ȡ֪ͨ�¼���ʶ ID
		int GetEventId() const { return m_id; }

		/// ��ȡ�������¼�������
		wxEvtHandler* GetEventSinker() const { return m_sinker; }

	private:

		wxString m_url;
		int m_id;
		wxEvtHandler* m_sinker;
	};

	/// �첽�����������֪ͨ�¼�
	class FinishEvent : public wxCommandEvent
	{
	public:

		/// ���캯��
		FinishEvent(int id, const wxString& result);

		/// wxWidgets RTTI ����
		virtual wxEvent* Clone() const { return new FinishEvent( *this ); }

		/// ��ȡ HTTP ����ַ���
		wxString GetResult() const { return m_result; }

		/// ����(�޸ĺ��) HTTP ����ַ���
		void SetResult(wxString result) { m_result = result; }

	private:

		wxString m_result;
	};

	/// ���캯��
	/// 
	/// Ҫ��ʼ��������������� wxThread::Run() ��
	/// @a httpConn ָ��ᱻ�ӹ�
	VdkHttpThread(VdkHTTP* httpConn);

	/// ��������
	~VdkHttpThread();

	/// ���һ�����񵽶�����
	/// @a task ָ��ᱻ�ӹ�
	void AddTask(Task* task);

	/// �����ⲿ�ȴ��̰߳�ȫ�˳��������κ��ڴ�й¶
	///
	/// �����߱��뱣֤@a waiter ���̵߳��������������ж���Ч��
	void SetDestoryWaiter(wxSemaphore& waiter);

	/// ֹͣ�������˳��߳�
	void StopAndExit();
	
private:

	virtual ExitCode Entry();

	// �����������
	//
	// @attention �����ڲ������κ�ͬ�����ơ�
	void ClearTasks();

	/// ���ѽ���
	void WakeUp();

private:

	bool m_stop;
	wxSemaphore* m_destoryWaiter; // ���ⲿ�ȴ� VdkHttpThread ˳���ս�
	
	//------------------------------------------------

	VdkHTTP* m_httpConn;

	wxMutex m_taskListMutex;
	// �ȴ������߽�����ŵ������б���
	wxSemaphore m_listNotEmpty;
	
	typedef wxVector< Task* > TaskVec;
	TaskVec m_tasks;
};

wxDECLARE_EVENT( wxEVT_HTTP_FINISH, VdkHttpThread::FinishEvent );

//////////////////////////////////////////////////////////////////////////

typedef void ( wxEvtHandler::* VdkHttpFinishEventFunction )(VdkHttpThread::FinishEvent&);

#define EVT_HTTP_RANGE( id0, id1, fn )  \
	DECLARE_EVENT_TABLE_ENTRY( wxEVT_HTTP_FINISH, id0, id1, \
		( wxObjectEventFunction )( wxEventFunction )(VdkHttpFinishEventFunction) \
			&fn, (wxObject*) NULL ),

#define EVT_HTTP( id, fn ) EVT_HTTP_RANGE( id, -1, fn )
