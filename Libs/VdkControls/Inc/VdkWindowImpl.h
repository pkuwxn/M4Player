/***************************************************************
 * Name:      VdkWindowImpl.h
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-13
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkWindow.h"
#include "VdkControl.h"

#include <wx/display.h>
#include <wx/xml/xml.h>

/// \brief VdkWindow 的适配类
///
/// 使 VdkWindow 能适配各种窗体类型，如 wxDialog、wxWindow、wxPanel 等。
template<class T>
class VdkWindowImpl: public T, public VdkWindow {
public:

    /// \brief 构造函数
    VdkWindowImpl(long style = 0)
        : VdkWindow(this, style) {

    }
};

//////////////////////////////////////////////////////////////////////////

/// \brief VdkWindow 的适配类中可以作为顶层窗口的适配类
///
/// 顶层窗口直接与窗口管理器进行交互，例如能最小化、最大化等。
template<class T>
class VdkTopLevelWindow : public VdkWindowImpl<T> {
public:

    /// \brief 构造函数
    VdkTopLevelWindow(wxWindow *parent,
                      const wxString &title,
                      long style,
                      long vstyle);

    /// \brief XRC 动态创建的便利函数
    virtual bool FromXrc(const wxString &xrcFileName, MapOfCtrlIdInfo *ids = NULL);

    /// \brief 引入窗口最大大小作为限制
    virtual wxSize GetMaxSize() const;

private:

    // 派生类可以在此对窗口 XRC 定义做一些特殊处理
    virtual void HackXrc(wxXmlNode *winRoot) {}

    // 改变窗口大小
    virtual void DoResize(int w, int h, int width, int height,
                          int sizeFlags = wxSIZE_AUTO);

    // 最小化
    virtual void DoMinimize() {
        this->Iconize(true);
    }
};

template<class T>
VdkTopLevelWindow<T>::VdkTopLevelWindow(wxWindow *parent,
                                        const wxString &title,
                                        long style,
                                        long vstyle)
    : VdkWindowImpl<T>(vstyle) {
    T::Create(parent, wxID_ANY, title, wxDefaultPosition,
              wxDefaultSize,
              style |
              wxFULL_REPAINT_ON_RESIZE |
              wxSYSTEM_MENU |
              wxMAXIMIZE_BOX |
              wxMINIMIZE_BOX |
              wxCLOSE_BOX |
              wxFRAME_SHAPED |
              wxBORDER_NONE |
              wxCLIP_CHILDREN/* |
               wxCLIP_SIBLINGS */); // 加了这个会出现边框

#ifdef __WXDEBUG__
    this->SetDebugCaption(title);
#endif // __WXDEBUG__
}

template<class T>
bool VdkTopLevelWindow<T>::FromXrc(const wxString &xrcFileName,
                                   MapOfCtrlIdInfo *ids) {
    wxString xrcPath(VdkControl::GetFilePath(xrcFileName));
    wxXmlDocument docSkin(xrcPath);
    wxXmlNode *root = docSkin.GetRoot();
    if ((root == NULL) || (root->GetName() != L"resource")) {
        wxLogError(L"皮肤 XML 文件不存在或者格式错误:\r\n%s", xrcPath);
        return false;
    }

    //-----------------------------------------------

    wxXmlNode *winRoot = root->GetChildren();

    HackXrc(winRoot);
    VdkWindow::Create(winRoot, ids);

    return true;
}

template<class T>
wxSize VdkTopLevelWindow<T>::GetMaxSize() const {
    return wxDisplay().GetClientArea().GetSize();
}

template<class T>
void VdkTopLevelWindow<T>::DoResize(int x, int y, int w, int h, int sizeFlags) {
    // 假如是相同的大小就直接 SetShape
    this->SetSize(x, y, w, h, sizeFlags);

    if (this->IsMaximized()) {
        // TODO: 层叠窗口时的情况
        this->SetShape(wxRegion());
    } else {
        this->m_bkCanvas.WindowSetShape
        (this, VdkControl::GetMaskColour(), VdkScalableBitmap::WSM_LAYERED);
    }
}

//////////////////////////////////////////////////////////////////////////

#define DEFINE_USABLE_WINDOW_CLASS( T ) \
class Vdk ## T : public VdkTopLevelWindow< wx ## T > \
{ \
public: \
\
    Vdk ## T(wxWindow* parent, \
             const wxString& title, \
             long style = 0, \
             long vstyle = VWS_DRAGGABLE) \
        : VdkTopLevelWindow< wx ## T >( parent, title, style, vstyle ) \
    {} \
};

DEFINE_USABLE_WINDOW_CLASS(Dialog)
DEFINE_USABLE_WINDOW_CLASS(Frame)
