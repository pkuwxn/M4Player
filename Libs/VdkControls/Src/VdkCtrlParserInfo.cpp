/***************************************************************
 * Name:      VdkCtrlParserInfo.cpp
 * Purpose:   XRC 解释过程的调用信息
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-02-22
 * Copyright: vanxining
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
