/***************************************************************
 * Name:      MainPanel.h
 * Purpose:   声明 MainPanel
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-2-27
 **************************************************************/
#pragma once
#include "OOPWindow.h"

/// \brief 主窗口（播放器）
class MainPanel : public OOPWindow {
public:

    /// \brief 构造函数
    MainPanel();

private:

    // 设置窗口所用的皮肤
    virtual void DoSetSkin();
};
