/***************************************************************
 * Name:      VdkCtrlParserInfo.cpp
 * Purpose:   XRC 解释过程的调用信息
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2011-02-22
 * Copyright: Wang Xiaoning
 **************************************************************/
#include "StdAfx.h"
#include "VdkCtrlParserInfo.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkCtrlParserInfo::VdkCtrlParserInfo() {
    memset(this, 0, sizeof(VdkCtrlParserInfo));
}

VdkCtrlIdInfo::VdkCtrlIdInfo(VdkCtrlId id, VdkControl **ptr)
    : Id(id), Ptr(ptr) {
}
