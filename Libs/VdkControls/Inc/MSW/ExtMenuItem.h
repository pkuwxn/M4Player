#pragma once

#ifdef __WXMSW__

class ExtMenu;

struct MenuHwndInfo
{
	ExtMenu*		menu;
	HWND		hWnd;
	WNDPROC		oldProc;

	bool		bInit;
};

class ExtMenuItem : public wxMenuItem
{
public:

    ExtMenuItem(ExtMenu* parentMenu = (ExtMenu*)NULL,
                int id = wxID_SEPARATOR,
                const wxString& name = wxEmptyString,
                const wxString& help = wxEmptyString,
                wxItemKind kind = wxITEM_NORMAL,
                ExtMenu* subMenu = (ExtMenu*)NULL);

	virtual bool OnMeasureItem(size_t* pwidth, size_t* pheight);
	virtual bool OnDrawItem(wxDC& dc, const wxRect& rc, wxODAction act, wxODStatus stat);

private:

	void DrawColorRect(wxDC& dc, wxRect rt, wxODStatus st);

	//////////////////////////////////////////////////////////////////////////

	static wxPen					m_borderPen;
	static wxBrush					m_checkedBrush;
	static wxBrush					m_selectedBrush;
	static wxBrush					m_selectedBrushChecked;

	static bool						m_bLeftBitmapInited;	// 左端渐变色条是否已经初始化
	static wxBitmap					m_leftBar;				// 左端渐变色条
	static wxBitmap					m_leftBarForSep;		// 左端渐变色条
};

#else

#define ExtMenuItem		wxMenuItem

#endif // __WXMSW__