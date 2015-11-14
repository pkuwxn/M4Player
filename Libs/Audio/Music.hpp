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

#ifndef SFML_MUSIC_HPP
#define SFML_MUSIC_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Export.hpp"
#include "Time.hpp"
#include "Service.hpp"
#include "StopWatch.hpp"

#include <string> // for getSupportedFileTypes()
#include <wx/thread.h>


namespace sf {
class Codec;
class Output;
class InputStream;

////////////////////////////////////////////////////////////
/// \brief Streamed music played from an audio file
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API Music {
public :

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    Music();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Music();

    ////////////////////////////////////////////////////////////
    /// \brief Open a music from an audio file
    ///
    /// This function doesn't start playing the music (call play()
    /// to do so).
    /// Here is a complete list of all the supported audio formats:
    /// ogg, wav, flac, aiff, au, raw, paf, svx, nist, voc, ircam,
    /// w64, mat4, mat5 pvf, htk, sds, avr, sd2, caf, wve, mpc2k, rf64.
    ///
    /// \param filename Path of the music file to open
    ///
    /// \return True if loading succeeded, false if it failed
    ///
    /// \see openFromMemory, openFromStream
    ///
    ////////////////////////////////////////////////////////////
    bool openFromFile(const char *filename);

    ////////////////////////////////////////////////////////////
    /// \brief Open a music from an audio file in memory
    ///
    /// This function doesn't start playing the music (call play()
    /// to do so).
    /// Here is a complete list of all the supported audio formats:
    /// ogg, wav, flac, aiff, au, raw, paf, svx, nist, voc, ircam,
    /// w64, mat4, mat5 pvf, htk, sds, avr, sd2, caf, wve, mpc2k, rf64.
    /// Since the music is not loaded completely but rather streamed
    /// continuously, the \a data must remain available as long as the
    /// music is playing (ie. you can't deallocate it right after calling
    /// this function).
    ///
    /// \param data        Pointer to the file data in memory
    /// \param sizeInBytes Size of the data to load, in bytes
    ///
    /// \return True if loading succeeded, false if it failed
    ///
    /// \see openFromFile, openFromStream
    ///
    ////////////////////////////////////////////////////////////
    bool openFromMemory(const void *data, std::size_t sizeInBytes);

    ////////////////////////////////////////////////////////////
    /// \brief Open a music from an audio file in a custom stream
    ///
    /// This function doesn't start playing the music (call play()
    /// to do so).
    /// Here is a complete list of all the supported audio formats:
    /// ogg, wav, flac, aiff, au, raw, paf, svx, nist, voc, ircam,
    /// w64, mat4, mat5 pvf, htk, sds, avr, sd2, caf, wve, mpc2k, rf64.
    /// Since the music is not loaded completely but rather streamed
    /// continuously, the \a stream must remain alive as long as the
    /// music is playing (ie. you can't destroy it right after calling
    /// this function).
    ///
    /// \param stream Source stream to read from
    ///
    /// \return True if loading succeeded, false if it failed
    ///
    /// \see openFromFile, openFromMemory
    ///
    ////////////////////////////////////////////////////////////
    bool openFromStream(InputStream &stream);

    ////////////////////////////////////////////////////////////
    /// \brief Start or resume playing the audio stream
    ///
    /// This function starts the stream if it was stopped, resumes
    /// it if it was paused, and restarts it from beginning if it
    /// was it already playing.
    /// This function uses its own thread so that it doesn't block
    /// the rest of the program while the stream is played.
    ///
    /// \return True if playing succeeded, false if it failed
    ///
    /// \see pause, stop
    ///
    ////////////////////////////////////////////////////////////
    bool play();

    ////////////////////////////////////////////////////////////
    /// \brief Pause the audio stream
    ///
    /// This function pauses the stream if it was playing,
    /// otherwise (stream already paused or stopped) it has no effect.
    ///
    /// \see play, stop
    ///
    ////////////////////////////////////////////////////////////
    void pause();

    ////////////////////////////////////////////////////////////
    /// \brief Stop playing the audio stream
    ///
    /// This function stops the stream if it was playing or paused,
    /// and does nothing if it was already stopped.
    /// It also resets the playing position (unlike pause()).
    ///
    /// \see play, pause
    ///
    ////////////////////////////////////////////////////////////
    void stop();

    ////////////////////////////////////////////////////////////
    /// \brief Enumeration of the sound source states
    ///
    ////////////////////////////////////////////////////////////
    enum Status {
        Stopped, ///< Sound is not playing
        Paused,  ///< Sound is paused
        Playing  ///< Sound is playing
    };

    ////////////////////////////////////////////////////////////
    /// \brief Get the current status of the stream (stopped, paused, playing)
    ///
    /// \return Current status
    ///
    ////////////////////////////////////////////////////////////
    Status getStatus() const;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position of the stream
    ///
    /// The playing position can be changed when the stream is
    /// either paused or playing.
    ///
    /// \param timeOffset New playing position, from the beginning of the stream
    ///
    /// \return Ture if succeeded
    ///
    /// \see getPlayingOffset
    ///
    ////////////////////////////////////////////////////////////
    bool setPlayingOffset(Time timeOffset);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current playing position of the stream
    ///
    /// \return Current playing position, from the beginning of the stream
    ///
    /// \see setPlayingOffset
    ///
    ////////////////////////////////////////////////////////////
    Time getPlayingOffset() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the volume of the sound
    ///
    /// The volume is a value between 0 (mute) and 1 (full volume).
    /// The default value for the volume is 1.
    ///
    /// \param volume Volume of the sound
    ///
    /// \see getVolume
    ///
    ////////////////////////////////////////////////////////////
    void setVolume(float volume);

    ////////////////////////////////////////////////////////////
    /// \brief Get the volume of the sound
    ///
    /// \return Volume of the sound, in the range [0, 1]
    ///
    /// \see setVolume
    ///
    ////////////////////////////////////////////////////////////
    float getVolume() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the total duration of the music
    ///
    /// \return Music duration
    ///
    ////////////////////////////////////////////////////////////
    Time getDuration() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return the number of channels of the stream
    ///
    /// 1 channel means a mono sound, 2 means stereo, etc.
    ///
    /// \return Number of channels
    ///
    ////////////////////////////////////////////////////////////
    Uint32 getChannelCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the stream sample rate of the stream
    ///
    /// The sample rate is the number of audio samples played per
    /// second. The higher, the better the quality.
    ///
    /// \return Sample rate, in number of samples per second
    ///
    ////////////////////////////////////////////////////////////
    Uint32 getSampleRate() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the bit rate of the sound
    ///
    /// \return Bit rate, in kilobits per second
    ///
    ////////////////////////////////////////////////////////////
    Uint32 getBitRate() const;

    ////////////////////////////////////////////////////////////
    /// \brief 获取所有被支持的文件类型
    ///
    /// \return 类似于“OGG 音频文件(*.ogg)|*.ogg|”的文件掩码
    ///
    ////////////////////////////////////////////////////////////
    static std::string getSupportedFileTypes();

private :

    // 根据当前播放状态调整计时器的计时(播放时为当前计时，暂停时为下次起始计时)
    void ajustStopWatch(Time dst);

    // Utility class to manipulate threads
    class Thread : public wxThread {
    public :

        // Constructor
        Thread(Output *outdev);

        // 立即中止线程
        void StopAtOnce() {
            m_stopAtOnce = true;
        }

        // 获取输出设备的互斥使用权
        void Lock() {
            m_mutex.Lock();
        }

        // 释放输出设备的互斥使用权
        void Unlock() {
            m_mutex.Unlock();
        }

    private :

        // 线程入口函数
        virtual wxThread::ExitCode Entry();

    private :

        bool m_stopAtOnce;  ///< 立即停止

        Output *m_outdev;
        wxMutex m_mutex;    ///< 确保输出设备的互斥使用权
    };

    void newThread();
    void deleteThread();

    ////////////////////////////////////////////////////////////
    // \brief 管理所有可能的解码器
    //
    ////////////////////////////////////////////////////////////
    class DecoderMgr : public ServiceMgr<Codec> {
    public:

        // 尝试根据文件名添加正确的解码器
        bool tryAddDecoder(const char *fileName);
    };

    ////////////////////////////////////////////////////////////
    // \brief 管理使用的输出设备
    //
    ////////////////////////////////////////////////////////////
    class OutdevMgr : public ServiceMgr<Output> {
    public:

        /// 初始化输出设备
        bool prepare();
    };

    ////////////////////////////////////////////////////////////

    enum {
        BufferCount = 3 ///< Number of audio buffers used by the streaming loop
    };

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Thread             *m_thread;       ///< Thread running the background tasks

    DecoderMgr          m_decoder;      ///< Sound decoder
    OutdevMgr           m_outdev;       ///< Output device

    StopWatch           m_stopWatch;    ///< 播放进度计数器
};


} // namespace sf


