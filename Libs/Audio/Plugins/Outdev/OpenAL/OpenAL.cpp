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
#include "OpenAL.hpp"
#include "ALCheck.hpp"
#include "AudioDevice.hpp"
#include "Codec.hpp"

#include <assert.h>
#include <stdio.h> // for printf()

#include <Debug.hpp>




////////////////////////////////////////////////////////////
enum {
    MY_INVALID_SOURCE_ID = ~0, // TODO: 是否可以？
};


////////////////////////////////////////////////////////////
OpenAL::OpenAL() :
    OutputHelper<VolumePolicy>("OpenAL"),
    m_source(MY_INVALID_SOURCE_ID),
    m_alFormat(0),
    m_isStreaming(false),
    m_requestStop(false) {

}


////////////////////////////////////////////////////////////
OpenAL::~OpenAL() {
    close();
}


////////////////////////////////////////////////////////////
bool OpenAL::open(Uint32) {
    alCheck(alGenSources(1, &m_source));
    alCheck(alSourcei(m_source, AL_BUFFER, 0));

    return isOk();
}


////////////////////////////////////////////////////////////
void OpenAL::close() {
    if (m_source != MY_INVALID_SOURCE_ID) {
        // Stop the sound if it was playing
        stop();

        alCheck(alSourcei(m_source, AL_BUFFER, 0));
        alCheck(alDeleteSources(1, &m_source));

        m_source = MY_INVALID_SOURCE_ID;
    }
}


////////////////////////////////////////////////////////////
bool OpenAL::isOk() const {
    return (m_source != MY_INVALID_SOURCE_ID);
}


////////////////////////////////////////////////////////////
void *OpenAL::getBuffer() {
    return NULL;
}


////////////////////////////////////////////////////////////
bool OpenAL::setSampleFormat(SampleFormat fmt) {
    // Deduce the format from the number of channels
    m_alFormat = m_device.getFormat(fmt);

    // Check if the format is valid
    if (m_alFormat == 0) {
        fprintf(stderr, "[%s:%d] Unsupported audio format: channels: %d, bps: %d.\n",
                __FILE__, __LINE__, fmt.channels, fmt.getBits());

        return false;
    }

    m_fmt = fmt;
    return true;
}


////////////////////////////////////////////////////////////
bool OpenAL::play(Codec *decoder) {
    // Check if the sound parameters have been set
    if (m_alFormat == 0) {
        fprintf(stderr, "[%s:%d]Failed to play audio stream: sound parameters have not been "
                "initialized (call Output::open() first).\n",
                __FILE__, __LINE__);

        return false;
    }

    // If the sound is already playing (probably paused), just resume it
    if (m_isStreaming) {
        assert(m_decoder == decoder);

        if (getQueuedCount() == 0) {
            fillQueue();
        }

        alCheck(alSourcePlay(m_source));

        return false;
    }

    //=================================================

    m_decoder = decoder;

    // Resize the buffer so that it can contain 1 second of audio samples
    m_samples.resize(m_fmt.sampleRate * m_fmt.channels);

    // 这个操作原来是在 newThread() 中做的
    m_isStreaming = true;

    //=================================================
    // 开始播放

    // Create the buffers
    alCheck(alGenBuffers(BufferCount, m_buffers));

    // Fill the queue
    fillQueue();

    // Play the sound
    alCheck(alSourcePlay(m_source));

    // 要求在新的线程中继续后面的播放
    return true;
}


////////////////////////////////////////////////////////////
bool OpenAL::write(void *buffer, Uint32 bufferSize) {
    return false;
}


////////////////////////////////////////////////////////////
void OpenAL::pause() {
    alCheck(alSourcePause(m_source));
}


////////////////////////////////////////////////////////////
void OpenAL::stop() {
    if (m_decoder) {
        m_decoder = NULL;
        m_isStreaming = false;
        m_requestStop = false;

        // Stop the playback
        alCheck(alSourceStop(m_source));

        // Unqueue any buffer left in the queue
        clearQueue();

        // Delete the buffers
        alCheck(alSourcei(m_source, AL_BUFFER, 0));
        alCheck(alDeleteBuffers(BufferCount, m_buffers));
    }

    assert(getStatus() == Stopped);
}


////////////////////////////////////////////////////////////
void OpenAL::preSetPlayingOffset(Time) {

}


////////////////////////////////////////////////////////////
void OpenAL::setPlayingOffset(Time) {
    flushAndRefill();
}


////////////////////////////////////////////////////////////
Time OpenAL::getPlayingOffset() const {
    assert(false);
    return seconds(0);
}


////////////////////////////////////////////////////////////
void OpenAL::setVolume(float volume) {
    alCheck(alSourcef(m_source, AL_GAIN, volume));
}


////////////////////////////////////////////////////////////
float OpenAL::getVolume() const {
    ALfloat gain;
    alCheck(alGetSourcef(m_source, AL_GAIN, &gain));

    return gain;
}


////////////////////////////////////////////////////////////
Output::Status OpenAL::getStatus() const {
    Status status = doGetStatus();

    // To compensate for the lag between play() and alSourceplay()
    if ((status == Stopped) && m_isStreaming) {
        // 在“暂停”时改变当前播放进度
        if (getQueuedCount() == 0) {
            status = Paused;
        } else {
            status = Playing;
        }
    }

    return status;
}


