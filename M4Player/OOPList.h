#pragma once
#include "VdkListCtrl.h"

class wxInputStream;
class wxOutputStream;
class OOPSong;
class OOPListEntry;
class OOPSongPtr;

/// \brief OOPlayer 专用列表控件
class OOPList : public VdkListCtrl
{
public:

	/// \brief 构造函数
	OOPList();

	/// \brief 析构函数
	~OOPList();

	//////////////////////////////////////////////////////////////////////////

	/// \brief 向列表中添加一首歌曲
	/// \param numRows 当前列表中的项目数，函数调用成功后项目数应该增加 1 。
	///				   主要是希望不用每次计算列表项目数(一般时间复杂度为O(n))。
	void AddToList(OOPSong& song, size_t numRows);

	/// \brief 根据当前条目数调整序号列的宽度
	void UpdateSnColumnWidth(int numRowsOld, int numRowsCurr = wxNOT_FOUND);

	/// \brief 获取第@a index 项附加的 OOPSong 类对象
	OOPSongPtr GetSongPtr(int index);

	/// \brief 获取第迭代器@a i 所指向的项附加的 OOPSong 类对象
	OOPSongPtr GetSongPtr(const LcCellIter& i);

	/// \brief 获取第迭代器@a i 所指向的项附加的 OOPListEntry 类对象
	OOPListEntry* GetListEntry(const LcCellIter& i);

	/// \brief 删除第 \a index 项
	///
	/// 不更新 GUI 。
	void RemoveSong(int index);

	/// \brief 删除第 \a index 项
	///
	/// 不更新 GUI 。
	void RemoveSong(const LcCellIter& i);

	/// \brief 移除所有单元格中附着的用户自定义信息( OOPSong 类对象)
	void RemoveAllSongPtrs();

	//////////////////////////////////////////////////////////////////////////

	/// \brief 以二进制方式保存播放列表
	void Serialize(wxInputStream& fin);

	/// \brief 以二进制方式保存播放列表
	void Serialize(wxOutputStream& fout);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 根据制定的命令选择一种排序方法来排序播放列表
	void SortList(VdkCtrlId cmd);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 当前是否存在已被锁定的项
	bool HasLocked() { return m_locked != end(); }

	/// \brief 获取正在播放的歌曲的 ID
	/// \note 这里不能加 const
	int GetLockedIndex();

	/// \brief 获取正在播放的歌曲
	LcCellIter GetLocked() { return m_locked; }

	/// \brief 获取正在播放的歌曲的关联对象
	/// \note 这里不能加 const
	OOPSongPtr GetLockedSong();

	/// \brief 设置正在播放的歌曲
	/// \param index 允许此值无效
	void SetLocked(int index, wxDC* pDC);

	/// \brief 设置正在播放的歌曲
	/// \param locked 允许此值无效
	void SetLocked(const LcCellIter& locked, wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 播放列表的第一首歌曲
	/// \note 这里不能加const修饰符
	LcCellIter begin();

	/// \brief 播放列表的最后一首歌曲
	/// \note 这里不能加const修饰符
	LcCellIter end();

protected:

	/// \brief 接收、处理通知信息
	virtual void OnNotify(const VdkNotify& notice);

private:

	// 移除单元格中附着的用户自定义信息( OOPSong 类对象)
	void RemoveSongPointer(const LcCellIter& i);

	// 移除单元格中附着的用户自定义信息( OOPSong 类对象)
	void RemoveSongPointer(int i);

private:

	// 处理鼠标事件
	virtual void OnMouseEvent(VdkMouseEvent& e);

	// 响应键盘事件
	virtual void OnKeyEvent(VdkKeyEvent& e);

	// XRC 动态创建
	virtual void OnXrcCreate(wxXmlNode* node);

	// 创建渐变色选择条
	void CreateGradientBrush();

	// 绘制单元格文本
	virtual VdkCusdrawReturnFlag
		DoDrawCellText(const VdkLcCell* cell,
					   int col_index,
					   int index,
					   wxDC& dc,
					   VdkLcHilightState state );

	// 擦除（绘制）行背景
	virtual VdkCusdrawReturnFlag DoEraseRow
		(const LcCellIter& it, int index, wxDC& dc);

	// 响应清空当前所有项事件
	virtual void DoClear(wxDC* pDC = NULL);

private:

	// @brief 向列表中添加一首歌曲
	// @param title 可以指定一个特定的列表项标题
	// @param numRows 当前列表中的项目数，函数调用成功后项目数应该增加 1 。
	//				  主要是希望不用每次计算列表项目数(一般时间复杂度为
	//				  O(n) )。
	void AddToList(OOPListEntry& entry, const wxString& title, size_t numRows);

	// 获取选择项的边框和文本颜色
	wxColour GetSelectedColour() const;

	// 更新第 @a row 行的时间显示
	void UpdateTime(int row, unsigned int len);

	// 更新指定单元格的时间显示
	void UpdateTime(VdkLcCell* cell, unsigned int len);

	// 更新某些的显示，这些行可能均为空（-1）
	void RedrawRows(const wxArrayInt& rows, wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	wxColour m_textColor; // 文本颜色
	wxColour m_snColor; // 序号的文本颜色
	wxColour m_timeColor; // 时间显示的文本颜色
	wxColour m_lockedColor; // “正在播放”条目的锁定颜色
	wxColour m_selectedColor; // 渐变色条(单击选中一个曲目后的加亮)

	LcCellIter m_locked; // 区分“正在播放”的曲目

	DECLARE_CLONEABLE_VOBJECT( OOPList )
};
