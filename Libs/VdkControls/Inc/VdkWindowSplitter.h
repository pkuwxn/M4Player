#pragma once
#include "VdkControl.h"

/// \brief 拉锯管理器，使两个控件能互相拉锯
///
/// 拖动两个控件中间的某区域能使其改变大小、互相拉锯。
class VdkWindowSplitter : public VdkControl {
public:

    /// \brief 默认构造函数
    VdkWindowSplitter() {
        Init();
    }

    /// \brief XRC 动态创建
    virtual void Create(wxXmlNode *node);

    /// \brief 执行实际构建函数
    ///
    /// 一个良好的设计是将拉锯管理器作为两个控件之一的子控件，
    /// 这样拉锯管理器能随其中一个控件改变在 VdkWindow 上的作用域，
    /// 简化设计。
    /// \param parent 所属的 VdkWindow
    /// \param strName 控件标识符号
    /// \param rc 控件作用域
    /// \param win1 第一个控件（左边/上边）
    /// \param win2 第二个控件（右边/下边）
    /// \param bVertical 两个控件的排列顺序（左右/上下）
    /// \attention \a win1 和 \a win2 的父控件必须相同且不能为空。
    void Create(VdkWindow *parent,
                const wxString &strName,
                const wxRect &rc,
                VdkControl *win1,
                VdkControl *win2,
                bool bVertical = true);

    /// \brief 两个控件拉锯时最小允许的大小
    void SetMinSize(int min) {
        m_min = min;
    }

private:

    // 初始化控件
    void Init();

    // 处理鼠标事件
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);

    // 绘制控件
    virtual void DoDraw(wxDC &dc);

    //////////////////////////////////////////////////////////////////////////

    VdkControl             *m_win1;
    VdkControl             *m_win2;

    int                     m_mousePos;
    int                     m_lastX;
    int                     m_lastY;

    int                     m_min;

    bool                    m_bVertical;

    DECLARE_DYNAMIC_VOBJECT
};
