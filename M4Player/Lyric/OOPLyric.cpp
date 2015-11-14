/***************************************************************
 * Name:      OOPLyric.h
 * Purpose:   �����ڸ���㴰�ڵ���Ƕ�����ʾ�ؼ�
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2010
 **************************************************************/
#include "StdAfx.h"
#include "OOPLyric.h"

#include "OOPLyricParser.h"
#include "../OOPStopWatch.h"
#include "../OOPDefs.h"
#include "../OOPApp.h"

#include "VdkDC.h"
#include "VdkWindow.h"
#include "wxUtil.h" // for XmlInsertChild()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

// OOPLyric ������ʱ״̬���Լ�
enum OOPLyricState {

	// �Ƿ��Ѿ���ʼ
	OLST_STARTED					=	1 << ( VLCST_USER + 0 ),
	// �Ƿ��Ѿ�ֹͣ
	OLST_STOPPED					=	1 << ( VLCST_USER + 1 ),
	// �Ƿ��Ѿ���ͣ
	OLST_PAUSED						=	1 << ( VLCST_USER + 2 ),
	// �Ƿ��Ѿ���ʼ�϶����
	OLST_DRAGGING_STARTED			=	1 << ( VLCST_USER + 3 ),
	// �Ƿ������϶����
	OLST_DRAGGING					=	1 << ( VLCST_USER + 4 ),
	// �϶����ǰ����Ƿ���������
	OLST_RUNNING_BEFORE_DRAGGING	=	1 << ( VLCST_USER + 5 ),
};

enum {
	/*! ʹ�ý���ɫ��ԭ��һ���ʳ�����ʱ��
	 */
	ALPHA_SHOW_LAST_LINE_MS = 1000,
};

IMPLEMENT_DYNAMIC_VOBJECT( OOPLyric );

//////////////////////////////////////////////////////////////////////////

OOPLyric::OOPLyric()
	: VdkListCtrl( 0 ), m_parser( NULL ), 
	  m_timerId( wxIdManager::ReserveId( 1 ) ), 
	  m_timer( wxTheApp, m_timerId )
{
	// ֱ�Ӱ󶨱������ݼ�
	Attach( &m_dataSet );
}

void OOPLyric::Init()
{
	wxASSERT( m_parser );

	m_currLine = m_parser->end();
	m_blankLinesTop = 0;
	m_blankLinesBottom = 0;

	m_draggDistance = 0;
	m_draggHit = m_parser->end();

	//---------------------------------------------------------
	// �������λ״̬
	// TODO: д������������

	unsigned bitfieldsClean = unsigned(~0);
	bitfieldsClean >>= sizeof( bitfieldsClean ) * 8 - VLCST_USER;

	SetState( GetState() & bitfieldsClean );
}

void OOPLyric::OnXrcCreate(wxXmlNode* node)
{
	// ���� VdkControl �ĺ���
	VdkControl::DoXrcCreate( node );

	XmlInsertChild( node, L"header", L"0" );
	VdkListCtrl::OnXrcCreate( node );

	wxArrayString colorArray;
	colorArray.Add( XmlGetChildContent( node, L"TextColor" ) );
	colorArray.Add( XmlGetChildContent( node, L"HilightColor" ) );
	colorArray.Add( XmlGetChildContent( node, L"BkgndColor" ) );

	Create( colorArray );
}

void OOPLyric::Create(const wxArrayString& colorArray)
{
	m_strName.assign( L"OOPLyric" );
	m_align = ALIGN_SYNC_X_Y;

	SetRowHeight( 15 ); // �и�
	SetLinesUpDown( 0 ); // ����ʧȥ������
	SetScrollRate( 1, 1 ); // ÿ�и� 1px
	SetShownItemsAddIn( 1 ); // ÿ������ʾ1��
	CalcShownItems();

	SetAddinStyle( VSWS_NO_SCROLLBAR | VLCS_NO_SELECT );

	//======================================================

	m_TextColor.Set( colorArray[0] );
	m_HilightColor.Set( colorArray[1] );
	m_BgColor.Set( colorArray[2] );
	m_crossBrush1 = m_crossBrush2 = wxBrush( m_BgColor, wxSOLID );

	InsertColumn( VdkLcColumnInitializer( this ).
				  percentage( 100 ).
				  width( m_Rect.width ).
				  textAlign( TEXT_ALIGN_CENTER_X_Y ).
                  textColor( m_TextColor ) );

	//======================================================

	m_WindowImpl->Bind( wxEVT_SHOW, &OOPLyric::OnParentShow, this );
	wxTheApp->Bind( wxEVT_TIMER, &OOPLyric::OnTimerNotify, this, m_timerId );
}

