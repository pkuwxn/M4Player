#pragma once
#include "VdkMenuBar.h"

/// \brief OOPlayer 播放列表的下拉菜单工具栏
class OOPToolBar : public VdkMenuBar {
public:

    /// \brief XRC 动态创建
    virtual void Create(wxXmlNode *node);

private:

    /// \brief 添加一项
    virtual VdkMenuBarEntry *CreateNewEntry(int &w, int &h);

    /// \brief 绘制菜单项当前状态
    void UpdateEntry(wxDC &dc, unsigned eid, int state);

private:

    VdkBitmapArray  m_normal;
    VdkBitmapArray  m_hot;

    friend class OOPToolBarEntry;

    DECLARE_DYNAMIC_VOBJECT
};

/// \brief OOPlayer 播放列表的下拉菜单工具栏的一项
class OOPToolBarEntry : public VdkMenuBarEntry {
public:

    /// \brief 默认构造函数
    OOPToolBarEntry();

private:

    /// \brief 擦除（绘制）背景
    virtual void DoEraseBackground(wxDC &dc, const wxRect &rc);

    /// \brief 处理鼠标事件
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);
};
