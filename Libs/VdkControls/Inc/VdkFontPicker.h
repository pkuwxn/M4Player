#pragma once
#include "VdkButton.h"

/// \brief 字体选择器
class VdkFontPicker : public VdkButton {
public:

    /// \brief 默认构造函数
    VdkFontPicker() {}

    /// \brief XRC 动态创建
    virtual void Create(wxXmlNode *node);

    /// \brief 设置当前字体
    const wxFont &SetSelectedFont(const wxString &strDesc, wxDC *pDC);

    /// \brief 设置当前字体
    const wxFont &SetSelectedFont(const wxFont &font, wxDC *pDC);

    /// \brief 获取用户已选的字体
    const wxFont &GetSelectedFont() const {
        return m_selectedFont;
    }

    /// \brief 根据用户已选的字体得到可读文本
    wxString BuildString(wxFont *pFont = NULL);

    /// \brief 根据可读文本生成指定的字体
    static wxFont GetFontFromDescString(const wxString &strDesc);

private:

    // 打开字体选择对话框
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);

    //////////////////////////////////////////////////////////////////////////

    wxFont m_selectedFont; // 选中的字体

    DECLARE_DYNAMIC_VOBJECT
};
