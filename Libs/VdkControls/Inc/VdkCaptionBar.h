#pragma once
#include "VdkControl.h"

/*!\brief 一个辅助类，可以简化标题栏相关控件的定义、创建工作
 *
 * 因为相当多的 VdkWindow 都需要一个标题栏，而一个标题栏通常由一个图标、\n
 * 一个标题文本（ VdkLabel ），几个窗口控制按钮（最小化、最大化、关闭，等等）。\n
 * 本类是就是基于这样的考虑而创设。\n
 * 例子
 \code
    <object class="VdkCaptionBar" name="captionbar">
        <padding-right>5</padding-right>
        <close>close_stats.bmp</close>
        <min>min.bmp</min>
        <object class="VdkLabel" name="caption">
            VdkControls All-in-One
            <rect>4, 9, 50, 9</rect>
            <bold>true</bold>
            <font-name>Arial</font-name>
            <font-color>#FFFFFF</font-color>
        </object>
    </object>
 \endcode
 * 注意其中定义的控件不是 VdkCaptionBar 的子控件，包括完整定义的 VdkLabel 。\n
 * 所有的这些定义都可以任意增删。\n
 * 很明显，本类只能用于 XRC 环境下。
**/
class VdkCaptionBar : public VdkControl {
public:

    /// \brief 默认构造函数
    VdkCaptionBar() {}

    /// \brief XRC 动态创建
    virtual void Create(wxXmlNode *node);

    /// \brief 关闭窗口
    static void ClosePanel(VdkVObjEvent &);

    /// \brief 最小化窗口
    static void MaximizePanel(VdkVObjEvent &);

    /// \brief 最小化窗口
    static void IconizePanel(VdkVObjEvent &);

private:

    /// \brief 处理鼠标事件
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);

    DECLARE_DYNAMIC_VOBJECT
};
