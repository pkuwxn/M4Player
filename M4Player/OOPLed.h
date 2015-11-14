#pragma once
#include "VdkControl.h"

class OOPStopWatch;

/// \brief 显示已播放时间的模拟 LED 进度表
class OOPLed : public VdkControl, public wxTimer
{
public:

	/// \brief 默认构造函数
	OOPLed();

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 
	void Create(VdkWindow* Window, 
			    const wxString& strName, 
			    const wxRect& rc,
				const wxBitmap& bmLed,
				const align_type& align);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 当前歌曲的计时器
	void SetStopWatch(OOPStopWatch* sw) { m_stopWatch = sw; }

	/// \brief 开始显示
	void StartLed();

	/// \brief 暂停显示
	void PauseLed();

	/// \brief 继续显示
	void ResumeLed();

	/// \brief 停止显示
	void StopLed(wxDC* pDC);

	/// \brief 显示某个确定的时间
	/// \param nTimeInMS 要显示的时间，以毫秒(MS)计算
	void Combine(long nTimeInMS, wxDC& pDC);

private:

	// 初始化控件
	void Init();

	// 开始显示
	void DoStart();

	// 绘制控件
	virtual void DoDraw(wxDC& dc);

	// 继承自 wxTimer
	virtual void Notify();

	// 获取当前时间除以 gs_Internal 的余数
	//
	// 因为 OOPLed 是以 1s 为步进进行显示，故当在 1s 中间进行暂停
	// 再直接恢复（忽略 < 1s 部分）就会造成较大误差。
	int GetRestTimeForThisSecond();

private:

	wxBitmap			m_bmAll;
	
	int					m_nElWidth;		// 元素宽度
	int					m_nElHeight;	// 元素高度

	int					m_nPos[4];		// 四个元素相对于 m_Rect.x 的偏移

	OOPStopWatch*		m_stopWatch;	// 见 OOPLyric::m_stopWatch

	DECLARE_DYNAMIC_VOBJECT
};
