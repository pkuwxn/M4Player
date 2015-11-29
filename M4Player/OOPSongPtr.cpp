/***************************************************************
 * Name:      OOPSongPtr.cpp
 * Purpose:   指向 OOPSong 的智能指针
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-05-29
 **************************************************************/
#include "StdAfx.h"
#include "OOPSongPtr.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

OOPSongPtr::OOPSongPtr(OOPSong *song)
    : m_song(song) {

}

OOPSongPtr::OOPSongPtr(const OOPSongPtr &other)
    : m_song(other.m_song) {

}

OOPSongPtr::~OOPSongPtr() {
    // TODO:
}

OOPSongPtr &OOPSongPtr::operator=(const OOPSongPtr &other) {
    m_song = other.m_song;
    return *this;
}

OOPSong *OOPSongPtr::operator->() const {
    return m_song.get();
}

OOPSong &OOPSongPtr::operator*() {
    return *m_song;
}

const OOPSong &OOPSongPtr::operator*() const {
    return *m_song;
}

template <>
bool operator == (wxSharedPtr<OOPSong> const &a, wxSharedPtr<OOPSong> const &b) {
    OOPSong *sa = a.get();
    OOPSong *sb = b.get();

    return sa && sb && (sa->path() == sb->path());
}
