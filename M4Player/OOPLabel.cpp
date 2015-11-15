/***************************************************************
 * Name:      OOPLabel.cpp
 * Purpose:   OOPLabel 实现文件
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
	CNST_LINE_SWITCH_TIME_SLICE = 5000, // 行间切换完成后稳定显示的时间段长度
	CNST_LINE_ROLLING_TIME_SLICE = 100, // 行内滚动时隙
	CNST_ROLLING_DISTANCE_SLICE = 2, // 行内滚动步进
};

OOPLabel::OOPLabel()
	: m_staticCaption( L"M4Player for Linux" ),
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

	// 因为我们需要截取一个作图区域，因此不能截断正常显示的文本
	int h;
	m_WindowImpl->GetTextExtent( m_staticCaption, NULL, &h, 0, 0, &m_Font );
	m_nTextInternal = ( m_Rect.height - h ) / 2;
	if( m_Rect.height < h )
    {
        // 仍然使文本居中
        m_Rect.y += m_nTextInternal; // 注意 m_nTextInternal 为负数
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

	// 因为第一次滚动之前，我们是不会更新界面的，
	// 所以还是需要在 DoDraw 里面画。
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

	// 假如当前行的文本长度比实际可显示区域要小，直接等待切换到下一行的时候到来
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

		// 向右到头了，返回左边吧
		if( m_nRollingCount > (node.nDelta / CNST_ROLLING_DISTANCE_SLICE) )
			m_Direction = wxWEST;

		// 向左也到头了，开始切换下一句吧
		// 我们只需要先向右再向左滚动一个来回即可
		if( (m_Direction == wxWEST) && (m_nRollingCount == wxNOT_FOUND) )
		{
			m_nNotityType = TNT_LINE_SWITCH;

			// 还有时间剩余，直接等待切换到下一行的时候到来
			if( m_StopWatch.Time() < CNST_LINE_SWITCH_TIME_SLICE )
			{
				Start( CNST_LINE_SWITCH_TIME_SLICE - m_StopWatch.Time() );
				return;
			}
			// 超时了！直接切换到下一行吧，不用 return，直接到达行间切换的代码
		}
		else // 还没到头，继续画
		{
			DrawLabel( node.strItem,
					   // 这里的“-”是有讲究的，因为我们是向右滚，所以第一个字符的
					   // x 值要比 rc.x 要小
					   rc.x - m_nRollingCount * CNST_ROLLING_DISTANCE_SLICE,
					   rc.y + m_nTextInternal );

			// 这样不友好的代码是避免 goto 的使用
			return;
		}

	} // if( m_nNotityType == TNT_LINE_ROLLING ) 行内滚动

	//======================================================
	// 行间滚动

	// 竖直方向上一共可以滚动多少次？
	const int nMaxRollingCount = rc.height / CNST_ROLLING_DISTANCE_SLICE;
	m_nRollingCount++;

	// 第一次滚动，直接设置 Timer 就返回
	if( m_nRollingCount == 0 )
	{
		Start( CNST_LINE_ROLLING_TIME_SLICE );
		return;
	}
	// 下一行已经滚到正确位置了，开始向右边滚吧
	else if( m_nRollingCount > nMaxRollingCount )
	{
		m_nRollingCount = wxNOT_FOUND;
		m_Direction = wxEAST;
		m_nNotityType = TNT_LINE_ROLLING;

		// 防止数组溢出
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
	// 下面是两行交替滚动的实际绘图代码

	// 得到正确的两行
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
