////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2012 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "../StopWatch.hpp"
#include <wx/time.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif


namespace sf {

StopWatch::StopWatch() :
    m_t0(0),
    m_elapsed(0),
    m_sliceStart(0),
    m_paused(true) { // Pause at startup
}

void StopWatch::Reset(Uint32 t0) {
    m_t0 = t0;
    m_elapsed = 0;
    m_sliceStart = wxGetLocalTimeMillis().GetValue();
    m_paused = true;
}

void StopWatch::Start(Uint32 t0) {
    Reset(t0);
    m_paused = false;
}

void StopWatch::Stop() {
    m_t0 = 0;
    m_elapsed = 0;
    m_sliceStart = 0;
    m_paused = true;
}

void StopWatch::Pause() {
    if (m_paused) {
        return;
    }

    m_elapsed += GetElapsedTime();
    m_paused = true;
}

void StopWatch::Resume() {
    if (!m_paused) {
        return;
    }

    m_sliceStart = wxGetLocalTimeMillis().GetValue();
    m_paused = false;
}

Uint32 StopWatch::Time() const {
    Uint32 ret = m_t0 + m_elapsed;
    if (!m_paused) {
        ret += GetElapsedTime();
    }

    return ret;
}

Uint32 StopWatch::GetElapsedTime() const {
    Int64 curr = wxGetLocalTimeMillis().GetValue();
    return curr - m_sliceStart;
}

} // namespace sf