OOPLyric::~OOPLyric()
{
	wxTheApp->Unbind( wxEVT_TIMER, &OOPLyric::OnTimerNotify, this, m_timerId );
	m_WindowImpl->Unbind( wxEVT_SHOW, &OOPLyric::OnParentShow, this );

	//======================================================

	m_timer.Stop();
}

void OOPLyric::DoClear(wxDC*)
{
	if( m_timer.IsRunning() )
		Stop( NULL ); // VdkListCtrl::Clear() ��ֱ���ػ������ؼ�
}

bool OOPLyric::IsOk() const
{
	bool isParserOk = m_parser && m_parser->IsOk();

	// �����ṩ����ͬ�����ⲿ��ʱ��
	return m_stopWatch && isParserOk && !VdkListCtrl::IsEmpty();
}

void OOPLyric::Start()
{
	if( !IsOk() )
	{
		wxLogError( L"��ʿؼ���δ��ȷ��ʼ����" );
		return;
	}

	// �������ν��뱾����
	if( TestState( OLST_STARTED ) )
	{
		if( m_stopWatch->IsRunning() )
		{
			Resume();
		}

		return;
	}

	SetAddinState( OLST_STARTED );
	RemoveState( OLST_STOPPED | OLST_PAUSED );

	//======================================================

	UpdateProgress( NULL );
}

void OOPLyric::AttachParser(const OOPLyricParser& parser)
{
	// ��������б���е�����
	Clear( NULL );

	m_parser = &parser;

	// �������ʼ����ȷ�е����
	// ��Ҫ�������õ��� m_parser�����뱣֤����Ч��
	Init();

	//======================================================
	// ���� VdkListCtrl �ĸ���

	LineIter i( m_parser->begin() ), e( m_parser->end() );
	for( int curr = 0; i != e; ++i, ++curr )
	{
		VdkLcCell* cell = Append( (*i)->GetLyric(), false );
		cell->SetClientData( (void *) (curr + 1) );
	}

	InsertBlankLines();
}

void OOPLyric::UpdateProgress(wxDC* pDC)
{
	wxASSERT( m_parser );

	double now = m_stopWatch->Time();
	GoTo( now / m_parser->GetTimeSum(), pDC, !m_stopWatch->IsRunning() );
}

void OOPLyric::ClearLyric(wxDC* pDC)
{
	Clear( pDC );
}

void OOPLyric::Stop(wxDC* pDC)
{
	GoTo( 0, pDC, true );
}

void OOPLyric::Pause()
{
	SetAddinState( OLST_PAUSED );
	m_timer.Stop();
}

bool OOPLyric::Resume()
{
	// ������ֹͣ״̬�µ��ñ�����
	wxASSERT( !TestState( OLST_STOPPED ) );

	// �����Ƚ��� StartLyric()
	if( !TestState( OLST_STARTED ) || m_timer.IsRunning() )
		return false;

	RemoveState( OLST_PAUSED );

	//--------------------------------------------------------

	m_timer.Start( REFRESH_INTERVAL_MS );
	return true;
}

void OOPLyric::GoTo(double percentage, wxDC* pDC, bool bPaused)
{
	wxASSERT( IsOk() );

	if( percentage == 1 )
	{
		MoveToEnd( pDC );
		return;
	}

	//==============================================

	m_currLine = m_parser->WhichLine( m_parser->GetTimeSum() * percentage );
	if( m_currLine == m_parser->end() )
	{
		wxLogDebug( L"Wrong position." );
		return;
	}

	//==============================================

	m_timer.Stop();
	RefreshLyric( pDC, bPaused );
}

