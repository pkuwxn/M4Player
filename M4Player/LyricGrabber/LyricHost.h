/***************************************************************
 * Name:      LyricHost.cpp
 * Purpose:   ��ʷ�����
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-25
 **************************************************************/
#pragma once
#include "Types.h"

namespace LyricGrabber
{
	class Task;

	/// ��ʷ�����
	class LyricHost
	{
	public:

		/// ����������
		virtual ~LyricHost();

		/// ���ظ���б�
		/// @param task ����������Ϣ
		/// @return ����б� URL
		wxString PrepareForTask(const Task& task);

		/// ���첽�ķ�ʽ���ص�ַΪ@url �ĸ��
		void GrabbLyric(const wxString& url);

		/// ��ȡ��ʷ��������󣨹���������
		static LyricHost* Create(int hostId);

	public:

		// �����õ��ĸ���б�
		// @param list ��õ�ԭʼ����б��ַ���
		// @param rs ��������������б����������ȱ����
		virtual bool ParseList(const wxString& list, ResultSet& rs) const = 0;

		// �����õ���ԭʼ����ı�
		virtual bool ParseLyric(wxString& lyric) const { return true; }

	protected:

		/// ��ȡ���ظ���б�
		LyricHost(const wxString& listUrlTemplate);

		/// ���ò�ѯ URL ת���Ŀ�ı���
		void SetUrlEscapeDestCharset(const wxCSConv& conv);

		//////////////////////////////////////////////////////////////////////////
		// �������������ǲ����Ѷ�Ϊ֮�ģ���Ϊ��Щ��ʷ��������ص�������û��
		// ��������Ϣ�����Ի���Ҫ������Щ״̬��Ϣ
		// TODO: �ڵȴ����ݷ��ص�ʱ���û��ύ��һ���µ���������ô����

		/// ��ȡ�뱾������������ص�������
		wxString GetArtist() const { return m_artist; }

		/// ��ȡ�뱾������������صĸ�������
		wxString GetTitle() const { return m_title; }

	private:

		// ���ظ���б�ʱ�� URL ģ��
		wxString m_listUrlTemplate;
		// ��ѯ URL ת���Ŀ�ı���
		wxCSConv m_escapeTo;

		wxString m_artist;
		wxString m_title;
	};
}
