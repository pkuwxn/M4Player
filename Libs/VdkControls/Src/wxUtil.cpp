/***************************************************************
 * Name:      wxUtil.cpp
 * Purpose:   Code for some useful functions
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2010-04-05
 * Copyright: Wang Xiaoning
***************************************************************/
#include "StdAfx.h"
#include "wxUtil.h"

// Platform-specific includes
#ifdef __WXGTK__
#    include <gtk/gtk.h> // for GtkIMContex
#    include <gdk/gdkx.h> // for XSync
#endif

#include "wxCharsetHelper.h" // for GetGbkConv()
#include "VdkDefs.h" // for event names

#include <wx/file.h>
#include <wx/wfstream.h> // wxFile
#include <wx/mstream.h> // for wxMemoryInputStream
#include <wx/clipbrd.h> // wxTheClipboard
#include <wx/rawbmp.h> // TransparentBmp
#include <wx/xml/xml.h>

#include <time.h>
#include <stdio.h> // for sprintf

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

#ifdef _MSC_VER
#   pragma comment( lib, "imm32.lib" )
#endif

//////////////////////////////////////////////////////////////////////////

int CompareInts(int n1, int n2) {
    return n1 - n2;
}

bool CopyText(const wxString &strText) {
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(strText));
        wxTheClipboard->Flush();
        wxTheClipboard->Close();

        return true;
    }

    return false;
}

bool IsTextOnClipborad() {
    bool ret(false);
    if (wxTheClipboard->Open()) {
        ret = wxTheClipboard->IsSupported(wxDF_TEXT);
        wxTheClipboard->Close();
    }

    return ret;
}

void MoveImeWindow(wxWindow *win, int x, int y) {
#ifdef __WXMSW__
    HWND hwnd = GetHwndOf(win);
    HIMC imm = ImmGetContext(hwnd);
    if (imm) {
        COMPOSITIONFORM compinfo;
        compinfo.dwStyle = CFS_POINT;
        compinfo.ptCurrentPos.x = x;
        compinfo.ptCurrentPos.y = y;

        // 我也不知道为什么要多加 90，反正输入法状态条就是会向偏西北各 90px
        compinfo.ptCurrentPos.x += 90;
        compinfo.ptCurrentPos.y += 90;

        ImmSetCompositionWindow(imm, &compinfo);
        ImmReleaseContext(hwnd, imm);
    }
#elif defined( __WXGTK__ ) && defined( TODO )
    GtkIMContext *imCtx = *((GtkIMContext **) win->m_imData);
    GdkRectangle rc = { x, y, 1, 1 };
    gtk_im_context_set_cursor_location(imCtx, &rc);
#endif // __WXMSW__
}

bool IsImeWindowVisiable(wxWindow *win) {
#ifdef __WXMSW__
    HWND hwnd = GetHwndOf(win);
    HIMC imm = ImmGetContext(hwnd);
    if (imm) {
        bool empty = ImmGetCompositionString(imm, GCS_COMPSTR, 0, NULL) == 0;
        ImmReleaseContext(hwnd, imm);

        return !empty;
    }

    return false;
#elif defined( __WXGTK__ )
    return false;
#endif // __WXMSW__
}

void MoveWindowSynchronously(wxWindow *win, int x, int y) {
    win->Move(x, y);

#ifdef __WXGTK__
    GtkWidget *widget = win->GetHandle();
    gtk_widget_realize(widget);   // Mandatory. Otherwise, a segfault happens.

    Display *display = GDK_WINDOW_XDISPLAY(widget->window);
    XSync(display, /*Flase*/ 0);
#endif
}

void TransparentBmp(wxBitmap &bmp, const wxColour &bgColour) {
    wxASSERT(bmp.IsOk());

    wxBitmap bmpMask(DuplicateBitmap(bmp));
    wxAlphaPixelData data(bmpMask);

    if (!data) {
        return;
    }

    unsigned char r0(bgColour.Red()),
             g0(bgColour.Green()),
             b0(bgColour.Blue());
    int w = bmpMask.GetWidth(), h = bmpMask.GetHeight();
    wxAlphaPixelData::Iterator p(data);

    for (int hh = 0; hh < h; ++hh) {
        wxAlphaPixelData::Iterator row0 = p;

        for (int ww = 0; ww < w; ++ww, ++p) {
            unsigned char &r = p.Red();
            unsigned char &g = p.Green();
            unsigned char &b = p.Blue();

#           ifdef __WXMSW__

            if (r == r0 && g == g0 && b == b0) {
                r = g = b = 255;
            } else {
                r = g = b = 0;
            }

#           else

            if (r != r0 || g != g0 || b != b0) {
                p.Alpha() = 0;
            }

#           endif
        }

        p = row0;
        p.OffsetY(data, 1);
    }

#   ifdef __WXMSW__
    wxMask *mask;
    bmpMask.SetDepth(1);
    mask = new wxMask(bmpMask);
    bmp.SetMask(mask);
#   endif // __WXMSW__
}

wxBitmapType TestImageType(wxInputStream *is) {
    char data[10];
    is->Read(data, 10);
    is->SeekI(0);

    return TestImageType(data);
}

wxBitmapType TestImageType(char *data) {
    if (0 == memcmp(data, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8)) {
        return wxBITMAP_TYPE_PNG;
    } else if (0 == memcmp(&data[6], "JFIF\x00", 5)) {
        return wxBITMAP_TYPE_JPEG;
    } else if ((0 == memcmp(data, "GIF87a", 6)) ||
               (0 == memcmp(data, "GIF89a", 6))) {
        return wxBITMAP_TYPE_GIF;
    } else {
        return wxBITMAP_TYPE_ANY;
    }
}

void DrawHotKeyCaption(const wxString &strText, wxDC &dc, int x, int y) {
    wxString caption(strText);

    // 不要再画下划线了，跨平台支持不好，Windows 7 也不画了
    int nPos = caption.Find(L'&', true);   // fromEnd = true
    if (nPos != wxString::npos) {
        caption.erase(nPos, 1);
    }

    dc.DrawText(caption, x, y);
}

int XmlGetContentOfNum(wxXmlNode *xmlNode, int defaultValue) {
    if (!xmlNode) {
        return defaultValue;
    }

    return wxAtoi(xmlNode->GetNodeContent());
}

bool XmlGetContentOfBoolean(wxXmlNode *xmlNode, bool defv) {
    if (!xmlNode) {
        return defv;
    }

    wxString v(xmlNode->GetNodeContent());
    return (v == L'1') || (v.CmpNoCase(L"true") == 0);
}

wxXmlNode *FindChildNode(wxXmlNode *xmlNode, const wxString &strName) {
    wxXmlNode *chd = xmlNode->GetChildren();
    while (chd) {
        if (chd->GetName() == strName) {
            return chd;
        }

        chd = chd->GetNext();
    }

    return NULL;
}

