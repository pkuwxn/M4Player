/***************************************************************
 * Name:      OOPDesktopLyric.cpp
 * Purpose:   桌面歌词(On Screen Display Lyric)
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-02
 **************************************************************/
#pragma once
#include <wx/popupwin.h>
#include <wx/geometry.h> // for wxRect2DDouble

#include "ILyric.h"
#include "OOPLyricParser.h"

typedef wxPopupWindow DeskLrcSuperClass;
class wxGraphicsContext;
class wxGraphicsPath;

/// 桌面歌词(On Screen Display Lyric)
class OOPDesktopLyric : public DeskLrcSuperClass, public ILyric
{
public:

	/// 风格定义
	struct Style
	{
		wxString fontFace; ///< 使用的字体
		size_t pxFontSize; ///< 字体大小(量纲:像素px)
		bool bold; ///< 字体是否需要加粗显示

		size_t borderSize; ///< 背景矩形框的边框大小
		unsigned char alpha; ///< 整体透明度

		/// 默认构造函数
		Style();
	};

	/// 构造函数
	OOPDesktopLyric(wxWindow* parent);

	/// 析构函数
	~OOPDesktopLyric();

	/// 立即更新显示
	bool UpdateAtOnce();

public:

	/// 以下函数用以实现 ILyric 接口
	virtual bool IsOk() const;
	virtual void AttachParser(const OOPLyricParser& parser);
	virtual void Start();
	virtual void Pause();
	virtual void Stop(wxDC* pDC);
	virtual void ClearLyric(wxDC* pDC);
	virtual void GoTo(double percentage, wxDC* pDC, bool bPaused);

private:

	// 做一些开始前的初始化工作
	void Initialize();

	// 做一些退出时的清理工作
	void Finalize();

	// 初始化当前歌曲文本行的 GDI 路径
	//
	// 工作主要是将文本轮廓加到其中。
	wxGraphicsPath InitLineTextPath(wxGraphicsContext* gc);

	// 显示背景矩形框
	void ShowBackgound(wxGraphicsContext* gc);

	// 将我们所画的显示出来
	bool Present(wxDC& drawings);

private:

	// 显示交互性文本输出
	virtual void DoSetInteractiveOutput(wxDC* pDC);

	// 中止显示当前行，直接显示下一行
	//
	// 用于防止当前行所用时间超出 LRC 指定的时间。
	void NextLine();

	// 根据定时器修正当前进度
	void UpdateProgress();

	// 获取当前歌词行的进度(百分比，范围[0, 1])
	double GetLineProgress() const;

	// 当前歌词文本行是否有效？
	// 
	// 若无效则应该显示当前交互性文本。
	bool IsCurrLineValid() const;

private:

	void OnEnterWindow(wxMouseEvent&);
	void OnLeaveWindow(wxMouseEvent&);

	void OnLeftDown(wxMouseEvent&);

	// wxTimer 回调
	void OnTimerNotify(wxTimerEvent&);

private:

	Style m_style; // 当前正使用的风格
	const OOPLyricParser* m_parser;

	LineIter m_currLine; // 当前高亮行
	bool m_showBackgound; // 当前是否需要显示背景矩形框

	wxRect2DDouble m_textPathBounds; // 文本路径的包围盒

	const wxWindowID m_timerId;
	wxTimer m_timer;

private:

	wxDECLARE_EVENT_TABLE();
};
