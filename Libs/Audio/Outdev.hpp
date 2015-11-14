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
    /// \brief Streaming �����Ƿ�Ϊ�첽��
    ///
    /// ������Ҫ�ڷ�����߳��г������� streamData() ��
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
    /// \return ����豸�Ƿ��ѱ��ɹ���
    ////////////////////////////////////////////////////////////
    virtual bool open(Uint32 deviceId) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Close the device
    ///
    ////////////////////////////////////////////////////////////
    virtual void close() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief ����豸�Ƿ��ѱ��ɹ���
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
        /// \brief �Ƿ��Ը�����������
        ///
        ////////////////////////////////////////////////////////////
        bool isFloat() const {
            return (infmt == Codec::SAMPLE_FMT_FLT) ||
                   (infmt == Codec::SAMPLE_FMT_DBL);
        }

        ////////////////////////////////////////////////////////////
        /// \brief ��ȡ�ѽ�����������ݵ�һ�� sample �ı�����
        ///
        ////////////////////////////////////////////////////////////
        Uint32 getBits() const {
            return (Uint32) infmt;
        }
    };

    ////////////////////////////////////////////////////////////
    /// \brief ������Ƶ���Ҫʹ�õ�����(sample)��ʽ
    ///
    /// \return ָ���ĸ�ʽ�Ƿ�����豸����
    ////////////////////////////////////////////////////////////
    virtual bool setSampleFormat(SampleFormat fmt) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Begin playback
    ///
    /// \return �Ƿ�һ��ȫ�µĲ��ţ���ʱ��Ҫ�¿�һ���̱߳�����������Ӧ�ó���
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
    /// \brief �ڽ��������ⲿ���赱ǰ����֮ǰ֪ͨ����豸
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
/// \brief �����˸����Ի���
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

    /// ���캯��
    OutputHelper(const char *name)
        : m_name(name), m_decoder(NULL),
          // �Ȳ�Ҫ�����κοռ�
          m_samples(0) {

    }

    /// Add a device to the list
    void addDevice(const char *dev) {
        m_devices.push_back(dev);
    }

    /// ��������豸������
    void setName(const char *name) {
        m_name = name;
    }

    /// ���ݵ�ǰ�����Խ�������Ƶ���ݽ���ת��(�ı����)
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
    Codec *m_decoder;   ///< ��Ӧ�Ľ�����

    typedef std::vector<Int16> Buffer;
    Buffer m_samples; ///< Temporary buffer of samples
};


////////////////////////////////////////////////////////////
/// \brief �������ڽ���������֧��
///
////////////////////////////////////////////////////////////
class BuiltinVolumePolicy {
public :

    /// �趨����
    void setVolume(float volume) {}

    /// ��ȡ��ǰ����
    float getVolume() const {
        return 0;    // TODO: �׳��쳣��
    }

    /// ���ݵ�ǰ�����Խ�������Ƶ���ݽ���ת��(�ı����)
    void adjustAmp(Int16 *samples, Uint32 bytes, Output::SampleFormat fmt) {}
};

////////////////////////////////////////////////////////////
/// \brief ������û���ڽ���������֧�֣�ʹ�����������ģ��
///
////////////////////////////////////////////////////////////
class SoftwareVolumePolicy {
public :

    /// ���캯��
    SoftwareVolumePolicy() : m_volume(1) {}

    /// �趨�������������(amplitude)����ΧΪ[0..1]
    void setVolume(float volume) {
        if (volume < 0) {
            volume = 0;
        }

        if (volume > 1) {
            volume = 1;
        }

        m_volume = volume;
    }

    /// ��ȡ��ǰ�������������(amplitude)����ΧΪ[0..1]
    float getVolume() const {
        return m_volume;
    }

    /// ���ݵ�ǰ�����Խ�������Ƶ���ݽ���ת��(�ı����)
    void adjustAmp(Int16 *samples, Uint32 bytes, Output::SampleFormat fmt);

private :

    float m_volume; // ��ǰ�������������(amplitude)����ΧΪ[0..1]
};

} // namespace sf

#endif
