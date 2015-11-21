/***************************************************************
 * Name:      OOPUtil.cpp
 * Purpose:   Code for some useful functions
 * Author:    Ning (vanxining@139.com)
 * Created:   2010-02-01
 * Copyright: Ning
 **************************************************************/
#include "StdAfx.h"
#include "OOPUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

void CheckXmlConf(wxXmlNode *xmlNode, const wxChar *szNode, int errNo) {
    if (!xmlNode) {
        wxLogError(L"Error!The <%s> node doesn't exist.\n", szNode);
        exit(errNo);
    }
}

wxScopedCharBuffer NarrowedPath(const wxString &path) {
#ifdef __WXMSW__
    return path.mb_str();
#else
    return path.utf8_str();
#endif
}
