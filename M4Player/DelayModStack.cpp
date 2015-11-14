/***************************************************************
 * Name:      DelayModStack.cpp
 * Purpose:   ÑÓ³ÙÐÞ¸ÄÕ»
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-10
 **************************************************************/
#include "StdAfx.h"
#include "DelayModStack.h"

#include <algorithm> // for std::find()

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

bool DelayModStack::Add(OOPSongPtr song)
{
	bool existed = 
		std::find( m_stack.begin(), m_stack.end(), song ) != m_stack.end();

	if( !existed )
	{
		m_stack.push_back( song );
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

class RenameSessionFinder
{
public:

	RenameSessionFinder(OOPSongPtr song)
		: m_song( song )
	{

	}

	bool operator()(const DelayModStack::DelayRenameAction& dra)
	{
		return dra.song == m_song;
	}

private:

	OOPSongPtr m_song;
};

bool DelayModStack::Add(OOPSongPtr song, const wxString& newPath)
{
	RenameStack::iterator it =
		std::find_if( m_renameStack.begin(), m_renameStack.end(), 
					  RenameSessionFinder( song ) );

	if( it != m_renameStack.end() )
	{
		it->newPath = newPath;
	}
	else
	{
		DelayRenameAction dra = { song, newPath };
		m_renameStack.push_back( dra );
	}

	return true;
}

bool DelayModStack::Commit(OOPSongPtr song)
{
	bool allOk = false;

	Stack::iterator iter = std::find( m_stack.begin(), m_stack.end(), song );

	if( iter != m_stack.end() )
	{
		if( song->Save() )
		{
			wxLogDebug( L"`%s` saved.", song->path() );

			m_stack.erase( iter );
			allOk = true;
		}
	}

	//-----------------------------------------------
	// ÖØÃüÃûÐÞ¸ÄÕ»

	RenameStack::iterator rnIt =
		std::find_if( m_renameStack.begin(), m_renameStack.end(), 
					  RenameSessionFinder( song ) );

	if( rnIt != m_renameStack.end() )
	{
		if( wxRename( rnIt->song->path(), rnIt->newPath ) == 0 )
		{
			wxLogDebug( L"`%s` renamed.", song->path() );

			rnIt->song->path( rnIt->newPath );
			m_renameStack.erase( rnIt );
			allOk = true;
		}
		else
		{
			allOk = false;
		}
	}

	return allOk;
}

void DelayModStack::CommitAll()
{
	Stack::const_iterator iter( m_stack.begin() );
	for( ; iter != m_stack.end(); ++iter )
	{
		(*iter)->Save();
	}

	RenameStack::const_iterator rnIt( m_renameStack.begin() );
	for( ; rnIt != m_renameStack.end(); ++rnIt )
	{
		if( wxRenameFile( rnIt->song->path(), rnIt->newPath, false ) )
			rnIt->song->path( rnIt->newPath );
	}

	Clear();
}

void DelayModStack::Clear()
{
	m_stack.clear();
	m_renameStack.clear();
}
