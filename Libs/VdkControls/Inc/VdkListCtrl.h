/***************************************************************
 * Name:      VdkListCtrl.h
 * Purpose:   Code for VdkListCtrl implementation
 * Author:    Ning (vanxining@139.com)
 * Created:   2010-02-01
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkScrolledWindow.h"

#include <list>

//////////////////////////////////////////////////////////////////////////

/// \brief VdkListCtrl 的风格定义
enum VdkListCtrlStyle {

	/// \brief 派生类风格属性的起始 ID
	VLCS_USER				= VSWS_USER + 10,
	/// \brief 是否允许多选
	VLCS_MULTI_SELECT		= 1 << ( VSWS_USER + 0 ),
	/// \brief 是否使用交替行背景色
	VLCS_CROSS_COLORS		= 1 << ( VSWS_USER + 1 ),
	/// \brief 是否生成标头
	VLCS_HEADER				= 1 << ( VSWS_USER + 2 ),
	/// \brief 是否产生高亮效果
	VLCS_HOVERING			= 1 << ( VSWS_USER + 3 ),
	/// \brief 是否不允许选中某行
	VLCS_NO_SELECT			= 1 << ( VSWS_USER + 4 ),
	/// \brief 是否不允许拖动某行
	VLCS_NO_DRAGG			= 1 << ( VSWS_USER + 5 ),
};

/// \brief 高亮 VdkListCtrl 某行的三种状态
enum VdkLcHilightState {

	VDKLC_HS_NORMAL,	///< 恢复正常状态
	VDKLC_HS_HILIGHT,	///< 鼠标置于其上
	VDKLC_HS_SELECTED	///< 已被选择
};

/// \brief VdkListCtrl 的运行时属性集位定义
enum VdkListCtrlState {

	/// \brief 派生类风格属性的起始 ID
	VLCST_USER				= VSWST_USER + 5,
	/// \brief 全选项目
	VLCST_SELECT_ALL		= 1 << ( VSWST_USER + 0 ),
};

class VdkListCtrl;
struct VdkLcColumn;

/// \brief VdkListCtrl 的一个单元格
class VdkLcCell : public VdkClientDataOwner
{
public:

	/// \brief 构造函数
	VdkLcCell(VdkLcColumn* col,
			  const wxString& label,
			  void* clientData);

	/// \brief 单纯绘制单元格文本
	/// 
	/// 假如只需要使用子类化重画背景颜色、文本颜色等内容，而将文本绘制
	/// 交由 VDK 处理，可以调用本函数。
	/// \note VdkLcCell 会自动处理文本对齐（如居中）。
	void DrawLabel(wxDC& dc, int x, int y) const;

	/// \brief 单元格文本是否为空
	bool IsEmpty() const { return m_label.IsEmpty(); }

	/// \brief 获取文本左边距
	int GetX_Padding() const;

	/// \brief 获取当前单元格的文本
	/// \param xPadding 获取文本左边距
	wxString GetLabel(int* xPadding = NULL) const;

	/// \brief 设置单元格文本
	void SetLabel(const wxString& label);

	/// \brief 获取单元格所属的列
	VdkLcColumn* GetColumn() const { return m_col; }

private:

	// 计算单元格文本绘制起始位置以及溢出时作相应处理
	void CalcTextPos();

	// 同步绘制属性集版本，使之和文本列的版本相同
	void SyncDrawingVersion() const;

	// 调低当前绘制属性集版本
	//
	// 实际上是将其设为列绘制属性集版本减一。
	void DecreaseDrawingVersion();

private:

	VdkLcColumn*		m_col; // 所属的列
	wxString			m_label; // 标题

	int					m_xFix; // 文本左边距
	// 当标题超长时显示的文本(一般在末尾加...)
	wxString			m_labelOverflow;

	// 当前绘图版本
	mutable int			m_drawingVersion;
};

typedef std::list< VdkLcCell* > ListOfLcCell;
typedef ListOfLcCell::iterator LcCellIter;

//////////////////////////////////////////////////////////////////////////

/*!\brief 代表 VdkListCtrl 的一列
 *
 * 注意第一列的特殊含义。
**/
struct VdkLcColumn {

