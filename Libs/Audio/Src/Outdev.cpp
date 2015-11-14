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

#include "../Outdev.hpp"
#include <memory.h> // for memset()
#include <stdio.h> // for fprintf(), stderr


#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif


namespace sf {
////////////////////////////////////////////////////////////
void SoftwareVolumePolicy::adjustAmp(Int16 *samples, Uint32 bytes, Output::SampleFormat fmt) {
    Int8 *stream = (Int8 *)samples;

    if (m_volume == 1) {
        return;
    } else if (m_volume == 0) {
        memset(stream, 0, bytes);
        return;
    }

    if (fmt.getBits() == 16) {
        Int16 ivolume = static_cast<Int16>(m_volume * 1000);
        for (Uint32 i = 0; i < bytes / 2; i++) {
            Int16 sample = *((Int16 *)stream);
            *((Int16 *)stream) = (Int16)(((Int32)sample) * ivolume / 1000);
            stream += 2;
        }
    } else if (fmt.getBits() == 8) {
        Int16 ivolume = static_cast<Int16>(m_volume * 255);
        for (Uint32 i = 0; i < bytes; i++) {
            *stream = (Int8)(((Int32)(*stream)) * ivolume / 1000);
            stream++;
        }
    } else if (fmt.getBits() == 24) {
        Int16 ivolume = static_cast<Int16>(m_volume * 1000);
        for (Uint32 i = 0; i < bytes / 3; i++) {
            Int32 sample = ((Uint8)stream[0]) | ((Uint8)stream[1]<<8) | (stream[2]<<16);
            Int32 newsample = (Int64)sample * ivolume / 1000;
            stream[0] = (newsample & 0x0000ff);
            stream[1] = (newsample & 0x00ff00)>>8;
            stream[2] = (newsample & 0xff0000)>>16;
            stream += 3;
        }
    } else if ((fmt.getBits() == 32) && !fmt.isFloat()) {
        Int16 ivolume = static_cast<Int16>(m_volume * 1000);
        for (Uint32 i = 0; i < bytes / 4; i++) {
            Int32 sample = *((Int32 *)stream);
            Int32 newsample = (Int64)sample * ivolume / 1000;
            *((Int32 *)stream) = newsample;
            stream += 4;
        }
    } else if ((fmt.getBits() == 32) && fmt.isFloat()) {
        float fvolume = m_volume;
        for (Uint32 i = 0; i < bytes / 4; i++) {
            *((float *)stream) = (*((float *)stream)) * fvolume;
            stream += 4;
        }
    } else {
        fprintf(stderr, "[%s:%d]Unsupported audio format.\n", __FILE__, __LINE__);
    }
}


} // namespace sf