wxXmlNode *FindSibling(wxXmlNode *node) {
    wxString name(node->GetName());
    wxXmlNode *sibling = node->GetNext();
    while (sibling) {
        if (sibling->GetName() == name) {
            return sibling;
        }

        sibling = sibling->GetNext();
    }

    return NULL;
}

wxString GetRamDiskPath(const wchar_t *szFileName) {
#ifdef __WXGTK__
    int nLen = wcslen(szFileName) + 9;
    wchar_t *szResult = new wchar_t[ nLen + 1 ];
    swprintf(szResult, nLen+1, L"/dev/shm/%S", szFileName);
#elif defined(__WXMSW__)
    int nLen = wcslen(szFileName) + 4;
    wchar_t *szResult = new wchar_t[ nLen + 1 ];
    swprintf(szResult, L"Z:\\%s", szFileName);
#endif

    wxString strResult = szResult;
    wxDELETEA(szResult);

    return strResult;
}

void RandSave(const wxBitmap &bm, const wchar_t *szPostfix) {
    if (!bm.IsOk()) {
        return;
    }

    static unsigned seed = 1;

    wxString strFileName;
    if (szPostfix) {
        strFileName.Printf(GetRamDiskPath(L"%d_%s.png"), seed++, szPostfix);
    } else {
        strFileName.Printf(GetRamDiskPath(L"%d.png"), seed++);
    }

#if wxUSE_LIBPNG
    if (!wxImage::FindHandler(wxBITMAP_TYPE_PNG)) {
        wxImage::AddHandler(new wxPNGHandler);
    }

    bm.SaveFile(strFileName, wxBITMAP_TYPE_PNG);
#else
    bm.SaveFile(strFileName, wxBITMAP_TYPE_BMP);
#endif
}

void RandSaveDC(wxDC &dc, const wxRect &rc, const wchar_t *szPostfix) {
    wxBitmap bm(rc.width, rc.height);
    wxMemoryDC mdc(bm);
    mdc.Blit(0, 0, rc.width, rc.height, &dc, rc.x, rc.y);
    mdc.SelectObject(wxNullBitmap);

    RandSave(bm, szPostfix);
}

void XmlSetContent(wxXmlNode *node, const wxString &strContent) {
    wxASSERT(node);

    if (!node->GetChildren()) {
        new wxXmlNode(node, wxXML_TEXT_NODE, L"TEXT_NODE", strContent);
    } else {
        node->GetChildren()->SetContent(strContent);
    }
}

wxString XmlGetChildContent(wxXmlNode *node, const wxString &strChdName,
                            const wxString &defaultValue) {
    wxString strResult(defaultValue);
    wxXmlNode *chd(FindChildNode(node, strChdName));
    if (chd) {
        strResult = chd->GetNodeContent().Trim().Trim(false);
    }

    return strResult;
}

wxXmlNode *XmlInsertChild(wxXmlNode *parent, const wxString &strName,
                          const wxString &strContent) {
    wxXmlNode *chd = new wxXmlNode(wxXML_ELEMENT_NODE, strName);
    XmlSetContent(chd, strContent);
    parent->InsertChildAfter(chd, parent->GetChildren());

    return chd;
}

bool wxIsAutoStart(const wxString &strAppName) {
#if defined( __WXMSW__ ) && defined( USE_MSWREG )
    return RegIsValueExists(HKEY_CURRENT_USER,
                            L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                            strAppName.c_str());
#else
    return false;
#endif // defined( __WXMSW__ ) && defined( USE_MSWREG )
}

bool wxAutoStart(const wxString &strAppName, const wxString &strAppPath, bool bAdd) {
#if defined( __WXMSW__ ) && defined( USE_MSWREG )

    wxRegKey *pRegKey = new wxRegKey(L"HKEY_CURRENT_USER\\Software\\"
                                     L"Microsoft\\Windows\\CurrentVersion\\Run");

    // will return false if the key does not exist
    if (!pRegKey->Exists()) {
        delete pRegKey;
        return false;
    }

    if (bAdd) {
        if (strAppPath[0] != '"') {
            strAppPath = L"\"" + strAppPath + L"\"";
        }
        if (!pRegKey->SetValue(strAppName, strAppPath)) {
            delete pRegKey;
            return false;
        }
    } else {
        pRegKey->DeleteValue(strAppName);
    }

    delete pRegKey;
    return true;
#endif

    return false;
}

wxFont wxEasyCreatFont(const wxString &strFaceName, int nPointSize, bool bBold) {
    return *(wxTheFontList->FindOrCreateFont(
                 nPointSize,
                 wxFONTFAMILY_DEFAULT,
                 wxFONTSTYLE_NORMAL,
                 bBold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
                 false,
                 strFaceName));
}

int wxGetFontHeight(const wxFont &font, wxWindow *win) {
    int h;
    wxString s(L"王小宁ABCbg-123");
    if (win) {
        win->GetTextExtent(s, NULL, &h, NULL, NULL, &font);
    } else {
        wxMemoryDC mdc;
        mdc.SetFont(font);
        mdc.GetTextExtent(s, NULL, &h, NULL, NULL, &font);
    }

    return h;
}

unsigned char utf8BOM[3] = {
    0xEF, 0xBB, 0xBF
};

void WriteUtf8BOM(wxFile &f) {
    f.Write(utf8BOM, sizeof utf8BOM);
}

bool IsEncodedInUtf8(const char *header) {
    return memcmp(header, utf8BOM, sizeof utf8BOM) == 0;
}

wxString ReadAll(const wxString &strFileName, RA_FileEncoding type) {
    wxString strData;

    wxFile file(strFileName);
    if (file.IsOpened()) {
        // get the file size
        wxFileOffset nSize = file.Length();
        if (nSize != wxInvalidOffset) {
            // read the whole file into memory
            wxUint8 *data = new wxUint8[ nSize + 1 ];
            if (file.Read(data, static_cast<size_t>(nSize)) == nSize) {
                data[ nSize ] = 0;

                if (type == RA_ANSI) {
                    strData = wxString(data, *wxConvCurrent);
                } else {
                    wxASSERT(false);      // TODO
                }
            }

            delete [] data;
            file.Close();
        }
    }

    return strData;
}

int CompareUnicodeStrings(const wxString &s1, const wxString &s2) {
    GetGbkConv();

    wxCharBuffer ps1(conv.cWX2MB(s1));
    wxCharBuffer ps2(conv.cWX2MB(s2));

    // TODO: 返回什么值才是正确的？
    if (!ps1) {
        return -1;
    } else if (!ps2) {
        return 1;
    }

    return strcmp(ps1, ps2);
}

void WriteString(wxOutputStream &fout, const wxString &s) {
#if wxUSE_UNICODE_WCHAR
    int len = s.length();
    WriteVar(fout, len);
    fout.Write((const wchar_t *) s, sizeof(wchar_t) * len);
#elif wxUSE_UNICODE_UTF8
    wxCharBuffer buf((const char *) s);
    int len = buf.length();
    WriteVar(fout, len);
    fout.Write(buf, sizeof(char) * len);
#endif
}

