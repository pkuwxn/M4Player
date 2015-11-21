#pragma once

class VdkWindow;

/// \brief 全程重画使用双缓冲
///
/// 我经常在一个 VdkControl 的回调函数中随意地使用 wxClientDC ，导致一个很大的弊端:
/// 出现重绘时深为诟病的闪烁。所以写了这个类。自动化地实现双缓冲，不用自己再申请
/// 一个 wxMemoryDC 了。\n
/// 使用方法: 请勿在堆上创建本类的指针，这会造成严重的性能问题。
/// #define __VDK_NOT_USE_DOUBLE_BUFFERING__
#ifndef __VDK_NOT_USE_DOUBLE_BUFFERING__
class VdkDC : public wxMemoryDC {
public:

    /// \brief 构造函数
    /// \param pdc 提供一个原始设备上下文作为目标设备上下文
    VdkDC(VdkWindow *Window, const wxRect &rc, wxDC *pdc = NULL);

    /// \brief 构造函数
    ///
    /// 此时默认的剪切区域为整个窗口。
    VdkDC(VdkWindow *Window);

    /// \brief 析构函数
    ~VdkDC();

private:

    // 实际构建函数
    void Construct(VdkWindow *Window, const wxRect &rc, wxDC *pdc);

    // 将当前缓冲内容提交到实际 DC 上
    void Flush(wxDC &dc);

    //////////////////////////////////////////////////////////////////////////

    VdkWindow                  *m_Window;
    wxRect                      m_Rect;

    unsigned long               m_timeStamp;
    static unsigned long        ms_timeStamp;
    static VdkWindow           *ms_window;

    wxDC                       *m_pdc;

    wxDECLARE_NO_COPY_CLASS(VdkDC);
};
#else
class VdkDC : public wxClientDC {
public:

    /// \brief 构造函数
    /// \param pdc 提供一个原始设备上下文作为目标设备上下文
    VdkDC(VdkWindow *Window, const wxRect &rc, wxDC *pdc = NULL);

    /// \brief 构造函数
    ///
    /// 此时默认的剪切区域为整个窗口。
    VdkDC(VdkWindow *Window);

    wxDECLARE_NO_COPY_CLASS(VdkDC);
};
#endif // __VDK_NOT_USE_DOUBLE_BUFFERING__

/// \brief 用于确保使用 wxDC* pDC 指针传进来的 DC 指针一定可用
///
/// 有点奇怪吧，主要既然传入一个指针就是为了有时候可以不必创建一个 wxClientDC ,
/// 那为什么还要确保它有效呢？\n
/// 当必须要更新 GUI 时（或许是 bRedraw = true ），还是要确保指针的有效性的
class VdkValidDC {
public:

    /// \brief 构造函数
    VdkValidDC(wxDC *pDC, VdkWindow *window, wxRect *prc);
    /// \brief 析构函数
    ~VdkValidDC();

    /// \brief 自动转换到 wxDC
    operator wxDC *() {
        return m_pDC;
    }

    /// \brief 获取 wxDC 引用
    wxDC &operator*() {
        return *m_pDC;
    }

    /// \brief 获取 wxDC 句柄
    wxDC *get() {
        return m_pDC;
    }

private:

    wxDC *m_pDC;
    bool m_bOwnDC;
};

/// \brief 设备上下文的设备起始坐标自动恢复器
class VdkDcDeviceOriginSaver {
public:

    /// \brief 构造函数
    VdkDcDeviceOriginSaver(wxDC &dc);
    /// \brief 析构函数
    ~VdkDcDeviceOriginSaver();

    /// \brief 手动恢复设备上下文的设备起始坐标
    void Destroy();

private:

    wxDC                       &m_dc;
    int                         m_x, m_y;
};

/// \brief DC 裁剪区域自动恢复器
class VdkDcClippingRegionDestroyer {
public:

    /// \brief 构造函数
    /// \param dc 要操作的设备上下文
    /// \param region 要裁剪成的区域
    VdkDcClippingRegionDestroyer(wxDC &dc, const wxRect &region);
    /// \brief 析构函数
    ~VdkDcClippingRegionDestroyer();

private:

    VdkDcDeviceOriginSaver      m_saver;
    wxRect                      m_clpbox0;
    wxDC                       &m_dc;
};

/// \brief 在 VdkDC 提交更改的的最后一刻允许修改最终内容
class VdkDcPostListener {
public:

    /// \brief 执行绘图操作
    virtual void OnPost(wxDC &dc) = 0;

    /// \brief 虚析构函数
    void RemoveFromWindow(VdkWindow *win);
};

/// \brief 在\a DC 的设备起始坐标的基础上增加\a dX 和\a dY
void VdkDcDeviceOriginAdder(wxDC &dc, int dX, int dY);
