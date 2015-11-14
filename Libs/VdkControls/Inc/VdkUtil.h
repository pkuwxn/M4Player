/***************************************************************
 * Name:      VdkUtil.h
 * Purpose:   VdkUtil 的头文件
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-02-15
 * Copyright: vanxining
 **************************************************************/
#pragma once

class wxXmlNode;

//////////////////////////////////////////////////////////////////////////

typedef unsigned long align_type;

/// \brief 区分一个用于生成 wxRect 的字符串是用 Windows 方式还是 wxWidgets 方式来表现的
enum RectType {

	PRT_POSITION,	///< Windows 方式，四个数字的后两位定义了矩形的右下角点的坐标
	PRT_RECTANGLE	///< wxWidgets 方式，四个数字的后两位定义了矩形的长和宽
};

/// \brief 从一个 XRC 定义中提取文本属性
struct TextInfo {

	wxFont			font;			///< 字体
	wxColour		background;		///< 文本前景色
	wxColour		foreground;		///< 文本背景色

	int				xPadding;		///< 文本距作用域左边的距离
	int				yPadding;		///< 文本距作用域右边的距离
};

/// \brief 从一个 XRC 定义中提取背景属性
///
/// 当前主要应用于 VdkPanel 的背景色和边框的定义
struct BackgroundInfo {

	wxColour		bgColor;
	wxPen			borderPen;
};

/// \brief 从一个 XRC 定义中提取渐变色背景的属性
///
/// 当前主要应用于 VdkPanel 的背景色
struct GradientBgInfo {

	wxColour		beg;			///< 起始色
	wxColour		end;			///< 终止色
	wxDirection		direction;		///< 方向（东南西北中的一个）
};

//////////////////////////////////////////////////////////////////////////
/// \brief 派生类保存的风格属性集和运行时属性集
class VdkStyleAndStateOwner
{
public:

	/// \brief 构造函数
	VdkStyleAndStateOwner(long style = 0, long state = 0)
		: m_style( style ), m_state( state )
	{

	}

	/// \brief 获取当前控件的属性（风格）集
	long GetStyle() const { return m_style; }

	/// \brief 设置当前控件的属性（风格）集
	void SetStyle(long style) { m_style = style; }

	/// \brief 将指定值与当前风格属性集相或
	void SetAddinStyle(long style) { m_style |= style; }

	/// \brief 移除当前风格属性集的指定属性\a style
	void RemoveStyle(long style) { m_style &= ~style; }

	/// \brief 取反当前风格属性集中的指定属性\a style
	void ReverseStyle(long style) { m_style ^= style; }

	/// \brief 测试当前风格属性集中是否设置有指定属性\a style
	bool TestStyle(long style) const { return (m_style & style) != 0; }

	/// \brief 获取当前窗口的运行时状态属性集
	long GetState() const { return m_state; }

	/// \brief 设置当前窗口的运行时状态属性集
	void SetState(long state) { m_state = state; }

	/// \brief 将指定值与当前运行时状态属性集相或
	void SetAddinState(long state) { m_state |= state; }

	/// \brief 移除当前运行时状态属性集中的指定属性\a state
	void RemoveState(long state) { m_state &= ~state; }

	/// \brief 取反当前运行时状态属性集中的指定属性\a state
	void ReverseState(long state) { m_state ^= state; }

	/// \brief 测试当前运行时状态属性集中是否设置有指定属性\a state
	///
	/// \param state 若传递 X | Y 这种或值，则意义为只要 X 和 Y 中任意一位被
	/// 设置即返回真。
	bool TestState(long state) const { return (m_state & state) != 0; }

private:

	long			m_style; ///< 控件风格集
	long			m_state; ///< 控件运行时属性集，避免大量使用布尔变量
};

//////////////////////////////////////////////////////////////////////////
/// \brief 派生类保存的一个与类相关的、VDK 保证不会与之他用的指针
class VdkClientDataOwner
{
public:

	/// \brief 构造函数
	VdkClientDataOwner() : m_clientData( NULL ) {}

	/// \brief 获取保存的一个用户指定的空指针
	void* GetClientData() const { return m_clientData; }

