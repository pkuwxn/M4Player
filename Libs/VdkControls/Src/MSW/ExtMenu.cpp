#include "StdAfx.h"
#include "MSW/ExtMenu.h"

#ifndef MN_GETHMENU
#   define MN_GETHMENU         0x01E1
#endif // MN_GETHMENU

#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


//////////////////////////////////////////////////////////////////////////

size_t						ExtMenu::m_refCount(0);
WNDPROC						ExtMenu::m_oldMenuWndProc(NULL);
WNDPROC						ExtMenu::m_oldMainWndProc(NULL);
VectorOfMenuHwndInfo		ExtMenu::m_menuHandles;

//////////////////////////////////////////////////////////////////////////

HWND ExtMenu::GetActiveExtMenu()
{
	// 因为子菜单肯定后于父菜单创建并显示，故从后开始查找，
	// 避免先碰到仍在显示的父菜单
	int size( m_menuHandles.size() ), i;
	for( i = size - 1; i >= 0 ; i-- )
	{
		if( IsWindowVisible( m_menuHandles[i].hWnd ) )
			break;
	}

	HWND hwnd( NULL );
	if( i != -1 )
		hwnd = m_menuHandles[i].hWnd;

	return hwnd;
}

void ExtMenu::InitExtMenu()
{
	m_refCount++;
	::SetWindowsHookEx(WH_CALLWNDPROC, TryHookMenuWnd, wxGetInstance(), ::GetCurrentThreadId());
}

#define BitsOfType( T ) sizeof( (T) ) * 8

