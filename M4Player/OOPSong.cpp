/***************************************************************
 * Name:      OOPSong.cpp
 * Purpose:   ����������
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-19
 * Copyright: Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPSong.h"

#include "OOPTagBatch.h"
#include "OOPCharsetConv.h"
#include "wxUtil.h"

#include "FileRewriter.h" // TODO:

#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <Music.hpp>

#include <fstream>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

OOPListEntry::OOPListEntry(const wxString& p)
	: m_playCount( 0 ),
	  m_addTime( 0 ),
	  m_path( p ),
	  m_length( 0 )
{

}

//////////////////////////////////////////////////////////////////////////

// һЩ��������һ�汾�͸ı���ڲ�ʵ����Ϣ
struct OOPSong::Impl
{
public:

	// ���캯��
	Impl();
};

OOPSong::Impl::Impl()
{

}

//////////////////////////////////////////////////////////////////////////

OOPSong::OOPSong(const wxString& p)
	: m_listEntry( new OOPListEntry( p ) ),
	  m_ownListEntry( true ),
	  m_trackNo( 0 ),
	  m_bitRate( 0 ),
	  m_sampleRate( 0 ),
	  m_bits( 0 ),
	  m_channels( 0 ),
	  m_parsed( false ),
	  m_impl( NULL )
{

}

OOPSong::OOPSong(OOPListEntry* listEntry)
	: m_listEntry( listEntry ),
	  m_ownListEntry( false ),
	  m_trackNo( 0 ),
	  m_bitRate( 0 ),
	  m_sampleRate( 0 ),
	  m_bits( 0 ),
	  m_channels( 0 ),
	  m_parsed( false ),
	  m_impl( NULL )
{
	wxASSERT( m_listEntry );
}

OOPSong::~OOPSong()
{
	wxDELETE( m_impl );

	if( m_ownListEntry )
	{
		wxDELETE( m_listEntry );
	}
}

wxString GetTagHandler(size_t index)
{
	wxString handlers[] = {
		L"APEv2", L"ID3v1", L"ID3v2", L"Lyrics3v2"
	};

	return handlers[index];
}

void OOPSong::ParseTag(bool forceReparse)
{
	// ��Ȼ��ȡ������
	if( IsParsed() && !forceReparse )
		return;

	wxASSERT( !m_listEntry->path().empty() );
	if( !wxFileExists( m_listEntry->path() ) )
		return;

	//if( !m_impl )
	//	m_impl = new Impl;

	//=====================================================

	{
		sf::Music music;
		if( music.openFromFile( m_listEntry->path() ) )
		{
			m_listEntry->length( music.getDuration().asSeconds() );
			m_bitRate = music.getBitRate();
			m_channels = music.getChannelCount();
			m_sampleRate = music.getSampleRate();
			m_bits = 16; // TODO:
		}
	}

	//=====================================================
	// ����Ҫ��� IsTaggingSupported()

	OOPTagBatch& bat = SingleTagBatch::Instance();
	if( !bat.IsOk() )
	{
		OOPCharsetConv::CreateInstance();
			
		// ���ȶ�ȡ�����Լ�д���
		bat.AddHandler( OOPTagBatch::TH_APEv2, OOPTagBatch::OM_READ_WRITE );
		bat.AddHandler( OOPTagBatch::TH_ID3v2, OOPTagBatch::OM_READ );
		bat.AddHandler( OOPTagBatch::TH_ID3v1, OOPTagBatch::OM_READ_WRITE );
		bat.AddHandler( OOPTagBatch::TH_LYRICS3v2, 
						OOPTagBatch::OM_READ_WRITE );
	}

	if( bat.LoadFile( m_listEntry->path() ) )
	{
		m_artist = bat.GetArtist();
		m_title = bat.GetTitle();
		m_album = bat.GetAlbum();
		m_genre = bat.GetGenre();
		m_comment = bat.GetComment();
		m_year = bat.GetYear();
		m_lyric = bat.GetLyric();

		m_trackNo = bat.GetTrackNumber();
	}

	// ������Ҫ���ˣ��ر��ļ�
	bat.CloseFile();

	m_parsed = true;
}

bool OOPSong::Save()
{
	// TODO: ����һ���ǳ�׾�ӵ���ƣ�Ӧ��ÿһ���ļ�����ӵ��һ��������дջ

	// ���粻��֧�֣���ô��������ɹ������������� DelayModStack �ķ�������
	if( !IsTaggingSupported() )
		return true;

	wxASSERT( !m_listEntry->path().empty() );

	// �����ļ����������ˣ���ô���������������壬����д��ɹ�
	if( !wxFileExists( m_listEntry->path() ) )
		return true;

	OOPTagBatch& bat = SingleTagBatch::Instance();
	if( bat.GetWriterCount() == 0 )
		return true; // ��Ҫ�ٴγ��ԣ�����д��ɹ�

	//---------------------------------------------

	using namespace std;
	fstream strm( (const char *) m_listEntry->path(), ios::in | ios::out | ios::binary );
	if( !strm )
	{
		return false;
	}

	MyTagLib::FileRewriter rw;

	OOPTagBatch::writer_iterator iter( bat.writer_begin() );
	for( ; iter != bat.writer_end(); ++iter )
	{
		MyTagLib::Tag* writer = *iter;

		// TODO: ����ǰ�����ͬһ���ļ�����ôӦ�ò����ض�
		bool toModify = writer->load( strm );
		if( !toModify )
		{
			if( !writer->init() )
			{
				continue;
			}
		}

		writer->setTitle( (const wchar_t *) m_title );
		writer->setArtist( (const wchar_t *) m_artist );
		writer->setAlbum( (const wchar_t *) m_album );
		writer->setGenre( MyTagLib::Genre( (const wchar_t *) m_genre ) );
		writer->setComment( (const wchar_t *) m_comment );
		writer->setYear( (const wchar_t *) m_year );
		writer->setLyric( (const wchar_t *) m_lyric );
		writer->setTrackNumber( m_trackNo );

		if( toModify )
		{
			rw.addRange( writer->getRange() );
		}
		else
		{
			rw.addWholeNewRange( writer->getRange() );
		}
	}

	// ���б�ǩ���ɹ�д������ύ�ɹ�
	return rw.commit( strm );
}

wxString OOPSong::BuildTitle(const wxString& format) const
{
	wxString title( format );
	title.MakeUpper();

	bool atLeastOne = false;
	atLeastOne |= ParseFormat( title, L"%A", m_artist, L"(δ֪������)" );
	atLeastOne |= ParseFormat( title, L"%T", m_title, L"(δ֪����)" );

	wxString fn( fileName() );
	if( !atLeastOne ) // �ǳ��������������������Һͱ����Ϊ�գ����ļ�������
	{
		title = fn;
	}
	else
	{
		atLeastOne |= ParseFormat( title, L"%F", fn, wxEmptyString );
	}

	atLeastOne |= ParseFormat( title, L"%I", wxString::Format( L"%d", m_trackNo ),
							   L"(δ֪�����)" );

	return !atLeastOne ? fn : title;
}

/*static*/
bool OOPSong::ParseFormat(wxString& ret,
						  const wxString& format,
						  const wxString& data,
						  const wxString& def)
{
	bool res = true;
	wxString temp = data;
	if( temp.IsEmpty() )
	{
		temp = def;
		res = false;
	}

	ret.Replace( format, temp );

	return res;
}

