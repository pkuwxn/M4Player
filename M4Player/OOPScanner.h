#pragma once
#include <wx/vector.h>

class OOPSongPtr;
class OOPProgressDlg;
class VdkVObjEvent;

wxDECLARE_EVENT(OOP_EVT_SCAN_DONE, wxCommandEvent);

/// \brief 搜索指定目录，查找歌曲
class OOPScanner : public wxThread {
public:

    /// \brief 构造函数
    /// \param songList 应该在堆上分配，会被接管
    OOPScanner(wxWindow *dlgParent, wxArrayString *songList);

    /// \brief  析构函数
    ~OOPScanner();

private:

    // 根据工作者线程反馈的信息更新对话框
    void OnUpdateUI(VdkVObjEvent &e);

    // 线程入口函数
    virtual ExitCode Entry();

    //--------------------------

    OOPProgressDlg *m_dlg;
    wxWindow *m_dlgParent;

    wxArrayString *m_songList;
    wxVector<OOPSongPtr> *m_songs;

    int m_idUpdateUi;
};