wxString ReadString(wxInputStream &fin) {
    if (!fin.IsOk()) {
        throw serialize_bad_format();
    }

    int len;
    fin.Read(&len, sizeof(len));

    wchar_t *buf = new wchar_t[len + 1];
    fin.Read(buf, sizeof(wchar_t) * len);
    buf[len] = 0;

    wxString ret(buf);
    delete [] buf;

    return ret;
}

wxString ReadString(wxMemoryInputStream &min) {
    if (!min.IsOk()) {
        throw serialize_bad_format();
    }

    const char *start = (const char *)
                        min.GetInputStreamBuffer()->GetBufferStart();
    const char *p = start + min.TellI();

    int len = *((int *) p);
    p += sizeof(len);

#if wxUSE_UNICODE_WCHAR
    min.SeekI((p - start) + len * sizeof(wchar_t));

    const wchar_t *str = (const wchar_t *) p;
    return wxString(str, str + len);
#elif wxUSE_UNICODE_UTF8
    min.SeekI((p - start) + len * sizeof(char));
    return wxString::FromUTF8Unchecked(p, len);
#endif
}

//////////////////////////////////////////////////////////////////////////

wxString GetISODateTime(const wxDateTime &dt) {
    return dt.FormatISODate() + L"T" + dt.FormatISOTime();
}

void DrawRectangle(wxDC &dc, const wxRect &rc) {
    // 左↓
    dc.DrawLine(rc.x, rc.y, rc.x, rc.GetBottom());
    // 上→
    dc.DrawLine(rc.x, rc.y, rc.GetRight(), rc.y);
    // 右↓
    dc.DrawLine(rc.GetRight(), rc.y, rc.GetRight(), rc.GetBottom());
    // 下→
    dc.DrawLine(rc.x, rc.GetBottom(), rc.x + rc.width, rc.GetBottom());
}

void InitRand(int seed) {
    srand((seed > 0) ? seed : time(NULL));
}

int Rand(int max) {
    // 初始化随机数产生器
    static bool init = false;
    if (!init) {
        init = true;
        InitRand(-1);
    }

    return rand() % max;
}

int GetNumBitCount(int num) {
    static char scount[20];
    return sprintf(scount, "%d", num);
}

//////////////////////////////////////////////////////////////////////////

wxString RectToString(const wxRect &rc) {
    return wxString::Format(L"%d, %d, %d, %d",
                            rc.x, rc.y, rc.width, rc.height);
}

void Print(const wxRect &rc, const wxString &desc) {
    wxLogDebug(L"%s{%s}", desc, RectToString(rc));
}

//////////////////////////////////////////////////////////////////////////

wxString PointToString(const wxPoint &point) {
    return wxString::Format(L"%d, %d", point.x, point.y);
}

void Print(const wxPoint &point, const wxString &desc) {
    wxLogDebug(L"%s(%s)", desc, PointToString(point));
}

//////////////////////////////////////////////////////////////////////////

wxString DirectionToString(unsigned d) {
    wxString outp;

    if (d == vdkDIRECTION_INVALID) {
        outp.assign(L"None ");
    } else {
        if (d & vdkEAST) {
            outp += L"East ";
        }

        if (d & vdkWEST) {
            outp += L"West ";
        }

        if (d & vdkNORTH) {
            outp += L"North ";
        }

        if (d & vdkSOUTH) {
            outp += L"South ";
        }
    }

    outp.RemoveLast();
    return outp;
}

wxString AlignmentToString(unsigned align) {
    wxString outp;

    if (align == vdkALIGN_INVALID) {
        outp.assign(L"None ");
    } else {
        if (align & vdkALIGN_LEFT) {
            outp += L"Left ";
        }

        if (align & vdkALIGN_RIGHT) {
            outp += L"Right ";
        }

        if (align & vdkALIGN_TOP) {
            outp += L"Top ";
        }

        if (align & vdkALIGN_BOTTOM) {
            outp += L"Bottom ";
        }
    }

    outp.RemoveLast();
    return outp;
}

//////////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp> // This is a magic incantation which must be done!
WX_DEFINE_OBJARRAY(ArrayOfGtmTextLines);

/*static*/
void GradientTextMethod::Draw(wxDC &dc,
                              const wxFont &font,
                              const wxBrush &BgBrush,
                              const wxColour &TextColor,
                              const wxRect &rc,
                              int rowHeight,
                              wxDirection direction,
                              const ArrayOfGtmTextLines &lines) {
    wxBitmap maskBitmap(rc.width, rc.height); // 保存抠出来的渐变色文本

    // 绘制渐变色背景
    dc.GradientFillLinear(rc, TextColor, BgBrush.GetColour(), direction);

    // 绘制白色的文字（我们的目标是将文字抠出来）
    wxMemoryDC maskDC(maskBitmap);
    maskDC.SetFont(font);

    maskDC.SetBackground(*wxBLACK_BRUSH);
    maskDC.SetTextForeground(*wxWHITE);
    DrawText(maskDC, rowHeight, 0, lines);

    // AND 后，文字有了渐变色的纹理，此时 dc 除了渐变色文字外，背景是黑的
    dc.Blit(rc.x, rc.y, rc.width, rc.height, &maskDC, 0, 0, wxAND);

    // 注意只有文本时是渐变色的，文本的背景还是纯色的
    maskDC.SetBackground(BgBrush);
    maskDC.SetTextForeground(*wxBLACK);
    DrawText(maskDC, rowHeight, 0, lines);

    // 执行 OR 操作，补全除文字外的背景色
    // maskDC 文字部分是黑色的，OR 操作保证不会覆盖掉已生成的渐变色文本
    dc.Blit(rc.x, rc.y, rc.width, rc.height, &maskDC, 0, 0, wxOR);
}

/*static*/
void GradientTextMethod::DrawText(wxDC &dc,
                                  int rowHeight, int y, const ArrayOfGtmTextLines &lines) {
    dc.Clear();

    int padding = (rowHeight - dc.GetCharHeight()) / 2;
    int size = lines.size();
    for (int i = 0; i < size ; i++) {
        dc.DrawText(lines[i].s,
                    lines[i].x,
                    y + rowHeight * i + padding);
    }
}

