/***************************************************************
 * Name:      OOPFilePropDlg.cpp
 * Purpose:   ���ļ����ԡ��Ի���
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-8-17
 **************************************************************/
#include "StdAfx.h"
#include "OOPFilePropDlg.h"

#include "DelayModStack.h"
#include "OOPSongPtr.h"

#include "VdkCtrlParserInfo.h"
#include "VdkEdit.h"
#include "VdkButton.h"
#include "VdkLabel.h"

#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT( OOP_EVT_SONG_INFO_UPDATED, wxCommandEvent );

class OOPFilePropDlg::PropDlgImpl
{
public:

	// ���캯��
	PropDlgImpl();

	// �󶨿ؼ� ID
	void BindCtrlIds(MapOfCtrlIdInfo& ids);

	// XRC �ļ��Ѿ��ɹ����룬���пؼ��Ѿ���
	void OnXrcLoaded();

	// ���Ի���ؼ�
	void UpdateDialog(OOPSongPtr song);

	// ���ݶԻ���ؼ����û�������������Ϣ
	void UpdateSongObj();

	// �����ı��浽��Ƶ�ļ�
	bool SaveToFile();

	// ���͸�����Ϣ�Ѹ��µ���Ϣ�¼�
	void SendInfoUpdatedEvent(wxEvtHandler* dst);

private:

	enum Controls {

		ID_GUESS_TAGS = 9789,

		ID_SC_N_TC, // �򷱻�ת
		ID_CHARSET_CONV,
		ID_TO_CPAPITAL,
		ID_DELETE_ALL,
		ID_UNDO,

		ID_SC2TC,
		ID_TC2SC,
	};

	enum VdkControls {

		ID_FILE_PATH = 6255,
		ID_TITLE,
		ID_ARTIST,
		ID_ALBUM,
		ID_TRACK_NO,
		ID_GENRE,
		ID_YEAR,
		ID_COMMENT,

		ID_CODEC,
		ID_BIT_RATE,
		ID_SAMPLE_RATE,
		ID_BITS,
		ID_CHANNELS,
		ID_LENGTH,
	};

	OOPSongPtr m_song;

	VdkEdit* m_filePath;
	VdkEdit* m_title;
	VdkEdit* m_artist;
	VdkEdit* m_album;
	VdkEdit* m_trackNo;
	VdkEdit* m_genre; // ����
	VdkEdit* m_year;
	VdkEdit* m_comment;

	VdkEdit* m_codec; // ����
	VdkEdit* m_stereo; // ������
	VdkEdit* m_sampleRate; // ����Ƶ��
	VdkEdit* m_bits; // ����
	VdkEdit* m_bitRate; // ����
	VdkEdit* m_channels; // ����
	VdkEdit* m_length; // ����
	VdkEdit* m_gain; // ����

	VdkButton* m_saveToFile; // �����浽�ļ�����ť
	VdkLabel* m_tip; // ��ʾ����������硰��֧�ֵ��ļ����͡�
};

OOPFilePropDlg::PropDlgImpl::PropDlgImpl()
{
	memset( this, 0, sizeof( *this ) );
}

void OOPFilePropDlg::PropDlgImpl::BindCtrlIds(MapOfCtrlIdInfo& ids)
{
	BindCtrl( L"filePath", ID_FILE_PATH, m_filePath );
	BindCtrl( L"title", ID_TITLE, m_title );
	BindCtrl( L"artist", ID_ARTIST, m_artist );
	BindCtrl( L"album", ID_ALBUM, m_album );
	BindCtrl( L"trackNo", ID_TRACK_NO, m_trackNo );
	BindCtrl( L"genre", ID_GENRE, m_genre );
	BindCtrl( L"year", ID_YEAR, m_year );
	BindCtrl( L"comment", ID_COMMENT, m_comment );

	BindCtrl( L"codec", ID_CODEC, m_codec );
	BindCtrl( L"bitRate", ID_BIT_RATE, m_bitRate );
	BindCtrl( L"sampleRate", ID_SAMPLE_RATE, m_sampleRate );
	BindCtrl( L"bits", ID_BITS, m_bits );
	BindCtrl( L"channels", ID_CHANNELS, m_channels );
	BindCtrl( L"length", ID_LENGTH, m_length );

	BindCtrl( L"save", wxID_ANY, m_saveToFile );
	BindCtrl( L"tip", wxID_ANY, m_tip );
}

void OOPFilePropDlg::PropDlgImpl::OnXrcLoaded()
{
	// TODO: �������Ǻ�׾�ӵ����
	// ����ϣ�����ɱ༭���ı��򱳾���ɫ��Ҫ��ɱ༭�����κα仯
	VdkEditStaticStyle* sstyle = m_filePath->GetStaticStyle();
	sstyle->uneditableBgBrush( *wxWHITE_BRUSH );
}

