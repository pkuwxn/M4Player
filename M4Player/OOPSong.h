#pragma once

/// \brief OOPList �е�һ�����������С��Ϣ
class OOPListEntry
{
public:

	/// \brief ���캯��
	OOPListEntry(const wxString& p);
	
	/// \brief ���ø����Ĳ��Ŵ���
	OOPListEntry& playCount(int c) { m_playCount = c; return *this; }

	/// \brief �������Ĳ��Ŵ�������1
	OOPListEntry& playCountPlus() { m_playCount++; return *this; }

	/// \brief ��ȡ�����Ĳ��Ŵ���
	int playCount() const { return m_playCount; }

	/// \brief ���ø��������б��ʱ��
	OOPListEntry& addTime(long t) { m_addTime = t; return *this; }

	/// \brief ��ȡ���������б��ʱ��
	long addTime() const { return m_addTime; }

	/// \brief ��ȡ����·��
	wxString path() const { return m_path; }

	/// \brief ���ø���·����
	OOPListEntry& path(const wxString& s) { m_path = s; return *this; }

	/// \brief ��ȡ�������ȣ����٣�s[��]��
	int length() const { return m_length; }

	/// \brief ���ø������ȣ����٣�s[��]��
	OOPListEntry& length(int l) { m_length = l; return *this; }

private:

	int m_playCount; // ���Ŵ���
	long m_addTime; // ����ʱ��

	wxString m_path;
	int m_length;
};

/// \brief ����������
class OOPSong
{
public:
	
	/// \brief ���캯��
	/// \param p ����·��
	explicit OOPSong(const wxString& p);

	/// \brief ���캯��
	/// \param listEntry ����һ�������б������Ӧ�� OOPSong ����
	explicit OOPSong(OOPListEntry* listEntry);

	/// \brief ��������
	~OOPSong();