void OOPLyric::MoveToEnd(wxDC* pDC)
{
	// ֹͣ���
	m_timer.Stop();
	SetAddinState( OLST_STOPPED );

	// ��׼��λ��������λ��
	int maxY;
	GetMaxViewStartCoord( NULL, &maxY );
	SetViewStart( 0, maxY, pDC );
}

void OOPLyric::NextLine(wxDC* pDC, bool bPaused)
{
	wxASSERT( m_parser );

#ifdef __WXDEBUG__
	{
		// �������ⲿ��֤����Խ��
		LineIter next( m_currLine );
		++next;
		wxASSERT( next != m_parser->end() );
	}
#endif

	m_timer.Stop();

	++m_currLine;

	//==============================================

	RefreshLyric( pDC, bPaused );
}

void OOPLyric::RefreshLyric(wxDC* pDC, bool bPaused)
{
	CorrectViewStart( pDC );

	if( !bPaused && !m_timer.IsRunning() )
		Resume();
}

void OOPLyric::OnParentShow(wxShowEvent& e)
{
	if( IsOk() )
	{
		if( !e.IsShown() )
		{
			m_timer.Stop();
		}
		else
		{
			UpdateProgress( NULL );
			Start();

			VdkDC dc( m_Window, GetAbsoluteRect(), NULL );
			Draw( dc );
		}
	}

	e.Skip( true );
}

void OOPLyric::OnTimerNotify(wxTimerEvent&)
{
	wxASSERT( IsOk() );

	if( m_Window->IsExiting() )
		return;

	if( m_stopWatch->Time() >= m_parser->GetTimeSum() )
	{
		m_timer.Stop();
		return;
	}

	//-----------------------------------------------------

	VdkDC dc( m_Window, GetAbsoluteRect(), NULL );
	LineIter next( m_currLine );
	++next;

	if( next != m_parser->end() )
	{
		if( (*next)->GetStartTime() <= m_stopWatch->Time() )
		{
			NextLine( &dc, false );
			return;
		}
	}

	CorrectViewStart( &dc );
}

void OOPLyric::OnDraw(wxDC& dc)
{
	// ��ʹû�и��ҲҪ��ձ���
    VdkListCtrl::OnDraw( dc );

	// ��û�и��ʱ����ʾ�������ı�
	if( !IsOk() )
	{
		wxString interactiveOutput( GetInteractiveOutput() );

		if( !interactiveOutput.empty() )
		{
			int w, h;
			dc.SetFont( m_Font );
			dc.SetTextForeground( m_TextColor );

			dc.GetTextExtent( interactiveOutput, &w, &h );
			int xFix, yFix;
			xFix = ( m_Rect.width - w ) / 2;
			yFix = ( m_Rect.height - h ) / 2;

			dc.DrawText( interactiveOutput, xFix, yFix );
		}
	}
}

