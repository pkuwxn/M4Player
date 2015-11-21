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

    /// ���캯��
    PulseAudio();

    /// ��������
    virtual ~PulseAudio();

public:

    /// ���ǲ���Ҫ sf::Music �ṩ�ķ����̻߳���
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

    // �ڷ����߳��м�⵱ǰ�Ƿ�����ͣ״̬
    //
    // ��Ϊ��ͣ״̬����ȴ��ָ����ŵ��ź�����������(��ʱӦ�ü���������ȥ)��
    // ��Ϊֹͣ״̬��ֱ�ӷ��ؼ٣��߳�Ӧ����ֹ��
    bool pauseAndResume();

    // �߳���ں���
    static void *pulse_entry(void *ptr);

    // ���õ�ǰ����״̬
    void setStatus(Status status);

    // �����ѱ���ͣ�Ĳ����߳�
    void waitUpPausedThread();

    // ����������Ƶ���ݻ�ȡ PulseAudio ��Ӧ�������ʽ
    pa_sample_format_t getFormat(SampleFormat fmt);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    pa_simple *s;
    pa_sample_spec m_sampleSpec; // PulseAudio �����������ʽ

    bool m_initialized; // �豸�Ƿ���Ȼ��ʼ����

    pthread_t m_thread;
    pthread_mutex_t m_mtxStatus; // ����״̬����(m_status)�Ļ�����
    pthread_mutex_t m_mtxData; // ��������ȡ���ݡ������Ϊ�Ļ�����

    sem_t m_resumeWaiter; // ��ͣʱ�ȴ��ָ���������
    bool m_stopAtOnce; // ����ֹͣ����

    Status m_status;
};


#endif // SFML_PULSE_AUDIO_HPP
