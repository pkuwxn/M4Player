#pragma once
#if defined(__WXMSW__) && !defined(NOT_USE_EXTMENU)

#include "ExtMenuItem.h"
#include <wx/vector.h>

_WX_DECLARE_VECTOR(MenuHwndInfo, VectorOfMenuHwndInfo, );

class ExtMenu : public wxMenu
{
public:

	ExtMenu() : wxMenu()
	{
		if( !m_refCount )
			InitExtMenu();
	}

	ExtMenuItem* Append(int itemid,
						const wxString& text = wxEmptyString,
						const wxString& help = wxEmptyString,
						wxItemKind kind = wxITEM_NORMAL)
	{
		return new ExtMenuItem(this, itemid, text, help, kind);
	}

	wxMenuItem* AppendSeparator()
	{
		return new ExtMenuItem(this, wxID_SEPARATOR);
	}

	/// \brief 得到当前正在活动的 ExtMenu
	static HWND GetActiveExtMenu();

	/// \brief 启用菜单Tooltip
	static void InstallMenuTips(wxWindow* win);

private:

	/// \brief 在这里使菜单平面化（去除边框）
	static void InitExtMenu();

	static HRESULT CALLBACK TryHookMenuWnd(int code, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK ExtMenuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK ExtMainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hMenu);
	static void OnEnterIdle(UINT nWhy, HWND hwndWho);

private:

	static size_t					m_refCount;				/// \brief 一共挂载了多少次钩子
	static VectorOfMenuHwndInfo		m_menuHandles;			/// \brief 所有Menu的Handle

	static WNDPROC					m_oldMenuWndProc;		/// \brief 菜单原来的窗口函数
	static WNDPROC					m_oldMainWndProc;		/// \brief 要显示菜单提示的窗口的原窗口函数
};

#else

#define ExtMenu		wxMenu

#endif // defined(__WXMSW__) && !defined(NOT_USE_EXTMENU)
