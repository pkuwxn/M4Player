/***************************************************************
 * Name:      OOPLyricMgr.h
 * Purpose:   歌词显示控件管理器
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-06-05
 **************************************************************/
#pragma once
#include <wx/vector.h>

class ILyric;
class OOPStopWatch;
class OOPLyricParser;

/// 歌词显示控件管理器
class OOPLyricMgr {
public:

    /// 构造函数
    OOPLyricMgr();

    /// 析构函数
    ~OOPLyricMgr();

    /// 添加一个新的控件
    void Attach(ILyric *lyric);

    /// 删去一个原有控件
    void Dettach(ILyric *lyric);

    /// 重载布尔类型转换操作符
    operator bool() const;

    /// 重载指针操作符
    OOPLyricMgr *operator->() {
        return this;
    }
    const OOPLyricMgr *operator->() const {
        return this;
    }

public:

    /// 以下函数与 ILyric 的接口相同

    bool IsOk() const;
    void SetStopWatch(OOPStopWatch &sw);
    void AttachParser(const OOPLyricParser &parser);
    void Start();
    void Pause();
    void Stop(wxDC *pDC);
    void ClearLyric(wxDC *pDC);
    void GoTo(double percentage, wxDC *pDC, bool bPaused);

    void SetDefualtInteractiveOutput(const wxString &msg);
    void SetInteractiveOutput(const wxString &msg, wxDC *pDC);
    void ResetInteractiveOutput(wxDC *pDC);

private:

    typedef wxVector<ILyric *> LyricVec;
    // 当前正在活动的歌词秀控件
    LyricVec m_lyrics;
};
