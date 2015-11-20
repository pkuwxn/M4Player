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

#ifndef SFML_FFMPEG_HPP
#define SFML_FFMPEG_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <Codec.hpp>

#pragma warning( push )
#pragma warning( disable: 4244 )
extern "C"
{
#   define __STDC_CONSTANT_MACROS
#   include <libavformat/avformat.h>
#   include <libavcodec/avcodec.h>

// SampleFormat 被 FFmpeg 内部使用了，使用 GCC 会造成冲突
#   ifdef SampleFormat
#       undef SampleFormat
#   endif
#   ifdef SAMPLE_FMT_NONE
#       undef SAMPLE_FMT_NONE
#   endif
#   ifdef SAMPLE_FMT_U8
#       undef SAMPLE_FMT_U8
#   endif
#   ifdef SAMPLE_FMT_S16
#       undef SAMPLE_FMT_S16
#   endif
#   ifdef SAMPLE_FMT_S32
#       undef SAMPLE_FMT_S32
#   endif
#   ifdef SAMPLE_FMT_FLT
#       undef SAMPLE_FMT_FLT
#   endif
#   ifdef SAMPLE_FMT_DBL
#       undef SAMPLE_FMT_DBL
#   endif
};
#pragma warning( pop )




namespace sf
{
	class InputStream;
}

using namespace sf;

////////////////////////////////////////////////////////////
/// 使用 FFmpeg 来进行解码
///
////////////////////////////////////////////////////////////
class FFmpeg : public Codec
{
public :

	////////////////////////////////////////////////////////////
	/// \brief Default constructor
	///
	////////////////////////////////////////////////////////////
	FFmpeg();

	////////////////////////////////////////////////////////////
	/// \brief Destructor
	///
	////////////////////////////////////////////////////////////
	virtual ~FFmpeg();

	virtual Uint32 getDuration() const;
	virtual Uint32 getChannelCount() const;
	virtual SampleFormat getSampleFormat() const;
	virtual Uint32 getSampleRate() const;
	virtual Uint32 getBitRate() const;

	virtual bool openRead(const char* filename);
	virtual bool openRead(const void* data, Uint32 sizeInBytes);
	virtual bool openRead(InputStream& stream);
	virtual bool openWrite(const char* filename, Uint32 channelCount, Uint32 sampleRate);

	virtual Uint32 read(Int16* data, Uint32 sampleCount);
	virtual void write(const Int16* data, Uint32 sampleCount);
	virtual bool seek(Time timeOffset);

private :

	void close();

	bool openStream(AVFormatContext* fc, Uint32 streamIndex);

	// 释放 m_rawPacket
	void freeRawPacket();

	////////////////////////////////////////////////////////////
	// Member data
	////////////////////////////////////////////////////////////
	AVStream*			m_audioStream;
	AVFormatContext*	m_formatContext;
	int					m_audioStreamIndex;
	
	AVFrame*            m_decodeBuffer;
	Uint32				m_decoded; // 已解码数据包的大小
	Uint32				m_decodedOffset; // 已解码数据包中的位置
	
	AVPacket			m_rawPacket;
	int					m_rawOffset; // 未解码数据包中的位置
};

#endif // SFML_FFMPEG_HPP
