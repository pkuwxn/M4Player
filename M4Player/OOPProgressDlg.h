/***************************************************************
 * Name:      OOPProgressDlg.cpp
 * Purpose:   进度显示对话框
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-8-19
 **************************************************************/
#pragma once
#include "VdkWindowImpl.h"

class VdkLabel;

/// \brief 进度显示对话框
class OOPProgressDlg : public VdkDialog {
public:

    /// \brief 构造函数
    OOPProgressDlg(wxWindow *parent,
                   const wxString &title,
                   const wxString &msg,
                   int maximum = 100);

    /// \brief 析构函数
    ~OOPProgressDlg();

    /// \brief 更新当前进度值
    /// \param value 当前进度值
    /// \param newmsg 新的提示字符串，假如为空，保持原来的不变
    /// \param skip 假如用户单击了“跳过”按钮，那么本函数最后一次的调用此值
    /// 会设为真
    /// \return 更新操作是否成功，若用户单击了“取消”按钮，则操作会失败
    virtual bool Update(int value,
                        const wxString &newmsg = wxEmptyString,
                        bool *skip = NULL);

    /// \brief 操作是否已被用户取消
    bool IsCanceled() const {
        return m_canceled;
    }

    /// \brief 清除对话框状态
    void ClearState();

    /// \brief 获取最大值
    int GetMaximum() const;

    /// \brief 获取当前进度值
    int GetValue() const;

private:

    // 原生关闭事件
    void OnClose(wxCloseEvent &);

    // 按下了界面上的“取消”按钮
    void OnCancel(VdkVObjEvent &);

private:

    VdkLabel *m_label;

    int m_maximum;
    int m_value;
    bool m_skipped;
    bool m_canceled;
};
