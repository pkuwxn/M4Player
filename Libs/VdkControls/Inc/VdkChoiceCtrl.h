#pragma once
#include "VdkMenuPopper.h"

class VdkMenu;
/// \brief ComboBox 的一种变种，不提供文本框编辑选中值
class VdkChoiceCtrl : public VdkMenuPopper {
public:

    /// \brief 构造函数
    VdkChoiceCtrl();

    /// \brief XRC 动态创建
    virtual void Create(wxXmlNode *node);

    /// \brief 担任主要构建工作
    /// \param Window 父窗口
    /// \param strName 控件名字（标识符）
    /// \param rt 控件作用域
    /// \param strChoirces 初始化后的所有选项（选项间使用半角分号“;”分割）
    void Create(VdkWindow *Window,
                const wxString &strName,
                const wxRect &rc,
                const wxString &strChoices);

    //////////////////////////////////////////////////////////////////////////

    /// \brief 在\a index 指定的位置插入一个项目
    void InsertItem(int index, const wxString &strLabel);

    /// \brief 选择指定序号的项
    void Select(int index, wxDC *pDC);

    /// \brief 得到已选项的 ID
    /// \return 假如当前无选中项，返回-1
    int GetSelected() const {
        return m_selected;
    }

    /// \brief 得到上次选中项的 ID
    /// \return 假如上次无选中项，返回-1
    int GetLastSelected() const {
        return m_lastSelected;
    }

private:

    /// \brief 响应菜单项选择
    void OnDropDownMenu(VdkVObjEvent &);

    int m_selected;
    int m_lastSelected;

    DECLARE_DYNAMIC_VOBJECT
};
