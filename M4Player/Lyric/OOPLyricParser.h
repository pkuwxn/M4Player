/***************************************************************
 * Name:      OOPLyricParser.h
 * Purpose:   解析 LRC/QRC/... 歌词文件
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-05
 **************************************************************/
#pragma once

/// 歌词的一行
class LineInfo
{
public:

	/// 构造函数
	LineInfo(const wxString& strLyric, int nStartTime);

	/// 获取歌词文本
	wxString GetLyric() const { return m_strLyric; }

	/// 获取起始显示时间
	size_t GetStartTime() const {
		return m_nStartTime;
	}

	/// 获取本句歌词持续时间
	size_t GetMilSeconds() const {
		return m_nMilSeconds;
	}

	/// 计算本句歌词持续的时间
	void CalcYourTime(const LineInfo* next);

	/// 排序静态函数
	static int IsBefore(const LineInfo** p1, const LineInfo** p2);

private:

	wxString m_strLyric; // 临时用以保存本句歌词

	size_t m_nStartTime;
	size_t m_nMilSeconds;	// 本句总共持续的时间

	friend class OOPLyricParser;
};

WX_DECLARE_LIST( LineInfo, _ListOfLineInfo );
typedef _ListOfLineInfo ListOfLineInfo;
typedef ListOfLineInfo::const_iterator LineIter;

//////////////////////////////////////////////////////////////////////////

/// 解析 LRC/QRC/... 歌词文件
class OOPLyricParser
{
public:

	/// 构造函数
	OOPLyricParser();

	/// 析构函数
	~OOPLyricParser();

	/// 清除已解析的内容
	void Clear();

	/// 是否可以进行显示
	bool IsOk() const;

	/// 从磁盘上读取一个 LRC 文件
	/// @param strLrcFile LRC 文件的路径
	/// @param nTimeSum 歌曲持续的时间[量纲：ms(毫秒)]
	bool LoadFile(const wxString& strLrcFile, int nTimeSum);

	/// 从磁盘上读取一个 LRC 文件，返回其内容
	/// @param strLrcFile LRC 文件的路径
	static wxString LoadFile(const wxString& strLrcFile);

	/// 直接解析给定的 LRC 文本
	/// @param nTimeSum 歌曲持续的时间[量纲：ms(毫秒)]
	bool Load(const wxString& lyric, int nTimeSum);

	/// 获取解析好的 LRC 文本
	wxString GetLyric() const;

	/// 读取 LRC 的每一行后，对这些行做一些集成的工作
	/// @param nTimeSum 歌曲持续的时间[量纲：ms(毫秒)]
	bool BuilidLines(int nTimeSum);

public:

	/// 获取 @a timeOffset 对应的行
	/// @param timeOffset 量纲:毫秒[ms]
	LineIter WhichLine(size_t timeOffset) const;

	/// 获取第一个歌词行的迭代器
	LineIter begin() const { return m_lines.begin(); }

	/// 获取超越最后一个歌词行的迭代器
	LineIter end() const { return m_lines.end(); }

	/// 获取指定迭代器在容器中的序号
	ListOfLineInfo::size_type IndexOf(LineIter it) const;

	/// 获取指定行的迭代器
	LineIter GetLine(ListOfLineInfo::size_type index) const;

	/// 获取文本行总数
	size_t GetLinesCount() const { return m_lines.size(); }

	/// 获取最大行宽度
	size_t GetMaxTextWidth(const wxFont& font) const;

	/// 获取歌词显示持续的时间
	size_t GetTimeSum() const { return m_nTimeSum; }

private:

	// 载入歌词内容并解析
	void Load(wxInputStream& stream, int nTimeSum);

	// 解释歌词文件（.LRC）的某一行
	void ParseLine(const wxString& strLine);

private:

	size_t			m_nTimeSum; // 歌词显示持续的时间
	ListOfLineInfo	m_lines;
};
