/***************************************************************
 * Name:      OOPSingleHttpThread.h
 * Purpose:   独立 HTTP 工作者线程单体定义
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-25
 **************************************************************/
#pragma once
#include "VdkHttpBuiltInImpl.h"
#include "VdkHttpThread.h"
#include <loki/Singleton.h>

template< class T > struct OOPHttpThreadCreator;
template<> struct OOPHttpThreadCreator< VdkHttpThread >
{
	static VdkHttpThread* Create()
	{
		wxCSConv gb2312( wxFONTENCODING_CP936 );
        VdkHttpBuiltInImpl* http = new VdkHttpBuiltInImpl( gb2312 );

		return new VdkHttpThread( http );
	}

    // 不要做任何事情，在 atexit() 阶段删除线程实在太晚了
	static void Destroy(VdkHttpThread* p) {}
};

typedef Loki::SingletonHolder< VdkHttpThread, 
							   OOPHttpThreadCreator >
	OOPSingleHttpThread;