VdkCusdrawReturnFlag OOPLyric::DoDrawCellText(const VdkLcCell* cell,
											  int col_index,
											  int index0,
											  wxDC& dc,
											  VdkLcHilightState state)
{
	wxASSERT( m_parser );

	// ע�⣺index �ǲ��������Ŀ��е�
	int index = (int) (cell->GetClientData()) - 1;
	if( index == -1 ) // ��ʱ ClientData == NULL����������ӵĿ���
		return VCCDRF_DODEFAULT;

	dc.SetTextForeground( m_TextColor );

	// ��ͣʱ������ǰ�У��龰�����û������϶���ʡ�
	// ��������ͣ��Ȼ���϶������ϣ���ʱ�����������Կ���OK
	// ��ʽ������ʾʱ����ô���ᱣ�ְ������״̬������ȫ������
	if( TestState( OLST_PAUSED ) )
	{
		int yStart;
		GetViewStartCoord( NULL, &yStart );

		int rowHeight = GetRowHeight();
		int dragRegion = yStart + rowHeight * m_blankLinesTop;
		int index2 = index + m_blankLinesTop;

		// ����϶����ʱ�м��������һ��
		if( rowHeight * index2 <= dragRegion &&
			rowHeight * (index2 + 1) > dragRegion )
		{
			m_draggHit = m_parser->GetLine( index );

			dc.SetTextForeground( m_HilightColor );
		}

		return VCCDRF_DODEFAULT;
	}

	LineInfo* currLine = *m_currLine;
	// TODO: �Ƿ����Ż���
	size_t currLineIndex = m_parser->IndexOf( m_currLine );
	int lineHasGone = m_stopWatch->Time() - currLine->GetStartTime();

	if( (index == currLineIndex - 1) && !cell->GetLabel().empty() )
	{
		// ʹ�ý���ɫ��ԭ��һ����
		if( lineHasGone < ALPHA_SHOW_LAST_LINE_MS )
		{
			unsigned char r, g, b;
			double alpha2 = double( lineHasGone ) / ALPHA_SHOW_LAST_LINE_MS;
			double alpha1 = 1 - alpha2;

			r = m_HilightColor.Red() * alpha1 + m_TextColor.Red() * alpha2;
			g = m_HilightColor.Green() * alpha1 + m_TextColor.Green() * alpha2;
			b = m_HilightColor.Blue() * alpha1 + m_TextColor.Blue() * alpha2;

			dc.SetTextForeground( wxColour( r, g, b ) );
		}
	}
	else if( index == currLineIndex ) // ������ǰ�ı���
	{
		// ��������һ�ֺܺ������������һ�������˾ͻᵼ������ (*) ���ʽ
		// �ĳ���Ϊ 0
		if( currLine->GetMilSeconds() == 0 )
			return VCCDRF_DODEFAULT;

		if( !cell->IsEmpty() )
		{
			/* �����ѵ��
			1. SetClippingRegiion �е���ЧӦ�������ִ���µ�
			   SetClippingRegiion ǰ����������ԭ���� ClippingRegiion ��
			2. ���ڱ��ʽ�������븡�������ã�ע���м���������
			   ��ǿ��ת���� int Ȼ��μ���һ�������㣬�����Ƕ�
			   ���ս������ת����ʹ֮��Ϊһ����������
			*/

			const int rowHeight = GetRowHeight();
			int y = (currLineIndex + m_blankLinesTop) * rowHeight;
			cell->DrawLabel( dc, 0, y );

			// (*)
			double lineProgress = double( lineHasGone ) / currLine->GetMilSeconds();

			// Ҫʵ�� KALA-OK Ч�����ı����
			int w = (m_Rect.width - cell->GetX_Padding() * 2) * lineProgress;

			wxRect rc( GetAbsoluteRect() );
			const int bottom = rc.y + rc.height;

			int yStart;
			VdkScrolledWindow::GetViewStartCoord( NULL, &yStart );
			rc.y += y - yStart;

			rc.width = cell->GetX_Padding() + w;
			rc.height = rowHeight;
			// ����ʹ KALA-OK Ч���� ClippingRegion �����б���
			if( (rc.y + rc.height) > bottom )
				rc.height = bottom - rc.y;

			VdkDcDeviceOriginSaver saver( dc );
			dc.SetDeviceOrigin( 0, 0 );
			VdkDcClippingRegionDestroyer destroyer( dc, rc );

#       ifdef __WXGTK__
            dc.SetBrush( m_crossBrush1 );
            dc.DrawRectangle( rc );
#       endif
			dc.SetTextForeground( m_HilightColor );

			cell->DrawLabel( dc, rc.x, rc.y );
		}

		return VCCDRF_SKIPDEFAULT;
	}

	return VCCDRF_DODEFAULT;
}

