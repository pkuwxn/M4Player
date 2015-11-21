#pragma once
#include "VdkControl.h"
#include <wx/vector.h>

//////////////////////////////////////////////////////////////////////////
/// \brief 滚动文本显示
class OOPLabel : public VdkControl, public wxTimer {
public:

    /// \brief 默认构造函数
    OOPLabel();

    /// \brief XRC 动态创建
    virtual void Create(wxXmlNode *node);

    /// \brief 实际构建函数
    void Create(VdkWindow *Window,
                const wxString &strName,
                const wxRect &Rect,
                const wxFont &font,
                const wxColour &color,
                const wxColour &bgColor,
                const align_type &align);

    /// \brief 析构函数
    ~OOPLabel();

    /// \brief 设置文本项
    void SetItems(const wxArrayString &items, wxDC *pDC);

    /// \brief 是否滚动文本控件有效
    bool IsOk() const {
        return !m_items.empty();
    }

    /// \brief 开始滚动
    void StartRolling();

    /// \brief 停止滚动，重置控件状态，删除一切信息
    void StopRolling(wxDC *pDC);

private:

    // 初始化控件
    void Init();

    // 绘制控件
    virtual void DoDraw(wxDC &dc);

    // 绘制标题
    void DrawLabel(const wxString &str,
                   int x,
                   int y,
                   const wxString &str2 = wxEmptyString,
                   int y2 = 0);

    // 继承自 wxTimer
    virtual void Notify();

    //////////////////////////////////////////////////////////////////////////

    enum TimerNotityType {

        TNT_LINE_SWITCH,
        TNT_LINE_ROLLING,
        TNT_WAIT_FOR_SWITCHING
    };

    wxColour                    m_TextColor;        // 文本颜色
    wxColour                    m_BkGndColor;       // 文本颜色
    wxString                    m_staticCaption;    // 停止滚动时显示的文本

    int                         m_nOnShowId;        // 正在显示的项目 ID
    TimerNotityType             m_nNotityType;      // Timer 通知的类型

    int                         m_nTextInternal;    // 文本离 OOPLabel 顶部的距离（使之居中）
    int                         m_nRollingCount;    // 当前文本向上或者向右滚动的次数
    wxDirection                 m_Direction;        // 当前滚动的方向
    wxStopWatch                 m_StopWatch;        // 切换上下句的时钟

    //////////////////////////////////////////////////////////////////////////

    struct ItemNode {

        int nTextWidth;         // 文本的长度
        int nDelta;             // 文本长度和显示区域长度相差的大小
        wxString strItem;       // 要显示的文本
    };

    typedef wxVector<ItemNode> ArrayOfItemNodes;
    ArrayOfItemNodes            m_items;            // 滚动显示的文本数组

    DECLARE_DYNAMIC_VOBJECT
};