template < class Int >
TCHAR* Dec2Bin(Int dec, TCHAR bin[ sizeof( Int ) * 8 + 1 ])
{
	typedef TCHAR Output[ sizeof( Int ) * 8 + 1 ];
	Output o;

	int count = 0;
	int i = -1;

	while( true ) {

		++count;

		o[++i] = '0' + dec % 2;
		dec /= 2;

		if( dec == 1 )
		{
			++count;

			o[++i] = '1';
			o[++i] = 0;

			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	for( int i = count - 1; i >= 0; --i )
		bin[count - 1 - i] = o[i];

	bin[count] = 0;

	//////////////////////////////////////////////////////////////////////////

	return bin;
}

HRESULT CALLBACK ExtMenu::TryHookMenuWnd(int code, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT* pStruct = (CWPSTRUCT*)lParam;

	while( code == HC_ACTION )
	{
		HWND hWnd = pStruct->hwnd;

		// 捕捉菜单创建WM_CREATE, 0x01E2是什么消息我不知道,呵呵,这一段参考了别人的代码
		if( pStruct->message != WM_CREATE && pStruct->message != 0x01E2)
			break;

		TCHAR sClassName[10];
		int Count = ::GetClassName( hWnd, sClassName, 10 );

		// 检查是否菜单窗口
		if( Count != 6 || _tcscmp(sClassName, _T("#32768")) != 0 )
			break;

		//////////////////////////////////////////////////////////////////////////

		HMENU hMenu((HMENU)SendMessage(hWnd, MN_GETHMENU, 0, 0));
		if( hMenu )
		{
#if 1
			DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
			/*TCHAR bin[ sizeof( DWORD ) * 8 + 1 ];
			dprintf( L"GWL_STYLE: [%u]%s", dwStyle, Dec2Bin( dwStyle, bin ) );*/

			if( dwStyle & WS_OVERLAPPED ) dprintf( L"WS_OVERLAPPED" );
			if( dwStyle & WS_POPUP ) dprintf( L"WS_POPUP" );
			if( dwStyle & WS_CHILD ) dprintf( L"WS_CHILD" );
			if( dwStyle & WS_MINIMIZE ) dprintf( L"WS_MINIMIZE" );
			if( dwStyle & WS_VISIBLE ) dprintf( L"WS_VISIBLE" );
			if( dwStyle & WS_DISABLED ) dprintf( L"WS_DISABLED" );
			if( dwStyle & WS_CLIPSIBLINGS ) dprintf( L"WS_CLIPSIBLINGS" );
			if( dwStyle & WS_CLIPCHILDREN ) dprintf( L"WS_CLIPCHILDREN" );
			if( dwStyle & WS_MAXIMIZE ) dprintf( L"WS_MAXIMIZE" );
			/*
			// WS_CAPTION = WS_BORDER | WS_DLGFRAME
			if( dwStyle & WS_CAPTION ) dprintf( L"WS_CAPTION" );
			*/
			if( dwStyle & WS_BORDER ) dprintf( L"WS_BORDER" );
			if( dwStyle & WS_DLGFRAME ) dprintf( L"WS_DLGFRAME" );
			if( dwStyle & WS_VSCROLL ) dprintf( L"WS_VSCROLL" );
			if( dwStyle & WS_HSCROLL ) dprintf( L"WS_HSCROLL" );
			if( dwStyle & WS_SYSMENU ) dprintf( L"WS_SYSMENU" );
			if( dwStyle & WS_THICKFRAME ) dprintf( L"WS_THICKFRAME" );
			if( dwStyle & WS_GROUP ) dprintf( L"WS_GROUP" );
			if( dwStyle & WS_TABSTOP ) dprintf( L"WS_TABSTOP" );
			if( dwStyle & WS_MINIMIZEBOX ) dprintf( L"WS_MINIMIZEBOX" );
			if( dwStyle & WS_MAXIMIZEBOX ) dprintf( L"WS_MAXIMIZEBOX" );

			DWORD myStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_BORDER;

			TCHAR bin[ sizeof( DWORD ) * 8 + 1 ];
			dprintf( L"MS STYLE: [%u]%s", dwStyle, Dec2Bin( dwStyle, bin ) );
			dprintf( L"My Style: [%u]%s", myStyle, Dec2Bin( myStyle, bin ) );

			//////////////////////////////////////////////////////////////////////////

			DWORD dwExtStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);

			if( dwExtStyle & WS_EX_DLGMODALFRAME ) dprintf( L"WS_EX_DLGMODALFRAME" );
			if( dwExtStyle & WS_EX_NOPARENTNOTIFY ) dprintf( L"WS_EX_NOPARENTNOTIFY" );
			if( dwExtStyle & WS_EX_TOPMOST ) dprintf( L"WS_EX_TOPMOST" );
			if( dwExtStyle & WS_EX_ACCEPTFILES ) dprintf( L"WS_EX_ACCEPTFILES" );
			if( dwExtStyle & WS_EX_TRANSPARENT ) dprintf( L"WS_EX_TRANSPARENT" );
			if( dwExtStyle & WS_EX_MDICHILD ) dprintf( L"WS_EX_MDICHILD" );
			if( dwExtStyle & WS_EX_TOOLWINDOW ) dprintf( L"WS_EX_TOOLWINDOW" );
			if( dwExtStyle & WS_EX_WINDOWEDGE ) dprintf( L"WS_EX_WINDOWEDGE" );
			if( dwExtStyle & WS_EX_CLIENTEDGE ) dprintf( L"WS_EX_CLIENTEDGE" );
			if( dwExtStyle & WS_EX_CONTEXTHELP ) dprintf( L"WS_EX_CONTEXTHELP" );
			if( dwExtStyle & WS_EX_RIGHT ) dprintf( L"WS_EX_RIGHT" );
			if( dwExtStyle & WS_EX_LEFT ) dprintf( L"WS_EX_LEFT" );
			if( dwExtStyle & WS_EX_RTLREADING ) dprintf( L"WS_EX_RTLREADING" );
			if( dwExtStyle & WS_EX_LTRREADING ) dprintf( L"WS_EX_LTRREADING" );
			if( dwExtStyle & WS_EX_LEFTSCROLLBAR ) dprintf( L"WS_EX_LEFTSCROLLBAR" );
			if( dwExtStyle & WS_EX_RIGHTSCROLLBAR ) dprintf( L"WS_EX_RIGHTSCROLLBAR" );
			if( dwExtStyle & WS_EX_CONTROLPARENT ) dprintf( L"WS_EX_CONTROLPARENT" );
			if( dwExtStyle & WS_EX_STATICEDGE ) dprintf( L"WS_EX_STATICEDGE" );
			if( dwExtStyle & WS_EX_APPWINDOW ) dprintf( L"WS_EX_APPWINDOW" );
			if( dwExtStyle & WS_EX_LAYERED ) dprintf( L"WS_EX_LAYERED" );
			if( dwExtStyle & WS_EX_NOINHERITLAYOUT ) dprintf( L"WS_EX_NOINHERITLAYOUT" );
			if( dwExtStyle & WS_EX_LAYOUTRTL ) dprintf( L"WS_EX_LAYOUTRTL" );
			if( dwExtStyle & WS_EX_COMPOSITED ) dprintf( L"WS_EX_COMPOSITED" );
			if( dwExtStyle & WS_EX_NOACTIVATE ) dprintf( L"WS_EX_NOACTIVATE" );

			dprintf( L"GWL_EXTSTYLE: [%u]%u", dwExtStyle, 
				WS_EX_DLGMODALFRAME | WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE );

#endif

			HWND phwnd = ::GetParent(hWnd);
			if( !phwnd )
				phwnd = ::GetAncestor( hWnd, GA_PARENT );
			else if( !phwnd )
				phwnd = ::GetWindow( hWnd, GW_OWNER );

			if( phwnd )
			{
				wchar_t	text[512];
				if( ::GetWindowText( phwnd, text, 512 ) )
					dprintf( text );
			}

			//////////////////////////////////////////////////////////////////////////

			// 将子菜单设置成自画
			MENUITEMINFO mii;
			ZeroMemory(&mii, sizeof(MENUITEMINFO));
			mii.cbSize = sizeof(MENUITEMINFO);

			mii.fMask = MIIM_TYPE;
			GetMenuItemInfo(hMenu, 0, TRUE, &mii);
			if( mii.fType != MFT_OWNERDRAW )
			{
				DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
				dwStyle &= ~WS_BORDER;
				::SetWindowLong(hWnd, GWL_STYLE, dwStyle);

				DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
				dwExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE);
				::SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);
			}

			WNDPROC lastWndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
			if( lastWndProc != (WNDPROC)&ExtMenu::ExtMenuWndProc )
			{
				//替换菜单窗口过程函数
				SetWindowLong(hWnd, GWL_WNDPROC, (long)ExtMenu::ExtMenuWndProc);

				//保留旧的窗口过程
				MenuHwndInfo mhi = { NULL, hWnd, lastWndProc, false };
				m_menuHandles.push_back(mhi);

				m_oldMenuWndProc = lastWndProc;
			}

		}

		break;
	}

	return CallNextHookEx((HHOOK)WH_CALLWNDPROC, code, wParam, lParam);
}