void OOPLyric::InsertBlankLines()
{
	wxASSERT( IsOk() );

	int shownItems = GetShownItems() - GetShownItemsAddin();
	int numBlankLinesTop = shownItems / 2;
	int numBlankLinesBottom = shownItems - numBlankLinesTop;

	int delta = m_blankLinesTop + m_blankLinesBottom -
				numBlankLinesTop - numBlankLinesBottom;

	if( delta > 0 )
	{
		int numLinesToDelete = m_blankLinesTop - numBlankLinesTop;
		for( int i = 0; i < numLinesToDelete; i++ )
			RemoveRow( 0, false, NULL );

		numLinesToDelete = m_blankLinesBottom - numBlankLinesBottom;
		for( int i = 0; i < numLinesToDelete; i++ )
			RemoveRow( GetRowCount() - 1, false, NULL );
	}
	else if( delta < 0 )
	{
		wxString blankLine( wxEmptyString );

		int numNewLines = numBlankLinesTop - m_blankLinesTop;
		for( int i = 0; i < numNewLines; i++ )
			InsertRow( 0, blankLine, false );

		numNewLines = numBlankLinesBottom - m_blankLinesBottom;
		for( int i = 0; i < numNewLines; i++ )
			Append( blankLine, false );
	}

	m_blankLinesTop = numBlankLinesTop;
	m_blankLinesBottom = numBlankLinesBottom;

	UpdateVirtualHeight();
}

void OOPLyric::UpdateVirtualHeight()
{
	wxASSERT( m_parser );

	int rowHeight = GetRowHeight();
	int topBlankLinesHeight = m_blankLinesTop * rowHeight;
	int bottomBlankLinesHeight = m_Rect.height - topBlankLinesHeight;
	int actualHeight = m_parser->GetLinesCount() * rowHeight;

	int ySize = topBlankLinesHeight + actualHeight + bottomBlankLinesHeight;

	// -1 ��Ϊ�˵õ�һ�����ǲ�������Ч������Ĵ�С
	SetVirtualSize( 0, ySize - 1 );
}

double OOPLyric::GetLineProgress() const
{
	LineInfo* currLine = *m_currLine;
	size_t lineHasGone = m_stopWatch->Time() - currLine->GetStartTime();
	if( lineHasGone >= currLine->GetMilSeconds() )
		return 1;

	// �������Ϊ 0��������������
	return double( lineHasGone ) / currLine->GetMilSeconds();
}

void OOPLyric::CorrectViewStart(wxDC* pDC)
{
	wxASSERT( m_parser );

	// TODO: ��� m_currIndex ��
	int rowHeight = GetRowHeight();
	int yStart = rowHeight * double
		( m_parser->IndexOf( m_currLine ) + GetLineProgress() );

	SetViewStart( 0, yStart, pDC );
}

