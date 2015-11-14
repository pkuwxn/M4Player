/***************************************************************
 * Name:      ILyric.h
 * Purpose:   歌词秀的抽象接口
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-05
 **************************************************************/
#pragma once

class OOPStopWatch;
class OOPLyricParser;

/// 歌词秀实现
/// 
/// 没有虚拟析构函数！不会通过 ILyric 指针来删除对象。
class ILyric
{
public:

	/// 默认构造函数
	ILyric();

	/// 是否可以进行显示
	virtual bool IsOk() const = 0;

	/// 设定当前歌曲的计时器
	virtual void SetStopWatch(OOPStopWatch& sw) { m_stopWatch = &sw; }

	/// 绑定歌词文件解析器，初始化列表控件
	virtual void AttachParser(const OOPLyricParser& parser) = 0;

	/// 开始歌词显示
	///
	/// @see SetStopWatch, AttachParser
	virtual void Start() = 0;

	/// 暂停歌词显示
	virtual void Pause() = 0;

	/// 停止歌词显示
	virtual void Stop(wxDC* pDC) = 0;

	/// 清除当前歌词
	virtual void ClearLyric(wxDC* pDC) = 0;

	/// 前往某进度(0 <= \a percentage <= 1)
	///
	/// 保留这个函数是因为 wxTimer::Notify() 中不会出现超过一行的跨越，
	/// 大幅度移动时可能会出现不断滚动的情况，而且不会正确处理向前跳跃
	/// 时的情况。
	virtual void GoTo(double percentage, wxDC* pDC, bool bPaused) = 0;

	/// \brief 设置默认的的交互性告示文本
	void SetDefualtInteractiveOutput(const wxString& msg);

	/// \brief 设置当歌词为空时，显示的一小段交互性告示文本
	void SetInteractiveOutput(const wxString& msg, wxDC* pDC);

	/// \brief 重置为默认的交互性告示文本
	void ResetInteractiveOutput(wxDC* pDC);

protected:

	/// \brief 获取当前有效的交互性告示文本
	wxString GetInteractiveOutput() const;

	enum {
		/*! 歌词刷新频率(单位：ms)，用来使用卡拉 OK 方式显示的歌词，以及使用渐变色
		 *  还原上一句歌词
		 */
		REFRESH_INTERVAL_MS = 40,
	};

private:

	/// \brief 知悉交互性告示文本已被改变
	virtual void DoSetInteractiveOutput(wxDC* pDC) = 0;

	/// \brief 知悉外部播放计时器已被设定
	virtual void DoSetStopWatch(OOPStopWatch& sw) {}

protected:

	// 外部播放计时器，精确同步歌词显示
	OOPStopWatch* m_stopWatch;

private:

	// 当歌词为空时，显示一小段交互性告示文本
	wxString m_defaultInteractiveOutput;
	wxString m_interactiveOutput;
};
