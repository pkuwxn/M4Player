#pragma once
#include "VdkControl.h"
#include "VdkSlider.h"

/// \brief 为 VdkScrollBar 设置初始化信息
class VdkScrollBarInitializer : public VdkSliderInitializer {
public:

    /// \brief 构造函数
    VdkScrollBarInitializer() {}

    friend class VdkScrollBar;
};

//////////////////////////////////////////////////////////////////////////

/*!\brief 滚动条的 VDK 实现
 *
 * 目前只实现了垂直滚动条。
**/
class VdkScrollBar : public VdkControl {
public:

    /// \brief 默认构造函数
    VdkScrollBar() {
        Init();
    }

    /// \brief XRC 动态创建
    virtual void Create(wxXmlNode *node);

    /// \brief 构建控件
    void Create(const VdkScrollBarInitializer &init_data);

    /// \brief 创建位图数组
    wxBitmap *GetBitmapArray(wxString strFileNames[3], int &nExpl);

    /////////////////////////////////////////////////////////////////////////

    /// \brief 得到当前所处的百分比
    double GetProgress();

    /// \brief 得到手柄高度
    int GetThumbHeight();

    /// \brief 手柄当前是否可见
    bool IsThumbOnShow() const;

    /// \brief 显示/隐藏手柄
    void ShowThumb(bool bShow, wxDC *pDC);

    /// \brief 设置手柄高度
    void SetThumbHeight(int height, wxDC *pDC);

    /// \brief 移动到指定百分比
    void GoTo(double percentage, wxDC *pDC);

private:

    /// \brief 初始化控件
    void Init();

    /// \brief 处理鼠标事件
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);

    /// \brief 接收、处理通知信息
    virtual void DoHandleNotify(const VdkNotify &notice);

    /////////////////////////////////////////////////////////////////////////

    int             m_nWidth;       ///< 滚动条宽度

    VdkSlider      *m_pSlider;      ///< 滚动功能的主要实现者 VdkSlider

    VdkButton      *m_pGoUp;        ///< “向上滚动”按钮
    VdkButton      *m_pGoDown;      ///< “向下滚动”按钮

    /////////////////////////////////////////////////////////////////////////

    /// \brief 滚动事件是否来源于鼠标滚轮或者拖动 VdkSlider
    ///
    /// 在滚动条上持续按住左键也可以使滚动条不断移动。但这样会带来一个问题。
    /// 就是当滚动条手柄的高度足够小，使之滚动一次就从鼠标指针的另一侧跳跃到另一侧，
    /// 根据我的算法，那么下一次周期，假使用户仍然舍不得放开鼠标左键，
    /// 滚动条就应该向与用户的初衷方向相反的进发，于是手柄跳到原先的一侧。如此死循环。
    /// 所以我加了一个判断，就是按住左键时，手柄只能向一个固定的方向，
    /// 即上文所说的“初衷方向”移动。但很明显，滚动鼠标滚轮也会使这个“初衷方向”改变，
    /// 而且无法将这种状态“置零”，因为滚动一次滚轮就只有一个事件，与按下按键不同，
    /// 无法及时更改事件状态。
    bool            m_bNotHoldOn;

    DECLARE_CLONEABLE_VOBJECT(VdkScrollBar)
};
