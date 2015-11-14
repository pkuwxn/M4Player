#include "StdAfx.h"
#include "OOPScanner.h"

#include "OOPProgressDlg.h"
#include "OOPSongPtr.h"
#include "OOPCharsetConv.h"

#include "VdkEvent.h"
#include <wx/filename.h> // for wxFileName::GetSize

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT( OOP_EVT_SCAN_DONE, wxCommandEvent );

OOPScanner::OOPScanner(wxWindow* dlgParent, wxArrayString* songList)
	: wxThread( wxTHREAD_JOINABLE ),
	  m_dlg( NULL ), m_dlgParent( dlgParent ),
	  m_songList( songList ), 
	  m_songs( new wxVector< OOPSongPtr > ),
	  m_idUpdateUi( VdkGetUniqueId() )
{
	wxASSERT( m_songList );
	
	enum {
		// ��Ҫʹ���߳�����û�����ĸ�����Ŀ����
		MIN_SIZE_TO_START_THREAD = 20,
	};

	// ����ȷ���ַ���ת������Ч
	OOPCharsetConv::CreateInstance();

	//============================================

	if( songList->size() > MIN_SIZE_TO_START_THREAD )
	{
		m_dlg = new OOPProgressDlg( m_dlgParent, L"���ڵ���...", wxEmptyString );

		Create();
		Run();

		// ������󱻵���
		if( IsRunning() ) // �����߳�̫���սᣬ��ʱ�Ի���û��ʾ
		{
			m_dlg->Bind( wxEVT_VOBJ, &OOPScanner::OnUpdateUI, 
						 this, m_idUpdateUi );

			m_dlg->CentreOnParent();
			m_dlg->ShowModal();
		}

		m_dlg->Destroy();
	}
	else
	{
		Entry();
	}
}

OOPScanner::~OOPScanner()
{
	wxCommandEvent e( OOP_EVT_SCAN_DONE );
	e.SetClientData( m_songs );
	wxPostEvent( m_dlgParent, e );

	wxDELETE( m_songList );
}

void OOPScanner::OnUpdateUI(VdkVObjEvent& e)
{
	wxASSERT( m_dlg );

	// �����ˡ�ȡ����
	if( !m_dlg->Update( e.GetInt(), e.GetString() ) )
	{
		Wait();
		m_dlg->EndModal( 0 );
	}
}

wxThread::ExitCode OOPScanner::Entry()
{
	wxASSERT( m_songList );
	wxASSERT( m_songs );

	VdkVObjEvent updateEvent( m_idUpdateUi );
	wxString filePath;

	wxArrayString::const_iterator i( m_songList->begin() );
	wxArrayString::const_iterator e( m_songList->end() );

	for( ; i != e; ++i )
	{
		if( m_dlg && m_dlg->IsCanceled() )
			break;

		// ��Ҫ����ļ���СΪ 0 ���ļ�
		if( wxFileName::GetSize( *i ) == 0 )
			continue;

		OOPListEntry* entry = new OOPListEntry( *i );
		OOPSongPtr song( new OOPSong( entry ) );
        song->ParseTag();

		m_songs->push_back( song );

		//==========================================

		if( m_dlg )
		{
			unsigned progress = i - m_songList->begin() + 1;
			progress /= double( m_songList->size() );
			progress *= 100;
			updateEvent.SetInt( progress );
			updateEvent.SetString( *i );

			wxPostEvent( m_dlg, updateEvent );
		}
	}

	// �����ļ��Ĵ�С��Ϊ 0
	if( m_songs->empty() )
	{
		updateEvent.SetInt( m_dlg->GetMaximum() );
		updateEvent.SetString( L"ɨ������������ļ��Ĵ�С��Ϊ�㡣" );

		wxPostEvent( m_dlg, updateEvent );
	}

	return (ExitCode) 0;
}