wxString GetKeyCodeName(int keycode) {
    // 32 -> Space 127 -> DEL
    if (keycode > 31 && keycode < 127) {
        return wxString::Format(L"%c", keycode);
    } else if (keycode >= WXK_F1 && keycode <= WXK_F12) {
        return wxString::Format(L"F%d", keycode - WXK_F1 + 1);
    } else if (keycode >= WXK_NUMPAD0 && keycode <= WXK_NUMPAD9) {
        return wxString::Format(L"Num %d", keycode - WXK_NUMPAD0);
    }

    switch (keycode) {
    case WXK_SHIFT:
        return L"Shift";
    case WXK_ALT:
        return L"Alt";
    case WXK_CONTROL:
        return L"Ctrl";
    case WXK_WINDOWS_MENU:
        return L"Windows Menu";
    case WXK_TAB:
        return L"Tab";
    case WXK_ESCAPE:
        return L"Esc";
    case WXK_CAPITAL:
        return L"Caps Lock";
    case WXK_LEFT:
        return L"Left";
    case WXK_RIGHT:
        return L"Right";
    case WXK_UP:
        return L"Up";
    case WXK_DOWN:
        return L"Down";
    case WXK_DELETE:
        return L"Delete";
    case WXK_PAGEDOWN:
        return L"Page Down";
    case WXK_PAGEUP:
        return L"Page Up";
    case WXK_HOME:
        return L"Home";
    case WXK_END:
        return L"End";
    case WXK_INSERT:
        return L"Insert";
    case WXK_NUMLOCK:
        return L"Num Lock";
    case WXK_SCROLL:
        return L"Scroll Lock";
    case WXK_NUMPAD_MULTIPLY:
        return L"Num *";
    case WXK_NUMPAD_ADD:
        return L"Num +";
    case WXK_NUMPAD_SEPARATOR:
        return L"Num !";
    case WXK_NUMPAD_SUBTRACT:
        return L"Num -";
    case WXK_NUMPAD_DECIMAL:
        return L"Num ~";
    case WXK_NUMPAD_DIVIDE:
        return L"Num /";

    default:
        return L"";
    }
}

#ifdef __WXDEBUG__
#ifdef __WXMSW__

#include "VdkDefs.h"

