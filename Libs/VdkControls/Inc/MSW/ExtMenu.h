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

	/// \brief �õ���ǰ���ڻ�� ExtMenu
	static HWND GetActiveExtMenu();

	/// \brief ���ò˵�Tooltip
	static void InstallMenuTips(wxWindow* win);

private:

	/// \brief ������ʹ�˵�ƽ�滯��ȥ���߿�
	static void InitExtMenu();

	static HRESULT CALLBACK TryHookMenuWnd(int code, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK ExtMenuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK ExtMainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hMenu);
	static void OnEnterIdle(UINT nWhy, HWND hwndWho);

private:

	static size_t					m_refCount;				/// \brief һ�������˶��ٴι���
	static VectorOfMenuHwndInfo		m_menuHandles;			/// \brief ����Menu��Handle

	static WNDPROC					m_oldMenuWndProc;		/// \brief �˵�ԭ���Ĵ��ں���
	static WNDPROC					m_oldMainWndProc;		/// \brief Ҫ��ʾ�˵���ʾ�Ĵ��ڵ�ԭ���ں���
};

#else

#define ExtMenu		wxMenu

#endif // defined(__WXMSW__) && !defined(NOT_USE_EXTMENU)
