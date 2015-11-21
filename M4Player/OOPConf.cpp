/***************************************************************

 * Name:      OOPConf.cpp
 * Purpose:   OOPlayer 的所有配置信息
 * Author:    Wang Xiao Ning (vanxining@139.com)
 * Created:   2011-05-07
 **************************************************************/
#include "StdAfx.h"
#include "OOPConf.h"

#include "OOPDefs.h"
#include "wxUtil.h" // for ZeroRect()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

bool FromString(const wxString &raw, wxString &out) {
    out = raw;
    return true;
}

void ToString(const wxString &raw, wxString &out) {
    out = raw;
}

bool FromString(const wxString &raw, int &out) {
    return raw.ToLong((long *) &out);
}

void ToString(int raw, wxString &out) {
    out.Printf(L"%d", raw);
}

bool FromString(const wxString &raw, double &out) {
    return raw.ToDouble(&out);
}

void ToString(double raw, wxString &out) {
    out.Printf(L"%g", raw);
}

bool FromString(const wxString &raw, bool &out) {
    out = raw == L"1" || raw.Lower() == L"true";
    return true;
}

void ToString(bool raw, wxString &out) {
    out = raw ? L"true" : L"false";
}

bool FromString(const wxString &raw, OOPStickyWindowState &out) {
    wxRect &rc = out.m_rect;

    return wxSscanf(raw, L"%d,%d,%d,%d:%d",
                   &rc.x, &rc.y, &rc.width, &rc.height, &out.m_stickyToMain) == 5;
}

void ToString(const OOPStickyWindowState &raw, wxString &out) {
    wxRect rc(raw.GetRect());
    int sticky = raw.IsStickyToMain() ? 1 : 0;

    out.Printf(L"%d,%d,%d,%d:%d", rc.x, rc.y, rc.width, rc.height, sticky);
}

//////////////////////////////////////////////////////////////////////////

#define PROPERTIES \
    SERIALIZE( m_currSong, L"currSong" ); \
    SERIALIZE( m_playMode, L"playMode" ); \
    SERIALIZE( m_skinFolderName, L"skin" ); \
    SERIALIZE( m_playing, L"playing" ); \
    SERIALIZE( m_currPos, L"currPos" ); \
    SERIALIZE( m_showTaskBarIconUnderUnity, L"showTaskBarIconUnderUnity" ); \
    SERIALIZE( m_playListShown, L"playListShown" ); \
    SERIALIZE( m_lyricShown, L"lyricShown" ); \
    SERIALIZE( m_eqShown, L"eqShown" ); \
    SERIALIZE( m_volume, L"volume" ); \
    SERIALIZE( m_mute, L"mute" ); \
    SERIALIZE( mainPanelWndState, L"mainPanelWndState" ); \
    SERIALIZE( playlistPanelWndState, L"playlistPanelWndState" ); \
    SERIALIZE( lyricPanelWndState, L"lyricPanelWndState" ); \
    SERIALIZE( eqPanelWndState, L"eqPanelWndState" ); \


OOPConf::OOPConf()
    : m_currSong(wxNOT_FOUND),
      m_playMode(PLAY_MODE_BY_LIST),
      m_skinFolderName(L"X10"),
      m_playing(true),
      m_currPos(0),
      m_showTaskBarIconUnderUnity(false),
      m_playListShown(false),
      m_lyricShown(false),
      m_eqShown(false),
      m_volume(1),
      m_mute(false) {
}

#define NEXT(xmlNode) xmlNode = xmlNode->GetNext()

bool OOPConf::Load(const wxString &path) {
    wxXmlDocument doc;
    if (doc.Load(path)) {
        wxXmlNode *root = doc.GetRoot();
        if (root) {
            wxXmlNode *child = root->GetChildren();

            if (child && Player(child)) {
                NEXT(child);
            }

            return true;
        }
    }

    return false;
}

bool OOPConf::Save(const wxString &path) {
    wxXmlNode *root = new wxXmlNode(wxXML_ELEMENT_NODE, L"M4Player");
    root->AddChild(Player());

    wxXmlDocument doc;
    doc.SetRoot(root);

    return doc.Save(path);
}

#define SERIALIZE(var, xmlAttrName) \
    if (!node->GetAttribute(xmlAttrName, &raw)) { \
        return false; \
    } \
    if (!FromString(raw, var)) { \
        return false; \
    }

bool OOPConf::Player(const wxXmlNode *node) {
    if (node->GetName() != "Player") {
        return false;
    }

    wxString raw;
    PROPERTIES

    return true;
}

#undef SERIALIZE
#define SERIALIZE(var, xmlAttrName) \
    ToString(var, out); \
    node->AddAttribute(xmlAttrName, out);

wxXmlNode *OOPConf::Player() {
    wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, L"Player");

    wxString out;
    PROPERTIES

    return node;
}

void OOPConf::ClearStickyWindowStates() {
    OOPStickyWindowState *state = &mainPanelWndState;
    OOPStickyWindowState *end = state + NUM_PANELS;

    while (state != end) {
        state->Reset();
        state++;
    }
}

//////////////////////////////////////////////////////////////////////////

OOPStickyWindowState::OOPStickyWindowState()
    : m_stickyToMain(false) {
}

void OOPStickyWindowState::Reset() {
    m_stickyToMain = false;
    ZeroRect(m_rect);
}
