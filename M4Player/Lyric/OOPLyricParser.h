/***************************************************************
 * Name:      OOPLyricParser.h
 * Purpose:   ���� LRC/QRC/... ����ļ�
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-05
 **************************************************************/
#pragma once

/// ��ʵ�һ��
class LineInfo
{
public:

	/// ���캯��
	LineInfo(const wxString& strLyric, int nStartTime);

	/// ��ȡ����ı�
	wxString GetLyric() const { return m_strLyric; }

	/// ��ȡ��ʼ��ʾʱ��
	size_t GetStartTime() const {
		return m_nStartTime;
	}

	/// ��ȡ�����ʳ���ʱ��
	size_t GetMilSeconds() const {
		return m_nMilSeconds;
	}

	/// ���㱾���ʳ�����ʱ��
	void CalcYourTime(const LineInfo* next);

	/// ����̬����
	static int IsBefore(const LineInfo** p1, const LineInfo** p2);

private:

	wxString m_strLyric; // ��ʱ���Ա��汾����

	size_t m_nStartTime;
	size_t m_nMilSeconds;	// �����ܹ�������ʱ��

	friend class OOPLyricParser;
};

WX_DECLARE_LIST( LineInfo, _ListOfLineInfo );
typedef _ListOfLineInfo ListOfLineInfo;
typedef ListOfLineInfo::const_iterator LineIter;

//////////////////////////////////////////////////////////////////////////

/// ���� LRC/QRC/... ����ļ�
class OOPLyricParser
{
public:

	/// ���캯��
	OOPLyricParser();

	/// ��������
	~OOPLyricParser();

	/// ����ѽ���������
	void Clear();

	/// �Ƿ���Խ�����ʾ
	bool IsOk() const;

	/// �Ӵ����϶�ȡһ�� LRC �ļ�
	/// @param strLrcFile LRC �ļ���·��
	/// @param nTimeSum ����������ʱ��[���٣�ms(����)]
	bool LoadFile(const wxString& strLrcFile, int nTimeSum);

	/// �Ӵ����϶�ȡһ�� LRC �ļ�������������
	/// @param strLrcFile LRC �ļ���·��
	static wxString LoadFile(const wxString& strLrcFile);

	/// ֱ�ӽ��������� LRC �ı�
	/// @param nTimeSum ����������ʱ��[���٣�ms(����)]
	bool Load(const wxString& lyric, int nTimeSum);

	/// ��ȡ�����õ� LRC �ı�
	wxString GetLyric() const;

	/// ��ȡ LRC ��ÿһ�к󣬶���Щ����һЩ���ɵĹ���
	/// @param nTimeSum ����������ʱ��[���٣�ms(����)]
	bool BuilidLines(int nTimeSum);

public:

	/// ��ȡ @a timeOffset ��Ӧ����
	/// @param timeOffset ����:����[ms]
	LineIter WhichLine(size_t timeOffset) const;

	/// ��ȡ��һ������еĵ�����
	LineIter begin() const { return m_lines.begin(); }

	/// ��ȡ��Խ���һ������еĵ�����
	LineIter end() const { return m_lines.end(); }

	/// ��ȡָ���������������е����
	ListOfLineInfo::size_type IndexOf(LineIter it) const;

	/// ��ȡָ���еĵ�����
	LineIter GetLine(ListOfLineInfo::size_type index) const;

	/// ��ȡ�ı�������
	size_t GetLinesCount() const { return m_lines.size(); }

	/// ��ȡ����п��
	size_t GetMaxTextWidth(const wxFont& font) const;

	/// ��ȡ�����ʾ������ʱ��
	size_t GetTimeSum() const { return m_nTimeSum; }

private:

	// ���������ݲ�����
	void Load(wxInputStream& stream, int nTimeSum);

	// ���͸���ļ���.LRC����ĳһ��
	void ParseLine(const wxString& strLine);

private:

	size_t			m_nTimeSum; // �����ʾ������ʱ��
	ListOfLineInfo	m_lines;
};
