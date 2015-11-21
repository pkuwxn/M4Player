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
#include "FFmpeg.hpp"
#include <InputStream.hpp>
#include <Debug.hpp>

using namespace sf;


template<typename T>
T min(T x, T y) {
    return x < y ? x : y;
}


////////////////////////////////////////////////////////////
static bool gsFFmpegIntialized = false;
enum {
    INVALID_AUDIO_STREAM_INDEX = ~0, // TODO: 这个值有没有被 FFmpeg 使用？
};



////////////////////////////////////////////////////////////
FFmpeg::FFmpeg()
    : m_audioStream(NULL),
      m_formatContext(NULL),
      m_audioStreamIndex(INVALID_AUDIO_STREAM_INDEX),
      m_decodeBuffer(NULL),
      m_decoded(0),
      m_decodedOffset(0),
      m_rawOffset(0),
      m_swrContext(NULL) {
    if (!gsFFmpegIntialized) {
        av_register_all();

        gsFFmpegIntialized = true;
    }

    m_rawPacket.data = NULL;
}


////////////////////////////////////////////////////////////
FFmpeg::~FFmpeg() {
    close();

    if (m_decodeBuffer) {
        av_free(m_decodeBuffer);
    }

    if (m_swrContext) {
        swr_free(&m_swrContext);
    }
}


////////////////////////////////////////////////////////////
void FFmpeg::close() {
    if (!m_audioStream) {
        return;
    }

    if (m_rawPacket.data) {
        freeRawPacket();
    }

    m_decoded = 0;
    m_decodedOffset = 0;

    avcodec_close(m_audioStream->codec);
    avformat_close_input(&m_formatContext);
    m_formatContext = NULL;

    m_audioStream = NULL;
    m_audioStreamIndex = INVALID_AUDIO_STREAM_INDEX;
}


////////////////////////////////////////////////////////////
Uint32 FFmpeg::getDuration() const {
    return Uint32(m_audioStream->duration * m_audioStream->time_base.num /
                  m_audioStream->time_base.den * 1000.0);
}


////////////////////////////////////////////////////////////
Uint32 FFmpeg::getChannelCount() const {
    return m_audioStream->codec->channels;
}


////////////////////////////////////////////////////////////
bool FFmpeg::isPlanar() const {
    return m_audioStream->codec->sample_fmt >= (AV_SAMPLE_FMT_NB / 2);
}


////////////////////////////////////////////////////////////
Codec::SampleFormat FFmpeg::getSampleFormat() const {
    // for AV_SAMPLE_FMT_XXP
    switch (m_audioStream->codec->sample_fmt % (AV_SAMPLE_FMT_NB / 2)) {
    case AV_SAMPLE_FMT_U8:
        return SAMPLE_FMT_U8;

    case AV_SAMPLE_FMT_S16:
        return SAMPLE_FMT_S16;

    case AV_SAMPLE_FMT_S32:
        return SAMPLE_FMT_S32;

    case AV_SAMPLE_FMT_FLT:
        return SAMPLE_FMT_FLT;

    case AV_SAMPLE_FMT_DBL:
        return SAMPLE_FMT_DBL;

    default:

        fprintf(stderr, "[%s:%d] Unsupported audio sample format: %d\n",
                __FILE__, __LINE__,
                static_cast<int>(m_audioStream->codec->sample_fmt));

        return SAMPLE_FMT_NONE;
    }
}


////////////////////////////////////////////////////////////
Uint32 FFmpeg::getSampleRate() const {
    return m_audioStream->codec->sample_rate;
}


////////////////////////////////////////////////////////////
Uint32 FFmpeg::getBitRate() const {
    // TODO: 这里似乎是一个约值
    return static_cast<Uint32>(m_audioStream->codec->bit_rate / 1000);
}


////////////////////////////////////////////////////////////
bool FFmpeg::openRead(const char *fileName) {
    close();

    //=========================================

    AVFormatContext *fc = NULL;
    int audioStream = -1; // 必须放在这里，注意下面的 goto OPEN_ERRROR;

    int ret = 0;
    char errorBuffer[1024];

    if (ret = avformat_open_input(&fc, fileName, NULL, NULL)) {
        fprintf(stderr, "avformat_open_input() failed: `%s`\n", fileName);
        
        av_strerror(ret, errorBuffer, sizeof(errorBuffer));
        fprintf(stderr, "%s\n", errorBuffer);

        return false;
    }

    if (ret = avformat_find_stream_info(fc, NULL)) {
        fprintf(stderr, "avformat_find_stream_info() failed: `%s`\n", fileName);

        av_strerror(ret, errorBuffer, sizeof(errorBuffer));
        fprintf(stderr, "%s\n", errorBuffer);

        goto OPEN_ERRROR;
    }

    //=========================================

    for (Uint32 i = 0; i < fc->nb_streams; i++) {
        if (fc->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (avcodec_find_decoder(fc->streams[i]->codec->codec_id)) {
                audioStream = i;
                break;
            }
        }
    }

    if (audioStream == -1) {
        fprintf(stderr, "No audio stream found: `%s`\n", fileName);
        goto OPEN_ERRROR;
    }

    if (!openStream(fc, audioStream)) {
        fprintf(stderr, "FFmpeg::openStream() failed: `%s`\n", fileName);
        goto OPEN_ERRROR;
    }

    m_formatContext = fc;
    m_audioStreamIndex = audioStream;
    m_audioStream = m_formatContext->streams[m_audioStreamIndex];

    assert(m_decoded == 0);
    assert(m_decodedOffset == 0);
    assert(m_rawPacket.data == NULL);
    assert(m_rawOffset == 0);

    return true;

OPEN_ERRROR:
    avformat_close_input(&fc);

    return false;
}


