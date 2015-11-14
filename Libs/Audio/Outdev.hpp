/*
 *  Copyright (C) 2006-2008 Leandro Nini
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SFML_OUTDEV_HPP
#define SFML_OUTDEV_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Service.hpp"
#include "Config.hpp"
#include "Time.hpp"
#include "Codec.hpp"

namespace sf {
////////////////////////////////////////////////////////////
/// \brief Interface class for output backends
///
////////////////////////////////////////////////////////////
class Output : public ServiceBase {
public :

    ////////////////////////////////////////////////////////////
    /// \brief Virtual destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~Output() {}

    ////////////////////////////////////////////////////////////
    /// \brief Get output backend name
    ///
    ////////////////////////////////////////////////////////////
    virtual const char *getName() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Streaming 过程是否为异步的
    ///
    /// 即不需要在分离的线程中持续调用 streamData() 。
    ////////////////////////////////////////////////////////////
    virtual bool isAsync() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get number of available devices
    ///
    ////////////////////////////////////////////////////////////
    virtual Uint32 getDeviceCount() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get device name
    ///
    ////////////////////////////////////////////////////////////
    virtual const char *getDevice(Uint32 i) const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Prepare the device for output
    ///
    /// \return 输出设备是否已被成功打开
    ////////////////////////////////////////////////////////////
    virtual bool open(Uint32 deviceId) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Close the device
    ///
    ////////////////////////////////////////////////////////////
    virtual void close() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief 输出设备是否已被成功打开
    ///
    ////////////////////////////////////////////////////////////
    virtual bool isOk() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get buffer
    ///
    ////////////////////////////////////////////////////////////
    virtual void *getBuffer() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Describes the format of audio samples
    ///
    ////////////////////////////////////////////////////////////
    struct SampleFormat {
        Codec::SampleFormat infmt; /*! Input sample format (Bits per sample) */
        Uint32 sampleRate; /*! Samples per second (in a single channel) */
        Uint32 channels; /*! Number of audio channels */

        ////////////////////////////////////////////////////////////
        /// \brief Default ctor
        ///
        ////////////////////////////////////////////////////////////
        SampleFormat()
            : infmt(Codec::SAMPLE_FMT_NONE), sampleRate(0), channels(0) {

        }

        ////////////////////////////////////////////////////////////
        /// \brief 是否以浮点数来储存
        ///
        ////////////////////////////////////////////////////////////
        bool isFloat() const {
            return (infmt == Codec::SAMPLE_FMT_FLT) ||
                   (infmt == Codec::SAMPLE_FMT_DBL);
        }

        ////////////////////////////////////////////////////////////
        /// \brief 获取已解码的输入数据的一个 sample 的比特数
        ///
        ////////////////////////////////////////////////////////////
        Uint32 getBits() const {
            return (Uint32) infmt;
        }
    };

    ////////////////////////////////////////////////////////////
    /// \brief 设置音频输出要使用的样本(sample)格式
    ///
    /// \return 指定的格式是否被输出设备接受
    ////////////////////////////////////////////////////////////
    virtual bool setSampleFormat(SampleFormat fmt) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Begin playback
    ///
    /// \return 是否一次全新的播放，此时需要新开一个线程避免阻塞整个应用程序
    ////////////////////////////////////////////////////////////
    virtual bool play(Codec *decoder) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Write data to output
    ///
    ////////////////////////////////////////////////////////////
    virtual bool write(void *buffer, Uint32 bufferSize) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Pause
    ///
    ////////////////////////////////////////////////////////////
    virtual void pause() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Stop
    ///
    ////////////////////////////////////////////////////////////
    virtual void stop() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief A single streaming loop process called in a seperate thread
    ///
    ////////////////////////////////////////////////////////////
    virtual bool streamData() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief 在解码器被外部重设当前进度之前通知输出设备
    ///
    ////////////////////////////////////////////////////////////
    virtual void preSetPlayingOffset(Time timeOffset) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position of the stream
    ///
    ////////////////////////////////////////////////////////////
    virtual void setPlayingOffset(Time timeOffset) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the current playing position of the stream
    ///
    ////////////////////////////////////////////////////////////
    virtual Time getPlayingOffset() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Set volume
    ///
    ////////////////////////////////////////////////////////////
    virtual void setVolume(float volume) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get volume
    ///
    ////////////////////////////////////////////////////////////
    virtual float getVolume() const = 0;

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
    /// \brief Get the current status of the sound (stopped, paused, playing)
    ///
    /// \return Current status of the sound
    ///
    ////////////////////////////////////////////////////////////
    virtual Status getStatus() const = 0;

