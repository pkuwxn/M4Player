/***************************************************************
 * Name:      OOPTagBatch.h
 * Purpose:   ������д������ǩ
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

/// ������д������ǩ
class OOPTagBatch
{
public:

	/// ��ǰ������д���Ƿ���ã��ɶ���
	bool IsOk() const { return !m_readers.empty(); }

	/// ���Զ�д�ı�ǩ��ʽ
	enum TagHandler {
		TH_ID3v1, ///< ID3v1
		TH_ID3v2, ///< ID3v2
		TH_LYRICS3v2, ///< Lyrics3v2
		TH_APEv2, ///< APEv2

		TH_COUNT,
	};

	/// ��ǩ�������Ĳ�����ʽ
	enum OpMode {
		OM_READ, /*! ֻ�� */
		OM_READ_WRITE, /*! ��/д */
	};

	/// ���һ����ǩ������
	/// 
	/// Ϊ������ң�������Ӧ�ñ�֤����ǩ��ʽ���������ڴ���ֻ����Ψһһ��ʵ����
	void AddHandler(TagHandler hdler, OpMode om);

	/// ���һ���ѹ����õ�(�ⲿ)��ǩ������
	/// 
	/// @param hdler ָ��ᱻ�ӹ�
	/// Ϊ������ң�������Ӧ�ñ�֤����ǩ��ʽ���������ڴ���ֻ����Ψһһ��ʵ����
	void AddHandler(MyTagLib::Tag* hdler, OpMode om);

	/// ��ȡд��������
	size_t GetWriterCount() const { return m_writers.size(); }

	typedef wxVector< MyTagLib::Tag* > WriterVec;
	typedef WriterVec::iterator writer_iterator;

	/// ��ȡд����������ʼ������
	writer_iterator writer_begin() { return m_writers.begin(); }

	/// ��ȡָ��д�������������һ���������һλ�õ���ֹ������
	writer_iterator writer_end() { return m_writers.end(); }

	/// ɾ��������ע��ı�ǩ��ʽ������
	void RemoveAllHandlers();

public:

	/// ����һ�������ļ�
	bool LoadFile(const wxString& path);

	/// �ر��Ѷ���������ļ�������ѷ������Դ
	void CloseFile();

	/// �����Ѵ򿪵������ļ�
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
