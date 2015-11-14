/***************************************************************
 * Name:      OOPTagBatch.cpp
 * Purpose:   批量读写歌曲标签
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-27
 **************************************************************/
#include "StdAfx.h"
#include "OOPTagBatch.h"

#include "ID3v1.h"
#include "ID3v2.h"
#include "Lyrics3v2.h"
#include "APEv2.h"

#include <fstream>

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

OOPTagBatch::~OOPTagBatch()
{
	RemoveAllHandlers();
}

void OOPTagBatch::AddHandler(TagHandler hdler, OpMode om)
{
	MyTagLib::Tag* tag = NULL;
	switch( hdler )
	{
	case TH_ID3v1:			tag = new MyTagLib::ID3v1; break;
	case TH_ID3v2:			tag = new MyTagLib::ID3v2; break;
	case TH_LYRICS3v2:		tag = new MyTagLib::Lyrics3v2; break;
	case TH_APEv2:			tag = new MyTagLib::APEv2; break;

	default:
		return;
	}

	AddHandler( tag, om );
}

void OOPTagBatch::AddHandler(MyTagLib::Tag* hdler, OpMode om)
{
	m_readers.push_back( HandlerPtr( hdler ) );

	// 这是一个读写处理器
	if( om == OM_READ_WRITE )
		m_writers.push_back( hdler );
}

void OOPTagBatch::RemoveAllHandlers()
{
	m_readers.clear();
	m_writers.clear();
}

bool OOPTagBatch::LoadFile(const wxString& path)
{
	m_path = path;
	std::ifstream musicFile( (const char *) m_path, std::ios::binary );

	bool ok = false;
	ReaderVec::const_iterator iter( m_readers.begin() );
	for( ; iter != m_readers.end(); ++iter )
	{
		ok |= (*iter)->load( musicFile );
	}

	musicFile.close();
	return ok;
}

void OOPTagBatch::CloseFile()
{
	ReaderVec::const_iterator iter( m_readers.begin() );
	for( ; iter != m_readers.end(); ++iter )
	{
		(*iter)->clear();
	}
}

bool OOPTagBatch::SaveFile()
{
	if( m_path.empty() || m_writers.empty() )
		return false;

	std::fstream musicFile( (const char *) m_path, 
	    std::ios::in | std::ios::out | std::ios::binary );
	if( !musicFile )
		return false;

	//------------------------------------------------------

	bool ok = true;

	ReaderVec::const_iterator iter( m_readers.begin() );
	for( ; iter != m_readers.end(); ++iter )
	{
		if( !(*iter)->save( musicFile ) )
		{
			ok = false;
			break;
		}
	}

	musicFile.close();
	return ok;
}

#define DEFINE_STRING_ACCESSORS( field ) \
	wxString OOPTagBatch::Get ## field() const \
	{ \
		wxString field; \
		\
		ReaderVec::const_iterator iter( m_readers.begin() ); \
		for( ; iter != m_readers.end(); ++iter ) \
		{ \
			field.assign( (*iter)->get ## field() ); \
			\
			if( !field.empty() ) \
				return field; \
		} \
		\
		return wxEmptyString; \
	}

DEFINE_STRING_ACCESSORS( Artist )
DEFINE_STRING_ACCESSORS( Title )
DEFINE_STRING_ACCESSORS( Album )
DEFINE_STRING_ACCESSORS( Year )
DEFINE_STRING_ACCESSORS( Comment )
DEFINE_STRING_ACCESSORS( Lyric )
DEFINE_STRING_ACCESSORS( Genre )

int OOPTagBatch::GetTrackNumber() const
{
	int trackNumber;
	
	ReaderVec::const_iterator iter( m_readers.begin() );
	for( ; iter != m_readers.end(); ++iter )
	{
		trackNumber = (*iter)->getTrackNumber();
		
		if( trackNumber != MyTagLib::TRACK_NUMBER_NOT_SET )
			return trackNumber;
	}
	
	return 0;
}
