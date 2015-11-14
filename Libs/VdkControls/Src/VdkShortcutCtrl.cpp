/***************************************************************
 * Name:      VdkShortcutCtrl.cpp
 * Purpose:   以可视化的方式定义键盘快捷键
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-18
 **************************************************************/
#include "StdAfx.h"
#include "VdkShortcutCtrl.h"

#include "wxUtil.h" // for GetKeyCodeName

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT( VdkShortcutCtrl );

//////////////////////////////////////////////////////////////////////////

VdkShortcutCtrl::VdkShortcutCtrl()
	: m_modifiers( 0 ), m_keycode( WXK_NONE )
{

}

void VdkShortcutCtrl::Create(wxXmlNode* node)
{
	VdkEdit::Create( node );
	SetEditable( false, NULL );
}

void VdkShortcutCtrl::SetShortcut(int modifiers, int keycode, wxDC* pDC)
{
	//if( m_modifiers == modifiers && m_keycode == keycode )
	//	return;

	wxArrayString modifierStrings;
	if( modifiers & wxMOD_CONTROL )
		modifierStrings.push_back( L"Ctrl" );

	if( modifiers & wxMOD_SHIFT )
		modifierStrings.push_back( L"Shift" );

	if( modifiers & wxMOD_ALT )
		modifierStrings.push_back( L"Alt" );

	if( modifiers & wxMOD_WIN )
		modifierStrings.push_back( L"Win" );

	if( keycode != WXK_NONE )
	{
		wxString strKeyCode( GetKeyCodeName( keycode ) );
		if( !strKeyCode.empty() )
		{
			modifierStrings.push_back( strKeyCode );
		}
		else
		{
			// Cannot dispalay it
			return;
		}
	}

	wxString caption;
	wxArrayString::size_type numModifiers = modifierStrings.size();
	for( size_t i = 0; i < numModifiers; i++ )
	{
		caption += modifierStrings[i];

		if( i != numModifiers - 1 )
			caption += L" + ";
	}

	//----------------------------------------
	// Commit changes

	m_modifiers = modifiers;
	m_keycode = keycode;

	SetValue( caption.empty() ? L"无" : caption, pDC );
}

void VdkShortcutCtrl::Clear(wxDC* pDC)
{
	m_modifiers = 0;
	m_keycode = WXK_NONE;

	SetValue( L"无", pDC );
}

void VdkShortcutCtrl::DoHandleKeyEvent(VdkKeyEvent& e)
{
	wxKeyEvent* ke = e.GetNativeObj();
	if( !ke )
		return;

	wxEventType et = ke->GetEventType();
	if( et == wxEVT_KEY_UP )
	{
		if( m_keycode == WXK_NONE )
			Clear( &e.dc );

		return;
	}
	else if( et != wxEVT_KEY_DOWN )
		return;

	//=====================================================

	int modifiers = m_modifiers, keycode = 0;

	if( !ke->ControlDown() )
		modifiers &= ~wxMOD_CONTROL;
	else
		modifiers |= wxMOD_CONTROL;

	if( !ke->ShiftDown() )
		modifiers &= ~wxMOD_SHIFT;
	else
		modifiers |= wxMOD_SHIFT;

	if( !ke->AltDown() )
		modifiers &= ~wxMOD_ALT;
	else
		modifiers |= wxMOD_ALT;

	if( !wxGetKeyState( WXK_WINDOWS_LEFT ) && 
		!wxGetKeyState( WXK_WINDOWS_RIGHT ) )
	{
		modifiers &= ~wxMOD_WIN;
	}
	else
	{
		modifiers |= wxMOD_WIN;
	}

	int kc = ke->GetKeyCode();
	switch( kc )
	{
	case WXK_CONTROL:
	case WXK_SHIFT:
	case WXK_ALT:
	case WXK_WINDOWS_LEFT:
	case WXK_WINDOWS_RIGHT:

		break;
		
	case WXK_DELETE:
	case WXK_RETURN:
	case WXK_BACK:

		// 清空当前已按下的快捷键
		modifiers = 0;
		keycode = 0;
		break;

	default:

		keycode = kc;
		break;
	}

	SetShortcut( modifiers, keycode, &e.dc );
}
