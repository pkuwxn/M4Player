///////////////////////////////////////////////////////////////////////////////
// Name:        VdkCtrlId.cpp
// Purpose:     控件 ID 相关
// Author:      Wang Xiaoning
// Created:     2011-02-26
///////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "VdkCtrlId.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


//////////////////////////////////////////////////////////////////////////

VdkCtrlId VdkGetUniqueId(int advanced) {
    static VdkCtrlId s_idLowest = - (wxID_HIGHEST * 2);
    VdkCtrlId ret = s_idLowest;
    s_idLowest += advanced;

    return ret;
}
