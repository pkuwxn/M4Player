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

#ifndef __unix__ //
#error "Compile this plugin only under Unixes!"
#endif // __unix__

#ifndef SFML_PULSE_AUDIO_HPP
#define SFML_PULSE_AUDIO_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Outdev.hpp"
#include <pulse/simple.h>
#include <pthread.h>
#include <semaphore.h>



using namespace sf;


////////////////////////////////////////////////////////////
/// \brief PulseAudio output backend
///
////////////////////////////////////////////////////////////
class PulseAudio : public OutputHelper<SoftwareVolumePolicy> {
public :

    typedef SoftwareVolumePolicy VolumePolicy;

    /// 构造函数
    PulseAudio();

    /// 析构函数
    virtual ~PulseAudio();

public:

    /// 我们不需要 sf::Music 提供的分离线程机制
    virtual bool isAsync() const {
        return true;
    }
    virtual bool open(Uint32 deviceId);
    virtual void close();
    virtual bool isOk() const;
    virtual void *getBuffer() {
        return NULL;
    }
    virtual bool write(void *buffer, Uint32 bufferSize) {
        return false;
    }
    virtual bool setSampleFormat(SampleFormat fmt);
    virtual bool play(Codec *decoder);
    virtual void pause();
    virtual void stop();
    virtual bool streamData();
    virtual void preSetPlayingOffset(Time timeOffset);
    virtual void setPlayingOffset(Time timeOffset);
    virtual Time getPlayingOffset() const {
        return seconds(0);
    }
    virtual void setVolume(float volume);
    virtual float getVolume() const;
    virtual Status getStatus() const;

private :

    // Request a new chunk of audio samples from the stream source
    Uint32 getData();

    // 在分离线程中检测当前是否处于暂停状态
    //
    // 若为暂停状态，则等待恢复播放的信号量并返回真(此时应该继续播放下去)；
    // 若为停止状态，直接返回假，线程应该中止。
    bool pauseAndResume();

    // 线程入口函数
    static void *pulse_entry(void *ptr);

    // 设置当前播放状态
    void setStatus(Status status);

    // 唤醒已被暂停的播放线程
    void waitUpPausedThread();

    // 根据输入音频数据获取 PulseAudio 对应的输出格式
    pa_sample_format_t getFormat(SampleFormat fmt);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    pa_simple *s;
    pa_sample_spec m_sampleSpec; // PulseAudio 的输出样本格式

    bool m_initialized; // 设备是否已然初始化？

    pthread_t m_thread;
    pthread_mutex_t m_mtxStatus; // 保护状态变量(m_status)的互斥量
    pthread_mutex_t m_mtxData; // 保护“获取数据”这个行为的互斥量

    sem_t m_resumeWaiter; // 暂停时等待恢复播放命令
    bool m_stopAtOnce; // 立即停止播放

    Status m_status;
};


#endif // SFML_PULSE_AUDIO_HPP
