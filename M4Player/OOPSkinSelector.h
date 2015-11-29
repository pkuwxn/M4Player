/***************************************************************
 * Name:      OOPSkinSelector.h
 * Purpose:   使用 VdkListCtrl 来显示大批量的皮肤
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2011.04.02
 * Copyright: Wang Xiaoning
 **************************************************************/
#pragma once
#include "VdkMenu.h"
#include "OOPSkin.h"

class OOPSkinMenuSelector;

/// \brief 使用滚动面板＋缩略图来显示大批量的皮肤
class OOPSkinSelector : public VdkMenuXrcCtrlWrapper {
public:

    /// \brief 构造函数
    OOPSkinSelector();

    /// \brief 析构函数
    ~OOPSkinSelector();

    /// \brief 实例化要附加到 VdkMenu 上的控件
    virtual VdkControl *Implement(VdkWindow *menuWin, const wxPoint &pos);

private:

    // 保存控件状态
    virtual void SaveState();

    // 添加一项缩略图
    void AddAvatar(const OOPSkin::SkinInfo &info);

    // 双击所略图，更换皮肤
    void OnAvatar(VdkVObjEvent &e);

private:

    OOPSkinMenuSelector *m_selector;
    int m_avatarCount; // 显示的皮肤数

    double m_progress; // 当前查看的进度
};
