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

#ifndef SFML_STOPWATCH_HPP
#define SFML_STOPWATCH_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Config.hpp"

namespace sf {
////////////////////////////////////////////////////////////
// 计时器
//
////////////////////////////////////////////////////////////
class StopWatch {
public :

    /// Default constructor
    ///
    /// Will not starts the stop watch automatically.
    StopWatch();

    /// Start the stop watch at the moment @a t0
    void Start(Uint32 t0);

    /// 设置起始时刻计数，但不启动计时器
    ///
    /// 此时计时器此时状态为“暂停”，后续启动计时器需要调用
    /// @link Resume() @endlink。
    /// @example
    /// StopWatch sw;
    /// // Do something...
    /// sw.Reset(XXX);
    /// // Do something...
    /// sw.Resume();
    void Reset(Uint32 t0);

    /// 中止计时
    void Stop();

    /// Pause the stop watch
    void Pause();

    /// Resume it
    void Resume();

    /// 是否正在计时（没有被暂停）
    bool IsRunning() const {
        return !m_paused;
    }

    /// Get elapsed time since the last Start() in milliseconds
    Uint32 Time() const;

private :

    // Returns the elapsed time since last resume the stop watch
    Uint32 GetElapsedTime() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Uint32      m_t0;           ///< The time of the last Start()

    Uint32      m_elapsed;
    Int64       m_sliceStart;
    bool        m_paused;       ///< 是否已被暂停
};


} // namespace sf


#endif // SFML_STOPWATCH_HPP
