/***************************************************************
 * Name:      OOPLyricSchDlg.cpp
 * Purpose:   歌词搜索对话框
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-08-20
 **************************************************************/
#include "StdAfx.h"
#include "OOPLyricSchDlg.h"

#include "ListView/VdkListView.h"
#include "ListView/LvpHeaderCtrl.h"
#include "ListView/LvpCrossColor.h"

#include "VdkChoiceCtrl.h"
#include "VdkEdit.h"
#include "VdkLabel.h"
#include "VdkButton.h"

#include "OOPDefs.h"
#include "OOPSongPtr.h"
#include "OOPSingleHttpThread.h"
#include "LyricGrabber/LyricGrabber.h"
#include "OOPSingleLyricTask.h"

#include "wxUtil.h" // for wxEasyCreatFont()

#include <wx/tokenzr.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

wxDECLARE_EVENT( EVT_UPDATE_TEXT, wxCommandEvent );

class LyricListAdapter : public VdkListView::Adapter
{
public:

	enum {
		NUM_COLUMNS = 2,
	};

	LyricListAdapter(int width)
		: m_list( NULL )
	{
		static ColumnStyle cs[NUM_COLUMNS] =
		{ { L"艺术家", static_cast<int>(width * 0.4), 0, TEXT_ALIGN_LEFT },
		  { L"标题",   static_cast<int>(width * 0.6), 1, TEXT_ALIGN_LEFT } };

		m_columns = cs;
	}

	virtual size_t GetRowCount() const
	{
		return m_list ? m_list->size() : 0;
	}

	virtual size_t GetColumnCount() const { return NUM_COLUMNS; }
	virtual bool IsEmpty() const { return !m_list || m_list->empty(); }

	virtual wxString GetCell(size_t row, size_t col) const
	{
		wxASSERT( m_list );

		switch( col )
		{
		case 0: return m_list->at( row ).artist;
		case 1: return m_list->at( row ).title;

		default:
			return L"Error!";
		}
	}

	virtual ColumnStyle& GetColumnStyle(size_t col) { return m_columns[col]; }

	void SetList(const LyricGrabber::ResultSet& list);
	void Clear();

	const LyricGrabber::SearchResult& GetResult(size_t index) const {
		return m_list->at( index );
	}

private:

	ColumnStyle* m_columns;
	const LyricGrabber::ResultSet* m_list;
};

void LyricListAdapter::SetList(const LyricGrabber::ResultSet& list)
{
	m_list = &list;
}

void LyricListAdapter::Clear()
{
	m_list = NULL;
}

//////////////////////////////////////////////////////////////////////////

OOPLyricSchDlg::OOPLyricSchDlg(wxWindow* parent)
	: VdkDialog( parent, L"搜索歌词", 0, 
				 VWS_DRAGGABLE |VWS_BASE_PANEL | VWS_DISMISS_BY_ESC ),
	  m_listAdapter( NULL ), m_clickPlugin( NULL )
{
	FromXrc( L"../../App/lyric_sch_dlg.xml", NULL );
	InitCtrls();

	//--------------------------------------

	VdkCtrlId id( VdkGetUniqueId() );
	FindCtrl( L"closeDlg" )->SetID( id );
	Bind( wxEVT_VOBJ, &OOPLyricSchDlg::OnCancel, this, id );
	Bind( wxEVT_CLOSE_WINDOW, &OOPLyricSchDlg::OnClose, this );

	//--------------------------------------

	LyricGrabber::Task& singleTask = OOPSingleLyricTask::Instance();

	singleTask.GetEventSinker()->
		Bind( LyricGrabber::OOP_EVT_LYRIC_LIST_LOADED, 
			  &OOPLyricSchDlg::OnListLoaded, 
			  this, singleTask.GetListLoadedEventId() );
	
	singleTask.GetEventSinker()->
		Bind( LyricGrabber::OOP_EVT_LYRIC_LOADED, 
			  &OOPLyricSchDlg::OnLyricLoaded, this, 
			  singleTask.GetLyricLoadedEvtId() );
}

