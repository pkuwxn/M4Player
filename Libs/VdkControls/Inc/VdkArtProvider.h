/***************************************************************
 * Name:      VdkArtProvider.h
 * Purpose:   Code for VdkArtProvider declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-04-07
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include <wx/artprov.h>
#include "VdkBitmapArray.h"

/// \brief 自画的文本框控件
///
/// 规避极度麻烦的自画控件与原生控件混合使用时的焦点转移问题。
class VdkArtProvider : public wxArtProvider {
public:

    /// \brief 构造函数
    VdkArtProvider();

    /// \brief 获取指定位置上的小位图
    VdkBitmapArrayId GetBitmaplet(int x, int y);

protected:

    /// \brief 获取指定位图
    wxBitmap CreateBitmap(const wxArtID &id,
                          const wxArtClient &client,
                          const wxSize &size);

private:

    VdkBitmapArray m_ba;
};