////////////////////////////////////////////////////////////
Output::Status OpenAL::doGetStatus() const {
    ALint status;
    alCheck(alGetSourcei(m_source, AL_SOURCE_STATE, &status));

    switch (status) {
    case AL_INITIAL :
    case AL_STOPPED :
        return Stopped;
    case AL_PAUSED :
        return Paused;
    case AL_PLAYING :
        return Playing;
    }

    return Stopped;
}


////////////////////////////////////////////////////////////
bool OpenAL::streamData() {
    // Get the number of buffers that have been processed
    // (ie. ready for reuse)
    ALint nbProcessed;
    alCheck(alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &nbProcessed));

    ALint rest = nbProcessed;
    while (rest--) {
        // Pop the first unused buffer from the queue
        ALuint buffer;
        alCheck(alSourceUnqueueBuffers(m_source, 1, &buffer));

        // Find its index
        Uint32 bufferIndex = 0;
        for (int i = 0; i < BufferCount; ++i) {
            if (m_buffers[i] == buffer) {
                bufferIndex = i;
                break;
            }
        }

        // Fill it and push it back into the playing queue
        if (!m_requestStop) {
            fillAndPushBuffer(bufferIndex);
        }

    } // END while (rest--)

    // 因为缓存用尽而停止播放
    if ((nbProcessed == 0) || (nbProcessed == BufferCount)) {
        if (doGetStatus() == Stopped) {
            alCheck(alSourcePlay(m_source));
        }
    }

    m_isStreaming = !m_requestStop;
    return m_isStreaming;
}


////////////////////////////////////////////////////////////
/// \brief Structure defining a chunk of audio data to stream
///
////////////////////////////////////////////////////////////
struct Chunk {
    const Int16    *samples;     ///< Pointer to the audio samples
    Uint32          sampleCount; ///< Number of samples pointed by Samples
};


////////////////////////////////////////////////////////////
void OpenAL::fillAndPushBuffer(Uint32 bufferIndex) {
    // Acquire audio data
    Chunk data = { NULL, 0 };
    m_requestStop = !getData(data);

    // Fill the buffer if some data was returned
    if (data.samples && data.sampleCount) {
        Uint32 buffer = m_buffers[bufferIndex];

        // Fill the buffer
        ALsizei size = static_cast<ALsizei>(data.sampleCount) * sizeof(Int16);
        alCheck(alBufferData(buffer, m_alFormat, data.samples, size, m_fmt.sampleRate));

        // Push it into the sound queue
        alCheck(alSourceQueueBuffers(m_source, 1, &buffer));
    }
}


////////////////////////////////////////////////////////////
void OpenAL::fillQueue() {
    // Fill and enqueue all the available buffers
    for (int i = 0; (i < BufferCount) && !m_requestStop; ++i) {
        fillAndPushBuffer(i);
    }
}


////////////////////////////////////////////////////////////
void OpenAL::clearQueue() {
    // Get the number of buffers still in the queue
    ALint nbQueued;
    alCheck(alGetSourcei(m_source, AL_BUFFERS_QUEUED, &nbQueued));

    // Unqueue them all
    ALuint buffer;
    for (ALint i = 0; i < nbQueued; ++i) {
        alCheck(alSourceUnqueueBuffers(m_source, 1, &buffer));
    }

#ifdef _DEBUG
    alCheck(alGetSourcei(m_source, AL_BUFFERS_QUEUED, &nbQueued));
    assert(nbQueued == 0);
#endif
}

class Tracker {
public:

    Tracker() {
        printf("Enter OpenAL::getData().\n");
    }
    ~Tracker() {
        printf("Exit OpenAL::getData().\n");
    }
};

////////////////////////////////////////////////////////////
bool OpenAL::getData(Chunk &data) {
    //Tracker tracker;

    // Fill the chunk parameters
    data.samples     = &m_samples[0];
    data.sampleCount = m_decoder->read(&m_samples[0], m_samples.size());

    // Check if we have reached the end of the audio file
    return data.sampleCount == m_samples.size();
}


////////////////////////////////////////////////////////////
Int32 OpenAL::getQueuedCount() const {
    // Get the number of buffers still in the queue
    ALint nbQueued;
    alCheck(alGetSourcei(m_source, AL_BUFFERS_QUEUED, &nbQueued));

    return nbQueued;
}


////////////////////////////////////////////////////////////
void OpenAL::flushAndRefill() {
    assert(m_decoder);
    assert(m_isStreaming);

    // 是否正在播放
    bool isPlaying = (doGetStatus() == Playing);

    // 必须总是停止播放才能更新缓冲区队列
    alCheck(alSourceStop(m_source));

    // Fill the queue
    clearQueue();

    // Play the sound
    if (isPlaying) {
        fillQueue();
        alCheck(alSourcePlay(m_source));

        // “暂停”时改变进度的情况不要填充缓冲区队列，这样可以作为一种标识，
        // 在 getStatus() 中能正确识别这种情况
    }
}


////////////////////////////////////////////////////////////
#include "Client.hpp"
DEFINE_STD_OOPLUGIN(OpenAL, "Outdev;")