OOPLyricSchDlg::~OOPLyricSchDlg()
{
	LyricGrabber::Task& singleTask = OOPSingleLyricTask::Instance();

	singleTask.GetEventSinker()->
		Unbind( LyricGrabber::OOP_EVT_LYRIC_LOADED, 
			    &OOPLyricSchDlg::OnLyricLoaded, this, 
				singleTask.GetLyricLoadedEvtId() );

	singleTask.GetEventSinker()->
		Unbind( LyricGrabber::OOP_EVT_LYRIC_LIST_LOADED, 
				&OOPLyricSchDlg::OnListLoaded, 
				this, singleTask.GetListLoadedEventId() );
}

void OOPLyricSchDlg::Popup(OOPSongPtr song)
{
	// 不要重复显示
	if( IsShown() )
	{
		return;
	}

	//-------------------------------------------------

	Fill( song );
	FocusCtrl( m_title, NULL );
	
	wxWindow* parent = GetParent();
	wxCoord parentX, parentY;
	parent->GetPosition( &parentX, &parentY );
	Move( parentX + 100, parentY + 100 );
	
	QueueRedrawEvent();
	Show();
}

void OOPLyricSchDlg::InitCtrls()
{
	VdkCtrlId id( VdkGetUniqueId( 10 ) );
	m_lyricList = (VdkListView *) FindCtrl( L"list" );
	m_lyricList->SetAddinStyle( VCS_BORDER_SIMPLE );
	m_lyricList->SetID( id );

	LvpHeaderCtrl* header = new LvpHeaderCtrl( m_lyricList );
	m_lyricList->AddPlugin( header );

	wxBrush b1( wxColour( 243,248,250 ) );
	wxBrush b2( *wxWHITE_BRUSH );
	LvpCrossColor* crossColor = new LvpCrossColor( m_lyricList, b1, b2 );
	m_lyricList->AddPlugin( crossColor );

	wxBrush selected( wxColour( 255,254,223 ) );
	m_clickPlugin = new LvpClick( m_lyricList, LvpClick::SM_SINGLE, selected );
	m_clickPlugin->SetOnItemClickListener( this );
	m_lyricList->AddPlugin( m_clickPlugin );

	int w;
	m_lyricList->GetSize( &w, NULL );
	m_listAdapter = new LyricListAdapter( w - 2 );
	m_lyricList->SetAdapter( m_listAdapter, VdkListView::ALTP_HELD );

	//-----------------------------------------

	m_lyricServer = (VdkChoiceCtrl *) FindCtrl( L"lyricServer" );
	m_lyricServer->Select( 0, NULL );
	m_lyricServer->SetID( id + 3 );

	m_artist = (VdkEdit *) FindCtrl( L"artist" );
	m_title = (VdkEdit *) FindCtrl( L"title" );
	m_resultLabel = (VdkLabel *) FindCtrl( L"result" );

	m_search = (VdkButton *) FindCtrl( L"search" );
	m_search->SetID( id + 1 );
	Bind( wxEVT_VOBJ, &OOPLyricSchDlg::OnSearch, this, id + 1 );

	m_download = (VdkButton *) FindCtrl( L"download" );
	m_download->SetID( id + 2 );
	Bind( wxEVT_VOBJ, &OOPLyricSchDlg::OnDownload, this, id + 2 );
}

void OOPLyricSchDlg::Fill(OOPSongPtr song)
{
	m_songPath = song->path();

	wxString strArtist, strTitle;
	if( !song->TryGetArtistAndTitle( strArtist, strTitle ) )
	{
		// 最终还是空白，用文件名来填充
		strTitle = song->fileName();
	}

	if( (strArtist == m_artist->GetValue()) &&
		(strTitle == m_title->GetValue()) )
	{
		return;
	}

	m_artist->SetValue( strArtist );
	m_title->SetValue( strTitle );

	ClearState( NULL );
}

