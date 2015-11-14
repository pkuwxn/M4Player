/***************************************************************
 * Name:      LyricGrabber.cpp
 * Purpose:   ���������
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-25
 **************************************************************/
#pragma once
#include "Types.h"

namespace LyricGrabber
{
	/*! ��ʣ��б����ز����Ľ��״̬˵�� */
	enum Result {
		LGR_FAILED, /*! ���ӷ�����ʧ�� */
		LGR_NO_MATCH, /*! û���ҵ�ƥ��ĸ�� */
		LGR_OK, /*! ƥ��ɹ� */
	};

	class LyricHost;

	/*! ��ѡ����б���Ȼ����/������� */
	wxDECLARE_EVENT( OOP_EVT_LYRIC_LIST_LOADED, wxCommandEvent );

	/*! �����Ȼ����/������� */
	wxDECLARE_EVENT( OOP_EVT_LYRIC_LOADED, wxCommandEvent );

	/// �����������ظ��ʱ��Ҫ�ṩ��������Ϣ
	class Task
	{
	public:

		/// ���캯��
		Task(wxEvtHandler* sinker);

		/// ��������
		~Task();

		/*! �µĸ�����ػỰ���� */
		enum SessionType {
			/*! �Զ�������������еĵ�һ������û���Ԥ */
			ST_AUTO_SELECT_AND_DOWNLOAD,
			ST_RETURN_LIST, ///< �򵥷�������������������û��ֶ�ѡ��Ҫ���ص���
		};

		/// ���첽�ķ�ʽ������������б�
		void GrabbList(SessionType st);

		/// ���첽�ķ�ʽ�������Ϊ@a serialNumber ����
		void GrabbLyric(int serialNumber);

		/// ��ȡ���سɹ��ĸ���б�
		const ResultSet& GetList() const { return m_result; }

	public:

		/// ��ȡʹ�õĸ�ʷ�����
		int GetHost() const { return m_hostId; }
		void SetHost(int host);

		/// ��ȡ������ػỰ����
		SessionType GetSessionType() const { return m_sessionType; }
		void SetSessionType(SessionType st) { m_sessionType = st; }

		/// ��ȡ����·��
		wxString GetPath() const { return m_path; }
		void SetPath(const wxString& path) { m_path = path; }

		/// ��ȡ������
		wxString GetArtist() const { return m_artist; }
		void SetArtist(const wxString& artist) { m_artist = artist; }

		/// ��ȡ��������
		wxString GetTitle() const { return m_title; }
		void SetTitle(const wxString& title) { m_title = title; }

		/// ��ȡ��������
		int GetLength() const { return m_length; }
		void SetLength(int length) { m_length = length; }

		/// ��ȡ����첽������Ϻ�����Ϣʱ�Ľ�����
		wxEvtHandler* GetEventSinker() const { return m_sinker; }

		/// ��ȡ֪ͨ�¼���ʶ ID
		int GetListLoadedEventId() const;

		/// ��ȡ���׸���������֪ͨ�¼� ID
		int GetLyricLoadedEvtId() const;

	private:

		// ɾ����ǰ��ʷ���������
		void DestoryHost();

		// ����б��������
		void OnListLoaded(wxCommandEvent& e);

		// ����б��������
		void OnLyricLoaded(wxCommandEvent& e);

	private:

		int m_hostId; // ʹ�õĸ�ʷ�����
		LyricHost* m_host;

		SessionType m_sessionType;

		wxString m_path; // ����·��
		wxString m_artist; // ������
		wxString m_title; // ����
		int m_length; // ��������

		int m_idListLoaded; // ������������ı�ʶ��
		int m_idLyricLoaded;

		wxEvtHandler* m_sinker; // ����첽������Ϻ�����Ϣʱ�Ľ�����

		ResultSet m_result;
	};

	/// ��������ݱ��浽�ļ���(ʹ�� UTF-8 ����)
	void SaveLrcFile(const wxString& lyric, const wxString& fileName);
}
