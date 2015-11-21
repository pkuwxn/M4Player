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
#include "PulseAudio.hpp"
#include "Codec.hpp"
#include <stdio.h> // for fprintf()
#include <errno.h> // for errno
#include <string.h> // for strerror()

#include <dlfcn.h> // for dlopen()
#include <pulse/error.h>



////////////////////////////////////////////////////////////
/// pthread 互斥体自动锁
///
////////////////////////////////////////////////////////////
class MutexLocker {
public :

    // 构造函数
    MutexLocker(pthread_mutex_t &mutex) : m_mutex(mutex) {
        pthread_mutex_lock(&m_mutex);
    }

    // 析构函数
    ~MutexLocker() {
        pthread_mutex_unlock(&m_mutex);
    }

private :

    pthread_mutex_t &m_mutex;
};


////////////////////////////////////////////////////////////
namespace pa {
static pa_simple *(*simple_new)(const char *server,
                                const char *name,
                                pa_stream_direction_t dir,
                                const char *dev,
                                const char *stream_name,
                                const pa_sample_spec *ss,
                                const pa_channel_map *map,
                                const pa_buffer_attr *attr,
                                int *error);

static void (*simple_free)(pa_simple *s);
static int (*simple_read) (pa_simple *s, void *data, size_t bytes, int *error);
static int (*simple_write)(pa_simple *s, void *data, size_t bytes, int *error);
static int (*simple_drain)(pa_simple *s, int *error);

const char *(*strerror)(int error);

//===================================================

static bool isLoaded() {
    return simple_new != NULL;
}

static bool loadSymbols() {
    if (isLoaded()) {
        return true;
    }

    void *handle = dlopen("libpulse-simple.so.0", RTLD_LAZY); // TODO: lib name
    if (!handle) {
        fprintf(stderr, __FILE__": dlopen() failed: %s\n", dlerror());
        return false;
    }

    dlerror(); /* Clear any existing error */

    /* Writing: cosine = (double (*)(double)) dlsym(handle, "cos");
       would seem more natural, but the C99 standard leaves
       casting from "void *" to a function pointer undefined.
       The assignment used below is the POSIX.1-2003 (Technical
       Corrigendum 1) workaround; see the Rationale for the
       POSIX specification of dlsym(). */

    *(void **) (&simple_new) = dlsym(handle, "pa_simple_new");
    *(void **) (&simple_free) = dlsym(handle, "pa_simple_free");
    *(void **) (&simple_read) = dlsym(handle, "pa_simple_read");
    *(void **) (&simple_write) = dlsym(handle, "pa_simple_write");
    *(void **) (&simple_drain) = dlsym(handle, "pa_simple_drain");
    *(void **) (&strerror) = dlsym(handle, "pa_strerror");

    const char *error = dlerror();
    if (error) {
        fprintf(stderr, __FILE__": dlsym() failed: %s\n", error);
        return false;
    }

    return true;
}
}


////////////////////////////////////////////////////////////
PulseAudio::PulseAudio() :
    OutputHelper<VolumePolicy>("PulseAudio"),
    s(NULL),
    m_initialized(false),
    m_stopAtOnce(false),
    m_status(Stopped) {

}


////////////////////////////////////////////////////////////
PulseAudio::~PulseAudio() {
    close();
}


////////////////////////////////////////////////////////////
bool PulseAudio::open(Uint32 deviceId) {
    if (m_initialized) {
        return true; // 当作初始化成功
    }

    //===================================================

    if (!pa::loadSymbols()) {
        fprintf(stderr, __FILE__": PulseAudio::loadSymbols() failed.\n");
        return false;
    }

    if (pthread_mutex_init(&m_mtxStatus, NULL) != 0) {
        fprintf(stderr, __FILE__": pthread_mutex_init() failed: %s\n", strerror(errno));
        return false;
    }

    if (pthread_mutex_init(&m_mtxData, NULL) != 0) {
        fprintf(stderr, __FILE__": pthread_mutex_init() failed: %s\n", strerror(errno));
        return false;
    }

    // 初始化“暂停后等待恢复”信号量
    if (sem_init(&m_resumeWaiter, 0, 0) != 0) {
        fprintf(stderr, __FILE__": sem_init() failed: %s\n", strerror(errno));
        return false;
    }

    m_initialized = true;
    return true;
}