	/// \brief 设置控件保存的一个用户自定义的空指针
	VdkClientDataOwner* SetClientData(void* clientData) {
		m_clientData = clientData;
		return this;
	}

	/// \brief 强制转型为派生类
	template< class DD >
	DD& castTo() { return static_cast< DD& >( *this ); }

private:

	void* m_clientData;
};

//////////////////////////////////////////////////////////////////////////
/// \brief 一个静态工具类，包含大量便利函数
class VdkUtil
{
public:

	/// \brief 将指定 DC 的某区域用某刷子清空
	static void ClrBkGnd(wxDC& dc, const wxBrush& bgBrush, 
						 const wxRect& rc);

	/// \brief 将指定 DC 的某区域保存到磁盘上
	static void SaveDC(wxDC& dc, const wxRect& rc, 
					   const wxChar* szSubfix = NULL);

	/// \brief 测试事件标识符是否鼠标左键激发的事件
	static bool IsLeftButtonEvent(int evtCode);

	//---------------------------------------------------

	/// \brief 获取当前进程空间的控件皮肤相关文件的根目录
	static wxString GetFilePath(const wxString& strFileName);

    /// \brief 从 XRC 文件中解析 <position> 结点
    /// \see GetXrcRect
	static wxRect ParseRect(wxXmlNode* xmlNode);

	/// \brief 直接从字符串\a strRect 中解析出一个 wxRect
	/// \param nType 可能取值: \n PRT_RECTANGLE wxWidgets 方式的 wxRect \n
	/// PRT_POSITION Windows SDK 方式
	/// \see RectType
	static wxRect ParseRect(const wxString& strRect, RectType nType);

    /// \brief 将位图平均分割成\a nExpl 部分
	static wxBitmap* ExplodeBitmap(const wxBitmap& bmOld, int nExpl);

	/// \brief 不知道为什么有时候 wxBitmap::GetSubBitmap 会得不到正确的结果
	static wxBitmap GetSubBitmapWithoutMask
		(const wxBitmap& bmp, const wxRect& subRegion);

	/// \brief 从文件路径\a path 中载入位图
	///
	/// 载入位图后会做一些必要的初始化工作，如设置透明色等。
	/// 调用者应保证\a path 字符串前后不含空格。
	static bool LoadMaskBitmap(wxBitmap& bitmap, const wxString& path);

	/// \brief 为位图创建掩码位图
	static void CreateMask(wxBitmap& bitmap, const wxColour* pMaskColor = NULL);

	/// \brief 从文件路径\a path 中载入多种格式的图片
	///
	/// 支持的文件格式： .BMP、.PNG、.JPG/.JPEG、.GIF、.ICO 等。
	/// 调用者应保证\a path 字符串前后不含空格。
	/// 对于 .ICO (Windows 图标) 文件，只会载入 16 * 16 大小的子图。
	/// 对于 .BMP (Windows 位图) 文件，不会自动创建透明位图。
	static bool ImRead(wxBitmap& bm, const wxString& path);

	/// \brief 从二进制代码中载入 PNG 图片
	static wxBitmap LoadPngFromRawData(const unsigned char* raw, int size);

	/// \brief 平铺位图
	///
	/// 可以同时指定新的高度和宽度。也可只指定其中一个，而将另一个实参设为 0 。
	/// \param mask 是否使用掩码位图进行复制
	/// \see StretchBitmap
	static wxBitmap TileBitmap(wxBitmap& bm,
							   int w, int h,
							   const wxRect& rcTile,
							   bool mask = true);

	/// \brief 平铺绘制位图
	///
	/// 可以同时指定新的高度和宽度。也可只指定其中一个，而将另一个实参设为 0 。
	/// 本函数是为了不保存太多不必要的位图变量而设立。
	/// \see StretchBitmap, TileBitmap
	static void TileDrawBitmap(wxDC& dc,
							   wxBitmap& bm,
							   int x,
							   int y,
							   int w,
							   int h,
							   const wxRect& rcTile);