const wxChar *wxGetMessageName(int message) {
    switch (message) {
    case 0x0000:
        return wxT("WM_NULL");
    case 0x0001:
        return wxT("WM_CREATE");
    case 0x0002:
        return wxT("WM_DESTROY");
    case 0x0003:
        return wxT("WM_MOVE");
    case 0x0005:
        return wxT("WM_SIZE");
    case 0x0006:
        return wxT("WM_ACTIVATE");
    case 0x0007:
        return wxT("WM_SETFOCUS");
    case 0x0008:
        return wxT("WM_KILLFOCUS");
    case 0x000A:
        return wxT("WM_ENABLE");
    case 0x000B:
        return wxT("WM_SETREDRAW");
    case 0x000C:
        return wxT("WM_SETTEXT");
    case 0x000D:
        return wxT("WM_GETTEXT");
    case 0x000E:
        return wxT("WM_GETTEXTLENGTH");
    case 0x000F:
        return wxT("WM_PAINT");
    case 0x0010:
        return wxT("WM_CLOSE");
    case 0x0011:
        return wxT("WM_QUERYENDSESSION");
    case 0x0012:
        return wxT("WM_QUIT");
    case 0x0013:
        return wxT("WM_QUERYOPEN");
    case 0x0014:
        return wxT("WM_ERASEBKGND");
    case 0x0015:
        return wxT("WM_SYSCOLORCHANGE");
    case 0x0016:
        return wxT("WM_ENDSESSION");
    case 0x0017:
        return wxT("WM_SYSTEMERROR");
    case 0x0018:
        return wxT("WM_SHOWWINDOW");
    case 0x0019:
        return wxT("WM_CTLCOLOR");
    case 0x001A:
        return wxT("WM_WININICHANGE");
    case 0x001B:
        return wxT("WM_DEVMODECHANGE");
    case 0x001C:
        return wxT("WM_ACTIVATEAPP");
    case 0x001D:
        return wxT("WM_FONTCHANGE");
    case 0x001E:
        return wxT("WM_TIMECHANGE");
    case 0x001F:
        return wxT("WM_CANCELMODE");
    case 0x0020:
        return wxT("WM_SETCURSOR");
    case 0x0021:
        return wxT("WM_MOUSEACTIVATE");
    case 0x0022:
        return wxT("WM_CHILDACTIVATE");
    case 0x0023:
        return wxT("WM_QUEUESYNC");
    case 0x0024:
        return wxT("WM_GETMINMAXINFO");
    case 0x0026:
        return wxT("WM_PAINTICON");
    case 0x0027:
        return wxT("WM_ICONERASEBKGND");
    case 0x0028:
        return wxT("WM_NEXTDLGCTL");
    case 0x002A:
        return wxT("WM_SPOOLERSTATUS");
    case 0x002B:
        return wxT("WM_DRAWITEM");
    case 0x002C:
        return wxT("WM_MEASUREITEM");
    case 0x002D:
        return wxT("WM_DELETEITEM");
    case 0x002E:
        return wxT("WM_VKEYTOITEM");
    case 0x002F:
        return wxT("WM_CHARTOITEM");
    case 0x0030:
        return wxT("WM_SETFONT");
    case 0x0031:
        return wxT("WM_GETFONT");
    case 0x0037:
        return wxT("WM_QUERYDRAGICON");
    case 0x0039:
        return wxT("WM_COMPAREITEM");
    case 0x0041:
        return wxT("WM_COMPACTING");
    case 0x0044:
        return wxT("WM_COMMNOTIFY");
    case 0x0046:
        return wxT("WM_WINDOWPOSCHANGING");
    case 0x0047:
        return wxT("WM_WINDOWPOSCHANGED");
    case 0x0048:
        return wxT("WM_POWER");

    case 0x004A:
        return wxT("WM_COPYDATA");
    case 0x004B:
        return wxT("WM_CANCELJOURNAL");
    case 0x004E:
        return wxT("WM_NOTIFY");
    case 0x0050:
        return wxT("WM_INPUTLANGCHANGEREQUEST");
    case 0x0051:
        return wxT("WM_INPUTLANGCHANGE");
    case 0x0052:
        return wxT("WM_TCARD");
    case 0x0053:
        return wxT("WM_HELP");
    case 0x0054:
        return wxT("WM_USERCHANGED");
    case 0x0055:
        return wxT("WM_NOTIFYFORMAT");
    case 0x007B:
        return wxT("WM_CONTEXTMENU");
    case 0x007C:
        return wxT("WM_STYLECHANGING");
    case 0x007D:
        return wxT("WM_STYLECHANGED");
    case 0x007E:
        return wxT("WM_DISPLAYCHANGE");
    case 0x007F:
        return wxT("WM_GETICON");
    case 0x0080:
        return wxT("WM_SETICON");

    case 0x0081:
        return wxT("WM_NCCREATE");
    case 0x0082:
        return wxT("WM_NCDESTROY");
    case 0x0083:
        return wxT("WM_NCCALCSIZE");
    case 0x0084:
        return wxT("WM_NCHITTEST");
    case 0x0085:
        return wxT("WM_NCPAINT");
    case 0x0086:
        return wxT("WM_NCACTIVATE");
    case 0x0087:
        return wxT("WM_GETDLGCODE");
    case 0x00A0:
        return wxT("WM_NCMOUSEMOVE");
    case 0x00A1:
        return wxT("WM_NCLBUTTONDOWN");
    case 0x00A2:
        return wxT("WM_NCLBUTTONUP");
    case 0x00A3:
        return wxT("WM_NCLBUTTONDBLCLK");
    case 0x00A4:
        return wxT("WM_NCRBUTTONDOWN");
    case 0x00A5:
        return wxT("WM_NCRBUTTONUP");
    case 0x00A6:
        return wxT("WM_NCRBUTTONDBLCLK");
    case 0x00A7:
        return wxT("WM_NCMBUTTONDOWN");
    case 0x00A8:
        return wxT("WM_NCMBUTTONUP");
    case 0x00A9:
        return wxT("WM_NCMBUTTONDBLCLK");
    case 0x0100:
        return wxT("WM_KEYDOWN");
    case 0x0101:
        return wxT("WM_KEYUP");
    case 0x0102:
        return wxT("WM_CHAR");
    case 0x0103:
        return wxT("WM_DEADCHAR");
    case 0x0104:
        return wxT("WM_SYSKEYDOWN");
    case 0x0105:
        return wxT("WM_SYSKEYUP");
    case 0x0106:
        return wxT("WM_SYSCHAR");
    case 0x0107:
        return wxT("WM_SYSDEADCHAR");
    case 0x0108:
        return wxT("WM_KEYLAST");

    case 0x010D:
        return wxT("WM_IME_STARTCOMPOSITION");
    case 0x010E:
        return wxT("WM_IME_ENDCOMPOSITION");
    case 0x010F:
        return wxT("WM_IME_COMPOSITION");

    case 0x0110:
        return wxT("WM_INITDIALOG");
    case 0x0111:
        return wxT("WM_COMMAND");
    case 0x0112:
        return wxT("WM_SYSCOMMAND");
    case 0x0113:
        return wxT("WM_TIMER");
    case 0x0114:
        return wxT("WM_HSCROLL");
    case 0x0115:
        return wxT("WM_VSCROLL");
    case 0x0116:
        return wxT("WM_INITMENU");
    case 0x0117:
        return wxT("WM_INITMENUPOPUP");
    case 0x011F:
        return wxT("WM_MENUSELECT");
    case 0x0120:
        return wxT("WM_MENUCHAR");
    case 0x0121:
        return wxT("WM_ENTERIDLE");

    case 0x0127:
        return wxT("WM_CHANGEUISTATE");
    case 0x0128:
        return wxT("WM_UPDATEUISTATE");
    case 0x0129:
        return wxT("WM_QUERYUISTATE");

    case 0x0132:
        return wxT("WM_CTLCOLORMSGBOX");
    case 0x0133:
        return wxT("WM_CTLCOLOREDIT");
    case 0x0134:
        return wxT("WM_CTLCOLORLISTBOX");
    case 0x0135:
        return wxT("WM_CTLCOLORBTN");
    case 0x0136:
        return wxT("WM_CTLCOLORDLG");
    case 0x0137:
        return wxT("WM_CTLCOLORSCROLLBAR");
    case 0x0138:
        return wxT("WM_CTLCOLORSTATIC");

    case 0x01E1:
        return wxT("MN_GETHMENU");

    case 0x0200:
        return wxT("WM_MOUSEMOVE");
    case 0x0201:
        return wxT("WM_LBUTTONDOWN");
    case 0x0202:
        return wxT("WM_LBUTTONUP");
    case 0x0203:
        return wxT("WM_LBUTTONDBLCLK");
    case 0x0204:
        return wxT("WM_RBUTTONDOWN");
    case 0x0205:
        return wxT("WM_RBUTTONUP");
    case 0x0206:
        return wxT("WM_RBUTTONDBLCLK");
    case 0x0207:
        return wxT("WM_MBUTTONDOWN");
    case 0x0208:
        return wxT("WM_MBUTTONUP");
    case 0x0209:
        return wxT("WM_MBUTTONDBLCLK");
    case 0x020A:
        return wxT("WM_MOUSEWHEEL");
    case 0x0210:
        return wxT("WM_PARENTNOTIFY");
    case 0x0211:
        return wxT("WM_ENTERMENULOOP");
    case 0x0212:
        return wxT("WM_EXITMENULOOP");

    case 0x0213:
        return wxT("WM_NEXTMENU");
    case 0x0214:
        return wxT("WM_SIZING");
    case 0x0215:
        return wxT("WM_CAPTURECHANGED");
    case 0x0216:
        return wxT("WM_MOVING");
    case 0x0218:
        return wxT("WM_POWERBROADCAST");
    case 0x0219:
        return wxT("WM_DEVICECHANGE");

    case 0x0220:
        return wxT("WM_MDICREATE");
    case 0x0221:
        return wxT("WM_MDIDESTROY");
    case 0x0222:
        return wxT("WM_MDIACTIVATE");
    case 0x0223:
        return wxT("WM_MDIRESTORE");
    case 0x0224:
        return wxT("WM_MDINEXT");
    case 0x0225:
        return wxT("WM_MDIMAXIMIZE");
    case 0x0226:
        return wxT("WM_MDITILE");
    case 0x0227:
        return wxT("WM_MDICASCADE");
    case 0x0228:
        return wxT("WM_MDIICONARRANGE");
    case 0x0229:
        return wxT("WM_MDIGETACTIVE");
    case 0x0230:
        return wxT("WM_MDISETMENU");
    case 0x0233:
        return wxT("WM_DROPFILES");

    case 0x0281:
        return wxT("WM_IME_SETCONTEXT");
    case 0x0282:
        return wxT("WM_IME_NOTIFY");
    case 0x0283:
        return wxT("WM_IME_CONTROL");
    case 0x0284:
        return wxT("WM_IME_COMPOSITIONFULL");
    case 0x0285:
        return wxT("WM_IME_SELECT");
    case 0x0286:
        return wxT("WM_IME_CHAR");
    case 0x0290:
        return wxT("WM_IME_KEYDOWN");
    case 0x0291:
        return wxT("WM_IME_KEYUP");

    case 0x0300:
        return wxT("WM_CUT");
    case 0x0301:
        return wxT("WM_COPY");
    case 0x0302:
        return wxT("WM_PASTE");
    case 0x0303:
        return wxT("WM_CLEAR");
    case 0x0304:
        return wxT("WM_UNDO");
    case 0x0305:
        return wxT("WM_RENDERFORMAT");
    case 0x0306:
        return wxT("WM_RENDERALLFORMATS");
    case 0x0307:
        return wxT("WM_DESTROYCLIPBOARD");
    case 0x0308:
        return wxT("WM_DRAWCLIPBOARD");
    case 0x0309:
        return wxT("WM_PAINTCLIPBOARD");
    case 0x030A:
        return wxT("WM_VSCROLLCLIPBOARD");
    case 0x030B:
        return wxT("WM_SIZECLIPBOARD");
    case 0x030C:
        return wxT("WM_ASKCBFORMATNAME");
    case 0x030D:
        return wxT("WM_CHANGECBCHAIN");
    case 0x030E:
        return wxT("WM_HSCROLLCLIPBOARD");
    case 0x030F:
        return wxT("WM_QUERYNEWPALETTE");
    case 0x0310:
        return wxT("WM_PALETTEISCHANGING");
    case 0x0311:
        return wxT("WM_PALETTECHANGED");
#if wxUSE_HOTKEY
    case 0x0312:
        return wxT("WM_HOTKEY");
#endif

    // common controls messages - although they're not strictly speaking
    // standard, it's nice to decode them nevertheless

    // listview
    case 0x1000 + 0:
        return wxT("LVM_GETBKCOLOR");
    case 0x1000 + 1:
        return wxT("LVM_SETBKCOLOR");
    case 0x1000 + 2:
        return wxT("LVM_GETIMAGELIST");
    case 0x1000 + 3:
        return wxT("LVM_SETIMAGELIST");
    case 0x1000 + 4:
        return wxT("LVM_GETITEMCOUNT");
    case 0x1000 + 5:
        return wxT("LVM_GETITEMA");
    case 0x1000 + 75:
        return wxT("LVM_GETITEMW");
    case 0x1000 + 6:
        return wxT("LVM_SETITEMA");
    case 0x1000 + 76:
        return wxT("LVM_SETITEMW");
    case 0x1000 + 7:
        return wxT("LVM_INSERTITEMA");
    case 0x1000 + 77:
        return wxT("LVM_INSERTITEMW");
    case 0x1000 + 8:
        return wxT("LVM_DELETEITEM");
    case 0x1000 + 9:
        return wxT("LVM_DELETEALLITEMS");
    case 0x1000 + 10:
        return wxT("LVM_GETCALLBACKMASK");
    case 0x1000 + 11:
        return wxT("LVM_SETCALLBACKMASK");
    case 0x1000 + 12:
        return wxT("LVM_GETNEXTITEM");
    case 0x1000 + 13:
        return wxT("LVM_FINDITEMA");
    case 0x1000 + 83:
        return wxT("LVM_FINDITEMW");
    case 0x1000 + 14:
        return wxT("LVM_GETITEMRECT");
    case 0x1000 + 15:
        return wxT("LVM_SETITEMPOSITION");
    case 0x1000 + 16:
        return wxT("LVM_GETITEMPOSITION");
    case 0x1000 + 17:
        return wxT("LVM_GETSTRINGWIDTHA");
    case 0x1000 + 87:
        return wxT("LVM_GETSTRINGWIDTHW");
    case 0x1000 + 18:
        return wxT("LVM_HITTEST");
    case 0x1000 + 19:
        return wxT("LVM_ENSUREVISIBLE");
    case 0x1000 + 20:
        return wxT("LVM_SCROLL");
    case 0x1000 + 21:
        return wxT("LVM_REDRAWITEMS");
    case 0x1000 + 22:
        return wxT("LVM_ARRANGE");
    case 0x1000 + 23:
        return wxT("LVM_EDITLABELA");
    case 0x1000 + 118:
        return wxT("LVM_EDITLABELW");
    case 0x1000 + 24:
        return wxT("LVM_GETEDITCONTROL");
    case 0x1000 + 25:
        return wxT("LVM_GETCOLUMNA");
    case 0x1000 + 95:
        return wxT("LVM_GETCOLUMNW");
    case 0x1000 + 26:
        return wxT("LVM_SETCOLUMNA");
    case 0x1000 + 96:
        return wxT("LVM_SETCOLUMNW");
    case 0x1000 + 27:
        return wxT("LVM_INSERTCOLUMNA");
    case 0x1000 + 97:
        return wxT("LVM_INSERTCOLUMNW");
    case 0x1000 + 28:
        return wxT("LVM_DELETECOLUMN");
    case 0x1000 + 29:
        return wxT("LVM_GETCOLUMNWIDTH");
    case 0x1000 + 30:
        return wxT("LVM_SETCOLUMNWIDTH");
    case 0x1000 + 31:
        return wxT("LVM_GETHEADER");
    case 0x1000 + 33:
        return wxT("LVM_CREATEDRAGIMAGE");
    case 0x1000 + 34:
        return wxT("LVM_GETVIEWRECT");
    case 0x1000 + 35:
        return wxT("LVM_GETTEXTCOLOR");
    case 0x1000 + 36:
        return wxT("LVM_SETTEXTCOLOR");
    case 0x1000 + 37:
        return wxT("LVM_GETTEXTBKCOLOR");
    case 0x1000 + 38:
        return wxT("LVM_SETTEXTBKCOLOR");
    case 0x1000 + 39:
        return wxT("LVM_GETTOPINDEX");
    case 0x1000 + 40:
        return wxT("LVM_GETCOUNTPERPAGE");
    case 0x1000 + 41:
        return wxT("LVM_GETORIGIN");
    case 0x1000 + 42:
        return wxT("LVM_UPDATE");
    case 0x1000 + 43:
        return wxT("LVM_SETITEMSTATE");
    case 0x1000 + 44:
        return wxT("LVM_GETITEMSTATE");
    case 0x1000 + 45:
        return wxT("LVM_GETITEMTEXTA");
    case 0x1000 + 115:
        return wxT("LVM_GETITEMTEXTW");
    case 0x1000 + 46:
        return wxT("LVM_SETITEMTEXTA");
    case 0x1000 + 116:
        return wxT("LVM_SETITEMTEXTW");
    case 0x1000 + 47:
        return wxT("LVM_SETITEMCOUNT");
    case 0x1000 + 48:
        return wxT("LVM_SORTITEMS");
    case 0x1000 + 49:
        return wxT("LVM_SETITEMPOSITION32");
    case 0x1000 + 50:
        return wxT("LVM_GETSELECTEDCOUNT");
    case 0x1000 + 51:
        return wxT("LVM_GETITEMSPACING");
    case 0x1000 + 52:
        return wxT("LVM_GETISEARCHSTRINGA");
    case 0x1000 + 117:
        return wxT("LVM_GETISEARCHSTRINGW");
    case 0x1000 + 53:
        return wxT("LVM_SETICONSPACING");
    case 0x1000 + 54:
        return wxT("LVM_SETEXTENDEDLISTVIEWSTYLE");
    case 0x1000 + 55:
        return wxT("LVM_GETEXTENDEDLISTVIEWSTYLE");
    case 0x1000 + 56:
        return wxT("LVM_GETSUBITEMRECT");
    case 0x1000 + 57:
        return wxT("LVM_SUBITEMHITTEST");
    case 0x1000 + 58:
        return wxT("LVM_SETCOLUMNORDERARRAY");
    case 0x1000 + 59:
        return wxT("LVM_GETCOLUMNORDERARRAY");
    case 0x1000 + 60:
        return wxT("LVM_SETHOTITEM");
    case 0x1000 + 61:
        return wxT("LVM_GETHOTITEM");
    case 0x1000 + 62:
        return wxT("LVM_SETHOTCURSOR");
    case 0x1000 + 63:
        return wxT("LVM_GETHOTCURSOR");
    case 0x1000 + 64:
        return wxT("LVM_APPROXIMATEVIEWRECT");
    case 0x1000 + 65:
        return wxT("LVM_SETWORKAREA");

    // tree view
    case 0x1100 + 0:
        return wxT("TVM_INSERTITEMA");
    case 0x1100 + 50:
        return wxT("TVM_INSERTITEMW");
    case 0x1100 + 1:
        return wxT("TVM_DELETEITEM");
    case 0x1100 + 2:
        return wxT("TVM_EXPAND");
    case 0x1100 + 4:
        return wxT("TVM_GETITEMRECT");
    case 0x1100 + 5:
        return wxT("TVM_GETCOUNT");
    case 0x1100 + 6:
        return wxT("TVM_GETINDENT");
    case 0x1100 + 7:
        return wxT("TVM_SETINDENT");
    case 0x1100 + 8:
        return wxT("TVM_GETIMAGELIST");
    case 0x1100 + 9:
        return wxT("TVM_SETIMAGELIST");
    case 0x1100 + 10:
        return wxT("TVM_GETNEXTITEM");
    case 0x1100 + 11:
        return wxT("TVM_SELECTITEM");
    case 0x1100 + 12:
        return wxT("TVM_GETITEMA");
    case 0x1100 + 62:
        return wxT("TVM_GETITEMW");
    case 0x1100 + 13:
        return wxT("TVM_SETITEMA");
    case 0x1100 + 63:
        return wxT("TVM_SETITEMW");
    case 0x1100 + 14:
        return wxT("TVM_EDITLABELA");
    case 0x1100 + 65:
        return wxT("TVM_EDITLABELW");
    case 0x1100 + 15:
        return wxT("TVM_GETEDITCONTROL");
    case 0x1100 + 16:
        return wxT("TVM_GETVISIBLECOUNT");
    case 0x1100 + 17:
        return wxT("TVM_HITTEST");
    case 0x1100 + 18:
        return wxT("TVM_CREATEDRAGIMAGE");
    case 0x1100 + 19:
        return wxT("TVM_SORTCHILDREN");
    case 0x1100 + 20:
        return wxT("TVM_ENSUREVISIBLE");
    case 0x1100 + 21:
        return wxT("TVM_SORTCHILDRENCB");
    case 0x1100 + 22:
        return wxT("TVM_ENDEDITLABELNOW");
    case 0x1100 + 23:
        return wxT("TVM_GETISEARCHSTRINGA");
    case 0x1100 + 64:
        return wxT("TVM_GETISEARCHSTRINGW");
    case 0x1100 + 24:
        return wxT("TVM_SETTOOLTIPS");
    case 0x1100 + 25:
        return wxT("TVM_GETTOOLTIPS");

    // header
    case 0x1200 + 0:
        return wxT("HDM_GETITEMCOUNT");
    case 0x1200 + 1:
        return wxT("HDM_INSERTITEMA");
    case 0x1200 + 10:
        return wxT("HDM_INSERTITEMW");
    case 0x1200 + 2:
        return wxT("HDM_DELETEITEM");
    case 0x1200 + 3:
        return wxT("HDM_GETITEMA");
    case 0x1200 + 11:
        return wxT("HDM_GETITEMW");
    case 0x1200 + 4:
        return wxT("HDM_SETITEMA");
    case 0x1200 + 12:
        return wxT("HDM_SETITEMW");
    case 0x1200 + 5:
        return wxT("HDM_LAYOUT");
    case 0x1200 + 6:
        return wxT("HDM_HITTEST");
    case 0x1200 + 7:
        return wxT("HDM_GETITEMRECT");
    case 0x1200 + 8:
        return wxT("HDM_SETIMAGELIST");
    case 0x1200 + 9:
        return wxT("HDM_GETIMAGELIST");
    case 0x1200 + 15:
        return wxT("HDM_ORDERTOINDEX");
    case 0x1200 + 16:
        return wxT("HDM_CREATEDRAGIMAGE");
    case 0x1200 + 17:
        return wxT("HDM_GETORDERARRAY");
    case 0x1200 + 18:
        return wxT("HDM_SETORDERARRAY");
    case 0x1200 + 19:
        return wxT("HDM_SETHOTDIVIDER");

    // tab control
    case 0x1300 + 2:
        return wxT("TCM_GETIMAGELIST");
    case 0x1300 + 3:
        return wxT("TCM_SETIMAGELIST");
    case 0x1300 + 4:
        return wxT("TCM_GETITEMCOUNT");
    case 0x1300 + 5:
        return wxT("TCM_GETITEMA");
    case 0x1300 + 60:
        return wxT("TCM_GETITEMW");
    case 0x1300 + 6:
        return wxT("TCM_SETITEMA");
    case 0x1300 + 61:
        return wxT("TCM_SETITEMW");
    case 0x1300 + 7:
        return wxT("TCM_INSERTITEMA");
    case 0x1300 + 62:
        return wxT("TCM_INSERTITEMW");
    case 0x1300 + 8:
        return wxT("TCM_DELETEITEM");
    case 0x1300 + 9:
        return wxT("TCM_DELETEALLITEMS");
    case 0x1300 + 10:
        return wxT("TCM_GETITEMRECT");
    case 0x1300 + 11:
        return wxT("TCM_GETCURSEL");
    case 0x1300 + 12:
        return wxT("TCM_SETCURSEL");
    case 0x1300 + 13:
        return wxT("TCM_HITTEST");
    case 0x1300 + 14:
        return wxT("TCM_SETITEMEXTRA");
    case 0x1300 + 40:
        return wxT("TCM_ADJUSTRECT");
    case 0x1300 + 41:
        return wxT("TCM_SETITEMSIZE");
    case 0x1300 + 42:
        return wxT("TCM_REMOVEIMAGE");
    case 0x1300 + 43:
        return wxT("TCM_SETPADDING");
    case 0x1300 + 44:
        return wxT("TCM_GETROWCOUNT");
    case 0x1300 + 45:
        return wxT("TCM_GETTOOLTIPS");
    case 0x1300 + 46:
        return wxT("TCM_SETTOOLTIPS");
    case 0x1300 + 47:
        return wxT("TCM_GETCURFOCUS");
    case 0x1300 + 48:
        return wxT("TCM_SETCURFOCUS");
    case 0x1300 + 49:
        return wxT("TCM_SETMINTABWIDTH");
    case 0x1300 + 50:
        return wxT("TCM_DESELECTALL");

    // toolbar
    case WM_USER+1:
        return wxT("TB_ENABLEBUTTON");
    case WM_USER+2:
        return wxT("TB_CHECKBUTTON");
    case WM_USER+3:
        return wxT("TB_PRESSBUTTON");
    case WM_USER+4:
        return wxT("TB_HIDEBUTTON");
    case WM_USER+5:
        return wxT("TB_INDETERMINATE");
    case WM_USER+9:
        return wxT("TB_ISBUTTONENABLED");
    case WM_USER+10:
        return wxT("TB_ISBUTTONCHECKED");
    case WM_USER+11:
        return wxT("TB_ISBUTTONPRESSED");
    case WM_USER+12:
        return wxT("TB_ISBUTTONHIDDEN");
    case WM_USER+13:
        return wxT("TB_ISBUTTONINDETERMINATE");
    case WM_USER+17:
        return wxT("TB_SETSTATE");
    case WM_USER+18:
        return wxT("TB_GETSTATE");
    case WM_USER+19:
        return wxT("TB_ADDBITMAP");
    case WM_USER+20:
        return wxT("TB_ADDBUTTONS");
    case WM_USER+21:
        return wxT("TB_INSERTBUTTON");
    case WM_USER+22:
        return wxT("TB_DELETEBUTTON");
    case WM_USER+23:
        return wxT("TB_GETBUTTON");
    case WM_USER+24:
        return wxT("TB_BUTTONCOUNT");
    case WM_USER+25:
        return wxT("TB_COMMANDTOINDEX");
    case WM_USER+26:
        return wxT("TB_SAVERESTOREA");
    case WM_USER+76:
        return wxT("TB_SAVERESTOREW");
    case WM_USER+27:
        return wxT("TB_CUSTOMIZE");
    case WM_USER+28:
        return wxT("TB_ADDSTRINGA");
    case WM_USER+77:
        return wxT("TB_ADDSTRINGW");
    case WM_USER+29:
        return wxT("TB_GETITEMRECT");
    case WM_USER+30:
        return wxT("TB_BUTTONSTRUCTSIZE");
    case WM_USER+31:
        return wxT("TB_SETBUTTONSIZE");
    case WM_USER+32:
        return wxT("TB_SETBITMAPSIZE");
    case WM_USER+33:
        return wxT("TB_AUTOSIZE");
    case WM_USER+35:
        return wxT("TB_GETTOOLTIPS");
    case WM_USER+36:
        return wxT("TB_SETTOOLTIPS");
    case WM_USER+37:
        return wxT("TB_SETPARENT");
    case WM_USER+39:
        return wxT("TB_SETROWS");
    case WM_USER+40:
        return wxT("TB_GETROWS");
    case WM_USER+42:
        return wxT("TB_SETCMDID");
    case WM_USER+43:
        return wxT("TB_CHANGEBITMAP");
    case WM_USER+44:
        return wxT("TB_GETBITMAP");
    case WM_USER+45:
        return wxT("TB_GETBUTTONTEXTA");
    case WM_USER+75:
        return wxT("TB_GETBUTTONTEXTW");
    case WM_USER+46:
        return wxT("TB_REPLACEBITMAP");
    case WM_USER+47:
        return wxT("TB_SETINDENT");
    case WM_USER+48:
        return wxT("TB_SETIMAGELIST");
    case WM_USER+49:
        return wxT("TB_GETIMAGELIST");
    case WM_USER+50:
        return wxT("TB_LOADIMAGES");
    case WM_USER+51:
        return wxT("TB_GETRECT");
    case WM_USER+52:
        return wxT("TB_SETHOTIMAGELIST");
    case WM_USER+53:
        return wxT("TB_GETHOTIMAGELIST");
    case WM_USER+54:
        return wxT("TB_SETDISABLEDIMAGELIST");
    case WM_USER+55:
        return wxT("TB_GETDISABLEDIMAGELIST");
    case WM_USER+56:
        return wxT("TB_SETSTYLE");
    case WM_USER+57:
        return wxT("TB_GETSTYLE");
    case WM_USER+58:
        return wxT("TB_GETBUTTONSIZE");
    case WM_USER+59:
        return wxT("TB_SETBUTTONWIDTH");
    case WM_USER+60:
        return wxT("TB_SETMAXTEXTROWS");
    case WM_USER+61:
        return wxT("TB_GETTEXTROWS");
    case WM_USER+41:
        return wxT("TB_GETBITMAPFLAGS");

    default:
        static wxString s_szBuf;
        s_szBuf.Printf(wxT("<unknown message = %d>"), message);
        return s_szBuf.c_str();
    }
}