////////////////////////////////////////////////////////////
void PulseAudio::close() {
    stop();

    if (s) {
        pa::simple_free(s);
        s = NULL;
    }

    if (m_initialized) {
        if (sem_destroy(&m_resumeWaiter) != 0) {
            fprintf(stderr, __FILE__": sem_destroy() failed: %s\n", strerror(errno));
        }

        if (pthread_mutex_destroy(&m_mtxData) != 0) {
            fprintf(stderr, __FILE__": pthread_mutex_destroy() failed: %s\n", strerror(errno));
        }

        if (pthread_mutex_destroy(&m_mtxStatus) != 0) {
            fprintf(stderr, __FILE__": pthread_mutex_destroy() failed: %s\n", strerror(errno));
        }
    }

    m_initialized = false;
}


////////////////////////////////////////////////////////////
bool PulseAudio::isOk() const {
    return m_initialized;
}


////////////////////////////////////////////////////////////
bool PulseAudio::setSampleFormat(SampleFormat fmt) {
    /* The Sample format to use */
    pa_sample_spec ss;
    ss.format = getFormat(fmt);
    if (ss.format == PA_SAMPLE_INVALID) {
        fprintf(stderr, "PulseAudio::setSampleFormat() failed: Unsupported sample format.\n");
        return false;
    }

    ss.channels = fmt.channels;
    ss.rate = fmt.sampleRate;

    if (s) {
        // Reuse it
        if (memcmp(&ss, &m_sampleSpec, sizeof(pa_sample_spec)) == 0) {
            return true;
        }

        pa::simple_free(s);
        s = NULL;
    }

    int ret = 1;
    int error;

    /* Create a new playback stream */
    s = pa::simple_new(NULL,                // Use the default server.
                      "M4Player",           // Our application's name.
                       PA_STREAM_PLAYBACK,
                       NULL,                // Use the default device.
                      "Music",              // Description of our stream.
                       &ss,                 // Our sample format.
                       NULL,                // Use default channel map
                       NULL,                // Use default buffering attributes.
                       &error);             // Error code.

    if (!s) {
        fprintf(stderr, "[%s:%d] pa_simple_new() failed: %s\n",
                __FILE__, __LINE__, pa::strerror(error));
        return false;
    }

    m_sampleSpec = ss;
    m_fmt = fmt;

    return true;
}


////////////////////////////////////////////////////////////
pa_sample_format_t PulseAudio::getFormat(SampleFormat fmt) {
    // Find the good format according to the given format
    switch (fmt.inputFormat) {
    case Codec::SAMPLE_FMT_U8:
        return PA_SAMPLE_U8;

    case Codec::SAMPLE_FMT_S16:
        return PA_SAMPLE_S16NE;

    case Codec::SAMPLE_FMT_S32:
        return PA_SAMPLE_S32NE;

    case Codec::SAMPLE_FMT_FLT:
        return PA_SAMPLE_FLOAT32NE;

    default:
        PA_SAMPLE_INVALID;
    }
}


////////////////////////////////////////////////////////////
bool PulseAudio::play(Codec *decoder) {
    switch (m_status) {
    case Stopped:

        assert(!m_decoder);
        m_decoder = decoder;

        // Resize the buffer so that it can contain 0.1 second of audio samples
        m_samples.resize(m_fmt.sampleRate * m_fmt.channels / 10);

        // 先于线程开启前设定状态值
        m_status = Playing;

        // 直接开始播放
        if (pthread_create(&m_thread, NULL, pulse_entry, this) != 0) {
            fprintf(stderr, __FILE__": pthread_create() failed!");
            return false; // TODO:
        }

        break;

    case Paused:

        assert(m_decoder == decoder);

        setStatus(Playing);

        // 唤醒播放线程
        waitUpPausedThread();

        break;

    case Playing:

        assert(m_decoder == decoder);
        // 无操作
        break;

    default:

        fprintf(stderr, __FILE__": play() with a unkown status!");
        return false; // TODO:
    }

    return true;
}


////////////////////////////////////////////////////////////
void PulseAudio::pause() {
    assert(m_status == Playing);
    setStatus(Paused);
}


