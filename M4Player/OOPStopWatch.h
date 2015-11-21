/***************************************************************
 * Name:      OOPStopWatch.h
 * Purpose:   播放引擎计时器
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-09
 **************************************************************/
#pragma once

namespace sf {
class Music;
}

class MusicEndingDetector;

/// 播放引擎计时器
class OOPStopWatch {
public:

    /// 构造函数
    OOPStopWatch(const sf::Music &music, MusicEndingDetector &endingDetector);

    /// Get elapsed time since the last Start() in milliseconds
    size_t Time();

    /// 是否正在计时（没有被暂停）
    bool IsRunning() const;

private:

    const sf::Music &m_music;
    MusicEndingDetector &m_endingDectector;
};

/*! 歌曲播放已顺利完结的通知事件 */
wxDECLARE_EVENT(MM_EVT_SONG_FINISHED, wxCommandEvent);

/// 歌曲播放完毕事件探测器
class MusicEndingDetector {
public:

    /// 构造函数
    MusicEndingDetector(const sf::Music &music);

    /// 开始播放
    void Play();

    /// 更新内部状态，使之能正确激发回调函数
    void Update();

    /// 重置探测器(手动停止播放)
    void Stop();

private:

    const sf::Music &m_music;

    // 是否已发送“歌曲顺利播放完毕”通知事件
    bool m_finishEventSent;
};
