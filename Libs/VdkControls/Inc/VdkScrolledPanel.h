/***************************************************************
* Name:      VdkScrolledPanel.h
* Purpose:   滚动面板（能作为其他控件父控件的滚动窗口派生类）声明
* Author:    vanxining (vanxining@139.com)
* Created:   2011-01-04
* Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkNativeCtrl.h"
#include "VdkWindowImpl.h"
#include "VdkScrolledWindow.h"
#include "VdkScrollBar.h"

/// \brief 滚动面板的实现基类
///
/// 简化设计压力，所以加多了一层继承。
class VdkScrolledPanelBase : public VdkWindowImpl<wxPanel> {
public:

    /// \brief 默认构造函数
    VdkScrolledPanelBase() {}

    /// \brief 构造函数
    VdkScrolledPanelBase(wxWindow *parent, long style = 0)
        : VdkWindowImpl<wxPanel>(style) {
        Create(parent, style);
    }

    /// \brief 承担实际构建工作
    void Create(wxWindow *parent, long style = 0);

private:

    virtual void DoResize(int x, int y, int width, int height,
                          int sizeFlags = wxSIZE_AUTO) {
        SetSize(x, y, width, height, sizeFlags);
    }

    virtual void DoMinimize() {}
};

//////////////////////////////////////////////////////////////////////////

/// \brief 能作为一个 VdkWindow 子控件的滚动面板
class VdkScrolledPanel : public VdkScrolledPanelBase,
    public VdkNativeCtrl<VdkScrolledPanel>,
    public VdkScrolled {
public:

    /// \brief 构造函数
    VdkScrolledPanel();

    /// \brief 析构函数
    ~VdkScrolledPanel();

    /// \brief XRC 动态创建
    ///
    /// 一个 VdkControl 的派生类假如要实现动态创建，则应该实现这个函数。
    /// \todo 父控件
    virtual void Create(wxXmlNode *node);

    /// \brief 根据控件数量计算虚拟画布的大小
    void CalcVirtualSize();

    /// \brief 设置已缓冲的双缓冲绘图上下文的某些初始属性
    virtual void PrepareCachedDC();

    /// \brief 子控件是否处于可视区域内
    bool IsChildVisible(VdkControl *chd) const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // 次级派生类必须覆写

    // 获取滚动条的父窗口
    virtual VdkWindow *ParentWindow() const {
        return dynamic_cast<VdkWindow *>(wxWindow::GetParent());
    }

    // 滚动条的父控件(假如有)
    virtual VdkControl *ScrollBarParent() const {
        return (VdkControl *) this;
    }

    // 滚动条的相对位置
    virtual wxRect ScrollBarRect() const {
        return Rect00();
    }

    // 获取滚动条对应的 DC
    virtual wxDC *GetScrollBarDC(wxDC *dc) const ;

    // 销毁从\link GetScrollBarDC \endlink 获得的 DC
    virtual void DestroyScrollBarDC(wxDC *dc) const;/* { delete dc; }*/

    // 获取滚动窗口的名字
    virtual const wxString &Name() const {
        return m_strName;
    }

    // 获取/设置滚动窗口的风格属性集
    virtual long Style() const {
        return VdkWindow::GetStyle();
    }

    // 获取/设置滚动窗口相对于父控件的相对位置
    ///
    /// 假如没有父控件，则返回值与\link AbsoluteRect() \endlink 相同。
    /// \see AbsoluteRect
    virtual wxRect &RelativeRect() {
        return m_Rect;
    }

    // 获取/设置滚动窗口相对于顶层 VdkWindow 的绝对位置
    virtual wxRect AbsoluteRect() const {
        return Rect00();
    }

    // 在此更新/绘制滚动窗口
    virtual void UpdateUI(wxDC &dc);

    // 检测是否已在 XRC 文件中定义并创建了需要的滚动条
    ///
    /// 用于滚动窗口创建缺省滚动条前。
    virtual VdkScrollBar *ScrollBarExists() {
        return NULL;
    }

    // 响应滚动条显示/隐藏事件通知
    virtual void OnScrollBarShowHide(const VdkScrollBar *sb);

    //////////////////////////////////////////////////////////////////////////
    // 次级派生类可以覆写

    // 决定需要显示/隐藏的子控件。
    virtual void DoRefreshState(wxDC &dc);

    //////////////////////////////////////////////////////////////////////////
    // 继承自 VdkControl

    // 绘制边框
    virtual void DoDraw(wxDC &dc);

    // 接收拖动 VdkSlider 的鼠标事件
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);

    // 接收控件的通知信息
    virtual void DoHandleNotify(const VdkNotify &notice);

    //////////////////////////////////////////////////////////////////////////
    // 继承自 VdkScrolled

    // 鼠标滚轮的事件不会发送到菜单上的滚动面板
    // 同时更灵敏地处理指针移动事件
    virtual void OnMouseEvent(VdkMouseEvent &e);

    //////////////////////////////////////////////////////////////////////////
    // 继承自 VdkWindow

    // 在 VdkWindow 处理鼠标事件之前修正起始坐标
    virtual bool FilterEventBefore(wxMouseEvent &evt, int evtCode);

    // 处理发生未被其他控件处理的滚轮事件
    virtual bool FilterEventAfter(wxMouseEvent &evt, int evtCode);

    // 便利重载，**非继承**
    bool FilterEventAfter(VdkMouseEvent &evt);

    // 在完全重画事件中清除背景
    //
    // VdkWindow 对于滚动窗口是毫无概念的，故它不知道自己在
    // EraseBackground 函数中擦除的部分是一开始那一屏。
    virtual void DoPaint(wxDC &dc);

    //////////////////////////////////////////////////////////////////////////

    // 有一个可能会导致获取滚动条专有 DC 时会出现产生两个
    // 滚动条所在窗口的 VdkDC
    // 滚动事件不是发送到当前活动菜单窗口的，所以向菜单实体窗体传播的过程中
    // 会申请一次，滚动面板重画时又会申请一次
    mutable bool m_bUsingCachedSbDC;

    DECLARE_DYNAMIC_VOBJECT
};