	/// \brief 默认构造函数
	VdkLcColumn();

	/// \brief 从\a rhs 复制本列
	///
	/// 不复制列的各项。
	void Clone(const VdkLcColumn &rhs);

	/// \brief 设置列绘制时使用的字体
	void SetFont(const wxFont& font);

	/// \brief 列表控件改变大小后更新列宽
	void UpdateWidth(size_t w);

	/// \brief 获取当前绘图版本
	/// 
	/// 我们希望实现这样一个效果：当某些绘图属性变化时，单元格不要立刻
	/// 重新计算溢出，而是在绘制时才即时修改。
	int GetDrawingVersion() const { return m_drawingVersion; }

	/// \brief 提高当前绘图版本
	void IncreaseDrawingVersion() { m_drawingVersion++; }

	//////////////////////////////////////////////////////////////////////////

	//! 当列表控件大小改变时本列分得的该变量（比如80%）
	int					percentage;
	int					leftPadding; ///< 文本左边距
	align_type			textAlign;	///< 列的文本对齐
	wxColour			textColor;	///< 本列的文本颜色
	wxFont				font; ///< 本列绘制时所用的字体

	wxString			heading; ///< 列的标题
	int					x; ///< 本列相对于 VdkListCtrl 的x轴起始坐标
	int					width; ///< 列的宽度
	int					height; ///< 列每单元格的高度

	ListOfLcCell		cells; ///< 列的各项

private:

	int					m_drawingVersion;

	DECLARE_NO_COPY_CLASS( VdkLcColumn )
};

typedef std::list< VdkLcColumn* > ListOfLcColumn;
typedef ListOfLcColumn::iterator LcColIter;

/// \brief 列表框数据集
class LcDataSet
{
public:

	/// 虚析构函数
	virtual ~LcDataSet();

	/// 删除所有内容(行和列)
	virtual void RemoveAllColumns();

	/// 删除各行
	virtual void RemoveAllRows();

public:

	/*! 列表框的各列 */
	ListOfLcColumn cols;
};

WX_DEFINE_SORTED_ARRAY_INT( int, ArrayOfSortedInts );

//////////////////////////////////////////////////////////////////////////

/// \brief 为 VdkLcColumn 设置初始化信息
///
/// \attention 必须设置字体！
class VdkLcColumnInitializer
{
public:

	/// \brief 构造函数
	/// \param lst 必须提供相关联的列表框控件用以正确设置列字体
	VdkLcColumnInitializer(VdkListCtrl* lst);

	/// \brief 设置要插入列的序号
	///
	/// 成功插入一列后，这列的序号就是\a i
	VdkLcColumnInitializer& index(int i) { Index = i; return *this; }

	/// \brief 当列表控件大小改变时本列分得的该变量
	/// \param p 0 <= p <= 100
	/// \note 这并不是代表本列占 VdkListCtrl 总宽度的 \a p%。\n
	/// 本列的初始宽度还需要以\a width 来指定。
	VdkLcColumnInitializer& percentage(int p) {
		Percentage = p;
		return *this;
	}

	/// \brief 设置文本左边距
	VdkLcColumnInitializer& leftPadding(int p) {
		LeftPadding = p;
		return *this;
	}

	/// \brief 设置显示表头控件时表头的说明文字
	VdkLcColumnInitializer& heading(const wxString& h) {
		Heading = h; return *this;
	}

	/// \brief 设置列的初始列宽
	VdkLcColumnInitializer& width(int w) { Width = w; return *this; }

	/// \brief 设置列文本的对齐属性
	VdkLcColumnInitializer& textAlign(align_type a) {
		TextAlign = a;
		return *this;
	}

	/// \brief 设置列文本的文本颜色
	VdkLcColumnInitializer& textColor(const wxColour& color) {
		TextColor = color;
		return *this;
	}

	/// \brief 设置列文本的字体
	///
	/// \attention 必须设置字体！
	VdkLcColumnInitializer& font(const wxFont& f) {
		Font = f;
		return *this;
	}

private:

