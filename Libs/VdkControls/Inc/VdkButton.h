#pragma once
#include "VdkControl.h"
#include "VdkBitmapArray.h"

/// \brief VdkButton 风格属性集
enum VdkButtonStyle {

	/// \brief 派生类起始属性位
	VBS_USER					= VCS_USER + 5,
	/// \brief 能否改变大小
	VBS_RESIZEABLE				= 1 << ( VCS_USER + 0 ),
	/// \brief 平铺还是拉伸？
	VBS_RESIZE_TYPE_TILE		= 1 << ( VCS_USER + 1 ),
	/// \brief 是否使用 wxRegion 作为作用域标识符
	VBS_REGION					= 1 << ( VCS_USER + 2 ),
};

/// \brief VdkButton 运行时属性集
enum VdkButtonState {

	/// \brief 派生类起始属性位
	VBST_USER					= VCST_USER + 5,
	/// \brief 不适宜使用 wxRegion 作为作用域标识符
	///
	/// 创建 wxRegion 失败。
	VBST_REGION_FAILED			= 1 << ( VCST_USER + 0 ),
};

//////////////////////////////////////////////////////////////////////////

/// \brief 为 VdkButton 设置初始化信息
class VdkButtonInitializer : public 
	VdkCtrlInitializer< VdkButtonInitializer >
{
public:

	typedef VdkButtonInitializer Button;

	/// \brief 构造函数
	VdkButtonInitializer();

	/// \brief 设置按钮所用的背景位图
	/// \see fileName
	Button& bitmap(const wxBitmap& bm) { BkGnd = bm; return *this; }

	/// \brief 设置按钮所用的背景位图文件在磁盘上的位置
	Button& fileName(const wxString& s) { FileName = s; return *this; }

	/// \brief 设置按钮的标题文本
	Button& caption(const wxString& s) { Caption = s; return *this; }

	/// \brief 设置按钮的标题文本对齐属性
	Button& textAlign(align_type a) { TextAlign = a; return *this; }

	/// \brief 设置按钮是否可以改变大小
	Button& resizeable(bool b) { Resizeable = b; return *this; }

	/// \brief 设置按钮可改变大小时背景位图的拉伸方式
	Button& tileType(VdkResizeableBitmapType type) {
		TileType = type; return *this;
	}

	/// \brief 设置按钮可改变大小时背景位图的叠放长度
	///
	/// 从正中间向一边扩展的长/宽度。
	Button& tileLen(unsigned len) { TileLen = len; return *this; }

	/// \brief 设置按钮要分割成的份数
	///
	/// 要求将表现按钮的各种状态的位图合并在一个位图里。
	Button& explode(unsigned e) { Expl = e; return *this; }

private:

	wxString Caption;
	wxBitmap BkGnd;
	wxString FileName;

	unsigned Expl;
	align_type TextAlign;
	bool Resizeable;
	VdkResizeableBitmapType TileType;
	unsigned TileLen;

	friend class VdkButton;
};

/// \brief 按钮是最常见的GUI组分
///
/// 一个按钮有四种状态（平常、高亮、按下、失活）。
/// 用户可以用单击鼠标一次从而在四种状态之间切换。
class VdkButton : public VdkControl
{
public:

	/// \brief 构造函数
	VdkButton();

	/// \brief XRC 动态创建
	virtual void Create(wxXmlNode* node);

	/// \brief 构建函数
	///
	/// 允许自定义位图，主要是因为可能某些准标准控件（如 VdkCheckBox ）\n
	/// 不需要由用户自己指定位图，而又需要从 XRC 文件中动态创建，\n
	/// 故所用位图直接由系统生成或者从 XPM 文件载入。
	/// \attention 主要由 VdkControl 内部调用，用于其它 VdkControl \n
	/// 中的 VdkButton 。
	VdkControl* Create(wxXmlNode* node,
					   const wxBitmap& bm,
					   int nExpl);