	/// \brief ��ȡ��Ӧ�Ĳ����б���
	///
	/// ������������� const �ġ�
	OOPListEntry* GetListEntry() { return m_listEntry; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���ø����Ĳ��Ŵ���
	OOPSong& playCount(int c) { m_listEntry->playCount( c ); return *this; }

	/// \brief �������Ĳ��Ŵ�������1
	OOPSong& playCountPlus() { m_listEntry->playCountPlus(); return *this; }

	/// \brief ��ȡ�����Ĳ��Ŵ���
	int playCount() const { return m_listEntry->playCount(); }

	/// \brief ���ø��������б��ʱ��
	OOPSong& addTime(long t) { m_listEntry->addTime( t ); return *this; }

	/// \brief ��ȡ���������б��ʱ��
	long addTime() const { return m_listEntry->addTime(); }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡ������
	wxString title() const { return m_title; }

	/// \brief ���ø�����
	OOPSong& title(const wxString& s) { m_title = s; return *this; }

	/// \brief ��ȡ������
	wxString artist() const { return m_artist; }

	/// \brief ���ø�����
	OOPSong& artist(const wxString& s) { m_artist = s; return *this; }

	/// \brief ��ȡ�����ļ���
	wxString fileName() const;

	/// \brief ��ȡ����·��
	wxString path() const { return m_listEntry->path(); }

	/// \brief ���ø���·����
	OOPSong& path(const wxString& s) { m_listEntry->path( s ); return *this; }

	/// \brief ��ȡ�����������ĸ���ļ�·��
	wxString lrcPath() const;

	/// \brief ���������������ĸ���ļ�·��
	OOPSong& lrcPath(const wxString& s) { m_lrcPath = s; return *this; }

	/// \brief ��ȡ��������ר��
	wxString album() const { return m_album; }

	/// \brief ���ø�������ר��
	OOPSong& album(const wxString& s) { m_album = s; return *this; }

	/// \brief ��ȡ������������
	wxString genre() const { return m_genre; }

	/// \brief ���ø�����������
	OOPSong& genre(const wxString& s) { m_genre = s; return *this; }

	/// \brief ��ȡ���������
	wxString year() const { return m_year; }

	/// \brief ���ø��������
	OOPSong& year(wxString y) { m_year = y; return *this; }

	/// \brief ��ȡ�����������
	int trackNo() const { return m_trackNo; }

	/// \brief ���ø����������
	OOPSong& trackNo(int i) { m_trackNo = i; return *this; }

	/// \brief ��ȡ�����ı�ע��Ϣ
	wxString comment() const { return m_comment; }

	/// \brief ���ø����ı�ע��Ϣ
	OOPSong& comment(const wxString& s) { m_comment = s; return *this; }

	/// \brief ��ȡ�������ȣ����٣�s[��]��
	int length() const { return m_listEntry->length(); }

	/// \brief ���ø������ȣ����٣�s[��]��
	OOPSong& length(int l) { m_listEntry->length( l ); return *this; }

	/// \brief ��ȡ����������Ϣ
	wxString codec() const;

	/// \brief ��ȡ���������ʣ����٣�kb/s��
	int bitRate() const { return m_bitRate; }

	/// \brief ��ȡ��������Ƶ�ʣ����٣�Hz��
	int sampleRate() const { return m_sampleRate; }

	/// \brief ��ȡ��������
	int bits() const { return m_bits; }

	/// \brief ��ȡ����������
	int channels() const { return m_channels; }

	/// \brief ��ȡǶ��ĸ��
	wxString embeddedLyric() const { return m_lyric; }

	/// \brief ��ȡǶ��ĸ��
	OOPSong& embeddedLyric(const wxString& lyric) {
		m_lyric = lyric; return *this;
	}

	/// \brief �Ƿ�Ϊ��̬����
	bool isVBR() const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief �����ļ����͵ı�ǩ��д�Ƿ�֧��
	bool IsTaggingSupported() const;

	/// \brief �����Ѹ��ĵı�ǩ
	bool Save();

	/// \brief �ӱ�ǩ�ж�ȡ������Ϣ
	/// \param forceReparse ǿ�����¶�ȡ������Ϣ
	void ParseTag(bool forceReparse = false);

	/// \brief ������Ϣ�Ƿ�ɹ�����ȡ������
	bool IsParsed() const { return m_parsed; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ����ָ���ĸ�ʽ\a format ���ɸ�����ʶ��
	wxString BuildTitle(const wxString& format) const;

	/// \brief ���ɱ�׼����ʱ���ַ������ʽ
	static wxString GetStdTimeStr(unsigned int len);

	/// ���Ի�ȡ��ȷ�������Һͱ���
	///
	/// ��Щ�����ļ�û����ȷ�������Һͱ�����Ϣ����ʱ����Գ��Դ��ļ��������ȡ��
	/// Ĭ��ʹ�á�%A-%T���ĸ�ʽ��
	/// @return �Ƿ�ɹ������@a strArtist �Լ�@a strTitle ������֮һ��
	/// @attention ���ñ����������@a strArtist �Լ�@a strTitle ԭ�е����ݡ�
	bool TryGetArtistAndTitle(wxString& strArtist, wxString& strTitle) const;

private:

	// ��\a ret �к���\a format ���ִ��滻��\a data
	// \return �Ƿ�ɹ��滻
	static bool ParseFormat(wxString& ret,
							const wxString& format,
							const wxString& data,
							const wxString& def);

private:

	OOPListEntry* m_listEntry;
	bool m_ownListEntry; // m_listEntry ָ���Ƿ��ѱ��ӹ�

	wxString m_title;
	wxString m_artist;
	mutable wxString m_lrcPath;
	wxString m_album;
	wxString m_genre;
	wxString m_comment;
	wxString m_year;
	wxString m_lyric;
	int m_trackNo;

	int m_bitRate;
	int m_sampleRate;
	int m_bits;
	int m_channels;

	bool m_parsed; // �Ƿ���Ȼ���� Tags

	struct Impl;
	Impl* m_impl;
};
