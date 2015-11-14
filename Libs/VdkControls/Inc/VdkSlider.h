#pragma once
#include "VdkControl.h"

class VdkButton;
class VdkWindow;
class VdkScrollBar;

/// \brief VdkSlider 风格属性
enum VdkSliderStyle {

	/// \brief 派生类风格位定义的起始位序号
	VSS_USER					= ( VCS_USER + 5 ),
	/// \brief 在滑块外点击滑块是否需要跳到鼠标点击处
	VSS_CLICK_TO_GO				= 1 << ( VCS_USER + 0 ),
	/// \brief 竖直滚动条的起始点在 y 轴最高处，和滚动条的行为刚好相反
	VSS_VERTICAL_BOTTOM_UP		= 1 << ( VCS_USER + 1 ),
};

/// \brief VdkSlider 的运行时状态属性集
enum VdkSliderState {

	/// \brief 派生类的运行时状态定义的起始位序号
	VSST_USER					= ( VCS_USER + 5 ),
	/// \brief 点击滑块，释放鼠标时不要发送事件
	VSST_DONT_FIRE_EVENT		= 1 << ( VCST_USER + 0 ),
	/// \brief 收到 NORMAL 事件时，区别对待“即点即达”释放鼠标与拖动滑块
	/// 释放鼠标后收到的不同版本事件
	VSST_LAST_CLICK_TO_GO		= 1 << ( VCST_USER + 1 ),
};

//////////////////////////////////////////////////////////////////////////

/// \brief 为 VdkSlider 设置初始化信息
class VdkSliderInitializer : public 
	VdkCtrlInitializer< VdkSliderInitializer >
{
public:

	typedef VdkSliderInitializer Slider;

	/// \brief 构造函数
	VdkSliderInitializer();

	/// \brief 析构函数
	~VdkSliderInitializer();

	/// \brief 设置所用的位图数组
	///
	/// \param hold 若为真，则会接管\a ba 的控制权，析构函数被调用时会收内存
	Slider& bitmapArray(wxBitmap* ba, bool hold)
	{
		BitmapArray = ba;
		HoldBitmapArrayPointer = hold;

		return *this;
	}

	/// \brief 设置滚动条的滚动方向
	Slider& vertical(bool b) { Vertical = b; return *this; }

	/// \brief 设置滚动条手柄所用位图的分割份数
	Slider& thumbExpl(int nExpl) { Expl = nExpl; return *this; }

	/// \brief 设置滚动条手柄所用位图的拉伸叠放长度
	Slider& thumbTileLen(int len) { ThumbTileLen = len; return *this; }

	/// \brief 设置滚动条手柄所用位图的拉伸方式
	Slider& thumbResizeType(VdkResizeableBitmapType type)
	{
		ThumbResizeType = type;
		return *this;
	}

protected:

	wxBitmap* BitmapArray;
	bool HoldBitmapArrayPointer;
	bool Vertical;

	int Expl;
	int ThumbTileLen;
	VdkResizeableBitmapType ThumbResizeType;

	friend class VdkSlider;
};

//////////////////////////////////////////////////////////////////////////

/*!\brief 拖动手柄（滚动条）
**/
class VdkSlider : public VdkControl
{
public:

	/// \brief 构造函数
	VdkSlider() { Init(); }

	/// \brief 第一阶段构建函数
	///
	/// 从一个文件名数组中获取一个位图数组。
	/// \param strFileNames 数组至少应有 3 项( 3 个文件名)
	static wxBitmap* GetBitmaps(wxString* strFileNames);

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 实际构建函数
	void Create(const VdkSliderInitializer& init_data);
	
	/////////////////////////////////////////////////////////////////////////

	/// \brief 移动到指定百分比
	void GoTo(double percentage, wxDC* pDC, bool fireCallback = false);

	/// \brief 得到 VdkSlider 当前的百分比
	/// \param prt 需要获取手柄作用域时可以指定此指针
	double GetProgress(wxRect* prt = NULL) const;

	/// \brief 上次滚动条向哪个方向拖动？
	enum SliderDirection {

		SD_NONE,
		SD_LESS,
		SD_MORE
	};

	/// \brief 提供一个点得到向哪个方向滚动
	/// \param fLastDirection 获取上次滚动的方向
	SliderDirection GetDirection(int y, SliderDirection& lastDirection) const;

	/// \brief 重置拖动方向
	void ResetDirection();

	/// \brief 设置擦除背景时所用的画刷
	void SetBackground(const wxBrush& brush) { m_bgBrush = brush; }

	/// \brief 滚动条可滚动的方向是否为竖直方向
	bool IsVertical() const { return m_bVertical; }

	/// \brief 获取手柄句柄
	/// \warning 请勿轻易修改手柄的各项属性！
	const VdkButton* GetThumb() const { return m_pThumb; }

	/// \brief 更新手柄状态
	///
	/// 通过某些不明显的方式改动了手柄的当前位置等信息时，\n
	/// 可以调用本函数将改动立即显现出来。
	void UpdateThumbState(wxDC* pDC);

private:

	/// \brief 初始化控件
	void Init();

	/// \brief 接收、处理通知信息
	virtual void DoHandleNotify(const VdkNotify& notice);

	/// \brief 处理鼠标事件
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief detect width and height for dc.Blit
	void DectectWH(int& w, int& h);

	/// \brief 擦除\a rc 指定的矩形
	/// \param rc 坐标起点为父控件的左上角
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 按钮位图的虚拟实现
	struct Bitmap {

		int work_side;
		int non_work_side;
	};

	const static int INVALID_MOUSE_ON_THUMB = -1;

	struct SliderImp {

		int curr; // 输出信息，经处理后手柄应该处在的位置
		int ptr; // 鼠标的位置

		/// \brief 连续拖动 VdkSlider 时上次拉动的方向
		///
		/// 当前主要应用是用于 VdkScrollBar 的持续按住连续滚动的实现上
		SliderDirection direction;
		double percent;

		Bitmap bmFull;
		Bitmap bmHandle;

		//////////////////////////////////////////////////////////////////////////

		/// \brief 复制赋值函数
		SliderImp& operator = (const SliderImp& rhs);

		/// \brief 初始化
		void Init();

		/// \brief 处理拖动
		void HandleDrag(int mousePos);

		/// \brief 移动到指定百分比
		void MoveTo(double percent_);

		/// \brief 释放鼠标指针
		void Release(){	ptr = INVALID_MOUSE_ON_THUMB; }

		/// \brief 鼠标指针是否位于手柄上，这时允许拖动
		bool IsOk() const { return ptr != INVALID_MOUSE_ON_THUMB; }
	};

	SliderImp					m_imp;

	//////////////////////////////////////////////////////////////////////////

	bool						m_bVertical;
	VdkButton*					m_pThumb;

	// 手柄的最小高度，即位图源文件的大小
	int							m_nMinHeight;
	// 改变大小时手柄的背景怎么操作？拉伸或者平铺
	VdkResizeableBitmapType		m_nThumbTileType;
	// 改变大小时手柄的背景中心两侧允许拉伸的长度之和
	int							m_nThumbTile;

	//////////////////////////////////////////////////////////////////////////

	// 进度条背景位图
	wxBitmap					m_bmBar;
	// 假如背景位图是相同的，直接使用纯色来填充进度条
	wxBrush						m_bgBrush; 
	// 进度显示
	wxBitmap					m_bmFull;
	// 用于回调函数（拖动的最小间隔40ms）
	wxMilliClock_t				m_dragTimeStamp;

	DECLARE_CLONEABLE_VOBJECT( VdkSlider )
};