////////////////////////////////////////////////////////////
bool FFmpeg::openStream(AVFormatContext *fc, Uint32 streamIndex) {
    if (streamIndex >= fc->nb_streams) {
        return false;
    }

    AVCodecContext *cdContex = fc->streams[streamIndex]->codec;
    AVCodec *codec = avcodec_find_decoder(cdContex->codec_id);
    if (!codec) {
        fprintf(stderr, "avcodec_find_decoder() failed.\n");
        return false;
    }

    if (avcodec_open2(cdContex, codec, NULL) < 0) {
        fprintf(stderr, "avcodec_open2() failed.\n");
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
bool FFmpeg::openRead(const void *data, Uint32 sizeInBytes) {
    return false;
}


////////////////////////////////////////////////////////////
bool FFmpeg::openRead(InputStream &stream) {
    return false;
}


////////////////////////////////////////////////////////////
bool FFmpeg::openWrite(const char *filename, Uint32 channelCount, Uint32 sampleRate) {
    return false;
}


////////////////////////////////////////////////////////////
bool FFmpeg::planarToInterleaved() {
    AVSampleFormat packedFmt = static_cast<AVSampleFormat>
        (m_audioStream->codec->sample_fmt - AV_SAMPLE_FMT_NB / 2);

    Uint64 channelLayout = m_audioStream->codec->channel_layout;
    if (channelLayout == 0) {
        channelLayout = (getChannelCount() > 1) ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO;
    }

    m_swrContext = swr_alloc_set_opts
        (m_swrContext,
         m_audioStream->codec->channel_layout,
         packedFmt,
         m_audioStream->codec->sample_rate,
         m_audioStream->codec->channel_layout,
         m_audioStream->codec->sample_fmt,
         m_audioStream->codec->sample_rate,
         0,
         NULL);

    if (!m_swrContext) {
        fprintf(stderr, "[%s:%d] swr_alloc_set_opts() failed.\n", __FILE__, __LINE__);
        return false;
    }

    int ret = swr_init(m_swrContext);
    if (ret < 0) {
        char buf[1024];
        av_strerror(ret, buf, sizeof(buf));

        fprintf(stderr, "[%s:%d] swr_init() failed.\n%s\n",
                __FILE__, __LINE__, buf);

        return false;
    }

    uint8_t *swrBuffer = &m_swrBuffer[0];

    // Convert audio data to interleaved format
    ret = swr_convert(m_swrContext,
                     &swrBuffer, 
                      m_decodeBuffer->nb_samples, 
                     (const uint8_t **)m_decodeBuffer->extended_data, 
                      m_decodeBuffer->nb_samples);

    if (ret < m_decodeBuffer->nb_samples) {
        fprintf(stderr, "[%s:%d] swr_convert: Error while converting %d samples: %d.\n",
                __FILE__, __LINE__, m_decodeBuffer->nb_samples, ret);
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
Uint8 *FFmpeg::getDecodeBuffer() {
    if (isPlanar()) {
        return &m_swrBuffer[0];
    } else {
        return m_decodeBuffer->data[0];
    }
}

////////////////////////////////////////////////////////////
Uint32 FFmpeg::read(Int16 *data, Uint32 sampleCount) {
    Uint8 *outBuf = (Uint8 *) data;
    const Uint32 bufferSize = sampleCount * sizeof(Uint16);
    Uint32 outBufOffset = 0;

    // 上次剩余的一些手尾
    if (m_decodedOffset < m_decoded) {
        Uint32 bytesToCopy = min(bufferSize, m_decoded - m_decodedOffset);
        memcpy(outBuf, getDecodeBuffer() + m_decodedOffset, bytesToCopy);

        outBufOffset += bytesToCopy;
        m_decodedOffset += bytesToCopy;
    }

    while (outBufOffset < bufferSize) {
        /* Read a frame */
        while (!m_rawPacket.data) {
            // 读取失败，直接返回
            int ret = av_read_frame(m_formatContext, &m_rawPacket);
            if (ret < 0) {
                // 读到文件末尾不算错误
                if (ret != AVERROR_EOF) {
                    fprintf(stderr, "[%s:%d] av_read_frame() failed：Error %d.\n",
                            __FILE__, __LINE__, ret);
                }

                return outBufOffset / sizeof(Int16);
            }

            /* Ignore any other substreams */
            if (m_rawPacket.stream_index != m_audioStreamIndex) {
                freeRawPacket();
            } else {
                // 成功读取一个原始帧，准备解压
                m_rawOffset = 0;
            }
        }

        //===========================================================
        // Decode the frame

        AVPacket tmp; /* 临时用 */
        av_init_packet(&tmp);
        tmp.data = m_rawPacket.data + m_rawOffset;
        tmp.size = m_rawPacket.size - m_rawOffset;

        if (!m_decodeBuffer) {
            m_decodeBuffer = av_frame_alloc();
        }

        Int32 frameDecoded = 0;
        const int rawUsed = avcodec_decode_audio4
            (m_audioStream->codec, m_decodeBuffer, &frameDecoded, &tmp);

        //===========================================================
        // Copy out

        if (rawUsed >= 0) {
            // TODO: 字节数的计算
            m_decoded = getSampleFormat() / 8 * getChannelCount() * m_decodeBuffer->nb_samples;
            m_decodedOffset = 0;

            if (isPlanar()) {
                m_swrBuffer.resize(m_decoded);

                if (!planarToInterleaved()) {
                    goto ERROR;
                }
            }

            m_rawOffset += rawUsed;
            if (m_rawOffset >= m_rawPacket.size) {
                freeRawPacket();
            }

            Uint32 bytesToCopy = min(m_decoded, bufferSize - outBufOffset);
            memcpy(outBuf + outBufOffset, getDecodeBuffer(), bytesToCopy);

            outBufOffset += bytesToCopy;
            m_decodedOffset += bytesToCopy;
        } else { // 错误检测

            // 在 Linux 下面 rawUsed == 0 后再调用 av_read_frame() 不会返回错误，而是不断提示：
            // Truncating packet of size 1024 to 1
            if (rawUsed < 0) {
                fprintf(stderr, "[%s:%d] avcodec_decode_audio4() failed：Error %d.\n",
                        __FILE__, __LINE__, rawUsed);
            }
ERROR:
            freeRawPacket();

            return outBufOffset / sizeof(Int16);
        }
    }

    return sampleCount;
}


////////////////////////////////////////////////////////////
void FFmpeg::write(const Int16 *data, Uint32 sampleCount) {

}


////////////////////////////////////////////////////////////
bool FFmpeg::seek(Time timeOffset) {
    assert(m_formatContext);
    assert(m_audioStreamIndex != INVALID_AUDIO_STREAM_INDEX);

    float timeBase = float(m_audioStream->time_base.num) / m_audioStream->time_base.den;
    int64_t timeStamp = int64_t(float(timeOffset.asMilliseconds()) / 1000 / timeBase);

    int ret = av_seek_frame(m_formatContext, m_audioStreamIndex, timeStamp, AVSEEK_FLAG_ANY);
    if (ret < 0) {
        fprintf(stderr, "[%s:%d]av_seek_frame() failed. Error: %d.\n",
                __FILE__, __LINE__, ret);

        return false;
    }

    //==================================================
    // 更新解码状态

    m_decoded = m_decodedOffset = 0;
    if (m_rawPacket.data) {
        freeRawPacket();
    }

    avcodec_flush_buffers(m_audioStream->codec);

    return true;
}


////////////////////////////////////////////////////////////
void FFmpeg::freeRawPacket() {
    av_packet_unref(&m_rawPacket);
    m_rawPacket.data = NULL;
    m_rawOffset = 0;
}


////////////////////////////////////////////////////////////
#include "Client.hpp"
DEFINE_STD_OOPLUGIN(FFmpeg, "Decoder/MP1;|MPEG Audio Layer 1;"
                   "Decoder/MP2;|MPEG Audio Layer 2;"
                   "Decoder/MP3;|MPEG Audio Layer 3;"
                   "Decoder/WMA;|Windows Media Audio 9;"
                   "Decoder/ASF;|Windows Media Audio 9;"
                   "Decoder/WAV;|PCM Audio;"
                   "Decoder/OGG;|Vorbis/Ogg Audio;"
                   "Decoder/FLAC;|Free Lossless Audio;"
                   "Decoder/MIDI;|MIDI Music;"
                   "Decoder/MP4;|MPEG-4 AAC Audio;"
                   "Decoder/AAC;|Advanced Audio Coding;"
                   "Decoder/AMR;|Adaptive Multi-Rate Audio;"
                   "Decoder/RA;|RealAudio;"
                   "Decoder/RM;|RealAudio;"
                   "Decoder/APE;|Monkey's Audio;")
