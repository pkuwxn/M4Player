/***************************************************************
 * Name:      OOPSingleLyricTask.h
 * Purpose:   歌词下载器单体定义
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-03-28
 **************************************************************/
#pragma once
#include "LyricGrabber/LyricGrabber.h"
#include <loki/Singleton.h>

template<class T> struct OOPLyricTaskCreator;

template<> struct OOPLyricTaskCreator<LyricGrabber::Task> {
    static LyricGrabber::Task *Create() {
        return new LyricGrabber::Task(wxTheApp);
    }

    static void Destroy(LyricGrabber::Task *p) {
        delete p;
    }
};

typedef Loki::SingletonHolder
    <LyricGrabber::Task, OOPLyricTaskCreator> OOPSingleLyricTask;
