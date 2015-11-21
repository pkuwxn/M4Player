/***************************************************************
 * Name:      OOPWindow.h
 * Purpose:   声明 OOPWindow
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-2-27
 **************************************************************/
#pragma once
#include "VdkWindowImpl.h"

class OOPStickyWindowState;

/// \brief OOPlayer 专用的可换肤窗口
class OOPWindow : public VdkDialog {
public:

    typedef VdkDialog Super;

    enum {
        /*! 默认 VdkWindow 风格 */
        DEFAULT_VSTYLE = VWS_DRAGGABLE | VWIS_DRAG_BY_SPACE,
    };

    /// \brief 构造函数
    OOPWindow(wxWindow *parent, const wxString &uuid, const wxString &title,
              long style = 0, long vstyle = DEFAULT_VSTYLE);

    /// \brief 析构函数
    ~OOPWindow();

    /// \brief 获取窗口的 UUID
    wxString GetUUID() const {
        return m_uuid;
    }

    /// \brief 重置皮肤
    void ResetSkin();

    /// \brief 处理与主窗口的粘附
    void NotifyShowHideAndStick(bool show);

    /// \brief 获取当前粘附状态信息对象用于持久化
    void UpdateStickyState(OOPStickyWindowState &sstate) const;

protected:

    /// \brief 设置窗口所用的皮肤
    void SetSkin();

    /// \brief 绑定“关闭”按钮的事件处理器
    void BindCloseBtnEvent(const wxString &objName = L"close");

private:

    // 改动 XRC 定义，使得窗口创建时大小即为上次关闭时的大小
    virtual void HackXrc(wxXmlNode *winRoot);

    // 重设皮肤前的准备工作
    virtual void PreResetSkin() {}

    // 设置窗口所用的皮肤
    // 派生类必须实现。
    virtual void DoSetSkin() = 0;

    // 重置皮肤事件响应函数
    void OnResetSkin(VdkVObjEvent &);

    //-----------------------------------------

    // 设定是否粘附到主窗口
    void SetStickyToMainState(bool sticky);

    // 获取(相对于主窗口的)窗口作用域用于持久化
    wxRect GetRectToSerialize() const;

    // 粘附到主窗口
    void OnAttachToMain(wxCommandEvent &);
    // 从主窗口脱离
    void OnDetachFromMain(wxCommandEvent &);

    //-----------------------------------------

    // 响应原生窗口关闭事件(Alt + F4激发)
    void OnCloseNative(wxCloseEvent &);

    // 响应关闭按钮激发的窗口关闭事件
    void OnClose(VdkVObjEvent &e);

    // 关闭窗口，更新状态
    void DoCloseWindow();

private:

    // 最小化(隐藏到托盘)
    virtual void DoMinimize();

private:

    // 标识窗口的 UUID
    wxString m_uuid;

    // 隐藏前是否已然粘附到主窗口
    bool m_stickyToMain;
    // 隐藏前已然粘附到主窗口时相对主窗口的位置
    wxPoint m_relativePos;
};
