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

#include "../Music.hpp"
#include "../Codec.hpp"
#include "../Outdev.hpp"

#include <map> // for getSupportedFileTypes()

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif


namespace {
static inline std::string &string_toupper(std::string &str) {
    for (size_t i = 0; i < str.size(); ++i) {
        str[i] = static_cast<char>(toupper(str[i]));
    }

    return str;
}

static inline std::string &string_tolower(std::string &str) {
    for (size_t i = 0; i < str.size(); ++i) {
        str[i] = static_cast<char>(tolower(str[i]));
    }

    return str;
}
}

namespace sf {
////////////////////////////////////////////////////////////
bool Music::DecoderMgr::tryAddDecoder(const char *fileName) {
    // Extract the extension
    std::string ext("wav");
    const char *p = strrchr(fileName, '.');
    if (p) {
        string_toupper(ext.assign(p + 1));
    }

    std::string require("Decoder/");
    require += ext;
    require += ';';

    //=============================================

    reset();
    for (; m_active != m_candidates.end(); ++m_active) {
        if ((*m_active)->testAbility(require.c_str())) {
            return true;
        }
    }

    FactoryBase *factory = FactoryBase::query(require.c_str());
    if (factory) {
        Codec *decoder = (Codec *) factory->create();
        if (decoder) {
            m_candidates.push_back(decoder);
            m_active = m_candidates.begin() + m_candidates.size() - 1;

            return true;
        }
    }

    return false;
}


////////////////////////////////////////////////////////////
bool Music::OutdevMgr::prepare() {
    if (*this) {
        return true;
    }

    FactoryBase *factory = FactoryBase::query("Outdev;");
    if (factory) {
        Output *outdev = (Output *) factory->create();
        if (outdev) {
            m_candidates.push_back(outdev);
            m_active = m_candidates.begin() + m_candidates.size() - 1;

            return true;
        }
    }

    return false;
}


////////////////////////////////////////////////////////////
const static char *gs_AllFileTypes = "All files (*.*)|*.*";

/*static*/
std::string Music::getSupportedFileTypes() {
    if (!FactoryBase::__internal__list) {
        return gs_AllFileTypes;
    }

    std::string ab, ext, desc;
    std::map<std::string, std::string> types;

    FactoryBase *factory = FactoryBase::__internal__list;
    while (factory) {
        ab.assign(factory->getAbility());
        const char *p = ab.c_str();

        std::string::size_type first = 0, last;
        while ((first = ab.find("Decoder/", first)) != std::string::npos) {
            first += 8; // #("Decoder/") == 8
            last = ab.find(";", first);
            if (last == std::string::npos) {
                break;
            }

            ext.assign(p + first, p + last);
            string_tolower(ext);

            //================================
            // 检测文件类型描述文本

            first = last + 1; // 跳过';'
            if ((first < ab.length()) && (ab[first] == '|')) {
                first++; // 跳过'|'
                last = ab.find(";", first);
                if (last == std::string::npos) {
                    break;
                }

                desc.assign(p + first, p + last);
            } else {
                desc.assign(ext + " Audio");
            }

            types[ext] = desc;
            first = last;
        }

        factory = factory->__internal__next;
    }

    //================================
    // 合成掩码字符串

    std::string ret("|");
    std::string maskAll("All supported types|");

    std::map<std::string, std::string>::iterator iter(types.begin());
    for (; iter != types.end(); ++iter) {
        maskAll += "*." + iter->first + ';';

        // OGG Audio (*.ogg)|*.ogg|
        ret += iter->second;
        ret += " (*." + iter->first + ")|*." + iter->first + '|';
    }

    maskAll.erase(maskAll.begin() + maskAll.length() - 1);
    maskAll += ret;
    maskAll += gs_AllFileTypes;

    return maskAll;
}

////////////////////////////////////////////////////////////
enum {
    INVALID_PLAYING_OFFSET = Uint32(~0),
};


////////////////////////////////////////////////////////////
Music::Music() :
    m_thread(NULL) {

}


////////////////////////////////////////////////////////////
Music::~Music() {
    // We must stop before destroying the file :)
    stop();

    wxASSERT(!m_thread);
}


////////////////////////////////////////////////////////////
bool Music::openFromFile(const char *filename) {
    // First stop the music if it was already running
    stop();

    if (!m_decoder.tryAddDecoder(filename)) {
        return false;
    }

    // Open the underlying sound file
    if (!m_decoder->openRead(filename)) {
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
bool Music::openFromMemory(const void *data, std::size_t sizeInBytes) {
    // First stop the music if it was already running
    stop();

    // Open the underlying sound file
    if (!m_decoder->openRead(data, sizeInBytes)) {
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
bool Music::openFromStream(InputStream &stream) {
    // First stop the music if it was already running
    stop();

    // Open the underlying sound file
    if (!m_decoder->openRead(stream)) {
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
bool Music::play() {
    if (!m_outdev) {
        if (!m_outdev.prepare()) {
            fprintf(stderr, "[%s:%d]No output device plugin found!\n",
                    __FILE__, __LINE__);

            return false;
        }
    }

    // 声卡可能被其它程序独占，每次播放时都重试获取使用权
    if (!m_outdev->isOk()) {
        if (!m_outdev->open(0)) {
            return false;
        }
    }

    Codec::SampleFormat infmt = m_decoder->getSampleFormat();
    Output::SampleFormat fmt;
    fmt.infmt = infmt;
    fmt.channels = getChannelCount();
    fmt.sampleRate = getSampleRate();

    if (!m_outdev->setSampleFormat(fmt)) {
        return false;
    }

    //================================================

    if (getStatus() == Stopped) {
        // Move to current position
        m_decoder->seek(seconds(0));

        // 开始计时
        m_stopWatch.Start(0L);

        // 先开始播放
        if (!m_outdev->play(m_decoder.get())) {
            return false;
        }

        // Start updating the stream in a separate thread
        if (!m_outdev->isAsync()) {
            newThread();
        }
    } else {
        m_stopWatch.Resume();

        // Resume
        if (!m_outdev->play(m_decoder.get())) {
            return false;
        }
    }

    return true;
}


////////////////////////////////////////////////////////////
void Music::pause() {
    m_stopWatch.Pause();
    m_outdev->pause();

    wxASSERT(m_outdev->getStatus() == Output::Paused);
}


////////////////////////////////////////////////////////////
void Music::stop() {
    if (!m_outdev) {
        return;
    }

    m_stopWatch.Stop();

    // 先删除线程，再对输出设备执行“停止”命令
    if (m_thread) {
        deleteThread();
    }

    m_outdev->stop();
}


////////////////////////////////////////////////////////////
void Music::newThread() {
    wxASSERT(m_thread == NULL);
    wxASSERT(!m_outdev->isAsync());

    m_thread = new Thread(m_outdev.get());
}


////////////////////////////////////////////////////////////
void Music::deleteThread() {
    wxASSERT(!m_outdev->isAsync());

    if (m_thread) {
        m_thread->StopAtOnce();

        // Wait for the thread to terminate
        m_thread->Wait();

        delete m_thread;
        m_thread = NULL;
    }
}


////////////////////////////////////////////////////////////
Music::Status Music::getStatus() const {
    return m_outdev ? (Status) m_outdev->getStatus() : Stopped;
}


////////////////////////////////////////////////////////////
bool Music::setPlayingOffset(Time timeOffset) {
    wxASSERT(getStatus() != Stopped);

    if (timeOffset >= getDuration()) {
        fprintf(stderr, "Music::setPlayingOffset(): Invalid time offset.\n");
        return false;
    }

    // 先启动计时器
    Time curr = milliseconds(m_stopWatch.Time());
    ajustStopWatch(timeOffset);

    if (m_thread) {
        m_thread->Lock();
    }

    m_outdev->preSetPlayingOffset(timeOffset);
    // TODO: 失败时是否需要重新回到原来的位置？
    bool ok = m_decoder->seek(timeOffset);
    m_outdev->setPlayingOffset(timeOffset);

    if (m_thread) {
        m_thread->Unlock();
    }

    // 寻道失败，恢复计时器
    if (!ok) {
        ajustStopWatch(curr);
    }

    return ok;
}


////////////////////////////////////////////////////////////
void Music::ajustStopWatch(Time dst) {
    wxASSERT(m_outdev.isOk());

    if (m_outdev->getStatus() == Output::Paused) {
        m_stopWatch.Reset(dst.asMilliseconds());
    } else {
        m_stopWatch.Start(dst.asMilliseconds());
    }
}


////////////////////////////////////////////////////////////
Time Music::getPlayingOffset() const {
    return milliseconds(m_stopWatch.Time());
}


////////////////////////////////////////////////////////////
void Music::setVolume(float volume) {
    if (m_thread) {
        m_thread->Lock();
    }

    m_outdev->setVolume(volume);

    if (m_thread) {
        m_thread->Unlock();
    }
}


////////////////////////////////////////////////////////////
float Music::getVolume() const {
    return m_outdev->getVolume();
}


////////////////////////////////////////////////////////////
Time Music::getDuration() const {
    return milliseconds(m_decoder->getDuration());
}


////////////////////////////////////////////////////////////
Uint32 Music::getChannelCount() const {
    return m_decoder->getChannelCount();
}


////////////////////////////////////////////////////////////
Uint32 Music::getSampleRate() const {
    return m_decoder->getSampleRate();
}


////////////////////////////////////////////////////////////
Uint32 Music::getBitRate() const {
    return m_decoder->getBitRate();
}


//////////////////////////////////////////////////////////////////////////
Music::Thread::Thread(Output *outdev) :
    wxThread(wxTHREAD_JOINABLE),
    m_stopAtOnce(false),
    m_outdev(outdev) {
    wxThread::Create();
    wxThread::Run();
}


////////////////////////////////////////////////////////////
wxThread::ExitCode Music::Thread::Entry() {
    enum {
        TIMESLICE_TO_CHECK_EXIT = 10, // 检测退出
        TIMESLICE_TO_STREAM = 1000 / (BufferCount - 1), // 填充流
    };

    // 每 500ms 执行实际检测工作一次
    Uint32 timer = TIMESLICE_TO_CHECK_EXIT;

    while (!m_stopAtOnce) {
        bool ok = true;
        if (timer % TIMESLICE_TO_STREAM == 0) {
            // Lock the decoder buffer
            wxMutexLocker lock(m_mutex);

            ok = m_outdev->streamData();
        }

        // Leave some time for the other threads if the stream is still playing
        if (ok) {
            wxThread::Sleep(TIMESLICE_TO_CHECK_EXIT);
            timer += TIMESLICE_TO_CHECK_EXIT;
        } else {
            break;
        }
    }

    return 0;
}


} // namespace sf
