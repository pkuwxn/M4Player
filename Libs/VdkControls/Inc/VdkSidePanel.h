/***************************************************************
 * Name:      VdkSidePanel.h
 * Purpose:   Code for VdkSidePanel declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-04-08
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkControl.h"

/// \brief VdkSidePanel 的风格属性集
enum VdkSidePanelStyle {

};

/// \brief VdkSidePanel 的运行时状态属性集
enum VdkSidePanelState {

};

/// \brief 类似 Office 2003 的侧边栏
class VdkSidePanel : public VdkControl {
public:

    /// \brief 默认构造函数
    VdkSidePanel();

    /// \brief XRC 动态创建
    virtual void Create(wxXmlNode *node);

private:

    /// \brief 绘制控件
    virtual void DoDraw(wxDC &dc);

private:

    wxString m_strCaption;

    DECLARE_CLONEABLE_VOBJECT(VdkSidePanel)
};