#endif // __WXMSW__

const wxChar *GetEventName(int evtCode) {
    switch (evtCode) {
    case NORMAL:
        return L"NORMAL";
    case HOVERING:
        return L"HOVERING";
    case LEFT_DOWN:
        return L"LEFT_DOWN";
    case LEFT_UP:
        return L"LEFT_UP";
    case DLEFT_DOWN:
        return L"DLEFT_DOWN";
    case RIGHT_UP:
        return L"RIGHT_UP";
    case DRAGGING:
        return L"DRAGGING";
    case WHEEL_UP:
        return L"WHEEL_UP";
    case WHEEL_DOWN:
        return L"WHEEL_DOWN";
    case SCROLLING_UP:
        return L"SCROLLING_UP";
    case SCROLLING_DOWN:
        return L"SCROLLING_DOWN";
    case MOUSE_LEAVE:
        return L"MOUSE_LEAVE";
    case MOUSE_LEAVE_WINDOW:
        return L"MOUSE_LEAVE_WINDOW";
    case MOUSE_HOLD_ON_RELEASED:
        return L"MOUSE_HOLD_ON_RELEASED";

    default:

        static wxString s_szBuf;
        s_szBuf.Printf(L"<unknown message = %d>", evtCode);

        return s_szBuf.c_str();
    }
}

const wxChar *GetNoticeName(int notice) {
    switch (notice) {
    case VCN_CREATE:
        return L"VCN_CREATE";
    case VCN_DESTROY:
        return L"VCN_DESTROY";
    case VCN_SIZE_CHANGED:
        return L"VCN_SIZE_CHANGED";
    case VCN_FONT_CHANGED:
        return L"VCN_FONT_CHANGED";
    case VCN_SHOW:
        return L"VCN_SHOW";
    case VCN_HIDE:
        return L"VCN_HIDE";
    case VCN_THAW:
        return L"VCN_THAW";
    case VCN_FREEZE:
        return L"VCN_FREEZE";
    case VCN_ENABLED:
        return L"VCN_ENABLED";
    case VCN_DISABLED:
        return L"VCN_DISABLED";
    case VCN_MENU_HID:
        return L"VCN_MENU_HID";

    default:

        static wxString s_szBuf;
        s_szBuf.Printf(L"<unknown notice = %d>", notice);

        return s_szBuf.c_str();
    }
}
#endif // __WXDEBUG__