	/// \brief 在此执行主要构建工作
	void Create(const VdkButtonInitializer& init_data);

	/// \brief 创建标识 wxRegion
	///
	/// 假如控件作用域由 wxRegion 来标识，则需调用本函数来创建标识 wxRegion。\n
	/// 正常的 VdkButton 由 wxRect 来标识作用域。
	void CreateRegion();

	/// \brief 标识 wxRegion 是否已然被正确创建
	bool IsRegionOk() const { return TestStyle( VBS_REGION ); }

	/// \brief 创建 wxRegion 作用域是否失败了
	bool IsRegionCreationFailed() const {
		return !TestState( VBST_REGION_FAILED );
	}

	/// \brief 判断\a mousePos 是否位于按钮的作用域里
	virtual bool HitTest(const wxPoint& mousePos) const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief 模拟一次鼠标单击事件
	/// \attention 不要再调用原来的回调函数!
	void EmuClick(wxDC& dc);

	/// \brief 按钮的所有状态
	enum State {
		NORMAL,
		HOVERING,
		PUSHED,
		DISABLED,
	};

	/// \brief 将按钮的当前状态更改为\a nState
	virtual void Update(State nState, wxDC* pDC);

	/// \brief 得到按钮上次处于的状态
	State GetLastState() const { return GetButtonState(); }

	/// \brief 按钮是否处于自维护的禁用状态
	///
	/// 注意，这个禁用状态与 VDK 原生禁用机制不同。
	bool IsDisabled() const { return m_nState == DISABLED; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 得到 VdkButton 的大小
	///
	/// 以背景图片的大小为准，可能不同于\link GetRect()\endlink得到的大小。
	void GetSize(int* w, int* h) const;

	/// \brief 得到 VdkButton 的最小大小
	///
	/// 即背景图片的大小。
	void GetMinSize(int* w, int* h) const;

	/// \brief 获取按钮标题
	wxString GetCaption() { return m_strCaption; }

	/// \brief 设置按钮标题
	void SetCaption(const wxString& strCaption, wxDC* pDC);

	/// \brief 改变 VdkButton 的大小
	/// \return true: 操作成功，false: 操作失败。
	bool Resize(int w, int h);

	/// \brief 根据按钮的标题自动调整大小（宽度）
	void Fit(wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 获取原始位图
	///
	/// 本函数的一个重要用途是复制一个按钮。
	const VdkBitmapArray& GetPrimaryBitmap() const;

protected:

	/// \brief 处理鼠标事件
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief 绘制控件
	virtual void DoDraw(wxDC& dc);

	/// \brief 接收、处理通知信息
	virtual void DoHandleNotify(const VdkNotify& notice);

	/// \brief 设置按钮状态
	void SetButtonState(State state) { m_nState = state; }

	/// \brief 获取按钮状态
	State GetButtonState() const { return m_nState; }

	/// \brief 根据对齐计算文本位置
	void CalcTextPos();

	//////////////////////////////////////////////////////////////////////////

	/// \brief 改变大小时平铺图片的中心两侧度量之和
	int m_nTile;

	/// \brief 文本对齐
	align_type m_TextAlign;

	/// \brief 各个状态的背景图片
	VdkBitmapArray m_bmArray;
	/// \brief 原始的位图“数组”
	/// 
	/// 可用于改变按钮的大小（如滚动条手柄的高度）等。
	VdkBitmapArray m_bmPrimArray;

	/// \brief 按钮文本四周的空白
	int m_nPaddingX, m_nPaddingY;

	/// \brief 按钮文本
	wxString m_strCaption;
	/// \brief 文本颜色
	wxColour m_TextColor;

	/// \brief 使用 wxRegion 标识不规则控件的特殊作用域
	wxRegion m_Region;

private:

	// 按钮当前状态
	State m_nState;

	DECLARE_CLONEABLE_VOBJECT( VdkButton )
};
