/***************************************************************
 * Name:      PlayListPanel.cpp
 * Purpose:   定义 PlayListPanel
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-2-27
 **************************************************************/
#include "StdAfx.h"
#include "PlayListPanel.h"

#include "OOPApp.h"
#include "OOPSong.h"
#include "OOPList.h"
#include "OOPFilePropDlg.h"
#include "OOPPlaylistSearchDlg.h"
#include "DelayModStack.h"
#include "VdkMenu.h"

#include <wx/dir.h>
#include <wx/filename.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

extern void AssignToolTip(VdkControl *pCtrl, const wxString &strToolTip);
extern OOPlayerApp *g_app;

PlayListPanel::PlayListPanel(wxWindow *parent, bool showAtOnce)
    : OOPWindow(parent, L"C5BC105E-ED0F-4578-9215-FA1EE71FC5A9",
                L"PlayListPanel",  0,
                VWS_DRAGGABLE |
                VWIS_DRAG_BY_SPACE |
                VWS_BASE_PANEL),
      m_playList(NULL),
      m_filePropDlg(NULL),
      m_songInfoUpdated(true),
      m_searchDlg(NULL),
      m_lastFound(wxNOT_FOUND),
      m_menuId(VdkGetUniqueId()) {
    SetSkin();

    //-----------------------------------------------------

    if (showAtOnce) {
        Show();
    }

    Bind(wxEVT_VOBJ, &PlayListPanel::OnFileProperties,
         this, OPLM_FILE_PROPERTIES);

    Bind(OOP_EVT_SCAN_DONE, &PlayListPanel::OnScanDone, this);
    Bind(OOP_EVT_SONG_INFO_UPDATED, &PlayListPanel::OnSongInfoUpdated, this);
    Bind(OOP_EVT_PLAYLIST_SEARCH, &PlayListPanel::OnSearch, this);
    Bind(wxEVT_CLOSE_WINDOW, &PlayListPanel::OnClose, this);

    Bind(wxEVT_VOBJ, &PlayListPanel::OnRename, this,
         OPLM_RENAME_LOWER_BOUND,
         OPLM_RENAME_UPPER_BOUND);

    // 菜单预显示事件
    Bind(wxEVT_VOBJ, &PlayListPanel::OnPreShowMenu, this, m_menuId);
}

void PlayListPanel::DoSetSkin() {
    FromXrc(L"playlist_window.xml");

    BindCloseBtnEvent();
    AssignToolTip(FindCtrl(L"close"), L"关闭");

    //-----------------------------------------------------

    m_playList = (OOPList *) FindCtrl(L"playlist");
    g_app->GeneratePlayList(m_playList);

    //-----------------------------------------------------

    GenerateMenu();
}

void PlayListPanel::OnClose(wxCloseEvent &e) {
    // 提交所有未成功的歌曲标签修改
    if (m_filePropDlg) {
        SingleDelayModStack::Instance().CommitAll();
    }

    e.Skip(true);
}

// TODO: 将其作为一个成员变量？
static wxString gsLastDir;

