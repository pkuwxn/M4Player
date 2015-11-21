#pragma once
#include "VdkControl.h"

class VdkWindow;
/// \brief 一个类似超链接的热点区域，可以使用背景位图
class VdkHotArea : public VdkControl {
public:

    /// \brief 默认构造函数
    VdkHotArea();

    /// \brief 执行实际构建操作
    void Create(VdkWindow *Window,
                const wxString &strName,
                const wxRect &rc,
                const wxString &strUrl);

    /// \brief XRC 动态创建
    virtual void Create(wxXmlNode *node);

protected:

    /// \brief 用于派生类的初始化
    void StartSense();

    /// \brief 处理鼠标事件
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);

    bool            m_bHand; ///< 鼠标指针是否已经改变
    wxString        m_strUrl; ///< URL 地址

    wxCursor        m_cursorHand; ///< 进入作用域后鼠标指针形状

    DECLARE_DYNAMIC_VOBJECT
};
