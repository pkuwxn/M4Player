/***************************************************************
 * Name:      VdkArtProvider.cpp
 * Purpose:   Code for VdkArtProvider implementation
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-04-07
 * Copyright: vanxining
 **************************************************************/
#include "StdAfx.h"
#include "VdkArtProvider.h"
#include "VdkUtil.h"/*
#include "Images/Office2003_icons.h"*/

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


// 每个方向上小位图的数目
const static int gs_bitmapsPerOrient = 50;

//////////////////////////////////////////////////////////////////////////

VdkArtProvider::VdkArtProvider() {
    /*
    wxBitmap Office2003_icons
       ( VdkLoadPngFromRawData( Office2003_icons_png ) );
    m_ba.Set( Office2003_icons, gs_bitmapsPerOrient, gs_bitmapsPerOrient );*/
}

wxBitmap VdkArtProvider::CreateBitmap(const wxArtID &id,
                                      const wxArtClient &client, const wxSize &size) {
    return wxArtProvider::CreateBitmap(id, client, size);
}

VdkBitmapArrayId VdkArtProvider::GetBitmaplet(int x, int y) {
    return VdkBitmapArrayId(m_ba, x, y);
}
