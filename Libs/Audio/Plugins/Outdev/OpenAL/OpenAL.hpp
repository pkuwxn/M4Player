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

#ifndef SFML_OPENAL_HPP
#define SFML_OPENAL_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <Outdev.hpp>
#include <vector>
#include "AudioDevice.hpp"



using namespace sf;

////////////////////////////////////////////////////////////
struct Chunk;



////////////////////////////////////////////////////////////
/// \brief OpenAL  output backend
///
////////////////////////////////////////////////////////////
class OpenAL : public OutputHelper<BuiltinVolumePolicy>
{
public :

	typedef BuiltinVolumePolicy VolumePolicy;

	/// 构造函数
	OpenAL();

	/// 析构函数
	virtual ~OpenAL();

public :

	virtual bool isAsync() const { return false; }
	virtual bool open(Uint32 deviceId);
	virtual void close();
	virtual bool isOk() const;
	virtual void* getBuffer();
	virtual bool write(void* buffer, Uint32 bufferSize);
	virtual bool setSampleFormat(SampleFormat fmt);
	virtual bool play(Codec* decoder);
	virtual void pause();
	virtual void stop();
	virtual bool streamData();
	virtual void preSetPlayingOffset(Time timeOffset);
	virtual void setPlayingOffset(Time timeOffset);
	virtual Time getPlayingOffset() const;
	virtual void setVolume(float volume);
	virtual float getVolume() const;
	virtual Status getStatus() const;

private :

	////////////////////////////////////////////////////////////
	/// \brief 严格按照 OpenAL 给出的状态信息组织输出
	///
	////////////////////////////////////////////////////////////
	Status doGetStatus() const;

	////////////////////////////////////////////////////////////
	/// \brief Fill a new buffer with audio samples, and append
	///        it to the playing queue
	///
	/// This function is called as soon as a buffer has been fully
	/// consumed; it fills it again and inserts it back into the
	/// playing queue.
	///
	/// \param bufferIndex Index of the buffer to fill (in [0, BufferCount - 1])
	///
	/// \return True if the stream source has requested to stop, false otherwise
	///
	////////////////////////////////////////////////////////////
	void fillAndPushBuffer(Uint32 bufferIndex);

	////////////////////////////////////////////////////////////
	/// \brief Fill the audio buffers and put them all into the playing queue
	///
	/// This function is called when playing starts and the
	/// playing queue is empty.
	///
	/// \return True if the derived class has requested to stop, false otherwise
	///
	////////////////////////////////////////////////////////////
	void fillQueue();

	////////////////////////////////////////////////////////////
	/// \brief Clear all the audio buffers and empty the playing queue
	///
	/// This function is called when the stream is stopped.
	///
	////////////////////////////////////////////////////////////
	void clearQueue();

	////////////////////////////////////////////////////////////
	/// \brief Request a new chunk of audio samples from the stream source
	///
	/// This function fills the chunk from the next samples
	/// to read from the audio file.
	///
	/// \param data Chunk of data to fill
	///
	/// \return True to continue playback, false to stop
	///
	////////////////////////////////////////////////////////////
	bool getData(Chunk& data);

	////////////////////////////////////////////////////////////
	/// \brief 清空缓冲区队列并依情况重新填充
	///
	////////////////////////////////////////////////////////////
	void flushAndRefill();

	////////////////////////////////////////////////////////////
	/// \brief 获取已被假如队列中的缓冲区数目
	///
	////////////////////////////////////////////////////////////
	Int32 getQueuedCount() const;

private :

	////////////////////////////////////////////////////////////
	// Member data
	////////////////////////////////////////////////////////////

	AudioDevice		m_device;		///< OpenAL device

	Uint32			m_source;		///< OpenAL source identifier
	Uint32			m_alFormat;     ///< Format of the internal sound buffers
	/*! Streaming state (true = playing, false = stopped) */
	bool			m_isStreaming;
	bool			m_requestStop;	///< Streaming state

	////////////////////////////////////////////////////////////

	enum
	{
		BufferCount = 3 ///< Number of audio buffers used by the streaming loop
	};

	/*! Sound buffers used to store temporary audio data */
	Uint32			m_buffers[BufferCount];	
};


#endif // SFML_OPENAL_HPP
