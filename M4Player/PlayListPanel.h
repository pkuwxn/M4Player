/***************************************************************
 * Name:      PlayListPanel.h
 * Purpose:   声明 PlayListPanel
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-2-27
 **************************************************************/
#pragma once
#include "OOPWindow.h"
#include "VdkBitmapArray.h"

class OOPSongPtr;
class OOPList;
class OOPFilePropDlg;
class OOPPlaylistSearchDlg;
class VdkLcCell;

/// \brief 播放列表窗口
class PlayListPanel : public OOPWindow {
public:

    /// \brief 构造函数
    PlayListPanel(wxWindow *parent, bool showAtOnce);

    /// \brief 向用户询问并添加得到的（多个）文件
    /// \param parent “打开文件”模态对话框的父窗口
    void AddFiles(wxWindow *parent);

    /// \brief 执行实际添加文件的操作
    void AddFiles(wxArrayString *files);

    //! 添加目录文件的模式
    enum RecursiveMode {
        RM_RECURSIVE, ///< 包括子文件夹
        RM_ONLY_TOP_LEVEL_FILES, ///< 只包含顶层文件
    };

    /// \brief 向用户询问并添加得到的一个文件夹
    void AddFolder(RecursiveMode rm);

    /// \brief 向用户询问并添加得到的一个文件夹
    void AddFolder(const wxString &dirPath, RecursiveMode rm, wxArrayString *files);

private:

    // 设置窗口所用的皮肤
    virtual void DoSetSkin();

    // 响应按键事件
    virtual bool DoHandleKeyEvent(wxKeyEvent &e);

    // 截获鼠标事件(实现“伪模态对话框”)
    virtual bool FilterEventBefore(wxMouseEvent &, int);

    // 检测当前状态，是否需要实现“伪模态对话框”
    bool TestPsuedoModal();

    // 截获窗口关闭事件
    void OnClose(wxCloseEvent &);

    // 文件扫描完成
    void OnScanDone(wxCommandEvent &e);

    // 打开“文件属性”对话框
    void OnFileProperties(VdkVObjEvent &);

    // 用户更改了歌曲的 TAG
    void OnSongInfoUpdated(wxCommandEvent &e);

    // 重命名歌曲文件
    void OnRename(VdkVObjEvent &e);

    // 用户提交了一次搜索
    void OnSearch(wxCommandEvent &e);

    // 执行具体搜索工作
    void Search();

    // 生成菜单
    void GenerateMenu();

    // 菜单项的按需显示
    void OnPreShowMenu(VdkVObjEvent &);

    // 获取列表中当前被选中的第一个条目
    OOPSongPtr GetCurrSel(VdkLcCell **pCurr, int *pIndex) const;

private:

    OOPList *m_playList;
    OOPFilePropDlg *m_filePropDlg; // “文件属性”对话框
    // 已处理“文件属性”对话框关闭后发过来的歌曲元信息已更新事件
    bool m_songInfoUpdated;

    OOPPlaylistSearchDlg *m_searchDlg; // “快速定位文件”对话框
    wxString m_lastKeyword; // 上次执行查找的关键字
    int m_lastFound; // 上次找到的项目的行号

    int m_menuId; // 菜单 ID

    VdkBitmapArray m_playListMenuBitmaps;
};
