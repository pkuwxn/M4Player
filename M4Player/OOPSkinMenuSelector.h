/***************************************************************
 * Name:      OOPSkinMenuSelector.h
 * Purpose:   酷狗2011 风格的皮肤选择器
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-18
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkScrolledPanel.h"

/// \brief 酷狗2011 风格的皮肤选择器
class OOPSkinMenuSelector : public VdkScrolledPanel {
public:

    /// \brief 构造函数
    OOPSkinMenuSelector();

    /// \brief 析构函数
    ~OOPSkinMenuSelector();

private:

    // 创建自己的滚动条
    virtual VdkScrollBar *ScrollBarExists();

    //////////////////////////////////////////////////////////////////////////

    DECLARE_DYNAMIC_VOBJECT
};

#include "OOPSkin.h"
typedef OOPSkin::SkinInfo OOPSkinInfo;

/// \brief 皮肤缩略图
class SkinAvatar : public VdkControl {
public:

    /// \brief 默认构造函数
    SkinAvatar();

    /// \brief 构建控件
    void Create(VdkWindow *win,
                const wxRect &rc,
                const OOPSkinInfo &info);

    /// \brief 皮肤信息
    void GetSkinInfo() {

    }

private:

    // 绘制控件
    virtual void DoDraw(wxDC &dc);

    // 处理鼠标事件
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);

    // 延迟载入皮肤缩略图(第一次显示后)
    void OnLoadAvatar(wxCommandEvent &e);

    //////////////////////////////////////////////////////////////////////////

    const OOPSkinInfo *m_skinInfo;
    VdkMouseEventType m_state; // 控件当前所处的状态：正常/高亮

    const static int ms_tnwidth = 80; // 缩略图宽度
    const static int ms_tnheight = 80; // 缩略图高度

    const static int ms_yPadding = 2; // 缩略图顶部距作用域的距离
    int m_xText; // 文本的 X 坐标
    wxString m_labelOverflow; // 截断了的文本

    wxPen m_borderNomal; // 正常状态下的边框
    wxBrush m_hilight; // 高亮状态下的背景
};
