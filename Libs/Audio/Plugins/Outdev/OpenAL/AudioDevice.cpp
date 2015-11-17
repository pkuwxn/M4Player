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
#include "AudioDevice.hpp"
#include "ALCheck.hpp"
#include <Codec.hpp> // for Codec::SampleFormat
#include <stdio.h> // for printf()

#include <Debug.hpp>


////////////////////////////////////////////////////////////
AudioDevice::AudioDevice()
{
    // Create the device
#ifndef _WIN32
    m_audioDevice = alcOpenDevice("PulseAudio Software");
    if (m_audioDevice)
    {
        printf("[%s:%d] Use PulseAudio.\n", __FILE__, __LINE__);
    }
    else
#endif
    {
        m_audioDevice = alcOpenDevice(NULL);
    }

    if (m_audioDevice)
    {
        // Create the context
        m_audioContext = alcCreateContext(m_audioDevice, NULL);

        if (m_audioContext)
        {
            // Set the context as the current one (we'll only need one)
            alCheck(alcMakeContextCurrent(m_audioContext));
        }
        else
        {
            fprintf(stderr, "[%s:%d] Failed to create the audio context.\n",
					__FILE__, __LINE__);
        }
    }
    else
    {
        fprintf(stderr, "[%s:%d] Failed to open the audio device.\n", 
                __FILE__, __LINE__);
    }
}


////////////////////////////////////////////////////////////
AudioDevice::~AudioDevice()
{	/*
	// see https://gist.github.com/LaurentGomila/SFML/issues/30
	// Access Violation error int the destructor of sf::AudioDevice//*/
#ifndef _WIN32
	// Destroy the context
	alCheck(alcMakeContextCurrent(NULL));
	if (m_audioContext)
		alCheck(alcDestroyContext(m_audioContext));

	// Destroy the device
	if (m_audioDevice)
		alCheck(alcCloseDevice(m_audioDevice));
#endif
}


////////////////////////////////////////////////////////////
bool AudioDevice::isExtensionSupported(const std::string& extension)
{
    if ((extension.length() > 2) && (extension.substr(0, 3) == "ALC"))
        return alcIsExtensionPresent(m_audioDevice, extension.c_str()) != AL_FALSE;
    else
        return alIsExtensionPresent(extension.c_str()) != AL_FALSE;
}


////////////////////////////////////////////////////////////
int AudioDevice::getFormatFromChannelCount(unsigned int channelCount)
{
    // Find the good format according to the number of channels
    switch (channelCount)
    {
        case 1  : return AL_FORMAT_MONO16;
        case 2  : return AL_FORMAT_STEREO16;
        case 4  : return alGetEnumValue("AL_FORMAT_QUAD16");
        case 6  : return alGetEnumValue("AL_FORMAT_51CHN16");
        case 7  : return alGetEnumValue("AL_FORMAT_61CHN16");
        case 8  : return alGetEnumValue("AL_FORMAT_71CHN16");
        default : return 0;
    }
}


////////////////////////////////////////////////////////////
int AudioDevice::getFormat(unsigned int channelCount, unsigned int bps)
{
	// Find the good format according to the given format
	switch (bps)
	{
	case sf::Codec::SAMPLE_FMT_U8  :
		return (channelCount == 2) ? AL_FORMAT_STEREO8  : AL_FORMAT_MONO8;
	case sf::Codec::SAMPLE_FMT_S16 :
		return (channelCount == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
	default : return 0;
	}
}