	int Index;
	int Percentage;
	int LeftPadding; // 文本左边距
	wxString Heading;
	int Width;
	align_type TextAlign;
	wxColour TextColor;
	wxFont Font;

	friend class VdkListCtrl;
};

//////////////////////////////////////////////////////////////////////////

/*!\brief 列表控件
 *
 * VdkListCtrl 的事件只有双击列表项时才会发送，用户选中的项行号
 * 可以通过 VdkVObjEvent 的 SetClientData() 成员获取。
 * \attention 重画整个控件不要直接调用 VdkControl::Draw(),
 * 而应该调用 VdkScrolled::RefreshState 。
**/
class VdkListCtrl : public VdkScrolledWindow
{
public:

	/// \brief 构造函数
	/// \param style 列表框的风格组合
	VdkListCtrl(long style = VCS_BORDER_SIMPLE | VLCS_HOVERING);

	/// \brief 析构函数
	~VdkListCtrl();

	/// \brief XRC 动态创建
	virtual void OnXrcCreate(wxXmlNode* node);

	/// \brief 创建控件
	void Create(VdkWindow* parent,
				const wxString& strName,
				const wxRect& rc);

	/// \brief 从\a o 克隆
	void Clone(VdkListCtrl* o, VdkControl* parent);

	/////////////////////////////////////////////////////////////////////////

	/// \brief 设置绑定的数据集
	/// \param dataSet 要绑定的数据集
	/// \attention VdkListCtrl 不会接管数据集指针，用户必须手动管理！
	void Attach(LcDataSet* dataSet);

	/// \brief 撤销已绑定的数据集
	LcDataSet* Datach();

	/// \brief 在列表中插入一列
	void InsertColumn(const VdkLcColumnInitializer& init_data);

	/// \brief 在列表末尾插入一项
	/// 
	/// \return 返回一个临时单向链表，链表每个结点为新插入行的每一个单元格。
	/// 每个节点的下一个单元格指针可以用 VdkLcCell::GetCellClientData() 来
	/// 获取。
	VdkLcCell* Append(const wxString& label, bool updateSize);

	/// \brief 在列表中插入一行
	/// \param index 成功插入一行后，这行的序号就是\a index
	/// \param label 第一列的标题
	/// \param updateSize 是否立即设置虚拟画布的大小
	/// \note 假如列表有多列，设置另外的列标题请使用
	/// \link SetCellLabel() \endlink 。
	VdkLcCell* InsertRow(size_t index, const wxString& label, bool updateSize);

	/// \brief 计算虚拟画布的大小
	///
	/// 可用于优化大批量插入操作，避免每插入一行都调用代价
	/// 昂贵的\link VdkScrolledWindow::SetVirtualSize \endlink 。
	void UpdateSize(wxDC* pDC);

	/// \brief 清空当前所有项
	///
	/// 当前的各个列保持不变。即只影响行。
	void Clear(wxDC* pDC);

	/// \brief 删除第 \a index 项
	void RemoveRow(int index, bool updateSize, wxDC* pDC);

	/// \brief 滑动列表
	/// \a dX 需要越过的列数（增量）
	/// \a dY 需要越过的行数（增量）
	bool ScrollList(int dX, int dY, wxDC* pDC = NULL);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 选中第 \a index 项
	/// \param index 以 0 为起点。
	void Select(int index, wxDC* pDC) {	Select( index, index + 1, pDC ); }

	/// \brief 选中 [\a first, \a last) 项
	/// \param first 以 0 为起点。
	/// \param last 不包括此项，若为 wxNOT_FOUND，则选中起始项\a first 
	/// 至最后一项。
	void Select(int first, int last, wxDC* pDC);

	/// \brief 全选项目
	void SelectAll(wxDC* pDC);

	/// \brief 清除已选择的列表项
	void SelectNone(wxDC* pDC);

	/// \brief 反向选择
	void SelectReverse(wxDC* pDC);

	/// \brief 获取所有已选择的项目
	///
	/// 数组中为已选项目的序号，升序排列。
	const ArrayOfSortedInts& GetSelectedItems() const {
		return m_selItems;
	}

