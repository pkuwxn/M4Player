/***************************************************************
 * Name:      VdkDefs.h
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2010-02-01
 * Copyright: Wang Xiaoning
 **************************************************************/
#pragma once
#include "VdkCommon.h"

/// \brief VDK 鼠标事件
enum VdkMouseEventType {

    NORMAL,                     ///< 正常状态
    HOVERING = 1,               ///< 鼠标置于其上

    LEFT_DOWN,                  ///< 鼠标左键按下
    LEFT_UP,                    ///< 鼠标左键释放
    DLEFT_DOWN,                 ///< 鼠标左键双击
    RIGHT_UP,                   ///< 鼠标右键弹起
    DRAGGING,                   ///< 鼠标拖动

    WHEEL_UP,                   ///< 鼠标滚轮向下滚动
    WHEEL_DOWN,                 ///< 鼠标滚轮向上滚动
    SCROLLING_UP,               ///< 竖直 VdkSlider 向上拖动
    SCROLLING_DOWN,             ///< 竖直 VdkSlider 向下拖动

    MOUSE_LEAVE,                ///< 鼠标离开控件作用域
    MOUSE_LEAVE_WINDOW,         ///< 鼠标离开 VdkWindow 作用域

    MOUSE_HOLD_ON_RELEASED,     ///< 释放持续按住的鼠标左键
};

/// \brief VDK 按键事件
enum VdkKeyEventType {

    KEY_DOWN,   ///< 按键被按下
    KEY_UP, ///< 按键弹起
    KEY_CHAR,   ///< 已翻译的按键
};

/// \brief VDK 通知信息
enum VdkCtrlNotifyCode {

    //////////////////////////////////////////////////////////////////////////
    // 创建、销毁

    VCN_CREATE,
    VCN_DESTROY,
    VCN_BIND_TO_ANOTHER_PARENT, ///< 绑定到另外的父窗口上

    VCN_ID_CHANGED, ///< 控件 ID 发生改变

    //////////////////////////////////////////////////////////////////////////
    // 可视状态

    VCN_FONT_CHANGED,
    VCN_SHOW,
    VCN_HIDE,
    VCN_THAW,
    VCN_FREEZE,
    VCN_ENABLED,
    VCN_DISABLED,

    VCN_PARENT_SHOW, /*! 父窗口显示 */
    VCN_PARENT_HID, /*! 父窗口隐藏 */

    //////////////////////////////////////////////////////////////////////////
    // 焦点

    VCN_FOCUS, ///< 获得焦点
    VCN_LOST_FOCUS, ///< 失去焦点

    VCN_MENU_HID, ///< 依附的弹出菜单被隐藏了

    //////////////////////////////////////////////////////////////////////////
    // 尺寸

    VCN_POSITION_CHANGED,   ///< SetRect 中的控件位置改变
    VCN_SIZE_CHANGED,       ///< SetRect 中的区域大小改变
    /// \brief VdkWindow 大小改变
    /// \note 与 VCN_SIZE_CHANGED 相区分主要是本事件可能会再次更改控件的大小，
    /// 造成重复
    VCN_WINDOW_RESIZED,
    /// \brief 窗口可视起始点改变了，消息的两个参数为新的可视起始点
    /// \note 注意不是原点，而是绘图 DC 的物理起始坐标。
    VCN_WINDOW_ORIGIN_CHANGED,
};

/// 可改变大小的位图以何种方式扩大（平铺或者拉伸）
enum VdkResizeableBitmapType {

    RESIZE_TYPE_TILE, ///< 平铺
    RESIZE_TYPE_STRETCH ///< 拉伸
};

// 对齐属性
enum VdkAlignment {

    vdkALIGN_INVALID        = 0,

    //--------

    vdkALIGN_LEFT           = 1 << 0,
    vdkALIGN_RIGHT          = 1 << 1,
    vdkALIGN_TOP            = 1 << 2,
    vdkALIGN_BOTTOM         = 1 << 3,

    //--------

    vdkALIGN_ALL            = vdkALIGN_LEFT |
                              vdkALIGN_RIGHT |
                              vdkALIGN_TOP |
                              vdkALIGN_BOTTOM,
};

// 方向属性
enum VdkDirection {

    vdkDIRECTION_INVALID    = 0,

    //--------

    vdkEAST                 = 1 << 0,
    vdkSOUTH                = 1 << 1,
    vdkWEST                 = 1 << 2,
    vdkNORTH                = 1 << 3,

    //--------

    vdkDIRECTION_ALL        = vdkEAST |
                              vdkSOUTH |
                              vdkWEST |
                              vdkNORTH,
};

/// \brief 控件对齐属性
enum AlignType {

    ALIGN_LEFT              = 0,
    ALIGN_RIGHT             = 1 << 0,
    ALIGN_TOP               = 1 << 1,
    ALIGN_BOTTOM            = 1 << 2,
    ALIGN_CENTER_X_Y        = 1 << 3,
    ALIGN_CENTER_X          = 1 << 4,
    ALIGN_CENTER_Y          = 1 << 5,
    /// \brief 与主窗体的大小改变同步，即同增减相等差量
    ALIGN_SYNC_X_Y          = 1 << 6,
    ALIGN_SYNC_X            = 1 << 7,
    ALIGN_SYNC_Y            = 1 << 8,

    /// \brief 原生控件专用
    ///
    /// 假如一个 VdkNativeCtrl 没有任何对齐信息的话，是无法更新自己的位置的。\n
    /// 因为没有任何对其信息，对齐调用\link VdkControl::HandleResize \endlink
    /// 是没有任何作用的。因此，这个属性只是一个虚拟的值，使得 VDK
    /// 认为对我们的原生控件是必要的。这样，VdkNativeCtrl 模板类便能顺利更新一个
    /// VdkNativeCtrl 相对于 VdkWindow 左上角的绝对位置。
    ALIGN_NATIVE_CTRL       = 1 << 9,
};

/// \brief 文本对齐属性
enum VdkTextAlign {

    TEXT_ALIGN_LEFT         = ALIGN_LEFT, ///< 左对齐
    TEXT_ALIGN_RIGHT        = ALIGN_RIGHT,
    TEXT_ALIGN_TOP          = ALIGN_TOP,
    TEXT_ALIGN_BOTTOM       = ALIGN_BOTTOM,
    TEXT_ALIGN_CENTER_X_Y   = ALIGN_CENTER_X_Y,
    TEXT_ALIGN_CENTER_X     = ALIGN_CENTER_X,
    TEXT_ALIGN_CENTER_Y     = ALIGN_CENTER_Y,
};

//////////////////////////////////////////////////////////////////////////

VDK_DECLARE_WX_PTR_LIST(VdkControl, VdkCtrlList, VdkCtrlIter)
VDK_DECLARE_WX_PTR_LIST(VdkWindow, VdkWindowList, VdkWindowIter)
