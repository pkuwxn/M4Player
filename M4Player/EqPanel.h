/***************************************************************
 * Name:      EqPanel.h
 * Purpose:   均衡器窗口
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-24
 **************************************************************/
#pragma once
#include "OOPWindow.h"

class VdkSlider;
class VdkToggleButton;
class VdkButton;

/// \brief 均衡器窗口
class EqPanel : public OOPWindow {
public:

    /// \brief 构造函数
    EqPanel(wxWindow *parent);

private:

    // 设置窗口所用的皮肤
    virtual void DoSetSkin();

    // 查找均衡器批量滚动条基准手柄的 XRC 定义
    wxXmlNode *FindEqfactor(wxXmlNode *windowRoot) const;

    // 启用/禁用所有控件
    void EnableAll(bool enable, wxDC *pDC);

private:

    VdkToggleButton *m_enableAll;
    VdkButton *m_profile;
    VdkButton *m_reset;

    VdkSlider *m_balance;
    VdkSlider *m_surround;
    VdkSlider *m_preamp;

    enum {
        NUM_EQFACTORS = 10,
    };

    VdkSlider *m_eqfactors[NUM_EQFACTORS];
};
