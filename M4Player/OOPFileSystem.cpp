/***************************************************************
 * Name:      OOPFileSystem.cpp
 * Purpose:   管理 OOPlayer 的路径
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-04-19
 **************************************************************/
#include "StdAfx.h"
#include "OOPFileSystem.h"

#include <wx/filename.h> // for wxFileName
#include <wx/stdpaths.h> // for wxStandardPaths

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

wxString OOPFileSystem::GetRootPath() {
    wxStandardPaths &stdPaths = wxStandardPaths::Get();
    wxFileName path(stdPaths.GetExecutablePath());

    wxString exePath(path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR));
#ifdef __WXDEBUG__
    return exePath + L"../../../Assets/";
#else
    return exePath;
#endif // __WXDEBUG__
}

wxString OOPFileSystem::GetSkinRootDir() {
    return GetRootPath() + L"Skins" + wxFileName::GetPathSeparator();
}

wxString OOPFileSystem::GetAppResDir() {
    return GetRootPath() + L"App" + wxFileName::GetPathSeparator();
}