////////////////////////////////////////////////////////////
void PulseAudio::stop() {
    if (m_decoder) {
        assert(s);

        setStatus(Stopped);

        // 假如播放已被暂停，先重启线程
        waitUpPausedThread();

        if (pthread_join(m_thread, NULL) != 0) {
            fprintf(stderr, __FILE__": pthread_join() failed: %s\n", strerror(errno));
        }

        m_decoder = NULL;
    }
}


////////////////////////////////////////////////////////////
void PulseAudio::setStatus(Status status) {
    MutexLocker lock(m_mtxStatus);
    m_status = status;
}


////////////////////////////////////////////////////////////
void PulseAudio::waitUpPausedThread() {
    // 用于恢复播放或者停止时唤醒线程
    assert(m_status != Paused);

    if (sem_post(&m_resumeWaiter) != 0) {
        fprintf(stderr, __FILE__": sem_destroy() failed: %s\n", strerror(errno));
    }
}


////////////////////////////////////////////////////////////
/*static*/
void *PulseAudio::pulse_entry(void *ptr) {
    PulseAudio *This = (PulseAudio *) ptr;
    This->streamData();

    return NULL;
}


////////////////////////////////////////////////////////////'
#define PAUSE_AND_RESUME() if (!pauseAndResume()) break


////////////////////////////////////////////////////////////
bool PulseAudio::streamData() {
    int error = 0;

    /* Make sure that every single sample was played */
    if (pa::simple_drain(s, &error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa::strerror(error));
        return false;
    }

    while (m_status != Stopped) {
        PAUSE_AND_RESUME();

        /* Read some data ... */
        Uint32 samplesRead = getData();
        if (samplesRead == 0) {
            setStatus(Stopped);
            break;
        }

        enum {
            SMALLER_GRANULARITY = 10, // 将当前缓冲区再细分(以其获得更平滑的音量控制)
        };

        Uint32 bytesLeft = samplesRead * sizeof(Buffer::value_type);
        const Uint32 stdLoopSize = bytesLeft / SMALLER_GRANULARITY;
        Buffer::value_type *p = &m_samples[0];

        while (bytesLeft > 0) {
            PAUSE_AND_RESUME();

            Uint32 loopSize = (bytesLeft >= stdLoopSize) ? stdLoopSize : bytesLeft;
            VolumePolicy::adjustAmp(p, loopSize, m_fmt);

            /* Play it */
            if (pa::simple_write(s, p, loopSize, &error) < 0) {
                fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa::strerror(error));
                return false;
            }

            bytesLeft -= loopSize;
            p += loopSize / sizeof(Buffer::value_type);
        }

        if (samplesRead < m_samples.size()) {
            setStatus(Stopped);
            break;
        }
    }

    return true;
}


////////////////////////////////////////////////////////////
bool PulseAudio::pauseAndResume() {
    // 有可能主线程只是在流线程被 pa_simple_*() 阻塞时暂停，同时又快速恢复播放，
    // 此时流线程可能根本就不知道有暂停这回事，所以要消费掉所有的信号量
    while (m_status == Paused) {
        sem_wait(&m_resumeWaiter);

        if (m_status == Stopped) {
            return false;
        }
    }

    return true;
}


////////////////////////////////////////////////////////////
void PulseAudio::preSetPlayingOffset(Time) {
    pthread_mutex_lock(&m_mtxData);
}


////////////////////////////////////////////////////////////
void PulseAudio::setPlayingOffset(Time) {
    pthread_mutex_unlock(&m_mtxData);
}


////////////////////////////////////////////////////////////
void PulseAudio::setVolume(float volume) {
    VolumePolicy::setVolume(volume);
}


////////////////////////////////////////////////////////////
float PulseAudio::getVolume() const {
    return VolumePolicy::getVolume();
}


////////////////////////////////////////////////////////////
Uint32 PulseAudio::getData() {
    MutexLocker lock(m_mtxData);
    return m_decoder->read(&m_samples[0], m_samples.size());
}


////////////////////////////////////////////////////////////
Output::Status PulseAudio::getStatus() const {
    return m_status;
}


////////////////////////////////////////////////////////////
#include "Client.hpp"
DEFINE_STD_OOPLUGIN(PulseAudio, "Outdev;")