void PlayListPanel::AddFiles(wxWindow *parent) {
    wxFileDialog fdlg(parent,
                      L"添加音乐文件",
#ifdef __WXDEBUG__
#   ifdef __WXMSW__
                      L"D:\\Music",
#   else
                      L"/media/D/Music",
#   endif // __WXMSW__
#else
                      gsLastDir,
#endif // __WXDEBUG__
                      wxEmptyString,
                      g_app->GetSupportedFileTypes(),
                      wxFD_OPEN |
                      wxFD_FILE_MUST_EXIST |
                      wxFD_MULTIPLE);

    if (fdlg.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxArrayString *paths = new wxArrayString;
    fdlg.GetPaths(*paths);

    AddFiles(paths);
}

void PlayListPanel::AddFiles(wxArrayString *paths) {
    wxASSERT(paths && !paths->IsEmpty());

    // “导入歌曲”对话框
    OOPScanner scanner(this, paths);
}

void PlayListPanel::OnScanDone(wxCommandEvent &evt) {
    typedef wxVector<OOPSongPtr> SongVec;
    wxSharedPtr<SongVec> songs((SongVec *) evt.GetClientData());

    // 更新上次选择的目录
    wxFileName fn((*(songs->begin()))->path());
    gsLastDir = fn.GetPath();

    // 可能用户选择的都是一些不正确的文件(如大小为 0 等情况)
    if (songs->empty()) {
        return;
    }

    size_t first = m_playList->GetItemCount();
    size_t index = first;

    SongVec::iterator i(songs->begin());
    SongVec::iterator e(songs->end());
    for (; i != e; ++i) {
        m_playList->AddToList(**i, index++);
    }

    m_playList->UpdateSnColumnWidth(first, index);
    m_playList->UpdateSize(NULL);

    // 选中新增加的歌曲
    m_playList->Select(first, index, NULL);

    {
        VdkDC dc(this);
        m_playList->GoTo(double(first) / index, &dc);
    }

    g_app->EnablePlayer();
    g_app->Unbind(OOP_EVT_SCAN_DONE, &PlayListPanel::OnScanDone, this);
}

void PlayListPanel::AddFolder(RecursiveMode rm) {
    wxDirDialog fdlg(this,
                     L"请选择一个文件夹:",
#ifdef __WXDEBUG__
#   ifdef __WXMSW__
                     L"D:\\Music",
#   else
                     L"/media/D/Music",
#   endif
#else
                     gsLastDir,
#endif // __WXDEBUG__
                     wxDD_DEFAULT_STYLE |
                     // 没有“新建文件夹”按钮
                     wxDD_DIR_MUST_EXIST);

    if (fdlg.ShowModal() == wxID_CANCEL) {
        return;
    }

    //-----------------------------------------------------

    wxArrayString *files = new wxArrayString;
    wxString dirPath(fdlg.GetPath());
    if (!wxFileName::IsPathSeparator(dirPath.Last())) {
        dirPath += wxFileName::GetPathSeparator(wxPATH_NATIVE);
    }

    AddFolder(dirPath, rm, files);

    if (files->empty()) {
        delete files;
    } else {
        AddFiles(files);
    }
}

void PlayListPanel::AddFolder
(const wxString &dirPath, RecursiveMode rm, wxArrayString *files) {
    wxDir dir(dirPath);

    if (!dir.IsOpened()) {
        return;
    }

    wxString name;
    bool cont = false;

    //--------------------------------------
    // 遍历目录

    if (rm == RM_RECURSIVE) {
        cont = dir.GetFirst(&name, wxEmptyString, wxDIR_DIRS);
        while (cont) {
            wxString subDir(dirPath + name);
            subDir += wxFileName::GetPathSeparator(wxPATH_NATIVE);

            AddFolder(subDir, rm, files);
            cont = dir.GetNext(&name);
        }
    }

    //--------------------------------------
    // 遍历文件

    wxFileName fileName;

    cont = dir.GetFirst(&name, wxEmptyString, wxDIR_FILES);
    while (cont) {
        fileName.SetFullName(name);
        if (g_app->TestFileType(L'.' + fileName.GetExt())) {
            files->Add(dirPath + name);
        }

        cont = dir.GetNext(&name);
    }
}

bool PlayListPanel::TestPsuedoModal() {
#ifdef __WXDEBUG__
    if (!m_songInfoUpdated) {
        wxASSERT(m_filePropDlg);
    }
#endif

    // 实现“伪模态对话框”(GTK+ 下自绘菜单无法在模态对话框中实现)
    return !m_songInfoUpdated;
}

bool PlayListPanel::FilterEventBefore(wxMouseEvent &, int) {
    return TestPsuedoModal();
}

bool PlayListPanel::DoHandleKeyEvent(wxKeyEvent &e) {
    // 实现“伪模态对话框”(GTK+ 下自绘菜单无法在模态对话框中实现)
    if (TestPsuedoModal()) {
        return true;
    }

    // 只需要 wxEVT_KEY_DOWN
    if (e.GetEventType() != wxEVT_KEY_DOWN) {
        return false;
    }

    bool processed = g_app->OnPlayListKeys(e);
    if (!processed) {
        switch (e.GetKeyCode()) {
        case WXK_F3:

            if (m_searchDlg && !m_searchDlg->GetKeyword().empty()) {
                Search();
                return true;
            }

        // 显示搜索对话框

        case 'F':

            if (e.ControlDown()) {
                if (!m_searchDlg) {
                    m_searchDlg = new OOPPlaylistSearchDlg(this);

                    wxPoint myPos(GetScreenPosition());
                    m_searchDlg->Move(myPos.x + 50, myPos.y + 50);
                }

                m_searchDlg->Show(true);

                return true;
            }

            break;

        default:

            break;
        }
    }

    return false;
}

void PlayListPanel::OnFileProperties(VdkVObjEvent &) {
    if (!m_filePropDlg) {
        m_filePropDlg = new OOPFilePropDlg(this);
    }

    m_songInfoUpdated = false;

    OOPSongPtr song = GetCurrSel(NULL, NULL);
    m_filePropDlg->Popup(song);
}

void PlayListPanel::OnSongInfoUpdated(wxCommandEvent &e) {
    const ArrayOfSortedInts &selItems = m_playList->GetSelectedItems();
    // TODO: 现在都是只是处理多选项中的第一项
    wxASSERT(selItems.size() == 1);

    VdkLcCell *cell = m_playList->GetCell(selItems[0], 1);
    OOPListEntry *entry = (OOPListEntry *) cell->GetClientData();
    wxASSERT(entry->path() == e.GetString());

    OOPSong song(entry);
    song.ParseTag();
    cell->SetLabel(song.BuildTitle(g_app->GetPlayListTitleFormat()));

    // 歌曲元信息已更新事件已处理
    m_songInfoUpdated = true;

    VdkDC dc(this);
    m_playList->Draw(dc);

    // 将其继续导向应用程序类
    wxPostEvent(wxTheApp, e);
}

// 将路径中禁止包含的字符替换成@a dst
wxString ReplaceForbiddenChars(const wxString &path, wchar_t dst) {
    wxString ret(path);

    wxString forbidden(wxFileName::GetForbiddenChars(wxPATH_DOS));
    size_t numFbd = forbidden.size();

    for (size_t i = 0; i < numFbd; i++) {
        ret.Replace(forbidden[i], dst, true);
    }

    return ret;
}

void PlayListPanel::OnRename(VdkVObjEvent &e) {
    const ArrayOfSortedInts &selItems = m_playList->GetSelectedItems();
    wxASSERT(!selItems.empty());

    wxString nameFormat;
    switch (e.GetId()) {
    case OPLM_RENAME_SONGNAME_ONLY:

        nameFormat = L"%T";
        break;

    case OPLM_RENAME_SONGNAME_FIRST:

        nameFormat = L"%T - %A";
        break;

    default:
    case OPLM_RENAME_SINGER_FIRST:

        nameFormat = L"%A - %T";
        break;
    }

    int numSel = selItems.size();
    for (int i = 0; i < numSel; i++) {
        OOPSongPtr song = m_playList->GetSongPtr(selItems[i]);
        song->ParseTag(false);

        wxFileName fn(song->path());
        wxString oldName(fn.GetName());
        wxString newName(song->BuildTitle(nameFormat));
        // 去除非法字符
        newName = ReplaceForbiddenChars(newName, L'_');

        if (oldName != newName) {
            fn.SetName(newName);
            wxString newFullPath(fn.GetFullPath());

            if ((song->path() != newFullPath) && !wxFileExists(newFullPath)) {
                if (wxRename(song->path(), newFullPath) == 0) {
                    song->path(newFullPath);
                } else {
                    wxLogDebug(L"Error rename file `%s`.", song->path());

                    SingleDelayModStack::Instance().Add(song, newFullPath);
                }
            }
        }
    }
}

OOPSongPtr PlayListPanel::GetCurrSel(VdkLcCell **pCurr, int *pIndex) const {
    const ArrayOfSortedInts &sel = m_playList->GetSelectedItems();
    wxASSERT(!sel.IsEmpty());

    VdkLcCell *curr = m_playList->GetCell(sel[0], 1);
    OOPSongPtr song(new OOPSong((OOPListEntry *) curr->GetClientData()));

    if (pCurr) {
        *pCurr = curr;
    }
    if (pIndex) {
        *pIndex = sel[0];
    }

    return song;
}

void PlayListPanel::OnSearch(wxCommandEvent &e) {
    Search();
}

void PlayListPanel::Search() {
    wxASSERT(m_searchDlg);

    wxString keyword(m_searchDlg->GetKeyword());
    if (!keyword.empty()) {
        keyword.MakeUpper();

        if (m_lastKeyword != keyword) {
            m_lastFound = wxNOT_FOUND;
            m_lastKeyword = keyword;
        } else {
            // 从头找起
            if (m_lastFound >= m_playList->GetRowCount()) {
                m_lastFound = wxNOT_FOUND;
            }
        }

        int index = 0;
        LcCellIter iter(m_playList->begin()), end(m_playList->end());
        for (; iter != end; ++iter, ++index) {
            // 从上次的搜起
            if (index <= m_lastFound) {
                continue;
            }

            wxString label((*iter)->GetLabel());
            label.MakeUpper();

            if (label.Contains(keyword)) {
                m_playList->Select(index, NULL);

                // 将可视区域起始点移至搜索结果
                int ystart;
                m_playList->GetViewStart(NULL, &ystart);
                int shownItems = m_playList->GetRect().GetHeight() /
                                 m_playList->GetRowHeight();

                if (index < ystart || index >= (ystart + shownItems)) {
                    m_playList->SetViewStart(0, index, NULL);
                }

                VdkDC dc(this);
                m_playList->Draw(dc);

                m_lastFound = index;
                return;
            }
        } // END for

        m_lastFound = wxNOT_FOUND;
    }
}

void PlayListPanel::OnPreShowMenu(VdkVObjEvent &e) {
    VdkMenu *menu = e.GetMenu();
    VdkCtrlId disabledOnMultiSel[] = {
        OPLM_PLAY_SELECTED, OPLM_FILE_PROPERTIES, OPLM_LOCATE_FILE
    };

    bool isMultiSel = m_playList->GetSelectedItems().size() > 1;

    int count = sizeof(disabledOnMultiSel) / sizeof(disabledOnMultiSel[0]);
    for (int i = 0; i < count; i++) {
        int id = disabledOnMultiSel[i] - OPLM_LOWER_BOUND;
        menu->GetItem(id)->disabled(isMultiSel);
    }
}
