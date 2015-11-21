/***************************************************************
 * Name:      VdkCtrlParserInfo.h
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-23
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkCtrlId.h"
#include <stdio.h>

class VdkWindow;
class VdkControl;
class VdkCtrlParserCallback;

class VdkCtrlIdInfo;
typedef VdkCtrlIdInfo *VdkCtrlIdInfoPtr;

WX_DECLARE_STRING_HASH_MAP(VdkCtrlIdInfoPtr, MapOfCtrlIdInfo);
typedef MapOfCtrlIdInfo::iterator IdInfoIter;

/// \brief XRC 解释过程的调用信息
class VdkCtrlParserInfo {
public:

    /// \brief 构造函数
    VdkCtrlParserInfo();

    /// \brief 设置下级控件所属的父窗口
    VdkCtrlParserInfo &window(VdkWindow *win) {
        Window = win;
        return *this;
    }

    /// \brief 设置下级控件所属的父控件
    VdkCtrlParserInfo &parent(VdkControl *p) {
        Parent = p;
        return *this;
    }

    /// \brief 设置 XRC 根节点
    ///
    /// 它的直接子节点应为有效的 XRC 定义，而非直接提供第一个字子控件的 XRC 定义。
    VdkCtrlParserInfo &node(wxXmlNode *n) {
        Node = n;
        return *this;
    }

    /// \brief 设置解析过程的回调对象
    VdkCtrlParserInfo &callback(VdkCtrlParserCallback *c) {
        Callback = c;
        return *this;
    }

    /// \brief 设置控件 ID - 名字 对应图
    VdkCtrlParserInfo &ids(MapOfCtrlIdInfo *ids_) {
        IDs = ids_;
        return *this;
    }

private:

    VdkWindow *Window;
    VdkControl *Parent;
    wxXmlNode *Node;
    VdkCtrlParserCallback *Callback;
    MapOfCtrlIdInfo *IDs;

    friend class VdkControl;
};

/// \brief 解释 XRC 文件时，每个控件的额外信息
///
/// 如控件 ID 、句柄等。
class VdkCtrlIdInfo {
public:

    /// \brief 构造函数
    VdkCtrlIdInfo(VdkCtrlId id, VdkControl **ptr = NULL);

    /// \brief 设置控件 ID
    VdkCtrlIdInfo &id(VdkCtrlId id) {
        Id = id;
        return *this;
    }

    /// \brief 获取控件 ID
    ///
    /// VdkWindow 补充设置指针所用。
    VdkCtrlId id() const {
        return Id;
    }

    /// \brief 获取控件句柄的地址
    ///
    /// VdkWindow 补充设置指针所用。
    VdkControl **ptr() const {
        return Ptr;
    }

    /// \brief 设置控件句柄的地址
    VdkCtrlIdInfo &ptr(VdkControl **ptr_) {
        Ptr = ptr_;
        return *this;
    }

    /// \brief 获取控件句柄
    template<class T>
    T *ptr() const {
        return (T *) *Ptr;
    }

private:

    VdkCtrlId Id;
    VdkControl **Ptr;

    friend class VdkControl;
};

//////////////////////////////////////////////////////////////////////////
// TODO: 添加注释

#define BindCtrl( name, id, ptr ) \
    BindCtrl_Named( ids, name, id, ptr )

#define BindCtrl_Named( map_, name, id, ptr ) \
    map_[(name)] = new VdkCtrlIdInfo( (id), (VdkControl**) &(ptr) )

#define BindCtrlID( name, id ) BindCtrlID_Named( ids, name, id )
#define BindCtrlID_Named( map_, name, id ) \
    map_[(name)] = new VdkCtrlIdInfo( (id) )
