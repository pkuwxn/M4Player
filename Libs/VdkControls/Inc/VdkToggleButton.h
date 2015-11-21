#pragma once
#include "VdkButton.h"

/// \brief VdkToggleButton 的风格属性集
enum VdkToggleButtonStyle {

    /// \brief 派生类其实属性位序号
    VTBS_USER               = VBS_USER + 5,
    /// \see VdkToggleButton::SetOwnerControl
    VTBS_OWNER_CONTROL      = 1 << (VBS_USER + 0),
};

/// \brief VdkToggleButton 的运行时状态属性集
enum VdkToggleButtonState {

    /// \brief 派生类起始属性位序号
    VTBST_USER              = VBST_USER + 5,
    /// \brief 是否已经被按下
    VTBST_TOGGLED           = 1 << (VBST_USER + 0),
    /// \brief 是否处于持续按下状态（可能某个耗时操作需要独占）
    VTBST_PUSHED            = 1 << (VBST_USER + 1),
    /// \brief 即将要被按下
    VTBST_TO_BE_PUSHED      = 1 << (VBST_USER + 2),
    /// \brief 因为 VdkToggledButton 处于按下状态时，m_nState
    /// 始终为 VBS_PUSHED（LEFT_DOWN），因此无法区分这个状态是由鼠标事件造成的，
    /// 还是 VdkToggleButton 所处的状态，故添加这一二进制位加以区分
    VTBST_LAST_LEFT_DOWN    = 1 << (VBST_USER + 3),
};

/// \brief 不会自动弹起、恢复原有状态的按钮
class VdkToggleButton : public VdkButton {
public:

    /// \brief 构造函数
    VdkToggleButton();

    /////////////////////////////////////////////////////////////////

    /// \brief 使按钮保持按下状态，不响应事件
    ///
    /// 这是一种“未来态”，即下一次点击会使按钮保持按下状态，\n
    /// 不在响应任何事件，直到用户再次调用本函数解除这种状态\n
    /// （使\a bPushed 为 false ）。
    void PushButton(bool bPushed, wxDC *pDC);

    /// \brief 切换按钮状态
    void Toggle(bool bToggled, wxDC *pDC = NULL, bool bFireCallback = false);

    /// \brief 按钮是否已经被按下
    bool IsToggled() const {
        return TestState(VTBST_TOGGLED);
    }

    /// \brief 按钮是否处于持续按下状态
    bool IsPushed() const {
        return TestState(VTBST_PUSHED);
    }

    /// \brief 按钮是否即将要被按下
    bool IsToBePushed() const {
        return TestState(VTBST_TO_BE_PUSHED);
    }

    /// \brief 按钮的弹起是否处于用户控制的状态下
    ///
    /// 此时 VDK 不对按钮接收到的事件作任何默认反应。
    /// \see SetOwnerControl
    bool IsOwnerControl() const {
        return TestStyle(VTBS_OWNER_CONTROL);
    }

    /// \brief 设置按钮被按下时再次点击时是否由用户控制弹起与否
    ///
    /// 当按钮处于“持续按下”(Toggleed)的状态时，设定再次单击它时由调用者决定\n
    /// 控制是否需要取反对象内部状态变量。
    void SetOwnerControl(bool bEnabled = true) {
        SetAddinStyle(VTBS_OWNER_CONTROL);
    }

protected:

    /// \brief 处理鼠标事件
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);

    DECLARE_DYNAMIC_VOBJECT
};
