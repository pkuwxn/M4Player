/***************************************************************
 * Name:      OOPList.cpp
 * Purpose:   OOPList ʵ���ļ�
 * Author:    Ning (vanxining@139.com)
 * Created:   2010
 * Copyright: Wang Xiao Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPList.h"

#include "OOPApp.h"
#include "OOPSong.h"
#include "Playlist.h"

#include "VdkDC.h"
#include "wxUtil.h"

#include <wx/wfstream.h>
#include <wx/mstream.h> // for wxMemoryInputStream

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLONEABLE_VOBJECT( OOPList, VdkListCtrl );
extern OOPlayerApp* g_app;

enum {
	CNST_FILE_VERSION = 6, // ��ǰ�ѱ��沥���б��ļ���ʽ�İ汾��
	// �����б��ļ������ַ���ʹ�õĵ����ַ���С
	// ��ν�������ַ�����һ��ָ���� Unicode �ַ���Ҳ�п����Ƕ�� char ��ϳ�
	// һ��������� Unicode �ַ�
#if wxUSE_UNICODE_WCHAR
	CNST_SIZE_OF_CHAR_T = sizeof( wchar_t ),
#elif wxUSE_UNICODE_UTF8
	CNST_SIZE_OF_CHAR_T = sizeof( char ),
#endif

	// �����б�ʱ���е��Ҳ��ڱ߾�
	CNST_TIME_COLUMN_RIGHT_PADDINGS = 2,
};

//////////////////////////////////////////////////////////////////////////

OOPList::OOPList()
	: VdkListCtrl( VLCS_MULTI_SELECT )
{

}

OOPList::~OOPList()
{
	
}

void OOPList::Clone(OOPList*)
{

}

void OOPList::OnXrcCreate(wxXmlNode* node)
{
	VdkListCtrl::DoXrcCreate( node );
	SetRowHeight( 15 );

	//----------------------------------------------------------

	wxString strColor;
	wxColour color;

	// ���к�
	strColor = XmlGetChildContent( node, L"Color_Number" );
	m_snColor.Set( strColor );

	// ��Ŀ�����ı���ɫ
	strColor = XmlGetChildContent( node, L"Color_Text" );
	m_textColor.Set( strColor );

	// ʱ��
	strColor = XmlGetChildContent( node, L"Color_Duration" );
	m_timeColor.Set( strColor );

	// ������ɫ
	strColor = XmlGetChildContent( node, L"Color_Bkgnd" );
	color.Set( strColor );
	m_crossBrush1.SetColour( color );

	strColor = XmlGetChildContent( node, L"Color_Bkgnd2" );
	color.Set( strColor );
	m_crossBrush2.SetColour( color );

	// ���ڲ��ŵ���Ŀ��������Ŀ������
	strColor = XmlGetChildContent( node, L"Color_Hilight" );
	m_lockedColor.Set( strColor );

	// ����ɫ��(����ѡ��һ����Ŀ��ļ���)
	strColor = XmlGetChildContent( node, L"Color_Select" );
	m_selectedColor.Set( strColor );

	// ��ʾ�϶���ĿĿ��λ�õ���
	unsigned char r = color.Red(),
				  g = color.Green(),
				  b = color.Blue();

	m_draggTarget.SetColour( wxColour( 255 - r, 255 - g, 255 - b ) );

	//----------------------------------------------------------

	bool selectedBarInit = false;
	wxString strSelectedImage
		( GetFilePath( XmlGetChildContent( node, L"selected_image" ) ) );
	if( wxFileExists( strSelectedImage ) )
	{
		// ��ҪΪ stippleBitmap ��������λͼ
		wxBitmap stippleBitmap( strSelectedImage, wxBITMAP_TYPE_BMP );
		if( stippleBitmap.IsOk() )
		{
			selectedBarInit = true;

			stippleBitmap = stippleBitmap.GetSubBitmap
				( wxRect( 0, 0, 1, stippleBitmap.GetHeight() ) );

			m_selected.SetStipple( stippleBitmap );
		}
	}

	if( !selectedBarInit )
		CreateGradientBrush();
}

void OOPList::OnNotify(const VdkNotify& notice)
{
	VdkListCtrl::OnNotify( notice );

	if( notice.GetNotifyCode() == VCN_CREATE )
	{
		Playlist& playList = Playlist::GetInstance();
		Attach( &playList );

		//-----------------------------------------------

		int numRows = GetRowCount();

		wxString maxSN;
		maxSN.Printf( L"%d.", numRows );

		//-----------------------------------------------

		int width0, width1, width2;
		m_WindowImpl->GetTextExtent( maxSN, &width0, NULL, 0, 0, &m_Font );

		m_WindowImpl->GetTextExtent( L"99:99", &width2, NULL, 0, 0, &m_Font );
		width2 += CNST_TIME_COLUMN_RIGHT_PADDINGS;

		width1 = m_Rect.width - width0 - width2;

		//-----------------------------------------------

		// �������Լ���¡�õ����б�Ҫ�ظ�������
		if( GetColumnCount() == 0 )
		{
			// ���������
			InsertColumn( VdkLcColumnInitializer( this ).
						  width( width0 ).
						  textAlign( TEXT_ALIGN_RIGHT ).
						  textColor( m_snColor ) );

			// ���������
			InsertColumn( VdkLcColumnInitializer( this ).
						  index( 1 ).
						  percentage( 100 ).
						  width( width1 ).
						  textColor( m_textColor ) );

			// �������ʱ����
			InsertColumn( VdkLcColumnInitializer( this ).
						  width( width2 ).
						  textColor( m_timeColor ) );
		}
		else
		{
			LcColIter it( playList.cols.begin() );
			VdkLcColumn* colSN = *it;
			VdkLcColumn* colTitle = *(++it);
			VdkLcColumn* colDuration = *(++it);

			// �����
			colSN->x = 0;
			colTitle->x = width0;
			colDuration->x = width0 + width1;

			colSN->UpdateWidth( width0 );
			colTitle->UpdateWidth( width1 );
			colDuration->UpdateWidth( width2 );

			colSN->textColor = m_snColor;
			colTitle->textColor = m_textColor;
			colDuration->textColor = m_timeColor;

			colSN->font = m_Font;
			colTitle->font = m_Font;
			colDuration->font = m_Font;
		}

		m_locked = end();
	}
}

void OOPList::UpdateSnColumnWidth(int numRowsOld, int numRowsCurr)
{
	if( numRowsCurr == wxNOT_FOUND )
		numRowsCurr = GetRowCount();

	wxString sn, sn0;

	sn0.Printf( L"%d.", numRowsOld );
	sn.Printf( L"%d.", numRowsCurr );

	if( sn.length() > sn0.length() )
	{
		int w;
		m_WindowImpl->GetTextExtent( sn, &w, NULL, NULL, NULL, &m_Font );

		AjustCollumn( w, 0, 1 );
	}
}

void OOPList::AddToList(OOPSong& song, size_t numRows)
{
	wxString title( song.BuildTitle( g_app->GetPlayListTitleFormat() ) );
	AddToList( *(song.GetListEntry()), title, numRows );
}

void OOPList::AddToList(OOPListEntry& entry, const wxString& title, size_t numRows)
{
	VdkLcCell* head = Append( wxEmptyString, false );
	VdkLcCell* midCell = (VdkLcCell *) head->GetClientData();

	midCell->SetLabel( title );
	UpdateTime( (VdkLcCell *) midCell->GetClientData(), entry.length() );
	// ע�������õ��� midCell �� ClientData
	midCell->SetClientData( &entry );

	entry.addTime( wxGetLocalTime() );
}

void OOPList::UpdateTime(int row, unsigned int len)
{
	SetCellLabel( row, 2, OOPSong::GetStdTimeStr( len ) );
}

void OOPList::UpdateTime(VdkLcCell* cell, unsigned int len)
{
	cell->SetLabel( OOPSong::GetStdTimeStr( len ) );
}

void OOPList::CreateGradientBrush()
{
	wxMemoryDC memDC;
	wxBitmap stippleBitmap( 1, GetRowHeight() );
	wxRect rc( 0, 0, 1, GetRowHeight() );

	memDC.SelectObject( stippleBitmap );
	memDC.GradientFillLinear( rc,
							  m_selectedColor,
							  m_crossBrush1.GetColour(),
							  wxSOUTH );
	memDC.SelectObject( wxNullBitmap );

	m_selected.SetStipple( stippleBitmap );
}

VdkCusdrawReturnFlag OOPList::DoDrawCellText(const VdkLcCell* cell,
											 int col_index,
											 int index,
											 wxDC& dc,
											 VdkLcHilightState state)
{
	switch( state )
	{
	case VDKLC_HS_SELECTED:

		dc.SetTextForeground( GetSelectedColour() );
		break;

	case VDKLC_HS_NORMAL:
		{
			if( m_locked != end() )
			{
				if( cell == *m_locked )
					dc.SetTextForeground( m_lockedColor );
			}

			break;
		}

	default:

		break;
	}

	// ��̬���������
	VdkLcColumn* col = cell->GetColumn();
	int y = index * GetRowHeight();
	y += (col->height - dc.GetCharHeight()) / 2;

	switch( col_index )
	{
	case 0:
		{
			int w, tens = -1;
			wxString sn;
			sn.Printf( L"%d.", index + 1 );

			if( tens < index / 10 )
			{
				tens = index / 10;
				dc.GetTextExtent( sn, &w, NULL );
			}

			int x = col->x + (col->width - w);
			dc.DrawText( sn, x, y );

			return VCCDRF_SKIPDEFAULT;
		}

	default:
		break;
	}

	return VCCDRF_DODEFAULT;
}

VdkCusdrawReturnFlag OOPList::DoEraseRow
	(const LcCellIter& it, int index, wxDC& dc)
{
	LcCellIter lastSel = GetLastSel();
	if( it == lastSel )
	{
		dc.SetPen( wxPen( GetSelectedColour() ) );
	}

	return VCCDRF_DODEFAULT;
}

inline wxColour OOPList::GetSelectedColour() const
{
	return (m_selectedColor != *wxWHITE) ? *wxWHITE : m_lockedColor;
}

void OOPList::RedrawRows(const wxArrayInt& rows, wxDC& dc)
{
	bool atLeastOne = false;
	int count = rows.size();
	for( int i = 0; i < count; i++ )
	{
		if( rows[i] != wxNOT_FOUND )
		{
			atLeastOne = true;
			break;
		}
	}

	if( !atLeastOne )
		return;

	VdkDcDeviceOriginSaver saver( dc );
	PrepareDC( dc );

	for( int i = 0; i < count; i++ )
	{
		if( rows[i] != wxNOT_FOUND )
		{
			UpdateRow( rows[i], dc );
		}
	}
}

void OOPList::OnMouseEvent(VdkMouseEvent& e)
{
	if( IsEmpty() )
		return;

	// ���������������ѡ�еİ׿�
	LcCellIter lastSelOld = GetLastSel();

	VdkListCtrl::OnMouseEvent( e );

	// �������õ�ǰ DC ��ͼ״̬(�豸����Ͳü�����)��������������� UpdateRow()
	// ����Ҫ�� DC ���Ѿ����չ������ڵĵ�ǰ��������������˵ġ�
	PrepareDC( e.dc );

	UpdateRow( GetLastSel(), e.dc );
	UpdateRow( lastSelOld, e.dc );
}

void OOPList::OnKeyEvent(VdkKeyEvent& e)
{
	if( e.evtCode != KEY_DOWN )
		return;

	int lastSelOld = GetLastSelIndex();
	VdkListCtrl::OnKeyEvent( e );

	//----------------------------------------------

	wxArrayInt rows;
	rows.Add( GetLastSelIndex() );
	rows.Add( lastSelOld );

	RedrawRows( rows, e.dc );
}

void OOPList::SetLocked(int index, wxDC* pDC)
{
	LcCellIter locked( GetCellIterator( 1, index ) );
	SetLocked( locked, pDC );
}

void OOPList::SetLocked(const LcCellIter& locked, wxDC* pDC)
{
	LcCellIter old = m_locked;
	m_locked = locked;

	int index = wxNOT_FOUND;
	if( m_locked != end() )
	{
		index = IndexOf( *m_locked );

		int ystart;
		GetViewStart( NULL, &ystart );
		int lastVisuable = ystart + GetShownItems() - 1;

		if( index == lastVisuable ) // ��ǰ�������һ�����»�һ��
			SetViewStart( 0, ++ystart, pDC );
		else if( index == ystart - 1 ) // ��ǰ���ĵ�һ�����ϻ�һ��
			SetViewStart( 0, --ystart, pDC );
		else if( (index < ystart) || (index > lastVisuable) ) // ���ڿ��ӷ�Χ��
			SetViewStart( 0, index, pDC );
	}

	if( pDC )
	{
		VdkDcDeviceOriginSaver saver( *pDC );
		PrepareDC( *pDC );

		if( index != wxNOT_FOUND )
			UpdateRow( index, *pDC );

        if( old != end() )
        {
            int oldindex = IndexOf( *old );
            if( oldindex != wxNOT_FOUND )
                UpdateRow( oldindex, *pDC );
        }

		RestoreDC( *pDC );
	}
}

OOPSongPtr OOPList::GetSongPtr(int index)
{
	return GetSongPtr( GetCellIterator( 1, index ) );
}

OOPSongPtr OOPList::GetSongPtr(const LcCellIter& i)
{
	wxASSERT( i != end() );
	return OOPSongPtr( new OOPSong( GetListEntry( i ) ) );
}

OOPListEntry* OOPList::GetListEntry(const LcCellIter& i)
{
	wxASSERT( i != end() );
	return (OOPListEntry *) (*i)->GetClientData();
}

void OOPList::DoClear(wxDC* pDC)
{
	RemoveAllSongPtrs();
}

void OOPList::RemoveSongPointer(const LcCellIter& i)
{
	wxASSERT( i != end() );
	wxASSERT( i != m_locked );

	VdkLcCell* cell = *i;
	delete (OOPListEntry*) cell->GetClientData();

	cell->SetClientData( NULL );
}

void OOPList::RemoveSongPointer(int i)
{
	RemoveSongPointer( GetCellIterator( 1, i ) );
}

void OOPList::RemoveAllSongPtrs()
{
	// ���뱣֤��Ų���ʹ����ס����
	wxASSERT( !HasLocked() );

	LcCellIter i( begin() ), e( end() );
	for( ; i != e; ++i )
		RemoveSongPointer( i );
}

void OOPList::RemoveSong(int index)
{
	// ���뱣֤��Ų���ʹ����ס����
	wxASSERT( index != GetLockedIndex() );

	int count0 = GetRowCount(), count;

	RemoveSongPointer( index );
	RemoveRow( index, false, NULL );

	count = count0 - 1;
	count0 = GetNumBitCount( count0 ) + 1;
	count = GetNumBitCount( count ) + 1;

	// ��С�����
	if( count < count0 )
		AjustCollumn( m_WindowImpl->GetCharWidth() * count, 0, 1 );
}

void OOPList::RemoveSong(const LcCellIter& i)
{
	RemoveSong( IndexOf( *i ) );
}

int OOPList::GetLockedIndex()
{
	return (m_locked != end()) ? IndexOf( *m_locked ) : wxNOT_FOUND;
}

OOPSongPtr OOPList::GetLockedSong()
{
	return HasLocked() ? GetSongPtr( m_locked ) : OOPSongPtr( NULL );
}

void OOPList::Serialize(wxInputStream& fin)
{
	wxASSERT( fin.IsOk() );

	wxMemoryInputStream meminp( fin );

	// ����汾��
	int version = ReadVar< int >( meminp );
	if( version != CNST_FILE_VERSION )
	{
		wxLogDebug( L"�б��ļ��İ汾������\n�ļ��汾: %d��"
					L"����֧�ֵİ汾: %d",
					version, (int) CNST_FILE_VERSION );

		return;
	}

	// ��������б�ʹ�õĿ��ַ��Ĵ�С
	int szOfWch = ReadVar< int >( meminp );
	if( szOfWch != CNST_SIZE_OF_CHAR_T )
	{
		wxLogDebug( L"�б��ļ��Ĳ�֧�ֿ�ƽ̨ʹ�á�\n"
					L"�����ļ���ʱʹ�õĿ��ַ���СΪ %d �ֽڣ�"
					L"��ǰ����֧�ֵĿ��ַ���СΪ %d �ֽڡ�",
					szOfWch, (int) CNST_SIZE_OF_CHAR_T );
		return;
	}

	// �����������
	int count = ReadVar< int >( meminp );

	//----------------------------------------------------------

	int len;
	wxString title, songPath;
	OOPListEntry* entry;

	// ѭ����������б�
	for( int i = 0; i < count; i++ )
	{
		try
		{
			// �����б����
			title = ReadString( meminp );
			// �����������
			len = ReadVar< int >( meminp );
			// �������·��
			songPath = ReadString( meminp );
		}
		catch( serialize_bad_format )
		{
			wxLogDebug( L"�����б��ʽ����ȷ��" );
			break;
		}

		entry = new OOPListEntry( songPath );

		//======================================================

		entry->length( len );
		// ����������Ŵ���
		entry->playCount( ReadVar< int >( meminp ) );
		// ����������벥���б��ʱ��
		entry->addTime( ReadVar< long >( meminp ) );

		AddToList( *entry, title, i );
	}

	UpdateSnColumnWidth( 0, count );
	UpdateSize( NULL );
}

void OOPList::Serialize(wxOutputStream& fout)
{
	wxASSERT( fout.IsOk() );

	// д��汾��
	WriteVar( fout, (int) CNST_FILE_VERSION );
	// д�봴������б�Ĳ���ϵͳ
	WriteVar( fout, (int) CNST_SIZE_OF_CHAR_T );
	// д���������
	WriteVar( fout, GetRowCount() );

	//----------------------------------------------------------

	wxString title;

	LcCellIter e( end() );
	for( LcCellIter i( begin() ); i != e; ++i )
	{
		VdkLcCell* cell = *i;
		OOPListEntry* entry = (OOPListEntry *) cell->GetClientData();
		title = cell->GetLabel();

		// д���б������
		WriteString( fout, title );
		// д���������
		WriteVar( fout, entry->length() );
		// д�����·��
		WriteString( fout, entry->path() );

		// д��������Ŵ���
		WriteVar( fout, entry->playCount() );
		// д��������벥���б��ʱ��
		WriteVar( fout, entry->addTime() );
	}
}

//////////////////////////////////////////////////////////////////////////

bool CompareTitle(const VdkLcCell* c1, const VdkLcCell* c2)
{
	return CompareUnicodeStrings( c1->GetLabel(), c2->GetLabel() ) < 0;
}

bool ComparePath(const VdkLcCell* c1, const VdkLcCell* c2)
{
	OOPListEntry *e1 = (OOPListEntry *) c1->GetClientData(),
				 *e2 = (OOPListEntry *) c2->GetClientData();

	return CompareUnicodeStrings( e1->path(), e2->path() ) < 0;
}

bool CompareAlbum(const VdkLcCell* c1, const VdkLcCell* c2)
{
	OOPListEntry *e1 = (OOPListEntry *) c1->GetClientData(),
				 *e2 = (OOPListEntry *) c2->GetClientData();

	OOPSong s1( e1 );
	OOPSong s2( e2 );

	s1.ParseTag();
	s2.ParseTag();

	return CompareUnicodeStrings( s1.album(), s2.album() ) < 0;
}

bool CompareLength(const VdkLcCell* c1, const VdkLcCell* c2)
{
	OOPListEntry *e1 = (OOPListEntry *) c1->GetClientData(),
				 *e2 = (OOPListEntry *) c2->GetClientData();

	return e1->length() < e2->length();
}

bool CompareAddTime(const VdkLcCell* c1, const VdkLcCell* c2)
{
	OOPListEntry *e1 = (OOPListEntry *) c1->GetClientData(),
				 *e2 = (OOPListEntry *) c2->GetClientData();

	return e1->addTime() < e2->addTime();
}

bool ComparePlayCount(const VdkLcCell* c1, const VdkLcCell* c2)
{
	OOPListEntry *e1 = (OOPListEntry *) c1->GetClientData(),
				 *e2 = (OOPListEntry *) c2->GetClientData();

	return e1->playCount() < e2->playCount();
}

void OOPList::SortList(VdkCtrlId cmd)
{
	// wxList �� std::list ���������㲻ͬ��
	// �� ǰ�߲�֧�� splice()
	// �� ǰ�ߵ��� sort() �������ȫ��ָ��ͬ��ֵ(��һ�������ϵ�ʧЧ)

	switch( cmd )
	{
	case OTBM_SORT_BY_TITLE:

		Sort( 1, CompareTitle );
		break;

	case OTBM_SORT_BY_ALBUM:

		Sort( 1, CompareAlbum );
		break;

	case OTBM_SORT_BY_PATH:

		Sort( 1, ComparePath );
		break;

	case OTBM_SORT_BY_LENGTH:

		Sort( 1, CompareLength );
		break;

	case OTBM_SORT_BY_ADD_TIME:

		Sort( 1, CompareAddTime );
		break;

	case OTBM_SORT_BY_PLAY_COUNT:

		Sort( 1, ComparePlayCount );
		break;

	default:

		break;
	}

	//------------------------------------------------------------

	// ����ʱ���е���ʾ��������
	int row = 0;
	LcCellIter i1( begin() );
	LcCellIter i2( GetCellIterator( 2, 0 ) );
	LcCellIter e( end() );

	for( ; i1 != e; ++i2, ++i1, ++row )
	{
		OOPListEntry* entry = (OOPListEntry *) (*i1)->GetClientData();
		UpdateTime( *i2, entry->length() );
	}

	m_Window->QueueRedrawEvent();
}

LcCellIter OOPList::begin()
{
	return GetCellIterator( 1, 0 );
}

LcCellIter OOPList::end()
{
	return GetColumnEnd( 1 );
}
