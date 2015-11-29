/***************************************************************
 * Name:      VdkTextValidator.h
 * Purpose:   定义 VdkTextValidator 类
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2011-7-31
 **************************************************************/
#pragma once

/// \brief wxTextValidator 类只支持原生控件
class VdkTextValidator : public wxTextValidator {
public:

    /// \brief 复制构造函数
    VdkTextValidator(const VdkTextValidator &v);

    /// \brief 构造函数
    VdkTextValidator(long style = wxFILTER_NONE,
                     wxString *valPtr = NULL);

    /// \brief 复制自己
    virtual wxObject *Clone() const;

    /// \brief 复制自己
    bool Copy(const VdkTextValidator &v);

    /// \brief 验证指定字符串是否符合要求
    /// \param str 要验证的字符串
    /// \param parent 弹出错误对话框时对话框的父窗口
    virtual bool Validate(const wxString &str, wxWindow *parent);
};