protected :

    ////////////////////////////////////////////////////////////
    /// \brief Default ctor
    ///
    ////////////////////////////////////////////////////////////
    Output() {}

private:

    Output(const Output &);
    Output &operator=(const Output &);
};

}

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include <string>

namespace sf {
class BuiltinVolumePolicy;

////////////////////////////////////////////////////////////
/// \brief 输出后端辅助性基类
///
////////////////////////////////////////////////////////////
template<class VolumePolicy = BuiltinVolumePolicy>
class OutputHelper : public Output, protected VolumePolicy {
public :

    /// Get backend name
    virtual const char *getName() const {
        return m_name;
    }

    /// Get number of available devices
    virtual Uint32 getDeviceCount() const {
        return m_devices.size();
    }

    /// Get device name
    virtual const char *getDevice(Uint32 i) const {
        return m_devices[i].c_str();
    }

    /// Set volume
    virtual void setVolume(float volume) {
        VolumePolicy::setVolume(volume);
    }
    /// Get volume
    virtual float getVolume() const {
        return VolumePolicy::getVolume();
    }

protected :

    /// 构造函数
    OutputHelper(const char *name)
        : m_name(name), m_decoder(NULL),
          // 先不要分配任何空间
          m_samples(0) {

    }

    /// Add a device to the list
    void addDevice(const char *dev) {
        m_devices.push_back(dev);
    }

    /// 设置输出设备的名称
    void setName(const char *name) {
        m_name = name;
    }

    /// 根据当前音量对解码后的音频数据进行转换(改变振幅)
    void adjustAmp() {
        VolumePolicy::adjustAmp(&m_samples[0], m_samples.size() * sizeof(Int16), m_fmt);
    }

private :

    OutputHelper();
    OutputHelper(const OutputHelper &);
    OutputHelper &operator=(const OutputHelper &);

private :

    const char *m_name;
    std::vector<std::string> m_devices;

protected :

    SampleFormat m_fmt;  ///< Define the audio stream parameters
    Codec *m_decoder;   ///< 相应的解码器

    typedef std::vector<Int16> Buffer;
    Buffer m_samples; ///< Temporary buffer of samples
};


////////////////////////////////////////////////////////////
/// \brief 输出后端内建音量控制支持
///
////////////////////////////////////////////////////////////
class BuiltinVolumePolicy {
public :

    /// 设定音量
    void setVolume(float volume) {}

    /// 获取当前音量
    float getVolume() const {
        return 0;    // TODO: 抛出异常？
    }

    /// 根据当前音量对解码后的音频数据进行转换(改变振幅)
    void adjustAmp(Int16 *samples, Uint32 bytes, Output::SampleFormat fmt) {}
};

////////////////////////////////////////////////////////////
/// \brief 输出后端没有内建音量控制支持，使用软件来粗略模拟
///
////////////////////////////////////////////////////////////
class SoftwareVolumePolicy {
public :

    /// 构造函数
    SoftwareVolumePolicy() : m_volume(1) {}

    /// 设定音量，代表振幅(amplitude)，范围为[0..1]
    void setVolume(float volume) {
        if (volume < 0) {
            volume = 0;
        }

        if (volume > 1) {
            volume = 1;
        }

        m_volume = volume;
    }

    /// 获取当前音量，代表振幅(amplitude)，范围为[0..1]
    float getVolume() const {
        return m_volume;
    }

    /// 根据当前音量对解码后的音频数据进行转换(改变振幅)
    void adjustAmp(Int16 *samples, Uint32 bytes, Output::SampleFormat fmt);

private :

    float m_volume; // 当前音量，代表振幅(amplitude)，范围为[0..1]
};

} // namespace sf

#endif
