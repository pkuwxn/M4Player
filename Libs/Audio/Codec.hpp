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

#ifndef SFML_CODEC_HPP
#define SFML_CODEC_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Config.hpp"
#include "Service.hpp"
#include "Time.hpp"

namespace sf {
class InputStream;

////////////////////////////////////////////////////////////
/// \brief Provide read and write access to sound files
///
////////////////////////////////////////////////////////////
class Codec : public ServiceBase {
public :

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~Codec() {}

    ////////////////////////////////////////////////////////////
    /// \brief Get the total duration of the music
    ///
    /// \return Music duration, in microseconds
    ///
    ////////////////////////////////////////////////////////////
    virtual Uint32 getDuration() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of channels used by the sound
    ///
    /// \return Number of channels (1 = mono, 2 = stereo)
    ///
    ////////////////////////////////////////////////////////////
    virtual Uint32 getChannelCount() const = 0;

    /**
     * Audio Sample Formats
     *
     * @par
     * The data described by the sample format is always in native-endian order.
     * Sample values can be expressed by native C types, hence the lack of a signed
     * 24-bit sample format even though it is a common raw audio data format.
     *
     * @par
     * The floating-point formats are based on full volume being in the range
     * [-1.0, 1.0]. Any values outside this range are beyond full volume level.
     */
    enum SampleFormat {
        SAMPLE_FMT_NONE = -1,                   ///< Not use, only for validation
        SAMPLE_FMT_U8 = 8,                      ///< Unsigned 8 bits
        SAMPLE_FMT_S16 = 16,                    ///< Signed 16 bits
        SAMPLE_FMT_S32 = 32,                    ///< Signed 32 bits
        SAMPLE_FMT_FLT = sizeof(float) * 8 + 1, ///< Float
        SAMPLE_FMT_DBL = sizeof(double) * 8,    ///< Double
    };

    ////////////////////////////////////////////////////////////
    /// \brief Get the sample format of the sound
    ///
    ////////////////////////////////////////////////////////////
    virtual SampleFormat getSampleFormat() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the sample rate of the sound
    ///
    /// \return Sample rate, in samples per second
    ///
    ////////////////////////////////////////////////////////////
    virtual Uint32 getSampleRate() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the bit rate of the sound
    ///
    /// \return Bit rate, in kilobits per second
    ///
    ////////////////////////////////////////////////////////////
    virtual Uint32 getBitRate() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file for reading
    ///
    /// \param filename Path of the sound file to load
    ///
    /// \return True if the file was successfully opened
    ///
    ////////////////////////////////////////////////////////////
    virtual bool openRead(const char *filename) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file in memory for reading
    ///
    /// \param data        Pointer to the file data in memory
    /// \param sizeInBytes Size of the data to load, in bytes
    ///
    /// \return True if the file was successfully opened
    ///
    ////////////////////////////////////////////////////////////
    virtual bool openRead(const void *data, Uint32 sizeInBytes) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file from a custom stream for reading
    ///
    /// \param stream Source stream to read from
    ///
    /// \return True if the file was successfully opened
    ///
    ////////////////////////////////////////////////////////////
    virtual bool openRead(InputStream &stream) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief a the sound file for writing
    ///
    /// \param filename     Path of the sound file to write
    /// \param channelCount Number of channels in the sound
    /// \param sampleRate   Sample rate of the sound
    ///
    /// \return True if the file was successfully opened
    ///
    ////////////////////////////////////////////////////////////
    virtual bool openWrite(const char *filename, Uint32 channelCount, Uint32 sampleRate) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Read audio samples from the loaded sound
    ///
    /// \param data        Pointer to the sample array to fill
    /// \param sampleCount Number of samples to read
    ///
    /// \return Number of samples actually read (may be less than \a sampleCount)
    ///
    ////////////////////////////////////////////////////////////
    virtual Uint32 read(Int16 *data, Uint32 sampleCount) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Write audio samples to the file
    ///
    /// \param data        Pointer to the sample array to write
    /// \param sampleCount Number of samples to write
    ///
    ////////////////////////////////////////////////////////////
    virtual void write(const Int16 *data, Uint32 sampleCount) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current read position in the file
    ///
    /// \param timeOffset New playing position, from the beginning of the file
    ///
    /// \return Ture if succeeded
    ///
    ////////////////////////////////////////////////////////////
    virtual bool seek(Time timeOffset) = 0;

protected :

    Codec() {}

private :

    Codec(const Codec &);
    Codec &operator = (const Codec &);
};

} // namespace sf

#endif // SFML_CODEC_HPP