wxString OOPSong::fileName() const
{
	if( m_listEntry->path().IsEmpty() )
		return wxEmptyString;

	wxFileName fn( m_listEntry->path() );
	return fn.GetName();
}

/*static*/
wxString OOPSong::GetStdTimeStr(unsigned int len)
{
	int seconds = len % 60;
	int minutes = (len - seconds) / 60;

	wxString time;
	time.Printf( L"%02d:%02d", minutes, seconds );

	return time;
}

bool OOPSong::TryGetArtistAndTitle(wxString& strArtist, wxString& strTitle) const
{
	strArtist = m_artist;
	strTitle = m_title;

	if( strArtist.empty() && strTitle.empty() )
	{
		wxString fn( fileName() );
		wxString::size_type pos = fn.find( L'-' );
		if( pos != wxString::npos )
		{
			strArtist.assign( fn.substr( 0, pos ) );
			strTitle.assign( fn.substr( pos + 1 ) );
		}
	}

	strArtist.Trim( true ).Trim( false );
	strTitle.Trim( true ).Trim( false );

	return !strArtist.empty() || !strTitle.empty();
}

struct AudioType {
	const wchar_t* extName;
	const wchar_t* desc;
};

wxString OOPSong::codec() const
{
	wxFileName fn( m_listEntry->path() );
	wxString ext( fn.GetExt().MakeUpper() );

	AudioType types[] = {
		{ L"MP3", L"MPEG 1 Layer 3" },
		{ L"WMA", L"Windows Media Audio 9" },
		{ L"ASF", L"Windows Media Audio 9" },
		{ L"WAV", L"PCM Audio" },
		{ L"OGG", L"Vorbis/Ogg Audio" },
		{ L"FLAC", L"FLAC Audio" },
		{ L"MIDI", L"MIDI Music" },
		{ L"MP4", L"MPEG-4 AAC Audio" },
		{ L"APE", L"Monkey's Audio" }
	};

	for( size_t i = 0; i < sizeof( types ) / sizeof( AudioType ); i++ )
	{
		if( ext == types[i].extName )
			return types[i].desc;
	}

	return L"Other";
}

