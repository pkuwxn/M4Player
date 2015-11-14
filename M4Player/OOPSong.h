#pragma once

/// \brief OOPList 中的一项所必需的最小信息
class OOPListEntry
{
public:

	/// \brief 构造函数
	OOPListEntry(const wxString& p);
	
	/// \brief 设置歌曲的播放次数
	OOPListEntry& playCount(int c) { m_playCount = c; return *this; }

	/// \brief 将歌曲的播放次数递增1
	OOPListEntry& playCountPlus() { m_playCount++; return *this; }

	/// \brief 获取歌曲的播放次数
	int playCount() const { return m_playCount; }

	/// \brief 设置歌曲加入列表的时间
	OOPListEntry& addTime(long t) { m_addTime = t; return *this; }

	/// \brief 获取歌曲加入列表的时间
	long addTime() const { return m_addTime; }

	/// \brief 获取歌曲路径
	wxString path() const { return m_path; }

	/// \brief 设置歌曲路径名
	OOPListEntry& path(const wxString& s) { m_path = s; return *this; }

	/// \brief 获取歌曲长度（量纲：s[秒]）
	int length() const { return m_length; }

	/// \brief 设置歌曲长度（量纲：s[秒]）
	OOPListEntry& length(int l) { m_length = l; return *this; }

private:

	int m_playCount; // 播放次数
	long m_addTime; // 加入时间

	wxString m_path;
	int m_length;
};

/// \brief 歌曲对象定义
class OOPSong
{
public:
	
	/// \brief 构造函数
	/// \param p 歌曲路径
	explicit OOPSong(const wxString& p);

	/// \brief 构造函数
	/// \param listEntry 根据一个播放列表项创建相应的 OOPSong 对象
	explicit OOPSong(OOPListEntry* listEntry);

	/// \brief 析构函数
	~OOPSong();

	/// \brief 获取对应的播放列表项
	///
	/// 这个函数不会是 const 的。
	OOPListEntry* GetListEntry() { return m_listEntry; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 设置歌曲的播放次数
	OOPSong& playCount(int c) { m_listEntry->playCount( c ); return *this; }

	/// \brief 将歌曲的播放次数递增1
	OOPSong& playCountPlus() { m_listEntry->playCountPlus(); return *this; }

	/// \brief 获取歌曲的播放次数
	int playCount() const { return m_listEntry->playCount(); }

	/// \brief 设置歌曲加入列表的时间
	OOPSong& addTime(long t) { m_listEntry->addTime( t ); return *this; }

	/// \brief 获取歌曲加入列表的时间
	long addTime() const { return m_listEntry->addTime(); }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 获取歌曲名
	wxString title() const { return m_title; }

	/// \brief 设置歌曲名
	OOPSong& title(const wxString& s) { m_title = s; return *this; }

	/// \brief 获取歌手名
	wxString artist() const { return m_artist; }

	/// \brief 设置歌手名
	OOPSong& artist(const wxString& s) { m_artist = s; return *this; }

	/// \brief 获取歌曲文件名
	wxString fileName() const;

	/// \brief 获取歌曲路径
	wxString path() const { return m_listEntry->path(); }

	/// \brief 设置歌曲路径名
	OOPSong& path(const wxString& s) { m_listEntry->path( s ); return *this; }

	/// \brief 获取与歌曲相关联的歌词文件路径
	wxString lrcPath() const;

	/// \brief 设置与歌曲相关联的歌词文件路径
	OOPSong& lrcPath(const wxString& s) { m_lrcPath = s; return *this; }

	/// \brief 获取歌曲所属专辑
	wxString album() const { return m_album; }

	/// \brief 设置歌曲所属专辑
	OOPSong& album(const wxString& s) { m_album = s; return *this; }

	/// \brief 获取歌曲所属流派
	wxString genre() const { return m_genre; }

	/// \brief 设置歌曲所属流派
	OOPSong& genre(const wxString& s) { m_genre = s; return *this; }

	/// \brief 获取歌曲的年代
	wxString year() const { return m_year; }

	/// \brief 设置歌曲的年代
	OOPSong& year(wxString y) { m_year = y; return *this; }

	/// \brief 获取歌曲的音轨号
	int trackNo() const { return m_trackNo; }

	/// \brief 设置歌曲的音轨号
	OOPSong& trackNo(int i) { m_trackNo = i; return *this; }

	/// \brief 获取歌曲的备注信息
	wxString comment() const { return m_comment; }

	/// \brief 设置歌曲的备注信息
	OOPSong& comment(const wxString& s) { m_comment = s; return *this; }

	/// \brief 获取歌曲长度（量纲：s[秒]）
	int length() const { return m_listEntry->length(); }

	/// \brief 设置歌曲长度（量纲：s[秒]）
	OOPSong& length(int l) { m_listEntry->length( l ); return *this; }

	/// \brief 获取歌曲编码信息
	wxString codec() const;

	/// \brief 获取歌曲比特率（量纲：kb/s）
	int bitRate() const { return m_bitRate; }

	/// \brief 获取歌曲采样频率（量纲：Hz）
	int sampleRate() const { return m_sampleRate; }

	/// \brief 获取歌曲比特
	int bits() const { return m_bits; }

	/// \brief 获取歌曲声道数
	int channels() const { return m_channels; }

	/// \brief 获取嵌入的歌词
	wxString embeddedLyric() const { return m_lyric; }

	/// \brief 获取嵌入的歌词
	OOPSong& embeddedLyric(const wxString& lyric) {
		m_lyric = lyric; return *this;
	}

	/// \brief 是否为动态码率
	bool isVBR() const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief 歌曲文件类型的标签读写是否被支持
	bool IsTaggingSupported() const;

	/// \brief 保存已更改的标签
	bool Save();

	/// \brief 从标签中读取歌曲信息
	/// \param forceReparse 强制重新读取歌曲信息
	void ParseTag(bool forceReparse = false);

	/// \brief 歌曲信息是否成功被读取并保存
	bool IsParsed() const { return m_parsed; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 根据指定的格式\a format 生成歌曲标识符
	wxString BuildTitle(const wxString& format) const;

	/// \brief 生成标准歌曲时长字符串表达式
	static wxString GetStdTimeStr(unsigned int len);

	/// 尝试获取正确的艺术家和标题
	///
	/// 有些歌曲文件没有正确的艺术家和标题信息，这时候可以尝试从文件名上面获取。
	/// 默认使用“%A-%T”的格式。
	/// @return 是否成功填充了@a strArtist 以及@a strTitle 或其中之一。
	/// @attention 调用本函数会清空@a strArtist 以及@a strTitle 原有的内容。
	bool TryGetArtistAndTitle(wxString& strArtist, wxString& strTitle) const;

private:

	// 将\a ret 中含有\a format 的字串替换成\a data
	// \return 是否成功替换
	static bool ParseFormat(wxString& ret,
							const wxString& format,
							const wxString& data,
							const wxString& def);

private:

	OOPListEntry* m_listEntry;
	bool m_ownListEntry; // m_listEntry 指针是否已被接管

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

	bool m_parsed; // 是否已然解析 Tags

	struct Impl;
	Impl* m_impl;
};
