/***************************************************************
 * Name:      VdkCommon.h
 * Purpose:   一些数据结构的定义
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2011-07-27
 **************************************************************/
#pragma once

// 这个宏应该在类型声明之前使用，因为我们也进行了一次预声明
#define VDK_DECLARE_WX_PTR_LIST( T, Name, Iter ) \
    class T; \
    VDK_DECLARE_WX_PTR_LIST_NO_FORWARD_DECL( T, Name, Iter )

#define VDK_DECLARE_WX_PTR_LIST_NO_FORWARD_DECL( T, Name, Iter ) \
    WX_DECLARE_LIST_PTR( T, Raw ## Name ); \
    typedef Raw ## Name Name; \
    typedef Raw ## Name::iterator Iter; \
    typedef Raw ## Name::const_iterator Iter ## _Const;

#define VDK_DEFINE_WX_PTR_LIST( Name ) \
    void wx ## Raw ## Name ## Node::DeleteData() { \
        delete (_WX_LIST_ITEM_TYPE_ ## Raw ## Name *) GetData(); \
    }

#define VDK_DEFINE_INTERNAL_WX_PTR_LIST( superclass, listname ) \
    void superclass::wx ## Raw ## listname ## Node::DeleteData() { \
        delete (_WX_LIST_ITEM_TYPE_ ## Raw ## listname *) GetData(); \
    }