void OOPLyricSchDlg::OnSearch(VdkVObjEvent& e)
{
	wxASSERT( !m_songPath.empty() );

	if( m_artist->IsEmpty() && m_title->IsEmpty() )
		return;

	m_search->Enable( false, e.GetVObjDC() );
	ClearState( e.GetVObjDC() );

	LyricGrabber::Task& singleTask = OOPSingleLyricTask::Instance();
	singleTask.SetHost( m_lyricServer->GetSelected() );

	singleTask.SetArtist( m_artist->GetValue() );
	singleTask.SetTitle( m_title->GetValue() );
	singleTask.SetPath( m_songPath );
	singleTask.GrabbList( LyricGrabber::Task::ST_RETURN_LIST );
}

void OOPLyricSchDlg::OnListLoaded(wxCommandEvent&)
{
	const LyricGrabber::ResultSet& lst = 
		OOPSingleLyricTask::Instance().GetList();

	VdkDC dc( this );

	if( !lst.empty() )
	{
		m_listAdapter->SetList( lst );

		// TODO: 其实并不需要
		size_t numLyrics = lst.size();
		for( size_t i = 0; i < numLyrics; i++ )
		{
			m_lyricList->NotifyItemAdd( i );
		}
		
		m_lyricList->UpdateSize( NULL );
		m_download->Enable( true, &dc );
	}
	else
	{
		m_resultLabel->SetCaption( L"没有找到匹配的歌词", &dc );
	}

	m_search->Enable( true, &dc );
    // 刷新当前状态
    m_lyricList->RefreshState( &dc );
}

void OOPLyricSchDlg::OnItemClick(LvpClick* parent, int index, wxDC& dc)
{
	Download( index, &dc );
}

void OOPLyricSchDlg::OnDownload(VdkVObjEvent& e)
{
	const LvpClick::ArrayOfSortedInts& selItems = 
		static_cast< const LvpClick* >( m_clickPlugin )->GetSelItems();

	if( selItems.empty() )
		return;

	Download( selItems[0], e.GetVObjDC() );
}

void OOPLyricSchDlg::Download(int index, wxDC* pDC)
{
	// 清空可能的错误信息
	m_resultLabel->SetCaption( wxEmptyString, pDC );

	// 禁用下载按钮
	m_download->Enable( false, pDC );

	OOPSingleLyricTask::Instance().GrabbLyric( index );
}

void OOPLyricSchDlg::OnLyricLoaded(wxCommandEvent& e)
{
	LyricGrabber::Task& singTask = OOPSingleLyricTask::Instance();
	if( singTask.GetSessionType() == LyricGrabber::Task::ST_RETURN_LIST )
	{
		VdkDC dc( this );
		m_download->Enable( true, &dc );

		VdkHttpThread::FinishEvent& finishEvt = 
			(VdkHttpThread::FinishEvent &) e;

		// 下载失败
		if( finishEvt.GetResult().empty() )
		{
			m_resultLabel->SetCaption( L"网络错误: 歌词下载失败", &dc );
			return;
		}

		m_resultLabel->SetCaption( wxEmptyString, &dc );
	}

	// 由 wxApp 处理
	e.SetId( wxID_ANY );
	wxTheApp->ProcessEventLocally( e );
}

void OOPLyricSchDlg::ClearState(wxDC* pDC)
{
	m_resultLabel->SetCaption( wxEmptyString, pDC );
	m_download->Enable( false, pDC );

	m_listAdapter->Clear();
	m_lyricList->NotifyClear();
	m_lyricList->UpdateSize( pDC );
}

bool OOPLyricSchDlg::DoHandleKeyEvent(wxKeyEvent& e)
{
	switch( e.GetKeyCode() )
	{
	case WXK_RETURN:
		{
			VdkDC dc( this );
			VdkVObjEvent fakeEvent;
			fakeEvent.SetVObjDC( &dc );

			OnSearch( fakeEvent );
			return true;
		}

	default:

		break;
	}

	return false;
}

void OOPLyricSchDlg::OnCancel(VdkVObjEvent&)
{
	HideAndFocusParent();
}

void OOPLyricSchDlg::OnClose(wxCloseEvent&)
{
	HideAndFocusParent();
}

void OOPLyricSchDlg::CloseDlg(VdkVObjEvent&)
{
	HideAndFocusParent();
}

void OOPLyricSchDlg::HideAndFocusParent()
{
    Hide();
    GetParent()->SetFocus();
}