bool OOPSong::isVBR() const
{
	//if( !m_impl )
	//	return false;

	// TODO: ��һ�����ģ���ʱ������
	return false;
}

//////////////////////////////////////////////////////////////////////////

extern wxString GetStdLyricDir();

wxString OOPSong::lrcPath() const
{
	if( m_lrcPath.empty() || !wxFileExists( m_lrcPath ) )
	{
		wxFileName fn( path() );
		
		// ��׼��ʴ洢Ŀ¼���������ͬ��Ŀ¼
		wxString dirs[] = {
			GetStdLyricDir(), 
			fn.GetPath() + wxFileName::GetPathSeparator()
		};

		size_t numDirs = sizeof( dirs ) / sizeof( dirs[0] );

		// ���ܵ��ļ���
		const static size_t numNames = 2;
		wxString names[numNames] = { fn.GetName() };

		// ���ݱ�ǩ��Ϣ���ļ������б�Ҫ�Ĳ²�
		wxString strArtist, strTitle;
		TryGetArtistAndTitle( strArtist, strTitle );

		if( !strArtist.empty() && !strTitle.empty() )
		{
			names[1].assign( strArtist + L" - " + strTitle );
		}

		for( size_t i = 0; i < numDirs; i++ )
		{
			for( size_t j = 0; j < numNames; j++ )
			{
				if( !names[j].empty() )
				{
					m_lrcPath = dirs[i] + names[j] + L".lrc";

					if( wxFileExists( m_lrcPath ) )
					{
						goto RET;
					}
				}
			}
		}

		// �趨һ��Ĭ�ϵĸ���ļ�·���Ǳ�Ҫ�ģ���������ⲿ����ļ�ʱ
		// ��֪��Ҫ���Ƶ�����ȥ
		m_lrcPath = dirs[0] + names[0] + L".lrc";
	}

RET:

	wxASSERT( !wxFileName( m_lrcPath ).GetName().empty() );
	return m_lrcPath;
}

bool OOPSong::IsTaggingSupported() const
{
	//wxASSERT( m_impl );

	//// TODO: ��ǰֻ֧�� MP3 ��ʽ�ı�ǩ��д
	//return m_impl->audioType == MusicManager::AT_MPEG;

	// TODO: ��ǰֻ֧�� MP3 ��ʽ�ı�ǩ��д
	return wxFileName( m_listEntry->path() ).GetExt().CmpNoCase( L"mp3" ) == 0;
}
