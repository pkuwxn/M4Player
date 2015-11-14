#include "StdAfx.h"
#include "VdkLabel.h"

#include "VdkDC.h"
#include "VdkWindow.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkLabelInitializer::VdkLabelInitializer()
   : TextAlign( 0 )
{

}

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_VOBJECT( VdkLabel );

VdkLabel::VdkLabel()
		: m_xFix( 0 ), m_yFix( 0 ),
		  m_TextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y ), 
		  m_ghost( NULL )
{
	SetAddinStyle( VCS_IGNORE_ALL_EVENTS );
}

void VdkLabel::Create(wxXmlNode* node)
{
	TextInfo ti( GetXrcTextInfo( node, m_WindowImpl ) );

	// 提示信息(ToolTip)
	wxXmlNode* chd( FindChildNode( node, L"tip" ) );
	if( chd )
	{
		SetToolTip( chd->GetNodeContent() );
	}

	// 自动扩展
	chd = FindChildNode( node, L"auto-expand" );
	if( chd )
	{
		bool autoExpand = XmlGetContentOfBoolean( chd, false );
		if( autoExpand )
		{
			SetAddinStyle( VLBS_AUTO_EXPAND );
		}
	}

	Create( VdkLabelInitializer().
			textAlign( GetXrcTextAlign( node ) ).
			window( m_Window ).
			name( GetXrcName( node ) ).
			rect( GetXrcRect( node ) ).
			caption( GetXrcTextBlock( node ) ).
			align( GetXrcAlign( node ) ).
			font( ti.font ).
			textColor( ti.foreground ) );
}

void VdkLabel::Create(const VdkLabelInitializer& init_data)
{
	VdkControl::Create( init_data );

	m_TextAlign = init_data.TextAlign;
	m_TextColor = init_data.TextColor;
	if( !m_TextColor.IsOk() )
	{
		m_TextColor = *wxBLACK;
	}

	m_strCaption = init_data.Caption;
	m_strCaption.Replace( L"：", L":" );

	PrepareRect();
}

void VdkLabel::PrepareRect()
{
	if( m_strCaption.IsEmpty() )
	{
		if( m_Rect.width == 0 )
		{
			m_Rect.width = 1;
			m_Rect.height = m_WindowImpl->GetCharHeight();
		}

		return;
	}

	int w, h;
	m_WindowImpl->GetTextExtent( m_strCaption, &w, &h, 0, 0, &m_Font );

    RemoveState( VLBST_CLIPPING_REGION );
	bool autoExpand = TestStyle( VLBS_AUTO_EXPAND );
	if( autoExpand )
		m_Rect.width = w;
	
	if( (m_Rect.width < w) && !autoExpand )
	{
		SetAddinState( VLBST_CLIPPING_REGION );
	}
	
	// 防止高度不够
	if( m_Rect.height < h )
	    m_Rect.height = h;

	if( (m_TextAlign & TEXT_ALIGN_CENTER_X_Y) ||
		(m_TextAlign & TEXT_ALIGN_CENTER_X))
	{
		m_xFix = (m_Rect.width - w) / 2;
	}
	else if( m_TextAlign & TEXT_ALIGN_RIGHT )
	{
		m_xFix = m_Rect.width - w;
	}

	if( ( m_TextAlign & TEXT_ALIGN_CENTER_X_Y) ||
		( m_TextAlign & TEXT_ALIGN_CENTER_Y))
	{
		m_yFix = ( m_Rect.height - h ) / 2;
	}
	else if( m_TextAlign & TEXT_ALIGN_BOTTOM )
	{
		m_yFix = m_Rect.height - h;
	}
}

void VdkLabel::DoDraw(wxDC& dc)
{
	if( m_strCaption.IsEmpty() )
		return;

	dc.SetFont( m_Font );
	dc.SetTextForeground( m_TextColor );

	if( TestState( VLBST_CLIPPING_REGION ) )
		dc.SetClippingRegion( m_Rect );

	//------------------------------------------

	dc.DrawText( m_strCaption, m_Rect.x + m_xFix, m_Rect.y + m_yFix );

	//------------------------------------------

	if( TestState( VLBST_CLIPPING_REGION ) )
		dc.DestroyClippingRegion();
}

void VdkLabel::SetCaption(const wxString& strCaption, wxDC* pDC)
{
	m_strCaption = strCaption;

	// 首先擦除背景，防止后面新设的作用域比当前小
	if( pDC )
	{
	    // 擦除背景时，画矩形的画笔是透明的，会导致一定的残留，在这里进行适当
	    // 扩大要擦出的范围
	    wxRect rc( GetAbsoluteRect() );
		EraseBackground( *pDC, rc.Inflate( 1 ) );
	}

	PrepareRect();

	if( pDC )
	{
		Draw( *pDC );
	}
}

VdkLabel::~VdkLabel()
{
	if( m_ghost )
	{
		m_ghost->Stop();
		delete m_ghost;
	}
}

void VdkLabel::DoHandleNotify(const VdkNotify& notice)
{
	switch( notice.GetNotifyCode() )
	{
	case VCN_FONT_CHANGED:

		SetCaption( m_strCaption, NULL );

		break;

	default:

		break;
	}
}

//////////////////////////////////////////////////////////////////////////

VdkLabelGhost::VdkLabelGhost(VdkLabel* label, const wxString& strNewText, int ms)
{
	m_label = label;
	m_strNewText = strNewText;

	wxTimer::Start( ms, true );
}

void VdkLabelGhost::Notify()
{
	VdkDC vdc( m_label->GetVdkWindow(), m_label->GetAbsoluteRect() );

	m_label->SetCaption( m_strNewText, &vdc );
	m_label->SetGhost( NULL );
	delete this;
}
