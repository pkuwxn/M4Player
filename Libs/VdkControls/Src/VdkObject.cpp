#include "StdAfx.h"
#include "VdkObject.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkObject *VdkObject::CreateDynamicVObject(const wxString &strClassName) {
    static OBJECT_MAP &Creators = GetCreators();
    OBJECT_MAP::iterator omi;

    omi = Creators.find(strClassName);

    if (omi != Creators.end()) {
        return omi->second();
    } else {
        return NULL;
    }
}

inline OBJECT_MAP &VdkObject::GetCreators() {
    static OBJECT_MAP Creators;

    return Creators;
}

AddToHashMap::AddToHashMap(const wxString &strClassName,
                           VdkObjectCreator Creator) {
    VdkObject::GetCreators()[ strClassName ] = Creator;
}
