/***************************************************************
 * Name:      VdkEdit.cpp
 * Purpose:   Code for VdkEdit implementation
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-03-14
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkEdit.h"

#include "VdkDC.h"
#include "VdkWindow.h"
#include "VdkMenu.h"
#include "VdkTextValidator.h"

#include "wxUtil.h"
#include "XUtil.h"

#include <wx/clipbrd.h> // wxTheClipboard

#include <iterator> // for distance()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLONEABLE_VOBJECT( VdkEdit, VdkControl );
size_t VdkEdit::ms_editCount = 0;

VdkEdit::VdkEdit()
	: m_editable( true ),
	  m_validator( NULL ),
	  m_sstyle( NULL ),
	  m_TextAlign( TEXT_ALIGN_CENTER_Y ),
	  m_cursorSlice( 500 ),
	  m_cursorSize( 1 ),
	  m_textPadding( 2 ),
	  m_menuId0( wxID_ANY ),
	  m_fontHeight( 0 ),
	  m_yFix( 0 ),
	  m_cursorPos( 0 ),
	  m_cursorChar( m_text.end() ),
	  m_selEndPos( wxNOT_FOUND ),
	  m_selEndChar( m_text.end() ),
	  m_total( 0 ),
	  m_xViewStart( 0 ),
	  m_maxUndo( 20 ),
	  m_nextUndo( wxNOT_FOUND )
{
	++ms_editCount;
	memset( m_menuItems, 0, sizeof( m_menuItems ) );

	//------------------------------------------------

	SetStaticStyle( VESS_MS_WORD_2003 );
}

void VdkEdit::Create(wxXmlNode* node)
{
	TextInfo textinfo( GetXrcTextInfo( node, m_WindowImpl ) );
	bool editableVal = XmlGetContentOfBoolean
		( FindChildNode( node, L"editable" ), true );

	Create( VdkEditInitializer().window( m_Window ).
			addToWindow( false ).
			parent( m_parent ).
			name( GetXrcName( node ) ).
			rect( GetXrcRect( node ) ).
			style( VCS_BORDER_SIMPLE ).
			align( GetXrcAlign( node ) ).
			text( GetXrcTextBlock( node ) ).
			font( textinfo.font ).
			editable( editableVal ) );
}

void VdkEdit::Create(const VdkEditInitializer& init_data)
{
	VdkControl::Create( init_data );
	SetAddinStyle( VCS_KEY_EVENT | VCS_ERASE_BG );

	// ������һ��Ҫ��һ�������Ϊ֧��
	wxASSERT( m_Window->TestStyle( VWS_BASE_PANEL ) );

	m_validator = init_data.Validator;
	m_editable = init_data.Editable;

	//------------------------------------------------

	RefrshFontHeight();

	m_Window->AddPostDrawListener( this );

	// ��ʼ�������
	Start( m_cursorSlice );
}

void VdkEdit::Clone(VdkEdit* o)
{

}

VdkEdit::~VdkEdit()
{
	Clear();
	SetStaticStyle( NULL );
	wxDELETE( m_validator );

	//------------------------------------------------

	--ms_editCount;
	if( ms_editCount == 0 )
	{
		Character::DeleteAllInstances();
	}

	RemoveFromWindow( m_Window );
}

void VdkEdit::Clear(wxDC* pDC)
{
	xerase_ptrs( m_text, m_text.begin(), m_text.end() );
	m_text.clear();

	xerase_ptrs( m_actions, m_actions.begin(), m_actions.end() );
	m_actions.clear();
	m_nextUndo = wxNOT_FOUND;

	//------------------------------------------------

	m_cursorChar = end();
	m_cursorPos = 0;

	m_selEndChar = m_cursorChar;
	m_selEndPos = wxNOT_FOUND;

	//------------------------------------------------

	m_total = 0;
	m_xViewStart = 0;

	if( pDC )
	{
		ClearBgAndDrawBorder( *pDC );
	}
}

int VdkEdit::RefrshFontHeight()
{
	m_fontHeight = wxGetFontHeight( m_Font, m_WindowImpl );
	m_yFix = double( m_Rect.height - m_fontHeight ) / 2;

	return m_fontHeight;
}

void VdkEdit::PrepareDC(wxDC& dc)
{
	dc.GetDeviceOrigin( &m_dcOrigin.x, &m_dcOrigin.y );
	//m_Window->ResetDcOrigin( dc ); // TODO: �������Ҫ��

	wxRect rc( GetAbsoluteRect() );
	dc.SetDeviceOrigin( rc.x - m_xViewStart, rc.y );

	int canvasWidth = GetCanvasWidth();
	if( canvasWidth < m_total )
	{
		dc.GetClippingBox( m_dcClippingBoxOrigin );

		rc = wxRect( m_xViewStart, 0, canvasWidth, m_Rect.height );
		//dc.DestroyClippingRegion();
		dc.SetClippingRegion( rc );
	}
}

void VdkEdit::RestoreDC(wxDC& dc)
{
	if( GetCanvasWidth() < m_total )
	{
		if( !m_dcClippingBoxOrigin.IsEmpty() )
			dc.SetClippingRegion( m_dcClippingBoxOrigin );
		else
			dc.DestroyClippingRegion();
	}

	dc.SetDeviceOrigin( m_dcOrigin.x, m_dcOrigin.y );
}

void VdkEdit::DoDraw(wxDC& dc)
{
	ClearBgAndDrawBorder( dc );

	if( !m_text.empty() )
	{
		PrepareDC( dc );

		//-----------------------------------------

		// ����ѡ�񱳾�
		if( IsSelected() && IsFocused() )
		{
			int selStart( wxMin( m_selEndPos, m_cursorPos ) );

			ClrBkGnd( dc, m_sstyle->m_SelTextBgBrush,
					  wxRect( selStart + m_textPadding, m_yFix - m_textPadding,
							  abs( m_selEndPos - m_cursorPos ),
							  m_fontHeight + m_textPadding * 2 ) );
		}

		DrawCaption( dc );

		//-----------------------------------------

		RestoreDC( dc );
	}	
}

void VdkEdit::ClearBgAndDrawBorder(wxDC& dc)
{
	bool border = TestStyle( VCS_BORDER_SIMPLE );

	wxBrush bgBrush( m_sstyle->m_BgBrush );
	if( !IsEditable() )
	{
		bgBrush = m_sstyle->m_UneditableBgBrush;
	}

	wxRect bg( m_Rect );
	bg.Inflate( border ? 1 : 0 );
	VdkUtil::ClrBkGnd( dc, bgBrush, bg );

	// ���Ʊ߿�
	if( border )
	{
		dc.SetBrush( bgBrush );

		if( m_Window->GetCtrlOnHover() == this )
		{
			dc.SetPen( m_sstyle->m_OuterBorder );
			dc.DrawRoundedRectangle( bg, 1 );

			dc.SetPen( m_sstyle->m_InnerBorder );
			dc.DrawRectangle( m_Rect );
		}
		else
		{
			dc.SetPen( m_sstyle->m_NormalBorder );
			dc.DrawRectangle( m_Rect );
		}

	}

}

void VdkEdit::DrawCaption(wxDC &dc)
{
	dc.SetFont( m_Font );
	dc.SetTextForeground( m_sstyle->m_TextColor );

	// ���ﲻ�ÿ���ѡ�е��ı��Ƿ�Ϊ���ɼ��Ŀ����ַ�
	int selStart = m_selEndPos + m_textPadding,
		selEnd = m_cursorPos + m_textPadding;

	if( IsSelected() )
	{
		if( selStart > selEnd )
			wxSwap( selStart, selEnd );
	}

	const int textAreaSize = GetTextAreaSize();
	int x = m_textPadding;

	CharIter i( begin() ), last( end() );
	for( ; i != last; ++i )
	{
		// �����ڿ��ӷ�Χ֮����ַ�
		if( x > m_xViewStart + textAreaSize )
			break;

		EditChar* ch = *i;
		if( x + ch->GetWidth() >= m_xViewStart )
		{
			if( IsSelected() && IsFocused() )
			{
				if( x >= selStart && x < selEnd )
					dc.SetTextForeground( *wxWHITE );
				else
					dc.SetTextForeground( m_sstyle->m_TextColor );
			}

			int yFix = double( m_Rect.height - ch->GetHeight() ) / 2;
			ch->Draw( dc, x, yFix );
		}

		x += ch->GetWidth();
	}
}

void VdkEdit::Notify()
{
	if( !ShouldFlashCursor() )
		return;

	VdkDC dc( m_Window, GetAbsoluteRect(), NULL );
	ReverseState( VEST_CURSOR_ON );
}

void VdkEdit::DrawCursor(wxDC& dc)
{
	PrepareDC( dc );

	int x = m_textPadding + m_cursorPos;
	int y = m_yFix - m_textPadding;

	dc.SetPen( wxPen( *wxBLACK, m_cursorSize ) );
	dc.DrawLine( x, y, x, y + m_fontHeight + m_textPadding * 2 );

	RestoreDC( dc );
}

void VdkEdit::OnPost(wxDC& dc)
{
	if( TestState( VEST_CURSOR_ON ) )
		DrawCursor( dc );
}

void VdkEdit::ShowCursorAtOnce(wxDC& dc, bool stopTimer)
{
	SetAddinState( VEST_CURSOR_ON );

	VdkDC* cachedDC( m_Window->GetCachedDC() );
	if( !cachedDC )
		DrawCursor( dc );

	if( stopTimer )
		Stop();
}

void VdkEdit::MoveCursorOnScreen(int cursorPos)
{
	m_cursorPos = cursorPos;
	wxRect rc( GetAbsoluteRect() );
	
	MoveImeWindow( m_WindowImpl,
				   rc.x + m_textPadding + m_cursorPos - m_xViewStart,
				   rc.GetBottom() );
}

void VdkEdit::DoHandleMouseEvent(VdkMouseEvent& e)
{
	switch( e.evtCode )
	{
	case LEFT_DOWN:
	case RIGHT_UP:
	case DRAGGING:
		{
			wxRect rc( GetAbsoluteRect() );
			int x = 0;
			int charWidth = 0;
			// �������ָ���λ��
			int ptr = e.mousePos.x - (rc.x + m_textPadding) + m_xViewStart;

			if( ptr < 0 )
				ptr = 0;

			CharIter charAtPointer( begin() ), last( end() );
			for( ; charAtPointer != last; ++charAtPointer )
			{
				EditChar* ch = *charAtPointer;
				charWidth = ch->GetWidth();

				if( (x <= ptr) && (x + charWidth > ptr) )
				{
					// ��������ַ�������һ�봦��ͬ����ǿ������
					if( ptr > (float( x ) + charWidth / 2) )
					{
						++charAtPointer;
						x += charWidth;
					}

					break;
				}
				else
				{
					x += charWidth;
				}
			}

			if( e.evtCode == DRAGGING )
			{
				DraggTo( charAtPointer, x, &e.dc );

				break;
			}
			else // �� Edit �ϵ��
			{
				// ���Ҽ��˵��������⴦��
				if( e.evtCode == RIGHT_UP )
				{
					ResumeTimer();

					m_Window->ResetCursor();
					PopupMenu( e.mousePos.x, e.mousePos.y );

					if( IsSelected() )
					{
						// �����ֶ����»�ͼ
						ShowCursorAtOnce( e.dc, false );
						Draw( e.dc );

						break;
					}
				}

				SelectNone( charAtPointer, x, &e.dc );
			}

			break;
		}

	case DLEFT_DOWN:

		// ������ʾ���
		ShowCursorAtOnce( e.dc, true );

		// ѡ���������ı�
		SelectSuccessiveText( &e.dc );

		break;

	case HOVERING:

		m_Window->AssignCursor( wxCursor( wxCURSOR_IBEAM ) );
		Draw( e.dc );

		break;

	case NORMAL:

		m_Window->ResetCursor();
		Draw( e.dc );

		ResumeTimer();

		break;

	case LEFT_UP:

		ResumeTimer();

		break;

	default:

		break;
	}
}

void VdkEdit::DoHandleNotify(const VdkNotify& notice)
{
	wxDC* pDC = notice.GetVObjDC();
	
	switch( notice.GetNotifyCode() )
	{
	case VCN_MENU_HID:

		if( ContainsPointerGlobally() )
			m_Window->AssignCursor( wxCursor( wxCURSOR_IBEAM ) );

		break;

#if 0
	case VCN_PARENT_SHOW:

		if( !IsFocused() )
			break;

		notice.SetWparam( m_Rect.x );
		notice.SetLparam( m_Rect.y );

		// �����Ǽ����¼���������������
#endif

	case VCN_FOCUS:
		{
			Start();

#       ifdef __WXGTK__
			MoveCursorOnScreen( m_cursorPos );
#       endif

			// ��Ҫ�������л�������ʾ������Ӧ���� DoHandleMouseEvent() ������

			// ��������һ������ TAB traversal �����Ľ���ת���¼���
			// ȫѡ�����ı�
			// TODO: ����㷨�Ľ���
			wxRect rc( GetAbsoluteRect() );
			if( !rc.Contains( notice.GetWparam(), notice.GetLparam() ) )
			{
				// ���Ǳ����Լ�����������ʾ
				if( pDC )
					ShowCursorAtOnce( *pDC );

				SelectAll( pDC );
			}

			break;
		}

	//case VCN_PARENT_HID:
	case VCN_LOST_FOCUS:
	case VCN_HIDE:
	case VCN_FREEZE:
		{
			if( pDC )
			{
				Draw( *pDC );
			}

			Stop();
			RemoveState( VEST_CURSOR_ON );
		}

		break;

	case VCN_FONT_CHANGED:
		{
			RefrshFontHeight();

			wxMemoryDC mdc;
			mdc.SetFont( m_Font );

			// �����ı��ܳ���
			m_total = 0;
			CharIter i( begin() ), last( end() );
			for( ; i != last; ++i )
			{
				EditChar* ch = *i;
				m_total += ch->UpdateSize( mdc );
			}

			// ����ѡ�����
			int cursorPos = GetCharOffset( m_cursorChar );
			m_xViewStart = cursorPos - GetTextAreaSize();
			TestViewStart();

			MoveCursorOnScreen( cursorPos );

			// ����ѡ���յ�
			if( !IsSelected() )
			{
				ResetSelEnd();
			}
			else
			{
				m_selEndPos = GetCharOffset( m_selEndChar );
			}

			break;
		}

	default:

		break;
	}
}

void VdkEdit::SendTextChangedMsg(wxDC* pDC)
{
	if( TestStyle( VES_TEXT_CHANGED_MSG ) )
		FireEvent( pDC, (void *) VES_TEXT_CHANGED_MSG );
}

void VdkEdit::DoHandleKeyEvent(VdkKeyEvent& e)
{
	wxASSERT( e.GetNativeObj() );
	const wxKeyEvent& ke( *e.GetNativeObj() );
	
	switch( e.evtCode )
	{
	case KEY_CHAR:
		{
			// ������༭
			if( !IsEditable() )
				return;

			wxChar ch( ke.GetUnicodeKey() );

			// ���� ASCII ����ߺ���
			if( ( ch > 0x1F && ch < 0x7F ) || ( ch > 0x7F ) )
			{
				// ������ʾ���
				ShowCursorAtOnce( e.dc, true );

				if( IsSelected() )
					RemoveSelected( &e.dc );

				InsertChar( ch, true );

				// �����ı��ı��¼�
				SendTextChangedMsg( &e.dc );
			}

			break;
		}

	case KEY_DOWN:
		{
			int keyCode( ke.GetKeyCode() );
			switch( keyCode )
			{
			case WXK_LEFT:

				// ������ʾ���
				ShowCursorAtOnce( e.dc, true );

			case WXK_HOME:
				{
					// ��ѡ��ʱ���ƶ����
					if( !ke.ShiftDown() )
					{
						CharIter cursorChar( m_cursorChar );
						int cursorPos = m_cursorPos;

						if( m_cursorPos > m_selEndPos ) // ����ѡ��
						{
							cursorPos = m_selEndPos;
							cursorChar = m_selEndChar;
						}

						if( IsSelected() )
						{
							SelectNone( cursorChar, cursorPos, &e.dc );

							if( keyCode == WXK_LEFT )
								break;
						}
					}

					// ���˵�һ��
					if( m_cursorChar == begin() )
					{
						ResumeTimer();
						return;
					}

					// �Ȳ����ĵ�ǰ�ַ����Ա�ʵ���ַ��Ķ�ѡ
					CharIter cursorChar( m_cursorChar );
					int cursorPos = m_cursorPos;

					if( keyCode == WXK_LEFT )
					{
						if( cursorChar == end() )
							cursorChar = GetLastChar();
						else
							--cursorChar;

						cursorPos -= (*cursorChar)->GetWidth();
					}
					else // keyCode == WXK_HOME
					{
						cursorChar = begin();
						cursorPos = 0;
					}

					if( ke.ShiftDown() )
					{
						DraggTo( cursorChar, cursorPos, NULL );
					}
					else
					{
						SelectNone( cursorChar, cursorPos, NULL );
					}

					break;
				}

			case WXK_RIGHT:

				// ������ʾ���
				ShowCursorAtOnce( e.dc, true );

			case WXK_END:
				{
					// ��ѡ��ʱ���ƶ����
					if( !ke.ShiftDown() )
					{
						int cursorPos( m_cursorPos );
						CharIter cursorChar( m_cursorChar );
						if( m_cursorPos < m_selEndPos ) // ����ѡ��
						{
							cursorPos = m_selEndPos;
							cursorChar = m_selEndChar;
						}

						if( IsSelected() )
						{
							SelectNone( cursorChar, cursorPos, &e.dc );

							// �Ȳ�Ҫ�ƶ����
							if( keyCode == WXK_RIGHT )
								break;
						}
					}

					// �������һ��
					if( m_cursorChar == end() )
					{
						ResumeTimer();
						return;
					}

					// �Ȳ����ĵ�ǰ�ַ����Ա�ʵ���ַ��Ķ�ѡ
					CharIter cursorChar = m_cursorChar;
					int cursorPos = m_cursorPos;

					if( keyCode == WXK_RIGHT )
					{
						cursorPos += (*cursorChar)->GetWidth();
						++cursorChar;
					}
					else // keyCode == WXK_END
					{
						cursorPos = m_total;
						cursorChar = end();
					}

					if( ke.ShiftDown() )
					{
						DraggTo( cursorChar, cursorPos, NULL );
					}
					else
					{
						SelectNone( cursorChar, cursorPos, NULL );
					}

					break;
				}

			case WXK_BACK:

				// ������༭
				if( !IsEditable() )
					return;

				// ������ʾ���
				ShowCursorAtOnce( e.dc, true );

				if( IsSelected() )
				{
					RemoveSelected( &e.dc );
					break;
				}

				Backspace();

				break;

			case WXK_DELETE:

				// ������༭
				if( !IsEditable() )
					return;

				// ������ʾ���
				ShowCursorAtOnce( e.dc, true );

				if( IsSelected() )
				{
					RemoveSelected( &e.dc );
					break;
				}

				Delete();

				break;

			case 'A':
			case 'V':
			case 'C':
			case 'P':
			case 'X':
			case 'Y':
			case 'Z':

				if( ke.ControlDown() )
				{
					switch( keyCode )
					{
					case 'A':

						SelectAll( &e.dc );

						break;

					case 'V':

						Paste();

						break;

					case 'C':

						if( IsSelected() )
							Copy();
						else
							return e.Skip( true );

						break;

					case 'X':

						if( IsSelected() )
							Cut();
						else
							return e.Skip( true );

						break;

					case 'Y':

						if( CanRedo() )
							Redo();
						else
							return e.Skip( true );

						break;

					case 'Z':

						if( CanUndo() )
							Undo();
						else
							return e.Skip( true );

						break;

					default:

						break;
					}

					break;
				} // ���ټ�����

				// ֱ��ȡ���¼��Ĵ���

			default:
#				ifdef __WXMSW__
				// ��ÿ�ΰ���ǰ���������뷨���ڵ�λ�ã�
				// ��ΪĳЩ��ʽ�����뷨�ļ����Բ����
				MoveCursorOnScreen( m_cursorPos );
#				endif // __WXMSW__

				return e.Skip( true );
			}

			break;
		}

	default:

		return e.Skip( true );
	}

	Draw( e.dc );
	ResumeTimer();
}

void VdkEdit::PopupMenu(int x, int y)
{
	if( !m_menu )
		ImplementMenu();

	// �ӳ���ʾ�˵��������Ҽ�����ʱ�ƻ��ؼ�״̬
	m_Window->ShowContextMenu( this, x, y, true );
}

void VdkEdit::ImplementMenu()
{
	VdkCtrlId id = VdkGetUniqueId( 20 );
	m_menu = m_Window->CreateMenu( NULL, id );
	m_menu->SetBestWidth( 150 );
	// VMS_ERASE_ALL ȷ���˵����ػ��Ƚ�˳�����������
	m_menu->SetAddinStyle( VMS_ERASE_ALL | VMS_SEND_PRESHOW_MSG );

	m_menuId0 = id + 1;
	for( int i = 0; i < VEM_ITEM_COUNT ; ++i )
	{
		m_menuItems[i] = NewMenuItem();
	}

	int i = -1;
	m_menu->AppendItem( m_menuItems[++i]->caption( L"����(&U)" ).id( ++id ) );
	m_menu->AppendItem( m_menuItems[++i]->caption( L"����(&R)" ).id( ++id ) );
	m_menu->AppendSeperator();

	m_menu->AppendItem( m_menuItems[++i]->caption( L"����(&T)" ).id( ++id ) );
	m_menu->AppendItem( m_menuItems[++i]->caption( L"����(&C)" ).id( ++id ) );
	m_menu->AppendItem( m_menuItems[++i]->caption( L"ճ��(&P)" ).id( ++id ) );
	m_menu->AppendItem( m_menuItems[++i]->caption( L"ɾ��(&D)" ).id( ++id ) );
	m_menu->AppendSeperator();

	m_menu->AppendItem( m_menuItems[++i]->caption( L"ȫѡ(&A)" ).id( ++id ) );

	//------------------------------------------------

	m_Window->GetWindowHandle()->Bind( wxEVT_VOBJ, 
									   &VdkEdit::OnMenuEvent, 
									   this, 
									   m_menuId0 - 1, 
									   id );
}

void VdkEdit::OnMenuEvent(VdkVObjEvent& e)
{
	switch( e.GetId() - m_menuId0 )
	{
	case VEM_MENU_ENTITY:
		{
			m_menuItems[VEM_UNDO]->disabled( !CanUndo() );
			m_menuItems[VEM_REDO]->disabled( !CanRedo() );

			bool unselected = !IsSelected();
			m_menuItems[VEM_DELETE]->disabled( unselected || !m_editable );
			m_menuItems[VEM_CUT]->disabled( unselected || !m_editable );
			m_menuItems[VEM_COPY]->disabled( unselected );

			m_menuItems[VEM_PASTE]->disabled( !CanPaste() );
			m_menuItems[VEM_SELECT_ALL]->disabled( m_text.empty() );

			break;
		}

	case VEM_UNDO:

		Undo( NULL );

		break;

	case VEM_REDO:

		Redo( NULL );

		break;

	case VEM_COPY:

		Copy();

		break;

	case VEM_CUT:

		DoCut( NULL );

		break;

	case VEM_PASTE:

		DoPaste( NULL );

		break;

	case VEM_SELECT_ALL:

		SelectAll( NULL );

		break;

	case VEM_DELETE:

		RemoveSelected( NULL );

		break;

	default:

		break;
	}

	// ���簡������Ҫ������������أ��������������
	VdkDC dc( m_Window );
	Draw( dc );
}

bool VdkEdit::Validate() const
{
	if( !m_validator )
		return true;

	m_validator->SetWindow( m_Window->GetHiddenCtrl() );
	return m_validator->Validate( GetValue(), m_WindowImpl );
}

void VdkEdit::SetValidator(const VdkTextValidator& v)
{
	wxDELETE( m_validator );
	m_validator = reinterpret_cast< VdkTextValidator* >( v.Clone() );
}

wxString VdkEdit::DoGetValue() const
{
	if( m_text.empty() )
		return wxEmptyString;

	wxString ret;
	CharIter_Const i( begin() ), last( end() );
	for( ; i != last; ++i )
	{
		ret += (*i)->get();
	}

	return ret;
}

wxString VdkEdit::GetStringSelection() const
{
	if( !IsSelected() )
		return wxEmptyString;

	CharIter first, last;
	GetSelRange( first, last );

	wxString ret;
	CharIter i( first );
	for( ; i != last; ++i )
	{
		ret.Append( (*i)->get() );
	}

	return ret;
}

void VdkEdit::SetValue(const wxString& strText, wxDC* pDC)
{
	Clear();

	if( !strText.IsEmpty() )
	{
		wxMemoryDC mdc;
		mdc.SetFont( m_Font );

		wxCoord charWidth, charHeight;
		wxString::const_iterator si( strText.begin() );
		for( ; si != strText.end(); ++si )
		{
			mdc.GetTextExtent( *si, &charWidth, &charHeight );
			m_total += charWidth;

			EditChar* ch = new EditChar( *si, charWidth, charHeight );
			m_text.push_back( ch );
		}

		SelectNone( end(), m_total, pDC );
	}

	if( pDC )
		Draw( *pDC );
}

void VdkEdit::Insert(const wxString& strText, long pos, wxDC* pDC)
{
	if( strText.empty() )
		return;

	// ����ָ�����ַ���ţ�ȡ����ǰѡ�񣬲��뵽ָ��λ��
	if( pos != wxNOT_FOUND )
	{
		wxASSERT( pos >= 0 );

		CharIter insertPoint( begin() );
		if( pos >= long( m_text.size() ) )
		{
			insertPoint = end();
		}
		else
		{
			adv( insertPoint, pos );
		}

		SelectNone( insertPoint, wxNOT_FOUND, NULL );
	}
	else
	{
		// �滻��ѡ
		if( IsSelected() )
			RemoveSelected( NULL );

		pos = IndexOf( m_cursorChar );
	}

	wxString::const_iterator i( strText.begin() ), last( strText.end() );
	for( ; i != last; ++i )
		InsertChar( *i, false );

	if( pDC )
		Draw( *pDC );

	//------------------------------------------------

	if( !IsOnUndoRedo() )
	{
		InsRmv* action = new InsRmv( this, pos, strText, InsRmv::IRT_INSERT );
		SetUndoAction( action );
	}

	// �����ı��ı��¼�
	SendTextChangedMsg( pDC );
}

void VdkEdit::InsertChar(wxChar ch, bool saveUndo)
{
	// �����ڲ����ַ�֮ǰ��ȡ�������Ϣ
	if( !IsOnUndoRedo() && saveUndo )
	{
		long pos( IndexOf( m_cursorChar ) );
		InsRmv* action = new InsRmv
			( this, pos, wxString( ch ), InsRmv::IRT_INSERT );

		SetUndoAction( action );
	}

	//------------------------------------------------

	wxCoord charWidth, charHeight;
	GetCharSize( ch, &charWidth, &charHeight );

	EditChar* pch = new EditChar( ch, charWidth, charHeight );
	m_text.insert( m_cursorChar, pch );
	m_total += charWidth;

	SelectNone( m_cursorChar, m_cursorPos + charWidth, NULL );
}

void VdkEdit::Backspace()
{
	// ���λ�ڿ�ͷ
	if( m_cursorChar == begin() )
		return;

	CharIter del( m_cursorChar );
	if( del == end() )
	{
		del = begin();

		int count( m_text.size() );
		if( count > 1 )
			adv( del, count - 1 );
	}
	else
	{
		--del;
	}

	if( !IsOnUndoRedo() )
		SaveDeleteCharState( del, InsRmv::IRT_BACK );

	wxCoord charWidth = (*del)->GetWidth();
	m_total -= charWidth;
	m_cursorPos -= charWidth;

	delete *del;
	m_text.erase( del );

	// ���²���㣬�������ܶ���ʾһЩ������ı�
	SelectNone( m_cursorChar, m_cursorPos, NULL );
}

void VdkEdit::Delete()
{
	// ����Ϊ�ջ�ǰ���������ĩβ
	if( m_cursorChar == end() )
		return;

	if( !IsOnUndoRedo() )
		SaveDeleteCharState( m_cursorChar, InsRmv::IRT_DEL );

	CharIter del( m_cursorChar );
	++m_cursorChar;
	m_total -= (*del)->GetWidth();

	delete *del;
	m_text.erase( del );

	//------------------------------------------------

	// ����Ҫ����ʾһЩ������ı�����Ϊ֮ǰ��������Ƶ����������ĩβ
	// ʱ�Ѿ�������һ������
	TestMaxViewStart();
}

void VdkEdit::SaveDeleteCharState(const CharIter& insertPoint,
								  InsRmv::ActionType type)
{
	wxASSERT( insertPoint != end() );

	long pos( IndexOf( insertPoint ) );
	InsRmv* action =
		new InsRmv( this, pos, wxString( (*insertPoint)->get() ), type );

	SetUndoAction( action );
}

inline void VdkEdit::GetCharSize(wxChar ch, wxCoord* w, wxCoord* h)
{
	m_WindowImpl->GetTextExtent( ch, w, h, 0, 0, &m_Font );
}

int VdkEdit::GetCharOffset(const CharIter& ch)
{
	if( ch == end() )
		return m_total;

	int x( 0 );
	CharIter i, last( end() );
	for( i = begin(); i != ch && i != last; ++i )
		x += (*i)->GetWidth();

	return x;
}

//////////////////////////////////////////////////////////////////////////

void VdkEdit::WriteText(const wxString& text)
{
	// TODO:
}

bool VdkEdit::CanCopy() const
{
	return IsSelected();
}

bool VdkEdit::CanCut() const
{
	return m_editable && IsSelected();
}

bool VdkEdit::CanPaste() const
{
	return m_editable && IsTextOnClipborad();
}

void VdkEdit::Copy()
{
	if( !IsSelected() )
		return;

	wxString sel( GetStringSelection() );
	CopyText( sel );
}

void VdkEdit::DoCut(wxDC* pDC)
{
	if( !IsSelected() || !IsEditable() )
		return;

	Copy();
	RemoveSelected( pDC );
}

void VdkEdit::DoPaste(wxDC* pDC)
{
	if( !IsEditable() )
		return;

	if( wxTheClipboard->Open() )
	{
		if( wxTheClipboard->IsSupported( wxDF_TEXT ) )
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );

			wxString text( data.GetText() );
			if( !text.empty() )
			{
				Insert( text, wxNOT_FOUND, pDC );
			}
		}

		wxTheClipboard->Close();
	}
}

void VdkEdit::SetUndoAction(Action* action)
{
	wxASSERT( action );
	wxASSERT( m_nextUndo < (int) m_actions.size() );

	//------------------------------------------------

	// �����ʧЧ�ĳ�������
	ActionIter first( m_actions.begin() + m_nextUndo + 1 ),
			   last( m_actions.end() );
	xerase_ptrs( m_actions, first, last );

	//------------------------------------------------

	// ��鳷���б��Ƿ���Ȼ����
	int size( m_actions.size() );
	wxASSERT( size <= m_maxUndo );
	if( size == m_maxUndo )
	{
		ActionIter last = m_actions.begin() + size - 1;
		delete *last;
		m_actions.erase( last );

		--m_nextUndo;
	}

	//------------------------------------------------

	m_actions.push_back( action );
	++m_nextUndo;

	//------------------------------------------------

	// �����ı��ı��¼�

	if( IsReadyForEvent() )
	{
		wxCommandEvent changed( wxEVT_COMMAND_TEXT_UPDATED, m_id );
		wxPostEvent( m_WindowImpl, changed );
	}
}

void VdkEdit::Undo(wxDC* pDC)
{
	if( CanUndo() )
	{
		SetAddinState( VEST_UNDO_REDOING );
		
		//------------------------------------------------

		Action* action( m_actions[m_nextUndo] );
		action->Exec( pDC );

		// ��ǰ��
		--m_nextUndo;

		//------------------------------------------------

		RemoveState( VEST_UNDO_REDOING );
	}
}

void VdkEdit::Redo(wxDC* pDC)
{
	if( CanRedo() )
	{
		SetAddinState( VEST_UNDO_REDOING );
		
		//------------------------------------------------

		Action* action( m_actions[m_nextUndo + 1] );
		action->UnExec( pDC );

		// ��ǰ��
		++m_nextUndo;

		//------------------------------------------------

		RemoveState( VEST_UNDO_REDOING );
	}
}

void VdkEdit::Backward(long step)
{
	if( step == 0 )
		return;

	if( step < 0 )
		return Forward( - step );

	int dx( 0 );
	CharIter curr( m_cursorChar ), first( begin() );
	// ���統ǰ���λ���ı���ĩβ���������⴦����Ϊ --last ���ܵõ�
	// ���һ���ַ��ĵ�����
	if( curr == end() )
	{
		--step;
		curr = GetLastChar();
		dx += (*curr)->GetWidth();
	}

	for( int i = 0; i < step && curr != first ; i++ )
	{
		dx += (*curr)->GetWidth();
		--curr;
	}

	m_cursorChar = curr;
	m_cursorPos -= dx;
}

void VdkEdit::Forward(long step)
{
	if( step == 0 )
		return;

	if( step < 0 )
		return Backward( -step );

	int dx = 0;
	CharIter last( end() );
	for( int i = 0; i < step && m_cursorChar != last ; i++ )
	{
		dx += (*m_cursorChar)->GetWidth();
		++m_cursorChar;
	}

	m_cursorPos += dx;
}

void VdkEdit::SetInsertionPoint(long pos, wxDC* pDC)
{
	if( m_text.empty() )
		return;

	long len = m_text.size();
	if( pos < 0 )
		pos = len + pos + 1;
	else if( pos > len )
		pos = len;

	CharIter cursorChar( begin() );
	adv( cursorChar, pos );

	//-----------------------------------------------------

	SetInsertionPoint( cursorChar, wxNOT_FOUND, pDC );
}

void VdkEdit::MoveToEnd(wxDC* pDC)
{
	SelectNone( m_text.end(), m_total, pDC );
}

void VdkEdit::SetInsertionPoint(const CharIter& pos, int cursorPos, wxDC* pDC)
{
	// ����Ҫ���ָ��������Ƿ��뵱ǰ�����λ����ͬ����Ϊ���ǿ��ܽ�������Ҫ
	// ������������ʾһЩ������ı����������

	if( cursorPos == wxNOT_FOUND )
		cursorPos = GetCharOffset( pos );

	// �������
	int showArea( GetTextAreaSize() );
	if( cursorPos > (m_xViewStart + showArea) ||
		cursorPos < (m_xViewStart) )
	{
		// ���ܽ� maxAddIn ��Ϊ static �ģ���Ϊ���ǵ� m_Rect.width �ᷢ���仯
		const int maxAddIn = float( m_Rect.width ) * 0.3;

		if( cursorPos > m_cursorPos ) // �����ƶ����
		{
			m_xViewStart = cursorPos - showArea;

			// ����ʾһЩ������ı�
			int addIn = 0, charsize;
			CharIter i( pos ), last( end() );
			for( ; i != last; ++i )
			{
				charsize= (*i)->GetWidth();
				addIn += charsize;

				if( addIn > maxAddIn )
				{
					addIn -= charsize;
					break;
				}
			}

			m_xViewStart += addIn;

			TestViewStart();
		}
		else
		{
			m_xViewStart = cursorPos;

			// ����ʾһЩ������ı�
			int addIn = 0, charsize;
			CharIter i( pos );
			for( ; ; --i )
			{
				charsize= (*i)->GetWidth();
				addIn += charsize;

				if( addIn > maxAddIn )
				{
					addIn -= charsize;
					break;
				}

				if( i == begin() )
					break;
			}

			m_xViewStart -= addIn;

			if( m_xViewStart < 0 )
				m_xViewStart = 0;
		}
	}

	//------------------------------------------------
	// �ύ����

	m_cursorChar = pos;
	MoveCursorOnScreen( cursorPos );

	if( pDC )
	{
		ShowCursorAtOnce( *pDC );
	}
}

long VdkEdit::GetInsertionPoint() const
{
	VdkEdit* This = const_cast< VdkEdit* >( this );

	long index = 0;
	CharIter iter( This->m_text.begin() );
	for( ; iter != This->m_text.end(); ++iter, ++index )
	{
		if( iter == m_cursorChar )
			return index;
	}

	return 0;
}

long VdkEdit::GetLastPosition() const
{
	return m_text.size();
}

void VdkEdit::SetSelection(long from, long to, wxDC* pDC)
{
	CharIter first, last;
	if( !GetRangeInterators( from, to, first, last ) )
		return;

	SetSelection( first, last, pDC );
}

void VdkEdit::SetSelection(const CharIter& from, const CharIter& to, wxDC* pDC)
{
	m_selEndChar = from;
	m_selEndPos = GetCharOffset( from );

	m_cursorChar = to;
	MoveCursorOnScreen( GetCharOffset( to ) );

	if( pDC )
		Draw( *pDC );
}

void VdkEdit::SelectSuccessiveText(wxDC* pDC)
{
	if( m_text.empty() )
		return;

	CharIter first( m_cursorChar ), temp, last( m_cursorChar );
	CharIter end0( end() );
	CharType type0( CT_NONE ), type( CT_NONE );

	if( first == end0 )
		first = GetLastChar();

	type0 = GetCharType( first );
	CharIter beg0( begin() );

	// ��ǰ����
	temp = first; // ����ʩ�� ++temp; ���Ż�������ѡ�����һ���ַ�ʱ�ᵼ��
	// ����Ĵ��벻��ִ��
	if( temp != end0 )
	{
		while( true )
		{
			type = GetCharType( temp );
			if( type0 != type )
				break;

			first = temp;
			if( first == beg0 )
				break;

			--temp;
		}
	}

	if( last != end0 )
	{
		// ������
		while( true )
		{
			type = GetCharType( last );
			if( type0 == type )
				++last;
			else
				break;

			if( last == end0 )
				break;
		}
	}

	if( first == last )
		return;

	SetSelection( first, last, pDC );
}

VdkEdit::CharType VdkEdit::GetCharType(const CharIter& iter)
{
	wxChar ch( (*iter)->get() );

	// һ��Ҫ���ж������ַ���MS �ı��ػ��ᵷ��
	if( ch > 0x7F )
		return CT_NON_ENGLISH;

	if( (ch >= 'A' && ch <= 'Z') ||
		(ch >= 'a' && ch <= 'z') ||
		(ch >= '0' && ch <= '9') )
	{
		return CT_ALPHA;
	}

	if( isspace( ch ) )
		return CT_SPACE;

	if( ispunct( ch ) )
		return CT_PUNCTUATE;

	wxASSERT_MSG( false, L"δ֪�ַ����͡�" );
	return CT_NONE;
}

bool VdkEdit::IsEditable() const
{
	return m_editable;
}

void VdkEdit::SetEditable(bool editable, wxDC* pDC)
{
	m_editable = editable;

	if( pDC )
	{
		Draw( *pDC );
	}
}

void VdkEdit::RecoverChar(const CharIter& i, wxDC &dc)
{
	int x = m_textPadding + GetCharOffset( i );
	EditChar* ch( *i );

	wxBrush brush( m_sstyle->m_BgBrush );
	if( IsSelected() &&
		m_selEndPos > m_cursorPos && // ����ѡ��
		x == m_cursorPos + m_textPadding )
	{
		brush = m_sstyle->m_SelTextBgBrush;
		dc.SetTextForeground( *wxWHITE ); // TODO:
	}

	ClrBkGnd( dc, brush, wxRect( x, m_yFix - m_textPadding, ch->GetWidth(),
								 m_fontHeight + m_textPadding * 2 ) );

	dc.SetFont( m_Font );

	int yFix = double( m_Rect.height - ch->GetHeight() ) / 2;
	ch->Draw( dc, x, yFix );
}

void VdkEdit::SelectAll(wxDC* pDC)
{
	if( m_text.empty() )
		return;

	m_selEndPos = 0;
	m_selEndChar = begin();

	MoveCursorOnScreen( m_total );
	m_cursorChar = end();

	if( pDC )
		Draw( *pDC );
}

bool VdkEdit::IsSelected() const
{
	return m_selEndChar != m_cursorChar;
}

inline void VdkEdit::ResetSelEnd()
{
	m_selEndChar = m_cursorChar;
	m_selEndPos = m_cursorPos;
}

void VdkEdit::SelectNone(const CharIter& i, int cursorPos, wxDC* pDC)
{
	SetInsertionPoint( i, cursorPos, pDC );
	ResetSelEnd();

	if( pDC )
	{
		Draw( *pDC );
	}
}

void VdkEdit::DraggTo(const CharIter& i, int cursorPos, wxDC* pDC)
{
	if( i == m_cursorChar )
		return;

	// �������ָ���ƶ�
	SetInsertionPoint( i, cursorPos, NULL );

	if( pDC )
	{
		Draw( *pDC );
	}
}

void VdkEdit::GetSelection(long* from, long* to) const
{
	CharIter beg( const_cast< VdkEdit* >( this )->m_text.begin() );
	size_t distFirst = distance( beg, m_cursorChar );
	size_t distLast = distance( beg, m_selEndChar );

	if( distLast < distFirst )
	{
		wxSwap( distFirst, distLast );
	}

	if( from ) *from = distFirst;
	if( to ) *to = distLast;
}

void VdkEdit::GetSelRange(CharIter& first, CharIter& last) const
{
	first = m_cursorChar;
	last = m_selEndChar;

	// �п���ѡ�е��ı��ǲ�����ʾ�Ŀ����ַ�
	if( m_selEndPos != m_cursorPos )
	{
		if( m_selEndPos < m_cursorPos )
			wxSwap( first, last );
	}
	else
	{
		CharIter beg( const_cast< VdkEdit* >( this )->m_text.begin() );
		size_t distFirst = distance( beg, first );
		size_t distLast = distance( beg, last );

		if( distLast < distFirst )
		{
			wxSwap( first, last );
		}
	}
}

void VdkEdit::Remove(long from, long to, wxDC* pDC)
{
	CharIter first, last;
	if( !GetRangeInterators( from, to, first, last ) )
		return;

	DoRemove( first, last, from, pDC );
}

void VdkEdit::RemoveSelected(wxDC* pDC)
{
	if( !IsSelected() )
		return;

	CharIter first, last;
	GetSelRange( first, last );

	DoRemove( first, last, wxNOT_FOUND, pDC );
}

void VdkEdit::DoRemove(const CharIter& first, 
					   const CharIter& last, 
					   long from, 
					   wxDC* pDC)
{
	int dx = 0;
	wxString deleted;
	CharIter i;
	EditChar* ch;

	for( i = first; i != last; ++i )
	{
		ch = *i;

		// ��ȡ��ȸı���
		dx += ch->GetWidth();
		// ������ɾ���ַ���Ϊ����������׼��
		deleted.Append( ch->get() );
	}

	if( !IsOnUndoRedo() && from == wxNOT_FOUND )
		from = IndexOf( first );

	xerase_ptrs( m_text, first, last );
	m_total -= dx;

	//------------------------------------------------

	m_cursorChar = last;
	MoveCursorOnScreen( GetCharOffset( m_cursorChar ) );
	ResetSelEnd();

	//------------------------------------------------

	TestMaxViewStart();

	if( pDC )
		Draw( *pDC );

	//------------------------------------------------

	if( !IsOnUndoRedo() )
	{
		InsRmv* undo = new InsRmv( this, from, deleted, InsRmv::IRT_REMOVE );
		SetUndoAction( undo );
	}
}

void VdkEdit::TestViewStart()
{
	if( m_xViewStart < 0 )
	{
		m_xViewStart = 0;
		return;
	}

	int maxViewStart = m_total - GetTextAreaSize();
	if( m_xViewStart >= maxViewStart )
	{
		// ĩβ���һ�� m_textPadding
		m_xViewStart = maxViewStart + m_textPadding;
	}
}

void VdkEdit::TestMaxViewStart()
{
	int showArea( GetTextAreaSize() );
	if( m_xViewStart + showArea > m_total )
	{
		m_xViewStart = m_total - showArea;
		TestViewStart();
	}
}

void VdkEdit::SaveCurrentState()
{

}

VdkEdit::CharIter VdkEdit::GetLastChar()
{
	CharIter cursorChar( begin() );

	if( !m_text.empty() )
	{
		adv( cursorChar, m_text.size() - 1 );
	}

	return cursorChar;
}

long VdkEdit::IndexOf(const CharIter& i)
{
	long ret( 0 );
	CharIter iter, last( end() );
	for( iter = begin(); iter != i && iter != last; ++iter )
	{
		ret++;
	}

	return ret;
}

VdkEdit::CharIter VdkEdit::GetCharIterator(long pos)
{
	return begin();
}

bool VdkEdit::GetRangeInterators(long from, long to, CharIter& first, CharIter& last)
{
	if( from < 0 || to < 0 || from >= to )
		return false;

	wxASSERT( from <= (long) m_text.size() && to <= (long) m_text.size() );

	//------------------------------------------------

	first = begin();
	if( from > 0 )
		adv( first, from );

	last = first;
	adv( last, to - from );

	return true;
}

void VdkEdit::SetStaticStyle(VdkEditStaticStyle* sstyle)
{
	if( m_sstyle == sstyle )
		return;

	if( m_sstyle )
	{
		m_sstyle->decRef();
		m_sstyle = NULL;
	}

	m_sstyle = sstyle;
}

VdkEditStaticStyle* VdkEdit::GetStaticStyle() const
{
	return m_sstyle;
}

//////////////////////////////////////////////////////////////////////////

VdkEditInitializer::VdkEditInitializer()
	: TextAlign( TEXT_ALIGN_LEFT ),
	  TextColor( *wxBLACK ),
	  BgColor( *wxWHITE ),
	  Validator( NULL ),
	  Editable( true )
{

}

VdkEditInitializer& VdkEditInitializer::validator(const VdkTextValidator& v)
{
	Validator = reinterpret_cast< VdkTextValidator* >( v.Clone() );
	return *this;
}

//////////////////////////////////////////////////////////////////////////

VdkEdit::Character::CharacterPool VdkEdit::Character::ms_pool;

VdkEdit::Character* VdkEdit::Character::GetInstance(wxChar ch)
{
	CharIter iter( ms_pool.find( ch ) );
	if( iter != ms_pool.end() )
	{
		return iter->second;
	}

	Character* pch = new Character( ch );
	ms_pool[ch] = pch;

	return pch;
}

void VdkEdit::Character::Draw(wxDC& dc, int x, int y)
{
	dc.DrawText( m_char, x, y );
}

void VdkEdit::Character::DeleteAllInstances()
{
	CharIter i, last( ms_pool.end() );
	for( i = ms_pool.begin(); i != last; ++i )
	{
		delete i->second;
	}

	ms_pool.clear();
}

//////////////////////////////////////////////////////////////////////////

VdkEdit::EditChar::EditChar(wxChar ch, wxDC& dc)
	: m_char( NULL ), m_width( 0 ), m_height( 0 )
{
	m_char = Character::GetInstance( ch );
	UpdateSize( dc );
}

VdkEdit::EditChar::EditChar(wxChar ch, wxCoord width, wxCoord height)
	: m_char( NULL ), m_width( width ), m_height( height )
{
	m_char = Character::GetInstance( ch );
}

int VdkEdit::EditChar::UpdateSize(wxDC& dc)
{
	dc.GetTextExtent( m_char->get(), &m_width, &m_height );
	return m_width;
}

void VdkEdit::EditChar::Draw(wxDC& dc, wxCoord x, wxCoord y)
{
	m_char->Draw( dc, x, y );
}

//////////////////////////////////////////////////////////////////////////

void VdkEdit::InsRmv::Exec(wxDC* pDC)
{
	if( IsDelOperation() )
		UndoDel( pDC );
	else
		UndoIns( pDC );
}

void VdkEdit::InsRmv::UnExec(wxDC* pDC)
{
	if( IsDelOperation() )
		UndoIns( pDC );
	else
		UndoDel( pDC );
}

void VdkEdit::InsRmv::UndoIns(wxDC* pDC)
{
	wxString::size_type length( m_operand.length() );
	m_edit->Remove( m_insertPoint, m_insertPoint + length, pDC );

	if( pDC )
		m_edit->Draw( *pDC );
}

void VdkEdit::InsRmv::UndoDel(wxDC* pDC)
{
	m_edit->Insert( m_operand, m_insertPoint, NULL );

	wxString::size_type length( m_operand.length() );
	if( length > 1 )
	{
		// ɾ�������ַ�ʱ��Ҫѡ������ַ�
		m_edit->SetSelection
			( m_insertPoint, m_insertPoint + length, NULL );
	}
	// ����ʹ�� Delete ��ɾ��һ����������ַ����ָ���ǰ�ƹ��һ���ַ�
	else if( m_type == IRT_DEL )
		m_edit->Backward( 1 );

	if( pDC )
		m_edit->Draw( *pDC );
}

//////////////////////////////////////////////////////////////////////////

VdkEditStaticStyle::VdkEditStaticStyle()
	: m_TextColor( *wxBLACK ),
	  m_NormalBorder( wxColour( 165,195,212 ) ),
	  m_InnerBorder( wxColour( 51,139,192 ) ),
	  m_OuterBorder( wxColour( 96,200,253 ) ),
	  m_BgBrush( *wxWHITE_BRUSH ),
	  m_SelTextBgBrush( *wxBLUE_BRUSH ),
	  m_UneditableBgBrush( *wxLIGHT_GREY_BRUSH ),
	  m_Resetter( NULL )
{

}

/// \brief VdkEdit ��̬��񹤳�
class VdkEditStaticStyleFactory
{
public:

	/// \brief ��ȡָ���ľ�̬���
	static VdkEditStaticStyle* GetStyle(VdkEditStaticStyleID id)
	{
		switch( id )
		{
		case VESS_MS_WORD_2003:

			return Word2003Style::Implement();

		case VESS_MS_WINDOWS:

			return WindowsNormalStyle::Implement();

		default:

			wxASSERT_MSG( false, "��Ч����ʶ����" );

			return NULL;
		}
	}

private:

	/// \brief ��ȡ Word 2003 ��ʽ�ľ�̬���
	class Word2003Style
	{
	public:

		// ��������
		static VdkEditStaticStyle* Implement()
		{
			++ms_refCount;

			if( !ms_style )
			{
				ms_style = new VdkEditStaticStyle;
				ms_style->selTextBgBrush( *wxBLACK_BRUSH );

				ms_style->resetter( Reset );
			}

			return ms_style;
		}

		// ���ü�����һ
		static void Reset()
		{
			if( ms_refCount == 0 )
				return;

			--ms_refCount;
			if( ms_refCount == 0 )
			{
				delete ms_style;
				ms_style = NULL;
			}
		}

	private:

		static VdkEditStaticStyle* ms_style;
		static int ms_refCount;
	};

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡ Windows Ĭ����ʽ�ľ�̬���
	class WindowsNormalStyle
	{
	public:

		// ��������
		static VdkEditStaticStyle* Implement()
		{
			++ms_refCount;

			if( !ms_style )
			{
				ms_style = new VdkEditStaticStyle;
				ms_style->resetter( Reset );
			}

			return ms_style;
		}

		// ���ü�����һ
		static void Reset()
		{
			if( ms_refCount == 0 )
				return;

			--ms_refCount;
			if( ms_refCount == 0 )
			{
				delete ms_style;
				ms_style = NULL;
			}
		}

	private:

		static VdkEditStaticStyle* ms_style;
		static int ms_refCount;
	};
};

VdkEdit& operator << (VdkEdit& edit, int num)
{
	wxString val( edit.GetValue() );
	edit.SetValue( wxString::Format( L"%s%d", val, num ) );

	return edit;
}

//////////////////////////////////////////////////////////////////////////

VdkEditStaticStyle* VdkEditStaticStyleFactory::
					Word2003Style::
					ms_style = NULL;

int VdkEditStaticStyleFactory::
	Word2003Style::
	ms_refCount = 0;

VdkEditStaticStyle* VdkEditStaticStyleFactory::
					WindowsNormalStyle::
					ms_style = NULL;

int VdkEditStaticStyleFactory::
	WindowsNormalStyle::
	ms_refCount = 0;

void VdkEdit::SetStaticStyle(VdkEditStaticStyleID id)
{
	if( id <= VESS_BEGIN || id >= VESS_END )
		return;

	SetStaticStyle( VdkEditStaticStyleFactory::GetStyle( id ) );
}

bool VdkEdit::CanUndo() const
{
	return m_nextUndo != wxNOT_FOUND && IsEditable();
}

bool VdkEdit::CanRedo() const
{
	return m_nextUndo != (int) m_actions.size() - 1 && IsEditable();
}

bool VdkEdit::ShouldFlashCursor()
{
	return IsFocused() && IsRunning() && IsShownOnScreen();
}

void VdkEdit::ResumeTimer()
{
	if( !IsRunning() )
		Start();
}

bool VdkEdit::IsOnUndoRedo() const
{
	return TestState( VEST_UNDO_REDOING );
}

int VdkEdit::GetTextAreaSize() const
{
	return m_Rect.width - m_textPadding * 2;
}

int VdkEdit::GetCanvasWidth() const
{
	return GetTextAreaSize() + m_cursorSize;
}
