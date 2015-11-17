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

#include "mpg123.hpp"
#include <InputStream.hpp>
#include <wchar.h>
#include <stdio.h>
#include <assert.h>




////////////////////////////////////////////////////////////
/// 初始化 mpg123 动态库
///
////////////////////////////////////////////////////////////
class mpg123Initializer
{
public:

	mpg123Initializer()
	{
		int err = mpg123_init();
		if (err != MPG123_OK)
		{
			fprintf(stderr, "Basic setup goes wrong: %s", mpg123_plain_strerror(err));
		}
	}

	~mpg123Initializer()
	{
		/* It's really to late for error checks here;-) */
		mpg123_exit();
	}
};


////////////////////////////////////////////////////////////
mpg123Initializer& insureMpg123Library()
{
	static mpg123Initializer s_initializer;
	return s_initializer;
}


////////////////////////////////////////////////////////////
mpg123::mpg123()
	: m_handle( NULL ), m_channelCount( 0 ), m_encoding( 0 ), m_sampleRate( 0 )
{
	insureMpg123Library();

	int err;
	m_handle = mpg123_new(NULL, &err);

	if (m_handle == NULL)
	{
		fprintf(stderr, "mpg123_new() failed: %s", mpg123_plain_strerror(err));
	}
}


////////////////////////////////////////////////////////////
mpg123::~mpg123()
{
	cleanUp();
}


////////////////////////////////////////////////////////////
void mpg123::cleanUp()
{
	if (m_handle)
	{
		mpg123_delete(m_handle);
		m_handle = NULL;
	}
}


////////////////////////////////////////////////////////////
void mpg123::close()
{
	if (m_handle)
	{
		int err = mpg123_close(m_handle);
		if (err != MPG123_OK)
		{
			fprintf(stderr, "mpg123_close() failed: %s", mpg123_plain_strerror(err));
		}
	}
}


////////////////////////////////////////////////////////////
Uint32 mpg123::getDuration() const
{
	Uint32 sampleCount = mpg123_length(m_handle);
	if ((sampleCount > 0) && (m_sampleRate > 0)) {
		return Uint32(float(sampleCount) / m_sampleRate * 1000);
	}

	return 0;
}


////////////////////////////////////////////////////////////
Uint32 mpg123::getChannelCount() const
{
	return m_channelCount;
}


////////////////////////////////////////////////////////////
Codec::SampleFormat mpg123::getSampleFormat() const
{
	return SAMPLE_FMT_S16;
}


////////////////////////////////////////////////////////////
Uint32 mpg123::getSampleRate() const
{
	return m_sampleRate;
}


////////////////////////////////////////////////////////////
Uint32 mpg123::getBitRate() const
{
	mpg123_frameinfo fi;
	mpg123_info(m_handle, &fi);

	return fi.bitrate;
}


////////////////////////////////////////////////////////////
bool mpg123::openRead(const char* filename)
{
	close();

	/* Let mpg123 work with the file, that excludes MPG123_NEED_MORE messages. */
	if (   mpg123_open(m_handle, filename) != MPG123_OK
		/* Peek into track and get first output format. */
		|| mpg123_getformat(m_handle, &m_sampleRate, &m_channelCount, &m_encoding) != MPG123_OK)
	{
		fprintf(stderr, "Trouble with mpg123: %s\n", mpg123_strerror(m_handle));
		cleanUp(); // TODO: 是否需要？
		
		return false;
	}

    if (m_encoding != MPG123_ENC_SIGNED_16)
    {
		/* Signed 16 is the default output format anyways; it would actually 
		   by only different if we forced it.
           So this check is here just for this explanation. */
        fprintf(stderr, "Bad encoding: 0x%x!\n", m_encoding);
		cleanUp();

        return false;
    }

    /* Ensure that this output format will not change (it could, when we allow it). */
    mpg123_format_none(m_handle);
    mpg123_format(m_handle, m_sampleRate, m_channelCount, m_encoding);

	return true;
}


////////////////////////////////////////////////////////////
bool mpg123::openRead(const void* data, Uint32 sizeInBytes)
{
	return false;
}


////////////////////////////////////////////////////////////
bool mpg123::openRead(InputStream& stream)
{
	return false;
}


////////////////////////////////////////////////////////////
bool mpg123::openWrite(const char* filename, Uint32 channelCount, Uint32 sampleRate)
{
	return false;
}


////////////////////////////////////////////////////////////
Uint32 mpg123::read(Int16* data, Uint32 sampleCount)
{
	Uint32 bytesRead;
	mpg123_read(m_handle, (Uint8 *) data, sampleCount * sizeof(Int16), &bytesRead);

	return bytesRead / sizeof(Int16);
}


////////////////////////////////////////////////////////////
void mpg123::write(const Int16* data, Uint32 sampleCount)
{

}


////////////////////////////////////////////////////////////
bool mpg123::seek(Time timeOffset)
{
	assert(m_handle);

	// TODO: 不用考虑双声道时的情况？
	// 加了确实是不行的
	off_t sampleOffset = (off_t)(timeOffset.asMilliseconds() * (m_sampleRate / 1000));
	off_t realOffset = mpg123_seek(m_handle, sampleOffset, SEEK_SET);
	if (realOffset < 0) {
	    fprintf(stderr, "mpg123_seek() failed: %s\n", mpg123_strerror(m_handle));
	    return false;
	}
	
	return true;
}


////////////////////////////////////////////////////////////
#include "Client.hpp"
DEFINE_STD_OOPLUGIN( mpg123, "Decoder/MP1;|MPEG Audio Layer 1;"
					         "Decoder/MP2;|MPEG Audio Layer 2;"
					         "Decoder/MP3;|MPEG Audio Layer 3;" )