#endif // SFML_MUSIC_HPP


////////////////////////////////////////////////////////////
/// \class sf::Music
/// \ingroup audio
///
/// Musics are sounds that are streamed rather than completely
/// loaded in memory. This is especially useful for compressed
/// musics that usually take hundreds of MB when they are
/// uncompressed: by streaming it instead of loading it entirely,
/// you avoid saturating the memory and have almost no loading delay.
///
/// Apart from that, a sf::Music has almost the same features as
/// the sf::SoundBuffer / sf::Sound pair: you can play/pause/stop
/// it, request its parameters (channels, sample rate), change
/// the way it is played (pitch, volume, 3D position, ...), etc.
///
/// As a sound stream, a music is played in its own thread in order
/// not to block the rest of the program. This means that you can
/// leave the music alone after calling play(), it will manage itself
/// very well.
///
/// Usage example:
/// \code
/// // Declare a new music
/// sf::Music music;
///
/// // Open it from an audio file
/// if (!music.openFromFile("music.ogg"))
/// {
///     // error...
/// }
///
/// // Change some parameters
/// music.setPosition(0, 1, 10); // change its 3D position
/// music.setPitch(2);           // increase the pitch
/// music.setVolume(50);         // reduce the volume
/// music.setLoop(true);         // make it loop
///
/// // Play it
/// music.play();
/// \endcode
///
/// \see sf::Sound, sf::SoundStream
///
////////////////////////////////////////////////////////////
