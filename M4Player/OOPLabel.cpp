/***************************************************************
 * Name:      OOPLabel.cpp
 * Purpose:   OOPLabel ʵ���ļ�
 * Author:    Ning (vanxining@139.com)
 * Created:   2010
 * Copyright: Wang Xiao Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPLabel.h"

#include "VdkDC.h"
#include "VdkWindow.h"

#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT( OOPLabel );

enum {
	CNST_LINE_SWITCH_TIME_SLICE = 5000, // �м��л���ɺ��ȶ���ʾ��ʱ��γ���
	CNST_LINE_ROLLING_TIME_SLICE = 100, // ���ڹ���ʱ϶
	CNST_ROLLING_DISTANCE_SLICE = 2, // ���ڹ�������
};

OOPLabel::OOPLabel()
	: m_staticCaption( L"OOPlayer for Linux" ),
	  m_nTextInternal( 0 ),
	  m_Direction( wxEAST )
{
	Init();
}

void OOPLabel::Init()
{
	m_Direction = wxEAST;
	m_nOnShowId = 0;
	m_nNotityType = TNT_LINE_ROLLING;
	m_nRollingCount = wxNOT_FOUND;

	m_items.clear();
}

void OOPLabel::Create(wxXmlNode* node)
{
	TextInfo ti( GetXrcTextInfo( node ) );

	Create( m_Window,
		    GetXrcName(node),
		    GetXrcRect(node),
		    ti.font,
		    ti.foreground,
		    m_BkGndColor,
		    GetXrcAlign(node) );
}

void OOPLabel::Create(VdkWindow* Window,
					  const wxString& strName,
					  const wxRect& Rect,
					  const wxFont& font,
					  const wxColour& color,
				      const wxColour& bgColor,
				      const align_type& align)
{
	m_Window = Window;
	m_WindowImpl = m_Window->GetHandle();
	m_strName = strName;
	m_Rect = Rect;
	m_Font = font;

	SetAddinStyle( VCS_ERASE_BG | VCS_IGNORE_ALL_EVENTS );

	m_TextColor = color;
	m_BkGndColor = bgColor;
	m_align = align;

	//======================================================

	// ��Ϊ������Ҫ��ȡһ����ͼ������˲��ܽض�������ʾ���ı�
	int h;
	m_WindowImpl->GetTextExtent( m_staticCaption, NULL, &h, 0, 0, &m_Font );
	m_nTextInternal = ( m_Rect.height - h ) / 2;
	if( m_Rect.height < h )
    {
        // ��Ȼʹ�ı�����
        m_Rect.y += m_nTextInternal; // ע�� m_nTextInternal Ϊ����
        m_Rect.height = h;

        m_nTextInternal = 0;
    }
}

OOPLabel::~OOPLabel()
{
	StopRolling( NULL );
}

void OOPLabel::SetItems(const wxArrayString& items, wxDC* pDC)
{
	StopRolling( NULL );
	wxASSERT( m_items.empty() );

	//======================================================

	wxArrayString::const_iterator i( items.begin() );
	for( ; i != items.end(); ++i )
	{
		int nWidth;
		m_WindowImpl->GetTextExtent( *i, &nWidth, NULL, 0, 0, &m_Font );

		ItemNode node;

		node.nTextWidth = nWidth;
		node.strItem = *i;

		if( nWidth > m_Rect.width )
		{
			node.nDelta = nWidth - m_Rect.width;
		}
		else
		{
			node.nDelta = 0;
		}

		m_items.push_back( node );
	}

	if( pDC )
	{
		Draw( *pDC );
	}
}

void OOPLabel::DoDraw(wxDC& dc)
{
	if( IsRunning() && m_nRollingCount > 0 )
		return;

	VdkDcClippingRegionDestroyer destroyer( dc, GetAbsoluteRect() );
	dc.SetFont( m_Font );

	if( m_TextColor.IsOk() )
	{
		dc.SetTextForeground( m_TextColor );
	}

	wxCoord x = m_Rect.x;
	wxCoord y = m_Rect.y + m_nTextInternal;

	if( !IsOk() && !m_staticCaption.IsEmpty() )
	{
		dc.DrawText( m_staticCaption, x, y );
		return;
	}

	// ��Ϊ��һ�ι���֮ǰ�������ǲ�����½���ģ�
	// ���Ի�����Ҫ�� DoDraw ���滭��
	dc.DrawText( m_items[m_nOnShowId].strItem, x, y );
}

void OOPLabel::StartRolling()
{
	wxASSERT( IsOk() );

	if( IsRunning() )
		return;

	//======================================================

	m_StopWatch.Start();

	if( m_items[m_nOnShowId].nTextWidth > m_Rect.width )
	{
		m_nNotityType = TNT_LINE_ROLLING;
		Start( CNST_LINE_ROLLING_TIME_SLICE );
	}
	else
	{
		m_nNotityType = TNT_LINE_SWITCH;
		Start( CNST_LINE_SWITCH_TIME_SLICE );
	}
}

void OOPLabel::StopRolling(wxDC* pDC)
{
	m_items.clear();
	m_StopWatch.Pause();
	Stop();

	Init();

	if( pDC )
	{
		Draw( *pDC );
	}
}

void OOPLabel::Notify()
{
	const ItemNode& node = m_items[m_nOnShowId];

	// ���統ǰ�е��ı����ȱ�ʵ�ʿ���ʾ����ҪС��ֱ�ӵȴ��л�����һ�е�ʱ����
	if( (node.nTextWidth <= m_Rect.width) && 
		(m_nNotityType == TNT_LINE_ROLLING) )
	{
		m_nNotityType = TNT_LINE_SWITCH;
		Start( CNST_LINE_SWITCH_TIME_SLICE - m_StopWatch.Time() );

		return;
	}

	wxRect rc( GetAbsoluteRect() );

	//======================================================

	if( m_nNotityType == TNT_LINE_ROLLING )
	{
		if( m_Direction == wxEAST )
			++m_nRollingCount;
		else
			--m_nRollingCount;

		// ���ҵ�ͷ�ˣ�������߰�
		if( m_nRollingCount > (node.nDelta / CNST_ROLLING_DISTANCE_SLICE) )
			m_Direction = wxWEST;

		// ����Ҳ��ͷ�ˣ���ʼ�л���һ���
		// ����ֻ��Ҫ���������������һ�����ؼ���
		if( (m_Direction == wxWEST) && (m_nRollingCount == wxNOT_FOUND) )
		{
			m_nNotityType = TNT_LINE_SWITCH;

			// ����ʱ��ʣ�ֱ࣬�ӵȴ��л�����һ�е�ʱ����
			if( m_StopWatch.Time() < CNST_LINE_SWITCH_TIME_SLICE )
			{
				Start( CNST_LINE_SWITCH_TIME_SLICE - m_StopWatch.Time() );
				return;
			}
			// ��ʱ�ˣ�ֱ���л�����һ�аɣ����� return��ֱ�ӵ����м��л��Ĵ���
		}
		else // ��û��ͷ��������
		{
			DrawLabel( node.strItem,
					   // ����ġ�-�����н����ģ���Ϊ���������ҹ������Ե�һ���ַ���
					   // x ֵҪ�� rc.x ҪС
					   rc.x - m_nRollingCount * CNST_ROLLING_DISTANCE_SLICE,
					   rc.y + m_nTextInternal );

			// �������ѺõĴ����Ǳ��� goto ��ʹ��
			return;
		}

	} // if( m_nNotityType == TNT_LINE_ROLLING ) ���ڹ���

	//======================================================
	// �м����

	// ��ֱ������һ�����Թ������ٴΣ�
	const int nMaxRollingCount = rc.height / CNST_ROLLING_DISTANCE_SLICE;
	m_nRollingCount++;

	// ��һ�ι�����ֱ������ Timer �ͷ���
	if( m_nRollingCount == 0 )
	{
		Start( CNST_LINE_ROLLING_TIME_SLICE );
		return;
	}
	// ��һ���Ѿ�������ȷλ���ˣ���ʼ���ұ߹���
	else if( m_nRollingCount > nMaxRollingCount )
	{
		m_nRollingCount = wxNOT_FOUND;
		m_Direction = wxEAST;
		m_nNotityType = TNT_LINE_ROLLING;

		// ��ֹ�������
		if( unsigned( m_nOnShowId ) != m_items.size() - 1 )
		{
			m_nOnShowId++;
		}
		else
		{
			m_nOnShowId = 0;
		}

		m_StopWatch.Start( 0 );
		Start( CNST_LINE_ROLLING_TIME_SLICE );

		return;
	}

	//======================================================
	// ���������н��������ʵ�ʻ�ͼ����

	// �õ���ȷ������
	const ItemNode* pNode2;
	if( unsigned( m_nOnShowId ) != m_items.size() - 1 )
	{
		pNode2 = &(m_items[m_nOnShowId + 1]);
	}
	else
	{
		pNode2 = &(m_items[0]);
	}

	if( m_nRollingCount == nMaxRollingCount )
	{
		DrawLabel( pNode2->strItem, rc.x, rc.y + m_nTextInternal );
	}
	else
	{
		int y = rc.y + m_nTextInternal;
		y -= m_nRollingCount * CNST_ROLLING_DISTANCE_SLICE;

		DrawLabel( node.strItem,
				   rc.x, y,
				   pNode2->strItem,
				   y + m_Rect.height );
	}

}

void OOPLabel::DrawLabel(const wxString& str,
						 int x,
						 int y,
						 const wxString& str2,
						 int y2)
{
	wxRect rc( GetAbsoluteRect() );
	VdkDC dc( m_Window, rc, NULL );
	VdkDcClippingRegionDestroyer destroyer( dc, rc );

	EraseBackground( dc, m_Rect );

	dc.SetFont( m_Font );
	if( m_TextColor.IsOk() )
		dc.SetTextForeground( m_TextColor );

	dc.DrawText( str, x, y );

	if( !str2.IsEmpty() )
		dc.DrawText( str2, x, y2 );
}
