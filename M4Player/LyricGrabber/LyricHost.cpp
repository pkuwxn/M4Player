/***************************************************************
 * Name:      LyricHost.cpp
 * Purpose:   ��ʷ�����
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-25
 **************************************************************/
#include "StdAfx.h"
#include "LyricHost.h"

#include "LyricGrabber.h"
#include "wxCharsetHelper.h" // for wxCharsetHelper::Escape()

#include "wxUtil.h" // for XML parsing

#include <wx/xml/xml.h>
#include <wx/sstream.h> // for XML parsing

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

namespace LyricGrabber
{

	LyricHost::LyricHost(const wxString& listUrlTemplate)
		: m_listUrlTemplate( listUrlTemplate ), 
		  m_escapeTo( wxFONTENCODING_GB2312 )
	{

	}

	LyricHost::~LyricHost()
	{

	}

	void LyricHost::SetUrlEscapeDestCharset(const wxCSConv& conv)
	{
		m_escapeTo = conv;
	}

	wxString LyricHost::PrepareForTask(const Task& task)
	{
		wxString url( m_listUrlTemplate );

		m_artist.assign( task.GetArtist() );
		if( !m_artist.empty() )
		{
			wxString escaped( wxCharsetHelper::Escape( m_artist, m_escapeTo ) );
			url.Replace( L"{artist}", escaped );
		}

		m_title.assign( task.GetTitle() );
		if( !m_title.empty() )
		{
			wxString escaped( wxCharsetHelper::Escape( m_title, m_escapeTo ) );
			url.Replace( L"{title}", escaped );
		}

		return url;
	}

	//////////////////////////////////////////////////////////////////////////

	/*! ��ѡ�ĸ�����ط����� */
	enum HostId {
		LH_QQ_MUSIC, /*! QQ ���� */
		LH_BAIDU_ZHANGMEN, /*! �ٶ��������� */
		LH_TTPLAYER, /*! ǧǧ���� */
	};

	wxString Gb2312XmlHack(const wxString& xml)
	{
		wxString ret( xml );
		ret.Replace( L"gb2312", L"utf-8" ); // wxXml ֻ֧�� UTF-8 ����
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	// QQ ����

	class QQMusic : public LyricHost
	{
	public:

		// ���캯��
		QQMusic();

	private:

		// �������ص��ĸ���б�
		virtual bool ParseList(const wxString& list, ResultSet&) const;

		// �������ص��ĸ��
		virtual bool ParseLyric(wxString& lyric) const;
	};

	QQMusic::QQMusic()
		: LyricHost( L"http://qqmusic.qq.com/fcgi-bin/qm_getLyricId.fcg?"
					 L"name={title}&singer={artist}&from=qqplayer" )
	{
		SetUrlEscapeDestCharset( wxFONTENCODING_GB2312 );
	}

	bool QQMusic::ParseList(const wxString& list, ResultSet& lyrics) const
	{
		lyrics.clear();

		//-----------------------------------------

		if( !list.EndsWith( L"-qq-music>" ) )
			return false;

		wxStringInputStream stream( Gb2312XmlHack( list ) );
		wxXmlDocument doc( stream );

		if( !doc.IsOk() )
		{
			wxLogDebug( L"Error parsing the lyric list." );
			return false;
		}

		wxXmlNode* cmd( doc.GetRoot()->GetChildren() );
		wxXmlNode* numResultsNode( FindChildNode( cmd, L"songcount" ) );
		int numResults = XmlGetContentOfNum( numResultsNode, 0 );

		if( numResults == 0 )
		{
			return false;
		}

		//-----------------------------------------

		wxXmlNode* songInfoNode = FindChildNode( cmd, L"songinfo" );
		SearchResult res;

		do
		{			
			res.artist = XmlGetChildContent( songInfoNode, L"singername" );
			res.title = XmlGetChildContent( songInfoNode, L"name" );

			wxString lyricId( songInfoNode->GetAttribute( L"id" ) );
			res.url.Printf( L"http://music.qq.com/miniportal/static/lyric/%s/%s.xml",
							lyricId.Mid( lyricId.length() - 2 ),
							lyricId );
			
			lyrics.push_back( res );

			//-------------------------------------

			songInfoNode = FindSibling( songInfoNode );
		}
		while( songInfoNode );

		return !lyrics.empty();
	}


	bool QQMusic::ParseLyric(wxString& lyric) const
	{
		if( lyric.Trim( true ).Trim( false ).empty() || 
			!lyric.EndsWith( L"</lyric>" ) )
		{
			wxLogDebug( L"[%s:%d]���ص����ݲ�����(����:%d)", __FILE__, __LINE__, 
						lyric.length() );

			lyric.clear();
			return false;
		}

		wxStringInputStream stream( Gb2312XmlHack( lyric ) );
		wxXmlDocument doc( stream );

		if( doc.IsOk() )
		{
			lyric = doc.GetRoot()->GetNodeContent();
			return true;
		}
		else
		{
			lyric.clear();
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// �ٶ����ֺ�

	class BaiduZhangmen : public LyricHost
	{
	public:

		/// ���캯��
		BaiduZhangmen();

	private:

		virtual bool ParseList(const wxString& list, ResultSet&) const;
	};

	BaiduZhangmen::BaiduZhangmen()
		: LyricHost( L"http://box.zhangmen.baidu.com/x?op=12&count=1&"
					 L"title={title}$${artist}$$$$" )
	{
		SetUrlEscapeDestCharset( wxFONTENCODING_GB2312 );
	}

	bool BaiduZhangmen::ParseList(const wxString& list, ResultSet& lyrics) const
	{
		lyrics.clear();

		//==================================

		if( !list.EndsWith( L"</result>" ) )
			return false;

		wxStringInputStream stream( Gb2312XmlHack( list ) );
		wxXmlDocument doc( stream );

		// TODO:
		if( !doc.IsOk() )
		{
			wxLogDebug( L"Error parsing the lyric list." );
			return false;
		}

		SearchResult item;
		wxXmlNode* resultNode( doc.GetRoot() );
		wxXmlNode* urlNode = resultNode->GetChildren();

		while( urlNode )
		{
			if( urlNode->GetName() == L"url" )
			{
				wxXmlNode* lrcIdNode( FindChildNode( urlNode, L"lrcid" ) );
				int lrcId = XmlGetContentOfNum( lrcIdNode, 0 );

				item.artist = GetArtist();
				item.title = GetTitle();
				item.url.Printf( L"http://box.zhangmen.baidu.com/bdlrc/%d/%d.lrc",
								 lrcId / 100, lrcId );

				lyrics.push_back( item );
			}

			urlNode = urlNode->GetNext();
		}

		return !lyrics.empty();
	}

	//////////////////////////////////////////////////////////////////////////

	/*static*/
	LyricHost* LyricHost::Create(int hostId)
	{
		switch( hostId )
		{
		case LH_QQ_MUSIC:
			return new QQMusic;

		case LH_BAIDU_ZHANGMEN:
			return new BaiduZhangmen;

		default:
			return NULL;
		}
	}

	void LyricHost::GrabbLyric(const wxString& url)
	{

	}
}