	/// \brief 获取最后选中的行的第一列的迭代器
	LcCellIter GetLastSel() const;

	/// \brief 获取最后选中的行的序号
	int GetLastSelIndex() const;

	/// \brief 是否选中了所有项目
	bool IsAllSel() const;

	/// \brief 指定项是否已被选中
	bool IsSelected(int index) const;

	/// \brief 返回当前是否有项目被选中
	bool HasSelected() const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief 获取第一列第\a index 行单元格的内容
	wxString GetString(int index);

	/// \brief 获取第\a row 行、第\a col 列单元格的内容
	wxString GetCellLabel(int row, int col);

	/// \brief 设置指定行指定列小格的文本
	void SetCellLabel(int row, int col, const wxString& label);

	/// \brief 设置指定单元格的文本
	void SetCellLabel(LcCellIter cell, const wxString& label);

	/// \brief 设置指定行指定列小格的用户自定义指针
	void SetCellClientData(int row, int col, void* clientData);

	/// \brief 获取指定行指定列小格的用户自定义指针
	void* GetCellClientData(int row, int col);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 列表是否为空
	///
	/// 即没有任何行，拥有列但没有行仍然算作空。
	bool IsEmpty() const;

	/// \brief 得到行数
	int GetItemCount() const { return GetRowCount(); }

	/// \brief 得到行数
	int GetRowCount() const;

	/// \brief 得到列数
	int GetColumnCount() const;

	/// \brief 设置某一列的文本前景色
	void SetColumnTextColor(int index, const wxColour& color);

	/// \brief 获取起始序号
	virtual void GetViewStart(int* x, int* y) const;

	/// \brief 获取最大可能的起始序号
	virtual void GetMaxViewStart(int* x, int* y) const;

	/// \brief 得到行高
	int GetRowHeight() const { return m_rowHeight; }

	/// \brief 设置行高
	///
	/// 提供的数值不能小于字体的高度。
	void SetRowHeight(int height, wxDC* pDC = NULL);

	/// \brief 调整列宽
	///
	/// 从\a adjustFrom 调整\a w 宽度给\a adjust 。
	/// \return 操作是否成功。
	bool AjustCollumn(size_t w, size_t adjust, size_t adjustFrom);

	/// \brief 从指定\a cell 得到当前相应的行号
	/// \see GetColumn0Iterator
	int IndexOf(const VdkLcCell* cell) const;

	/// \brief 获取指定行指定列小格句柄
	VdkLcCell* GetCell(size_t row, size_t col) const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief 排序所用的谓词函数
	typedef bool (*CellComparer)(const VdkLcCell*, const VdkLcCell*);

	/// \brief 排序列表框
	void Sort(int col, CellComparer comp);

protected:

	/// \brief 处理鼠标事件
	virtual void OnMouseEvent(VdkMouseEvent& e);

	/// \brief 绘制控件
	virtual void OnDraw(wxDC& dc);

	/// \brief 接收、处理通知信息
	virtual void OnNotify(const VdkNotify& notice);

	/// \brief 响应用户按键事件
	virtual void OnKeyEvent(VdkKeyEvent& vke);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 在此擦除（绘制）行背景
	/// \param it 正要被擦除的行的第一列的迭代器
	/// \param index 正要被擦除的行的序号
	/// \return 指示 VdkListCtrl 的后续行为
	/// \see VdkCusdrawReturnFlag
	/// \attention 为提高性能，若某种状态信息是由行迭代器来保存，则测试
	/// 行相等时应该用\a it 来进行测试，若使用行号来保存，则应用\a index
	/// 来测试。
	virtual VdkCusdrawReturnFlag DoEraseRow
		(const LcCellIter& it, int index, wxDC& dc);

	/// \brief 在此绘制每一个单元格
	///
	/// 也可以只是更改绘图上下文的各种属性，返回相应值即可。
	virtual VdkCusdrawReturnFlag DoDrawCellText(const VdkLcCell* cell,
		int col_index, int index, wxDC& dc, VdkLcHilightState state);

	//////////////////////////////////////////////////////////////////////////
	// 以下这几个函数适用于派生类