LRESULT CALLBACK ExtMenu::ExtMenuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HRESULT lResult(0);

	HMENU hMenu((HMENU)SendMessage(hWnd, MN_GETHMENU, 0, 0));
	if( hMenu )
	{
		MENUITEMINFO mii;
		ZeroMemory(&mii, sizeof(MENUITEMINFO));
		mii.cbSize = sizeof(MENUITEMINFO);

		mii.fMask = MIIM_TYPE;
		GetMenuItemInfo(hMenu, 0, TRUE, &mii);
		if( mii.fType != MFT_OWNERDRAW )
			return CallWindowProc(m_oldMenuWndProc, hWnd, message, wParam, lParam);
	}

	switch(message)
	{
    //*
	case WM_ERASEBKGND:

		return 0;//*/

	// 这里阻止非客户区的绘制
	case WM_PRINT:

		return CallWindowProc(m_oldMenuWndProc, hWnd, WM_PRINTCLIENT, wParam, lParam);

	// 因为在MeasureItem里指定的菜单大小，系统会自动替你加上边框，我们必须去掉这部分额外的尺寸
	// 同时在这里去掉菜单窗口的WS_BORDER风格和WS_EX_DLGMODALFRAME , WS_EX_WINDOWEDGE扩展风格
	case WM_WINDOWPOSCHANGING:
		{
			DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
			dwStyle &= ~WS_BORDER;
			::SetWindowLong(hWnd, GWL_STYLE, dwStyle);

			DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
			dwExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE);
			::SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);

			//将菜单大小改小
			LPWINDOWPOS lpPos = (LPWINDOWPOS)lParam;
			lpPos->cx -= 2 * GetSystemMetrics(SM_CXBORDER);
			lpPos->cy -= 2 * GetSystemMetrics(SM_CYBORDER);
			lResult = CallWindowProc(m_oldMenuWndProc, hWnd, message, wParam, lParam);

			return lResult;
		}

	case WM_GETICON:

		return 0;

	default:

		return CallWindowProc(m_oldMenuWndProc, hWnd, message, wParam, lParam);
	}
}

void ExtMenu::InstallMenuTips(wxWindow* win)
{
	HWND hwnd((HWND)win->GetHWND());
	WNDPROC lastWndProc = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);

	if( lastWndProc != ExtMainWndProc )
		SetWindowLong(hwnd, GWL_WNDPROC, (long)ExtMainWndProc);
	else
		return;

	m_oldMainWndProc = lastWndProc;
}

LRESULT CALLBACK ExtMenu::ExtMainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if( message == WM_MENUSELECT )
	{
		OnMenuSelect(LOWORD(wParam), HIWORD(wParam), (HMENU)lParam);
	}
	/*else if( msg == WM_ENTERIDLE )
	{
		OnEnterIdle(wp, (HWND)lp);
	}*/

	return CallWindowProc(m_oldMainWndProc, hWnd, message, wParam, lParam);
}

void ExtMenu::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hMenu)
{
	wxToolTip tip(L"Menu tips!");
}
