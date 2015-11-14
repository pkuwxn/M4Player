/***************************************************************
 * Name:      OOPSingleHttpThread.h
 * Purpose:   ���� HTTP �������̵߳��嶨��
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

    // ��Ҫ���κ����飬�� atexit() �׶�ɾ���߳�ʵ��̫����
	static void Destroy(VdkHttpThread* p) {}
};

typedef Loki::SingletonHolder< VdkHttpThread, 
							   OOPHttpThreadCreator >
	OOPSingleHttpThread;