	/// \brief 拉伸位图
	/// \param dcsrc 原始 DC
	/// \param dcdest 目标 DC
	/// \param src 原始区域
	/// \param dest 目标区域（指定起点和大小）
	/// \param bmTmp 避免多次分配临时位图的开销
	static void StretchBlit(wxDC& dcdest,
							const wxRect& src,
							wxDC& dcsrc,
							const wxRect& dest,
							wxBitmap& bmTmp);

	/// \brief 从中间扩展位图
	///
	/// 可以同时指定新的高度和宽度。本函数存在的意义是拉伸位图，
	/// 使边界不变形（对于 VdkWindow 的背景位图极为重要）。
	/// \see InflateDrawByCenter
	static wxBitmap InflateBitmapByCenter(wxBitmap& bm,
									      const wxRect& center,
									      const wxSize& szDest);

	/// \brief 从中间扩展位图
	///
	/// 可以同时指定新的高度和宽度。本函数存在的意义是拉伸位图，
	/// 使边界不变形（对于 VdkWindow 的背景位图极为重要）。
	/// \see StretchBitmap
	static void InflateDrawByCenter(wxBitmap& bm,
									wxDC& dc, 
									const wxSize& newsize, 
									const wxRect& center);

    /// \brief 从 XRC 文件中解析 <align> 结点(或其他指定的名为\a strNodeName 
    /// 的结点)
	static align_type GetXrcAlign(wxXmlNode* xmlNode,
								  wxString strNodeName = wxEmptyString);

    /// \brief 从 XRC 文件中解析 <text-align> 结点
	static align_type GetXrcTextAlign(wxXmlNode* xmlNode)
	{
		return GetXrcAlign( xmlNode, L"text-align" );
	}

	/// \brief 从 XRC 文件中解析 <rect> 结点
	///
	/// 本函数只能解析 wxWidgets 方式的 <rect> 结点。
	/// 如需要解析 Windows 方式表示的结点（同时结点名应为 position ），
	/// 请使用\link ParseRect \endlink 函数。
	/// \see ParseRect
	static wxRect GetXrcRect(wxXmlNode* xmlNode);

	/// \brief 从 XRC 文件中解析 <name> 结点
	static wxString GetXrcName(wxXmlNode* xmlNode);

	/// \brief 从 XRC 文件中解析紧跟着\a xmlNode 的首个纯文本子结点
	///
	/// 这个节点必须是一个纯文本结点。VDK 会去掉得到文本的首尾空格。
	static wxString GetXrcTextBlock(wxXmlNode* xmlNode);

	/// \brief 从 XRC 文件中解析 <image> 结点
	///
	/// 此时 <image> 结点指定的文件格式可以是 .BMP、.PNG、.JPG/.JPEG、
	/// .GIF 四种格式。
	/// 会根据 VdkControl 内保存的透明色创建相应的位图掩码对象。
	static bool GetXrcImage(wxXmlNode* xmlNode, wxBitmap& bm);

	/// \brief 从 XRC 文件中解析与文本绘制相关的属性。如字号、字体、颜色等等。
	/// \param window 假如 XRC 文件不声明相关信息，则该窗口的相关信息可供参考
	static TextInfo GetXrcTextInfo(wxXmlNode* xmlNode, 
								   wxWindow* window = NULL);

	/// \brief 从 XRC 文件中解析 <gradient-bg> 结点（渐变色背景）
	static GradientBgInfo* GetXrcGradientBgInfo(wxXmlNode* xmlNode);

	/// \brief 从 XRC 文件中解析 <bg> 结点（背景定义）
	static BackgroundInfo* GetXrcBgInfo(wxXmlNode* xmlNode);

private:

	/// \brief 沿 X 轴方向平铺绘制
	static void TileDraw_X(const wxRect& rcSrc, 
						   wxDC& dcSrc, 
						   const wxRect& rcDest, 
						   wxDC& dcDest, 
						   bool mask = false);

	/// \brief 沿 Y 轴方向平铺绘制
	static void TileDraw_Y(const wxRect& rcSrc, 
						   wxDC& dcSrc, 
						   const wxRect& rcDest, 
						   wxDC& dcDest, 
						   bool mask = false);
};

#define VdkLoadPngFromRawData( RawData ) \
	VdkUtil::LoadPngFromRawData( RawData, sizeof( RawData ) )
