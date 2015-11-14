/***************************************************************
 * Name:      OOPTagBatch.h
 * Purpose:   批量读写歌曲标签
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-27
 **************************************************************/
#pragma once
#include <loki/Singleton.h>
#include <wx/vector.h>
#include <wx/sharedptr.h>

#include "Tag.h"

namespace MyTagLib {
	class Genre;
}

/// 批量读写歌曲标签
class OOPTagBatch
{
public:

	/// 当前批量读写器是否可用（可读）
	bool IsOk() const { return !m_readers.empty(); }

	/// 可以读写的标签格式
	enum TagHandler {
		TH_ID3v1, ///< ID3v1
		TH_ID3v2, ///< ID3v2
		TH_LYRICS3v2, ///< Lyrics3v2
		TH_APEv2, ///< APEv2

		TH_COUNT,
	};

	/// 标签处理器的操作方式
	enum OpMode {
		OM_READ, /*! 只读 */
		OM_READ_WRITE, /*! 读/写 */
	};

	/// 添加一个标签处理器
	/// 
	/// 为避免混乱，调用者应该保证各标签格式解析器在内存中只存在唯一一个实例。
	void AddHandler(TagHandler hdler, OpMode om);

	/// 添加一个已构建好的(外部)标签处理器
	/// 
	/// @param hdler 指针会被接管
	/// 为避免混乱，调用者应该保证各标签格式解析器在内存中只存在唯一一个实例。
	void AddHandler(MyTagLib::Tag* hdler, OpMode om);

	/// 获取写入器数量
	size_t GetWriterCount() const { return m_writers.size(); }

	typedef wxVector< MyTagLib::Tag* > WriterVec;
	typedef WriterVec::iterator writer_iterator;

	/// 获取写入器集合起始迭代器
	writer_iterator writer_begin() { return m_writers.begin(); }

	/// 获取指向写入器集合中最后一个对象的下一位置的终止迭代器
	writer_iterator writer_end() { return m_writers.end(); }

	/// 删除所有已注册的标签格式解析器
	void RemoveAllHandlers();

public:

	/// 读入一个音乐文件
	bool LoadFile(const wxString& path);

	/// 关闭已读入的音乐文件，清除已分配的资源
	void CloseFile();

	/// 保存已打开的音乐文件
	bool SaveFile();

public:

#define DECLARE_STRING_ACCESORS( field ) \
	wxString Get ## field() const; \
	bool Set ## field(const wxString& field)

	DECLARE_STRING_ACCESORS( Artist );
	DECLARE_STRING_ACCESORS( Title );
	DECLARE_STRING_ACCESORS( Album );
	DECLARE_STRING_ACCESORS( Year );
	DECLARE_STRING_ACCESORS( Comment );
	DECLARE_STRING_ACCESORS( Lyric );
	DECLARE_STRING_ACCESORS( Genre );

	int GetTrackNumber() const;
	bool SetTrackNumber(int TrackNumber);

#undef DECLARE_STRING_ACCESORS

private:

	typedef wxSharedPtr< MyTagLib::Tag > HandlerPtr;
	typedef wxVector< HandlerPtr > ReaderVec;
	ReaderVec m_readers;
	WriterVec m_writers;

	wxString m_path;

private:

	OOPTagBatch() {}
	~OOPTagBatch();
	OOPTagBatch(const OOPTagBatch&) {}
	OOPTagBatch& operator = (const OOPTagBatch&) { return *this; }
	OOPTagBatch* operator & () { return this; }

	friend struct Loki::CreateUsingNew< OOPTagBatch >;
};

typedef Loki::SingletonHolder< OOPTagBatch > SingleTagBatch;
