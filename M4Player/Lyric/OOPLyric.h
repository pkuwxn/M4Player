/***************************************************************
 * Name:      OOPLyric.h
 * Purpose:   集成在歌词秀窗口的内嵌歌词显示控件
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2010
 **************************************************************/
#pragma once
#include "VdkListCtrl.h"
#include "OOPLyricParser.h" // for LineIter
#include "ILyric.h"

class OOPStopWatch;
class VdkWindow;

/// \brief 歌词秀实现
class OOPLyric : public VdkListCtrl, public ILyric
{
public:

	/// \brief 构造函数
	OOPLyric();

	/// \brief 析构函数
	~OOPLyric();

	/// \brief 以下函数用以实现 ILyric 接口
	virtual bool IsOk() const;
	virtual void AttachParser(const OOPLyricParser& parser);
	virtual void Start();
	virtual void Pause();
	virtual void Stop(wxDC* pDC);
	virtual void ClearLyric(wxDC* pDC);
	virtual void GoTo(double percentage, wxDC* pDC, bool bPaused);

private:

	// 初始化
	void Init();

	// XRC 动态创建
	virtual void OnXrcCreate(wxXmlNode* node);

	// 做一些额外的构建工作
	// @param colorArray 绘制控件时各种元素的颜色，其中：
	//    [0]: 正常文本颜色
	//    [1]: 高亮时的文本颜色
	//    [2]: 背景颜色
	void Create(const wxArrayString& colorArray);

private:

	// 从暂停中恢复
	// @return 是否成功恢复歌词的显示。
	// @attention 歌词还没开始显示即会出现失败的情况。
	bool Resume();

	// 根据定时器修正当前进度
	void UpdateProgress(wxDC* pDC);

	// 中止显示当前行，直接显示下一行
	//
	// 用于防止当前行所用时间超出 LRC 指定的时间。
	void NextLine(wxDC* pDC, bool bPaused = false);

	// 转到最后
	void MoveToEnd(wxDC* pDC);

	// 为使当前行显示在控件中间，因此在歌词前后添加了空行
	// 
	// 函数会重设虚拟画布的大小
	void InsertBlankLines();

	// 计算插入空行后最终虚拟画布的高度
	void UpdateVirtualHeight();

	// 根据 m_nCurrIndex 修正滚动窗口的起始绘制位置
	// 如自动滚动时、移动至指定百分比需要实际更改滚动窗口的
	// 物理属性。
	// 总之有一个原则：新的一行开始时必须严格地只显示这一新行。
	void CorrectViewStart(wxDC* pDC);

	// 刷新当前歌词的状态
	// 
	// 如更新滚动窗口的物理起始坐标，并根据\a bPaused 的值是否
	// 从新的位置继续显示歌词。
	void RefreshLyric(wxDC* pDC, bool bPaused);

	// 获取当前歌词行的进度(百分比，范围[0, 1])
	double GetLineProgress() const;

private:

	virtual void DoSetInteractiveOutput(wxDC* pDC);

	// 清空当前显示的歌词
	virtual void DoClear(wxDC* pDC);

	// 处理鼠标事件
	virtual void OnMouseEvent(VdkMouseEvent& e);

	// 处理键盘事件
	virtual void OnKeyEvent(VdkKeyEvent& vke);

	// 绘制顶部和底部渐变色文本
	virtual void OnDraw(wxDC& dc);

	// 接收、处理通知信息
	virtual void OnNotify(const VdkNotify& notice);

	// 在此绘制每一个单元格
	virtual VdkCusdrawReturnFlag DoDrawCellText(const VdkLcCell* cell, 
												int col_index, int index, 
												wxDC& dc, 
												VdkLcHilightState state);

	// 窗口不可见时停止歌词显示
	void OnParentShow(wxShowEvent& e);

	// wxTimer 回调
	void OnTimerNotify(wxTimerEvent&);

private:

	LcDataSet m_dataSet;
	const OOPLyricParser* m_parser;

	const wxWindowID m_timerId;
	wxTimer m_timer;

	wxColour m_TextColor;
	wxColour m_HilightColor;
	wxColour m_BgColor;

	LineIter m_currLine; // 当前高亮

	int m_blankLinesTop; // 列表开头当前加进去的空行数
	int m_blankLinesBottom;

	// 拉动歌词时鼠标指针距歌词顶部的距离（状态量）
	int m_draggDistance;
	LineIter m_draggHit; // 拉动歌词时鼠标指针命中的歌词行

	DECLARE_DYNAMIC_VOBJECT
};