	/// \brief 获取选择的起始项 ID
	int GetSelStart() const { return m_selStart; }

	/// \brief 获取拖动项目时的目标位置 ID
	int GetLastDraggTarget() const { return m_lastDraggTarget; }

	/// \brief 从某一竖直坐标\a y 判断指针下的列表项 ID
	/// \param index 范围限制在可视区域之内
	/// \param indexMayOverflow 范围可能会超出可视区域一两行，一般用于判断用户
	/// 点击的项是否位于最后的空白区域中。
	void GetIndex(int y, int& index, int& indexMayOverflow);

	/// \brief 获取指定单元格(\a col, \a row)的迭代器
	///
	/// 因为\a index 变动很大，保存起来相当不安全（m_selItems 保存
	/// ID 实际上是想实现有序数组，兼之历史原因），而列表容器的
	/// 迭代器是保持不变的。所以这种情况下优先保存迭代器，
	/// 不用辛辛苦苦的到处追踪 index 的各种变动。
	LcCellIter GetCellIterator(int col, int row);

	/// \brief 获取指定单元格(\a col, \a row)的迭代器
	LcCellIter GetCellIterator(VdkLcColumn* col, int row);

	/// \brief 获取列单元格最后一个的后继
	/// \attention 注意得到的是一个无意义的迭代器！
	LcCellIter GetColumnEnd(int col) const ;

	/// \brief （重新）计算当前作用域下最大可以显示的条目数
	void CalcShownItems();

	/// \brief 当前作用域下最大可以显示的条目数
	int GetShownItems() const { return m_shownItems; }

	/// \brief 设置在正常的垂直显示范围之下还要显示的项目数
	void SetShownItemsAddIn(int addin) { m_nShownItemsAddIn = addin; }

	/// \brief 获取在正常的垂直显示范围之下还要显示的项目数
	int GetShownItemsAddin() const { return m_nShownItemsAddIn; }

	/// \brief 获取修正的起始绘制行号
	/// 
	/// 在某些变态需求下，如要求滚动鼠标滚轮时不是下移一个整数行，
	/// 而是一个像素一个像素地移动，此时需要将 yStep 设置得足够小（如1px），
	/// 那么 VdkScrolledWindow::GetViewStart()  得到的值就不是当前的
	/// 起始绘制行号，需要使用这个函数来进行修正。
	void FixViewStart(int* x, int* y) const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief 使用外来未知 DC 来重画控件内容，不更新滚动条状态
	///
	/// 绘制控件后会 DC 的起始绘制坐标和裁剪区域都不可信，调用者必须重新设定。
	void RefreshContent(wxDC* pDC);

	/// \brief 更新行状态（予以重画）
	void UpdateRow(int index, wxDC& dc);

	/// \brief 更新行状态（予以重画）
	/// \param it 要更新的行的第一列的迭代器
	void UpdateRow(const LcCellIter& it, wxDC& dc);

	/// \brief 擦除（绘制）行背景
	/// \param it 正要被擦除的行的第一列的迭代器
	/// \param index 正要被擦除的行的序号，若为 wxNOT_FOUND ，则会自动根据
	/// 第一个参数\a it 自动进行计算
	/// \return 返回被擦除行的行号
	int EraseRow(const LcCellIter& it, int index, wxDC& dc);

	/// \brief 绘制一行的文本
	///
	/// 本函数因还原被高亮的项而生。
	/// \attention 本函数效率比较低下，谨慎使用！推荐用途：
	/// \link UpdateRow \endlink 。
	void DrawRowText(int index, wxDC& dc);

private:

	// 在列表中插入一行
	// @param index 成功插入一行后，这行的序号就是\a index 。若为 -1，
	// 则在列表末尾添加一行。
	VdkLcCell* DoInsertRow(int index, const wxString& label, bool updateSize);

	// 清空当前所有项
	//
	// 派生类可覆写本函数知会清空事件。本函数会在第一时间被
	// 调用。
	virtual void DoClear(wxDC* pDC = NULL) {}

