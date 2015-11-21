/***************************************************************
 * Name:      OOPMenus.cpp
 * Purpose:   放置与菜单有关的代码
 * Author:    Ning (vanxining@139.com)
 * Created:   2010
 * Copyright: Wang Xiao Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPApp.h"

#include "MainPanel.h"
#include "PlayListPanel.h"
#include "LyricPanel.h"

#include "OOPList.h"
#include "Lyric/OOPLyric.h"
#include "OOPToolBar.h"
#include "OOPFileSystem.h"
#include "OOPDefs.h"

#include "VdkMenu.h"
#include "OnMenuSlider.h"
#include "OOPSkinSelector.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

extern OOPlayerApp *g_app;

enum {

    MMBID_PLAY,
    MMBID_PAUSE,
    MMBID_STOP,
    MMBID_NEXT,
    MMBID_PREV,
    MMBID_UP,
    MMBID_DOWN,
    MMBID_EXIT,
    MMBID_OPTIONS,
    MMBID_LINKS,

    MMBID_COUNT,
};

void OOPlayerApp::GenerateMainMenu() {
    VdkMenu *menu = m_mainPanel->CreateMenu(NULL);
    menu->SetBestWidth(150);

    if (!m_mainMenuBitmaps.IsOk()) {
        wxString imgPath(OOPFileSystem::GetAppResDir() + L"mainMenu.png");
        m_mainMenuBitmaps.Set(imgPath, MMBID_COUNT, 1);
    }

    //===============================================

    menu->AppendItem(NewMenuItem()->id(OMM_PLAYER_OPTIONS).
                     caption(L"选项").
                     bmpArrayID(VdkBitmapArrayId(m_mainMenuBitmaps,
                                MMBID_OPTIONS, 0)));

    VdkMenu *menuLinks = m_mainPanel->CreateMenu(menu);

    menuLinks->AppendItem(NewMenuItem()->id(CID_ABOUT).
                          caption(L"关于"));
    menuLinks->AppendItem(NewMenuItem()->id(OMM_LINKS_WEBSITE).
                          caption(L"网站"));
    menuLinks->AppendItem(NewMenuItem()->id(OMM_LINKS_FORUM).
                          caption(L"论坛"));

    menu->AppendItem(NewMenuItem()->caption(L"相关链接").
                     subMenu(menuLinks).
                     bmpArrayID(VdkBitmapArrayId(m_mainMenuBitmaps,
                                MMBID_LINKS, 0)));

    menu->AppendSeperator();

    //===============================================

    VdkMenu *menuPlayControl = m_mainPanel->CreateMenu(menu);

    m_playPauseItem = NewMenuItem();
    menuPlayControl->AppendItem(m_playPauseItem->id(OOM_PLAY_PAUSE).
                                caption(L"暂停"));

    m_stopItem = NewMenuItem();
    menuPlayControl->AppendItem(m_stopItem->id(OOM_STOP).
                                caption(L"停止"));

    menuPlayControl->AppendSeperator();

    m_prevItem = NewMenuItem();
    menuPlayControl->AppendItem(m_prevItem->id(OOM_PREV).
                                caption(L"上一首"));

    m_nextItem = NewMenuItem();
    menuPlayControl->AppendItem(m_nextItem->id(OOM_NEXT).
                                caption(L"下一首"));

    menu->AppendItem(NewMenuItem()->caption(L"播放控制").
                     subMenu(menuPlayControl));

    //===============================================

    VdkMenu *menuVolumeCtrl = m_mainPanel->CreateMenu(menu);

    menuVolumeCtrl->AppendItem(NewMenuItem()->id(OOM_VOLUME_UP).
                               caption(L"增大"));

    menuVolumeCtrl->AppendItem(NewMenuItem()->id(OOM_VOLUME_DOWN).
                               caption(L"减小"));

    menuVolumeCtrl->AppendSeperator();

    m_muteItem = NewMenuItem();
    menuVolumeCtrl->AppendItem(m_muteItem->id(OOM_MUTE).
                               caption(L"静音"));

    menu->AppendItem(NewMenuItem()->caption(L"音量控制").
                     subMenu(menuVolumeCtrl));

    //===============================================

    m_playModes1 = m_mainPanel->CreateMenu(menu);
    FillPlayModesMenu(m_playModes1);
    menu->AppendItem(NewMenuItem()->caption(L"播放模式").
                     subMenu(m_playModes1));
    menu->AppendSeperator();

    // 皮肤
    VdkMenu *menuSkin = m_mainPanel->CreateMenu(menu);
    menuSkin->SetAddinStyle(VMS_NO_EXTRA_SPACE);
    menuSkin->RemoveStyle(VMS_BITMAP);
    menuSkin->AttachCtrl(new OOPSkinSelector);

    menu->AppendItem(NewMenuItem()->caption(L"皮肤").
                     subMenu(menuSkin));

    // 透明度
    VdkMenu *menuTransparent = m_mainPanel->CreateMenu(menu);
    menuTransparent->AttachCtrl(new OnMenuSlider(m_mainPanel));

    menu->AppendItem(NewMenuItem()->caption(L"窗口透明").
                     subMenu(menuTransparent));
    menu->AppendSeperator();

    //===============================================

#if 0
    menu->AppendItem(NewMenuItem()->id(CID_FEEDBACK).caption(L"我要反馈"));
    menu->AppendSeperator();
#endif

    menu->AppendItem(NewMenuItem()->id(CID_MINIMIZE).caption(L"最小化"));
    menu->AppendItem(NewMenuItem()->id(CID_EXIT).
                     caption(L"退出").
                     bmpArrayID(VdkBitmapArrayId(m_mainMenuBitmaps,
                                MMBID_EXIT, 0))
                    );

    m_mainPanel->AttachMenu(menu);
    m_MainMenu = menu;
}

void OOPlayerApp::FillPlayModesMenu(VdkMenu *menu, bool ownedByPLP) {
    menu->SetBestWidth(150);

    const wchar_t *captions[] = {

        L"单曲播放",
        L"单曲循环",
        L"顺序播放",
        L"循环播放",
        L"随机播放",
    };

    VdkMenuItem *item;
    for (int i = 0; i < 5 ; i++) {
        item = NewMenuItem();
        item->id(PLAY_MODE_SINGLE + i).caption(captions[i]);
        menu->AppendItem(item);
    }

    menu->GetItem(m_playMode - PLAY_MODE_SINGLE)->checked(true);

    //===============================================

    if (ownedByPLP) {
        m_playModes2 = menu;
    }
}

//////////////////////////////////////////////////////////////////////////

enum PlayListMenuBitmapIds {

    PLMBID_PLAY,
    PLMBID_FILE_INFO,
    PLMBID_DELETE,
    PLMBID_LOCATE_FILE,
    PLMBID_OPTIONS,

    PLMBID_COUNT
};

enum {

    TBMBID_ADD_FILE,
    TBMBID_ADD_FOLDER,
    TBMBID_OPEN_PLAYLIST,
    TBMBID_SAVE_PLAYLIST,
    TBMBID_QUICK_SEARCH,
    TBMBID_DO_SEARCH,

    TBMBID_COUNT
};

void PlayListPanel::GenerateMenu() {
    VdkMenu *menu = CreateMenu(NULL, m_menuId);
    menu->SetAddinStyle(VMS_SEND_PRESHOW_MSG);

    if (!m_playListMenuBitmaps.IsOk()) {
        wxString imgPath(OOPFileSystem::GetAppResDir() + L"playList.png");
        m_playListMenuBitmaps.Set(imgPath, PLMBID_COUNT, 1);
    }

    //===============================================

    menu->AppendItem(NewMenuItem()->
                     id(OPLM_PLAY_SELECTED).
                     caption(L"播放").
                     bmpArrayID(VdkBitmapArrayId(m_playListMenuBitmaps,
                                PLMBID_PLAY, 0)));

    menu->AppendSeperator();
    menu->AppendItem(NewMenuItem()->
                     id(OPLM_FILE_PROPERTIES).
                     caption(L"文件属性").
                     bmpArrayID(VdkBitmapArrayId(m_playListMenuBitmaps,
                                PLMBID_FILE_INFO, 0)));

    menu->AppendSeperator();
    menu->AppendItem(NewMenuItem()->
                     id(OPLM_DELETE).
                     caption(L"删除").
                     bmpArrayID(VdkBitmapArrayId(m_playListMenuBitmaps,
                                PLMBID_DELETE, 0)));

    menu->AppendItem(NewMenuItem()->id(OPLM_DELETE_FILE).
                     caption(L"物理删除"));

    VdkMenu *renameFiles = CreateMenu(menu);

    renameFiles->AppendItem(NewMenuItem()->id(OPLM_RENAME_SONGNAME_ONLY).
                            caption(L"歌曲名.扩展名"));
    renameFiles->AppendItem(NewMenuItem()->id(OPLM_RENAME_SINGER_FIRST).
                            caption(L"歌手 - 歌曲名.扩展名"));
    renameFiles->AppendItem(NewMenuItem()->id(OPLM_RENAME_SONGNAME_FIRST).
                            caption(L"歌曲名 - 歌手.扩展名"));

    menu->AppendItem(NewMenuItem()->id(OPLM_FILE_PROPERTIES).
                     caption(L"重命名文件").
                     subMenu(renameFiles));

    menu->AppendItem(NewMenuItem()->id(OPLM_LOCATE_FILE).
                     caption(L"浏览文件").
                     bmpArrayID(VdkBitmapArrayId(m_playListMenuBitmaps,
                                PLMBID_LOCATE_FILE, 0)));

    menu->AppendSeperator();
    menu->AppendItem(NewMenuItem()->id(OPLM_PLAYLIST_OPTIONS).
                     caption(L"选项").
                     bmpArrayID(VdkBitmapArrayId(m_playListMenuBitmaps,
                                PLMBID_OPTIONS, 0)));

    m_playList->AttachMenu(menu);

    //===============================================

    OOPToolBar *toolbar = (OOPToolBar *) FindCtrl(L"toolbar");

    if (toolbar) {
        MenuGenerator gen(wxID_ANY);

        gen.itemMap[OTBM_ADD_FILE] = L"文件(&F)";
        gen.itemMap[OTBM_ADD_FOLDER_RECURSIVE] = L"文件夹(包括子目录)(&R)";
        gen.itemMap[OTBM_ADD_FOLDER] = L"文件夹(&O)";

        toolbar->Append(wxEmptyString, ::GenerateMenu(this, gen));
        gen.Clear();

        gen.itemMap[OPLM_DELETE] = L"选中的文件(&S)";
        gen.itemMap[OTBM_DELETE_DUPLICATED] = L"重复的文件(&D)";
        gen.itemMap[OTBM_DELETE_WRONG_FILE] = L"错误的文件(&E)";
        gen.itemMap[OTBM_DELETE_SEP_1] = L"<SEP>";
        gen.itemMap[OTBM_DELETE_ALL] = L"全部删除(&A)";
        gen.itemMap[OTBM_DELETE_SEP_2] = L"<SEP>";
        gen.itemMap[OTBM_DELETE_FILE] = L"物理删除(&P)";

        toolbar->Append(wxEmptyString, ::GenerateMenu(this, gen));
        gen.Clear();

        gen.itemMap[OTBM_LIST_OPEN] = L"打开列表(&O)";
        gen.itemMap[OTBM_LIST_SAVE] = L"保存列表(&S)";
        gen.itemMap[OTBM_LIST_NEW] = L"新建列表(&N)";

        toolbar->Append(wxEmptyString, ::GenerateMenu(this, gen));
        gen.Clear();

        gen.itemMap[OTBM_SORT_BY_TITLE] = L"按显示标题(&T)";
        gen.itemMap[OTBM_SORT_BY_PATH] = L"按路径名(&P)";
        gen.itemMap[OTBM_SORT_BY_ALBUM] = L"按专辑名(&A)";
        gen.itemMap[OTBM_SORT_BY_LENGTH] = L"按歌曲长度(&L)";
        gen.itemMap[OTBM_SORT_BY_PLAY_COUNT] = L"按播放次数(&C)";
        gen.itemMap[OTBM_SORT_BY_ADD_TIME] = L"按加入列表时间(&F)";

        toolbar->Append(wxEmptyString, ::GenerateMenu(this, gen));
        gen.Clear();

        gen.itemMap[OTBM_LOCATE_SHOW_DIALOG] = L"快速定位(&S)";
        gen.itemMap[OTBM_LOCATE_FIND_NEXT] = L"查找下一个(&N)";

        toolbar->Append(wxEmptyString, ::GenerateMenu(this, gen));
        gen.Clear();

        gen.itemMap[OTBM_SELECT_ALL] = L"全部选中(&A)";
        gen.itemMap[OTBM_SELECT_NONE] = L"全部不选(&U)";
        gen.itemMap[OTBM_SELECT_REVERSE] = L"反向选择(&I)";

        toolbar->Append(wxEmptyString, ::GenerateMenu(this, gen));
        gen.Clear();

        VdkMenu *menu = CreateMenu();
        g_app->FillPlayModesMenu(menu, true);
        toolbar->Append(wxEmptyString, menu);
    }
}

//////////////////////////////////////////////////////////////////////////

enum {

    LMBID_COPY_LRC,
    LMBID_REMOVE_LRC,
    LMBID_SEARCH_LRC,
    LMBID_RELOAD_LRC,
    LMBID_ASSOCIATE_LRC,
    LMBID_OPTIONS,

    LMBID_COUNT,
};

void LyricPanel::GenerateMenu() {
    VdkMenu *menu = CreateMenu(NULL, m_menuId);
    menu->SetAddinStyle(VMS_SEND_PRESHOW_MSG);
    menu->SetBestWidth(150);

    if (!m_menuBitmaps.IsOk()) {
        wxString imgPath(OOPFileSystem::GetAppResDir() + L"lyricMenu.png");
        m_menuBitmaps.Set(imgPath, LMBID_COUNT, 1);
    }

    //===============================================

    menu->AppendItem(NewMenuItem()->id(OLRCM_SEARCH).
                     caption(L"在线搜索(&D)...").
                     bmpArrayID(VdkBitmapArrayId(m_menuBitmaps,
                                LMBID_SEARCH_LRC, 0)));

    menu->AppendItem(NewMenuItem()->id(OLRCM_ASSOCIATE_LOCAL_FILE).
                     caption(L"关联歌词(&L)...").
                     bmpArrayID(VdkBitmapArrayId(m_menuBitmaps,
                                LMBID_ASSOCIATE_LRC, 0)));

    menu->AppendSeperator();

    menu->AppendItem(NewMenuItem()->id(OLRCM_COPY_LRC).
                     caption(L"复制歌词(&C)").
                     bmpArrayID(VdkBitmapArrayId(m_menuBitmaps,
                                LMBID_COPY_LRC, 0)));
    menu->AppendItem(NewMenuItem()->id(OLRCM_REMOVE_LRC).
                     caption(L"撤销歌词(&U)").
                     bmpArrayID(VdkBitmapArrayId(m_menuBitmaps,
                                LMBID_REMOVE_LRC, 0)));
    menu->AppendItem(NewMenuItem()->id(OLRCM_RELOAD_LRC).
                     caption(L"重新载入(&R)").
                     bmpArrayID(VdkBitmapArrayId(m_menuBitmaps,
                                LMBID_RELOAD_LRC, 0)));

    menu->AppendSeperator();
    menu->AppendItem(NewMenuItem()->id(OLRCM_LRC_OPTIONS).
                     caption(L"选项(&P)").
                     bmpArrayID(VdkBitmapArrayId(m_menuBitmaps,
                                LMBID_OPTIONS, 0)));

    m_lyric->AttachMenu(menu);
}