void OOPLyric::OnMouseEvent(VdkMouseEvent& e)
{
	switch( e.evtCode )
	{
	case RIGHT_UP:
		{
			// �϶����ʱ��Ҫ��Ӧ�Ҽ��¼�
			if( TestState( OLST_DRAGGING ) )
				return;

			wxPoint menuPos( AbsoluteRect().GetPosition() );
			menuPos.x += e.mousePos.x;
			menuPos.y += e.mousePos.y;

			m_Window->ShowContextMenu( this, menuPos );

			break;
		}

	case LEFT_DOWN:
		{
			m_draggDistance = e.mousePos.y;
			SetAddinState( OLST_DRAGGING_STARTED );

			break;
		}

	case DRAGGING:
		{
			// ����������һ�׸�ĻỰ���϶���ʣ�Ȼ����δ�ͷ����������
			// ��һ�׸迪ʼ���ţ�����ǰ����϶��¼�
			if( !TestState( OLST_DRAGGING_STARTED ) )
			{
				break;
			}

		    if( !IsOk() )
				break;

			if( m_timer.IsRunning() )
			{
				Pause();
				SetAddinState( OLST_RUNNING_BEFORE_DRAGGING );
			}

			SetAddinState( OLST_DRAGGING );

			//======================================================

			int ystart;
			GetViewStartCoord( NULL, &ystart );

			int dY = e.mousePos.y - m_draggDistance;
			m_draggDistance = e.mousePos.y;

			// ����һ�������ֹ�����ȥ�Ŀ���
			int rowHeight = GetRowHeight();
			int upperBound = 
				(*(m_parser->begin()))->GetLyric().empty() ? rowHeight : 0;

			// �޷��������������Ͼ�(������ȥ�͵���һ����)
			// ���ǽ��ϵ���ͷ���¼���Ϊ��Ч
			bool lastLine = false;

			// �龰�������⻭������һ��������
			// �϶���ͷ�ˣ������ٰѴ�����������һ��
			if( ystart - dY < upperBound )
			{
				dY = ystart - upperBound; // �ӼӼ�����ԭ�����(*)
			}
			else
			{
				int maxy;
				GetMaxViewStartCoord( NULL, &maxy );

				// ���������Ͼ�����ͷ�ˣ��پ���ȥ�ͻᵼ��
				// �޷�������ס����
				if( ystart - dY > maxy )
				{
					dY = ystart - maxy; // �ӼӼ�����ԭ�����(*)
					// ��Ч�϶��¼�
					lastLine = true;
				}
			}

			if( dY )
			{
				SetViewStart( 0, ystart - dY, &e.dc ); //������������(*)
			}

			// ��Ч�϶��¼�
			if( lastLine )
			{
				m_draggHit = m_parser->end();
			}

			//===================================================
			// �����м��߶�

			wxRect rc( GetAbsoluteRect() );
			int y = rc.y + m_blankLinesTop * GetRowHeight();

			m_Window->ResetDcOrigin( e.dc );
			e.dc.SetPen( wxPen( m_TextColor ) );
			e.dc.DrawLine( rc.x, y, rc.GetRight(), y );

			break;
		}

	case NORMAL:
	case LEFT_UP:
		{
			if( !TestState( OLST_DRAGGING ) ||
				!TestState( OLST_DRAGGING_STARTED ) )
			{
				break;
			}

			wxASSERT( IsOk() );

			//-----------------------------------------------------

			RemoveState( OLST_DRAGGING | OLST_DRAGGING_STARTED );

			// �ϵ��������ˣ���ͷ
			if( m_draggHit == m_parser->end() )
			{
				RefreshState( &e.dc );
			}
			else
			{
				int ystart; // ��ʼ��ͼ����
				GetViewStartCoord( NULL, &ystart );

				LineInfo* lineDraggHit = *m_draggHit;
				size_t timeToGo = lineDraggHit->GetStartTime();

				int rowHeight = GetRowHeight();
				double linePercentage = double(ystart % rowHeight) / rowHeight ;
				timeToGo += lineDraggHit->GetMilSeconds() * linePercentage;

				// ��ʿ��ܲ���ƥ�����ڲ��ŵĸ���
				if( timeToGo < m_parser->GetTimeSum() )
				{
					m_currLine = m_draggHit;

					if( IsReadyForEvent() )
					{
						FireEvent( &e.dc, (void *) timeToGo );
					}
				}
			}

			bool resume = TestState( OLST_RUNNING_BEFORE_DRAGGING );
			if( resume )
			{
				RemoveState( OLST_RUNNING_BEFORE_DRAGGING );
				Resume();
			}

			break;
		}

	default:

		break;
	}
}

void OOPLyric::OnKeyEvent(VdkKeyEvent& vke)
{
	// ȫ������
	vke.Skip( true );
}

void OOPLyric::OnNotify(const VdkNotify& notice)
{
	VdkListCtrl::OnNotify( notice );

	switch( notice.GetNotifyCode() )
	{
	case VCN_WINDOW_RESIZED:

		if( IsOk() && (notice.GetLparam() > 0) )
		{
			InsertBlankLines();
			UpdateSize( notice.GetVObjDC() );
		}

		break;

	default:

		break;
	}
}

void OOPLyric::DoSetInteractiveOutput(wxDC* pDC)
{
	if( !IsOk() && pDC )
	{
		VdkUtil::ClrBkGnd( *pDC, m_crossBrush1, AbsoluteRect() );
		Draw( *pDC );
	}
}