	// 在此绘制一列，可以控制列的背景
	//
	// 每次绘制一列
	// \param beg 需要绘制的第一列
	// \param end 需要绘制的最后一列
	// \attention \a end 与 STL 一样，指向有效值的下一个位置！循环中
	// 请勿使用 <= ！
	void DrawColumn(const VdkLcColumn& col, 
					int col_index, 
					wxDC& dc, 
					unsigned beg, 
					unsigned end);

	// 更新最后选中项
	// 
	// 主要是为了只保留一个修改 m_lastSelected 变量的入口。
	void UpdateLastSelected(const LcCellIter& iter);

	// 测试用户指定的、当列表控件大小改变时本列分得的
	// 改变量的总和是否小于或等于100%
	// \param percentage 假如改变量的总和大于100%，VdkListCtrl 会对此
	// 变量进行适配
	int TestWidthArragements(int percentage);

	// 处理按住 Shift 键时按动上下导航键的按键事件
	//
	// 模拟鼠标点击，由鼠标事件处理函数来处理。
	void OnShiftUpDownKeys(int sel, wxDC& dc);

	// 使用 Ctrl + Home/End 移动到列表开头或结尾
	void OnCtrlPlusHomeEnd(VdkKeyEvent& e);

	//////////////////////////////////////////////////////////////////////////

	class ColumnListAdapter;

	// 列表控件的标头控件
	class VdkLcHeader : public VdkControl
	{
	public:

		// 构造函数
		VdkLcHeader(VdkWindow* win,
					const wxString& strName,
					const wxRect& rc,
					VdkListCtrl* listctrl,
					ColumnListAdapter& cols);

	private:

		// 绘制控件
		virtual void DoDraw(wxDC& dc);

	private:

		int					m_yFix;
		VdkListCtrl*		m_listctrl;
		ColumnListAdapter&	m_cols;

		wxBitmap			m_bmBkGnd;
	};

	int m_rowHeight; // 行高
	int m_shownItems; // 当前的 VdkListCtrl 能容纳多少行
	// 在正常的垂直显示范围之下还要显示多少条项目，
	// 多用于允许自动滚动的滚动窗口，否则往往会使得当前屏的
	// 第一行移出画布上沿后，而最后一行却还没进入用户的视野。
	int m_nShownItemsAddIn;

	VdkLcHeader* m_pHeader; // 是否显示顶部的标头

	//////////////////////////////////////////////////////////////////////////
	// 下面是 VdkListCtrl 的当前状态信息

	int m_lastHilighted;
	int m_selStart; // 多项选择时指定的第一项的 ID
	int m_lastDraggTarget; // 当拖动条目进行重排时当前的重排目标
	LcCellIter m_lastSelected; // 最后选中的一项

	ArrayOfSortedInts m_selItems;

private:

	// 列链表的适配器
	class ColumnListAdapter
	{
	public:

		// 构造函数
		ColumnListAdapter(LcDataSet* dataSet);

		// 设置绑定的数据集
		void Attach(LcDataSet* dataSet);

		// 撤销已绑定的数据集
		LcDataSet* Datach();

		// 是否已正确绑定可用的数据集
		bool IsOk() const;

	public:

		typedef ListOfLcColumn::value_type value_type;

		typedef LcColIter iterator;
		typedef ListOfLcColumn::const_iterator const_iterator;

		LcColIter begin();
		LcColIter end();

		ListOfLcColumn::const_iterator begin() const;
		ListOfLcColumn::const_iterator end() const;

		bool empty() const;
		ListOfLcColumn::size_type size() const;

		LcColIter insert(LcColIter it, VdkLcColumn* col);
		void push_back(VdkLcColumn* col);

	private:

		LcDataSet* m_dataSet;
	};

	ColumnListAdapter m_cols;

protected:

	// VdkListCtrl 的风格定义，派生类可存取
	wxPen m_draggTarget; // 拖动项目时表示目标位置的线条颜色
	wxBrush m_hilighted; // 高亮项的背景画刷
	wxBrush m_selected; // 选中项的背景画刷
	wxBrush m_crossBrush1; // 交错颜色 1
	wxBrush m_crossBrush2; // 交错颜色 2

	DECLARE_CLONEABLE_VOBJECT( VdkListCtrl )
};
