/***************************************************************
 * Name:      OOPSong.cpp
 * Purpose:   歌曲对象定义
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-19
 * Copyright: Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPSong.h"

#include "OOPTagBatch.h"
#include "OOPCharsetConv.h"
#include "OOPUtil.h"
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

// 一些可能在下一版本就改变的内部实现信息
struct OOPSong::Impl
{
public:

	// 构造函数
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
	// 已然读取并保存
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
        wxScopedCharBuffer path( NarrowedPath( m_listEntry->path() ) );

		if( music.openFromFile( path ) )
		{
			m_listEntry->length( music.getDuration().asSeconds() );
			m_bitRate = music.getBitRate();
			m_channels = music.getChannelCount();
			m_sampleRate = music.getSampleRate();
			m_bits = 16; // TODO:
		}
	}

	//=====================================================
	// 不需要检测 IsTaggingSupported()

	OOPTagBatch& bat = SingleTagBatch::Instance();
	if( !bat.IsOk() )
	{
		OOPCharsetConv::CreateInstance();
			
		// 优先读取我们自己写入的
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

	// 不再需要它了，关闭文件
	bat.CloseFile();

	m_parsed = true;
}

bool OOPSong::Save()
{
	// TODO: 这是一个非常拙劣的设计，应该每一种文件类型拥有一个批量读写栈

	// 假如不被支持，那么当作保存成功来看待，避免 DelayModStack 的反复尝试
	if( !IsTaggingSupported() )
		return true;

	wxASSERT( !m_listEntry->path().empty() );

	// 假如文件都不存在了，那么后续尝试再无意义，当作写入成功
	if( !wxFileExists( m_listEntry->path() ) )
		return true;

	OOPTagBatch& bat = SingleTagBatch::Instance();
	if( bat.GetWriterCount() == 0 )
		return true; // 不要再次尝试，当作写入成功

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

		// TODO: 假如前面读了同一个文件，那么应该不用重读
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

	// 所有标签都成功写入才算提交成功
	return rw.commit( strm );
}

wxString OOPSong::BuildTitle(const wxString& format) const
{
	wxString title( format );
	title.MakeUpper();

	bool atLeastOne = false;
	atLeastOne |= ParseFormat( title, L"%A", m_artist, L"(未知艺术家)" );
	atLeastOne |= ParseFormat( title, L"%T", m_title, L"(未知标题)" );

	wxString fn( fileName() );
	if( !atLeastOne ) // 非常特殊的情况，假如艺术家和标题均为空，用文件名代替
	{
		title = fn;
	}
	else
	{
		atLeastOne |= ParseFormat( title, L"%F", fn, wxEmptyString );
	}

	atLeastOne |= ParseFormat( title, L"%I", wxString::Format( L"%d", m_trackNo ),
							   L"(未知音轨号)" );

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

	// TODO: 有一个表格的，暂时不管它
	return false;
}

//////////////////////////////////////////////////////////////////////////

extern wxString GetStdLyricDir();

wxString OOPSong::lrcPath() const
{
	if( m_lrcPath.empty() || !wxFileExists( m_lrcPath ) )
	{
		wxFileName fn( path() );
		
		// 标准歌词存储目录和与歌曲相同的目录
		wxString dirs[] = {
			GetStdLyricDir(), 
			fn.GetPath() + wxFileName::GetPathSeparator()
		};

		size_t numDirs = sizeof( dirs ) / sizeof( dirs[0] );

		// 可能的文件名
		const static size_t numNames = 2;
		wxString names[numNames] = { fn.GetName() };

		// 根据标签信息对文件名进行必要的猜测
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

		// 设定一个默认的歌词文件路径是必要的，否则关联外部歌词文件时
		// 不知道要复制到哪里去
		m_lrcPath = dirs[0] + names[0] + L".lrc";
	}

RET:

	wxASSERT( !wxFileName( m_lrcPath ).GetName().empty() );
	return m_lrcPath;
}

bool OOPSong::IsTaggingSupported() const
{
	//wxASSERT( m_impl );

	//// TODO: 当前只支持 MP3 格式的标签读写
	//return m_impl->audioType == MusicManager::AT_MPEG;

	// TODO: 当前只支持 MP3 格式的标签读写
	return wxFileName( m_listEntry->path() ).GetExt().CmpNoCase( L"mp3" ) == 0;
}
