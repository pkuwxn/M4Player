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

#ifndef SFML_MPG123_HPP
#define SFML_MPG123_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <Codec.hpp>

#ifdef _WIN32
#   define ssize_t int // TODO:
#endif
#include <mpg123.h>



namespace sf {
class InputStream;
}

using namespace sf;

////////////////////////////////////////////////////////////
/// 使用 mpg123 来进行解码
///
////////////////////////////////////////////////////////////
class mpg123 : public Codec {
public :

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    mpg123();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~mpg123();

    void cleanUp();

    virtual Uint32 getDuration() const;
    virtual Uint32 getChannelCount() const;
    virtual SampleFormat getSampleFormat() const;
    virtual Uint32 getSampleRate() const;
    virtual Uint32 getBitRate() const;

    virtual bool openRead(const char *filename);
    virtual bool openRead(const void *data, Uint32 sizeInBytes);
    virtual bool openRead(InputStream &stream);
    virtual bool openWrite(const char *filename, Uint32 channelCount, Uint32 sampleRate);

    virtual Uint32 read(Int16 *data, Uint32 sampleCount);
    virtual void write(const Int16 *data, Uint32 sampleCount);
    virtual bool seek(Time timeOffset);

private :

    // 关闭打开的文件
    void close();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    mpg123_handle      *m_handle;
    int                 m_channelCount;
    int                 m_encoding;
    long                m_sampleRate;
};

#endif // SFML_MPG123_HPP