void OOPFilePropDlg::PropDlgImpl::UpdateDialog(OOPSongPtr song)
{
	m_song = song;
	if( !m_song->IsParsed() )
		m_song->ParseTag( false );

	m_filePath->SetValue( m_song->path() );
	m_artist->SetValue( m_song->artist() );
	m_title->SetValue( m_song->title() );
	m_album->SetValue( m_song->album() );
	m_genre->SetValue( m_song->genre() );
	m_comment->SetValue( m_song->comment() );
	m_year->SetValue( m_song->year() );

	m_trackNo->Clear();
	int trackNo = m_song->trackNo();
	if( trackNo > 0 )
	{
		*m_trackNo << trackNo;
	}

	m_codec->SetValue( m_song->codec() );

	//------------------------------------------------

	int bitRate = m_song->bitRate();
	wxString strBitRate;
	strBitRate.Printf( L"%u Kbps", bitRate );
	if( m_song->isVBR() )
		strBitRate += L" (VBR)";

	m_bitRate->SetValue( strBitRate );

	//------------------------------------------------

	wxString strSampleRate;
	strSampleRate.Printf( L"%d Hz", m_song->sampleRate() );
	m_sampleRate->SetValue( strSampleRate );

	//------------------------------------------------

	//m_bits->SetValue( wxString::Format( L"%u Bits", m_song->bits() ) );
	m_channels->SetValue( wxString::Format( L"%u", m_song->channels() ) );
	m_length->SetValue( OOPSong::GetStdTimeStr( m_song->length() ) );

	//------------------------------------------------

	m_title->GetVdkWindow()->FocusCtrl( m_title, NULL );
	m_title->SelectAll( NULL );

	if( wxFileExists( m_song->path() ) )
	{
		bool supported = m_song->IsTaggingSupported();
		m_saveToFile->Enable( supported, NULL );
		m_tip->SetCaption( !supported ? L"�ļ���Ч��������ļ��ı�ǩд�벻��֧��" : 
										wxEmptyString, 
							NULL );
	}
	else
	{
		m_saveToFile->Enable( false, NULL );
		m_tip->SetCaption( L"�ļ�������", NULL );
	}
}

void OOPFilePropDlg::PropDlgImpl::UpdateSongObj()
{
	m_song->path( m_filePath->GetValue() );
	m_song->artist( m_artist->GetValue() );
	m_song->title( m_title->GetValue() );
	m_song->album( m_album->GetValue() );
	m_song->genre( m_genre->GetValue() );
	m_song->comment( m_comment->GetValue() );
	m_song->year( m_year->GetValue() );

	m_song->trackNo( wxAtoi( m_trackNo->GetValue() ) );
}

bool OOPFilePropDlg::PropDlgImpl::SaveToFile()
{
	wxASSERT( m_song->IsTaggingSupported() );

	//--------------------------------------------------

	UpdateSongObj();

	bool saved = m_song->Save();
	if( !saved )
	{
		wxLogDebug( L"Error saving tags for `%s`.", m_song->path() );

		// ��ӵ��ӳ��޸�ջ
		SingleDelayModStack::Instance().Add( m_song );
	}

	return saved;
}

void OOPFilePropDlg::PropDlgImpl::SendInfoUpdatedEvent(wxEvtHandler* dst)
{
	wxCommandEvent e( OOP_EVT_SONG_INFO_UPDATED );
	e.SetString( m_song->path() );

	wxPostEvent( dst, e );
}

//////////////////////////////////////////////////////////////////////////

OOPFilePropDlg::OOPFilePropDlg(wxWindow* parent)
	: VdkDialog( parent, L"�ļ�����", wxSTAY_ON_TOP, 
				 VWS_DRAGGABLE | VWS_BASE_PANEL | VWS_DISMISS_BY_ESC ),
	  m_impl( new PropDlgImpl )
{
	MapOfCtrlIdInfo ids;
	m_impl->BindCtrlIds( ids );
	FromXrc( L"../../App/fileprop_dlg.xml", &ids );
	ClearPtrMap( ids );

	m_impl->OnXrcLoaded();

	Bind( wxEVT_CLOSE_WINDOW, &OOPFilePropDlg::OnClose, this );

	//-------------------------------

	enum {
		ID_SAVE_TO_FILE = 6300,
		ID_CLOSE_DLG,
	};

	FindCtrl( L"closeDlg" )->SetID( ID_CLOSE_DLG );
	Bind( wxEVT_VOBJ, &OOPFilePropDlg::CloseDlg, this, ID_CLOSE_DLG );

	FindCtrl( L"save" )->SetID( ID_SAVE_TO_FILE );
	Bind( wxEVT_VOBJ, &OOPFilePropDlg::SaveToFile, this, ID_SAVE_TO_FILE );
}

OOPFilePropDlg::~OOPFilePropDlg()
{
	wxDELETE( m_impl );
}

void OOPFilePropDlg::Popup(OOPSongPtr song)
{
	m_impl->UpdateDialog( song );
	
	// ���Ի����ƶ��������ڵ����ģ����ุ���ڶ��� 100px
	wxWindow* parent = GetParent();
	wxCoord parentX, parentY, parentWidth, myWidth;
	parent->GetPosition( &parentX, &parentY );
	parent->GetSize( &parentWidth, NULL );
	GetSize( &myWidth, NULL );
	
	Move( parentX + (parentWidth - myWidth) / 2, parentY + 100 );
	
	QueueRedrawEvent();
	Show();
}

void OOPFilePropDlg::SaveToFile(VdkVObjEvent& e)
{
	CloseDlg( e );
	
	m_impl->SaveToFile();
}

void OOPFilePropDlg::SendInfoUpdatedEvent()
{
	m_impl->SendInfoUpdatedEvent( GetParent() );
}

void OOPFilePropDlg::OnClose(wxCloseEvent&)
{
	HideAndFocusParent();	
}

void OOPFilePropDlg::CloseDlg(VdkVObjEvent&)
{
	HideAndFocusParent();
}

void OOPFilePropDlg::HideAndFocusParent()
{
	Hide();
	GetParent()->SetFocus();

	// ʼ�շ��͸�����Ϣ�����¼�(����ɨ��õ��ı�ǩ��Ϣ�����ǲ���ȷ��)
	SendInfoUpdatedEvent();
}
