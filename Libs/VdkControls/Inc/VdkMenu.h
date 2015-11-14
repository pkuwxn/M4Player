/***************************************************************
 * Name:      VdkMenuImplBase.h
 * Purpose:   一个基于 VdkWindow 的富形式自绘菜单
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-02-26
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkDefs.h"
#include "VdkBitmapArray.h"
#include "VdkUtil.h"
#include "VdkCtrlId.h"

/// \brief 菜单项的类型
enum VdkMenuType {

	MIT_NORMAL,	///< 普通项
	MIT_SEPERATOR, ///< 分隔条
	MIT_USER_DEFINED, ///< 用户自定义控件
};

//////////////////////////////////////////////////////////////////////////

enum VdkMenuStyle {

	/// \brief 是否使用左边的小位图
	VMS_BITMAP						= 1 << 0,
	/// \brief 是否不要添加任何额外的空白
	///
	/// 主要使得菜单上的用户自定义控件能够精确控制菜单实体窗体的大小。
	VMS_NO_EXTRA_SPACE				= 1 << 1,
	/// \brief 不要延迟一段时间后删除菜单实体窗体
	VMS_NO_AUTO_DELETE				= 1 << 2,
	/// \brief 重画前擦除一切内容
	VMS_ERASE_ALL					= 1 << 3,
	/// \brief 是否需要发送菜单预创建消息
	///
	/// 可用于创建前增减菜单项。
	VMS_SEND_PRECREATE_MSG			= 1 << 7,
	/// \brief 是否需要发送菜单预显示消息
	///
	/// 可用于菜单每次显示前增减菜单项。
	VMS_SEND_PRESHOW_MSG			= 1 << 8,
	/// \brief 发送菜单项被选择事件(当菜单项没有具体ID信息时)
	VMS_SEND_GLOBAL_SELECTED_MSG	= 1 << 9,
};

/// \brief 菜单所处的状态
enum VdkMenuState {

	/// \brief 是否已然初始化
	///
	/// 多此一举是考虑到可能初始化后不一定来得及重画，
	/// 故 VDK 保存的状态是未初始化，导致再次初始化
	VMST_INITIALIZED				= 1 << 0,
};

class VdkWindow;
class VdkMenuImpl;
class VdkMenuItem;
class VdkMenuItemImpl;
class VdkMenuCtrlHandler;

class VdkControl;
class VdkVObjEvent;

/// \brief 上下文菜单
class VdkMenu : public VdkClientDataOwner,
				public VdkStyleAndStateOwner
{
public:

	/// \brief 构造函数
	VdkMenu(int mid, VdkWindow* win, VdkMenu* parent);

	/// \brief 析构函数
	~VdkMenu();

	//////////////////////////////////////////////////////////////////////////

	/// \brief 从末尾添加一项
	void AppendItem(VdkMenuItem* item);

	/// \brief 从末尾添加一项
	void AppendItem(const wxString& strCaption);

	/// \brief 从末尾添加一项
	void AppendItem(VdkMenuItem& item) { AppendItem( &item ); }

	/// \brief 往菜单末尾添加一个分隔条
	void AppendSeperator();

	/// \brief 获取菜单总共的项数
	size_t GetItemCount() const;

	/// \brief 菜单是否为空(未初始化)
	bool IsEmpty() const;

	/// \brief 获取指定菜单项
	VdkMenuItem* GetItem(size_t i);

	/// \brief 获取具有指定 ID 的菜单项
	VdkMenuItem* FindItem(VdkCtrlId id);

	/// \brief 获取具有指定 ID 的菜单项
	VdkMenuItem* FindCheckedItem();

	//////////////////////////////////////////////////////////////////////////

	/// \brief 将指定控件依附到菜单上
	void AttachCtrl(VdkMenuCtrlHandler* hdler);

	/// \brief 从依附列表中删除指定控件
	void DeAttachCtrl(VdkMenuCtrlHandler* hdler);

	//////////////////////////////////////////////////////////////////////////

	/// \brief 在<b>**以屏幕计量**</b>的位置(\a x, \a y)处显示菜单
	/// \param pCtrl 与菜单相关联的控件句柄，为空则与\a window 相关联
	void ShowContext(int x, int y, VdkControl* pCtrl);

    /**
        @overload
    */
	void ShowContext(const wxPoint& pos, VdkControl* pCtrl) {
		ShowContext( pos.x, pos.y, pCtrl );
	}

	/// \brief 隐藏菜单
	void Return();

	/// \brief 获取菜单的祖先
	/// \return 无父菜单时返回自身。
	VdkMenu* GetRootMenu();

	/// \brief 获取父菜单
	VdkMenu* GetParentMenu() const { return m_parent; }

	/// \brief 获取实体窗体
	VdkMenuImpl* GetImpl() const { return m_impl.GetImpl(); }

	/// \brief 获取实体窗体对应的 VdkWindow
	VdkWindow* GetVdkWindowImpl() const;

	/// \brief 获取父窗口
	VdkWindow* GetVdkWindow() const { return m_window; }

	/// \brief 获取当前正在显示的最后一个层次的子菜单
	VdkMenu* GetLastShownMenu() const;

	/// \brief 菜单当前是否正在处于活动状态
	bool IsShown() const { return m_impl.IsShown(); }

	/// \brief 创建 VdkMenuItemImpl
	void ImplementMenu();

	//////////////////////////////////////////////////////////////////////////

	/// \brief 鼠标指针是否位于菜单窗体内
	///
	/// 用于判断指针是否转移至子菜单作用域内。
	bool ContainsPointerGlobally() const ;

	/// \brief 鼠标指针是否位于菜单窗体内
	///
	/// 用于判断指针是否转移至子菜单作用域内。
	bool ContainsPointer(const wxPoint& mousePos) const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief 设置菜单的最佳宽度
	///
	/// 当菜单正常所需宽度小于\a w 时，菜单仍然会扩展到\a w 的长度。
	void SetBestWidth(int w);

	/// \brief 是否已然初始化
	bool IsInitialized() const { return TestState( VMST_INITIALIZED ); }

	/// \brief 响应实体窗体的销毁事件
	void OnMenuImplDestroy();

private:

	// 计算菜单宽度
	void CalcMenuWidth();

	// 计算依附到菜单的控件的最大宽度，以确定菜单的合适宽度
	// 返回菜单新宽度与旧宽度之间的差值。
	int MeasureOnMenuCtrls();

	// 单击事件的中转处理
	static void OnSelect(VdkVObjEvent& e);

	// 发送菜单预创建、预显示信息
	// 接受者可在菜单实体窗体创建前或每次显示前执行增减、禁用菜单项等操作。
	void SendPreOperateMessage(VdkMenuStyle style);

	// 触发选中事件
	void FireEvent(VdkMenuItem* item);

private:

	VdkWindow* m_window;
	VdkMenu* m_parent;

	VdkCtrlId m_id; // 菜单 ID
	static VdkCtrlId m_fakeId; // 假菜单项 ID ，用于事件中转处理

	int m_width;
	int m_height;
	int m_bestWidth; // 最佳宽度，不应该小于该值
	int	m_maxTextWidth;

	VdkControl* m_pRelativeCtrl; // 相关联的控件，用于通知控件菜单隐藏

private:

	VDK_DECLARE_WX_PTR_LIST_NO_FORWARD_DECL
		( VdkMenuItem, item_list, item_list_iterator )

	VDK_DECLARE_WX_PTR_LIST_NO_FORWARD_DECL
		( VdkMenuCtrlHandler, OnMenuCtrls, OnMenuCtrlIter )

public:

	typedef item_list_iterator_Const ItemIter_Const;

	/// \brief 返回菜单项常量迭代器的起始
	ItemIter_Const begin() const { return m_items.begin(); }

	/// \brief 返回菜单项常量迭代器的终止
	ItemIter_Const end() const { return m_items.end(); }

	typedef item_list_iterator ItemIter;

	/// \brief 返回菜单项迭代器的起始
	ItemIter begin() { return m_items.begin(); }

	/// \brief 返回菜单项迭代器的终止
	ItemIter end() { return m_items.end(); }

private:

	item_list m_items;
	OnMenuCtrls m_attached; // 依附于其上的用户自定义控件

	//////////////////////////////////////////////////////////////////////////
	/// \brief 使用对象对菜单实体窗体的生命周期进行有效管理
	///
	/// 即需要显示菜单时创建实体窗体，菜单被隐藏/选中后立即销毁实体窗体。
	class VdkMenuPtr : public wxTimer
	{
	public:

		/// \brief 构造函数
		VdkMenuPtr(VdkMenu* menu);

		/// \brief 析构函数
		~VdkMenuPtr();

		/// \brief 使对象能像普通指针一样运作
		VdkMenuImpl* operator -> ();

		/// \brief 获取实际窗体对象
		VdkMenuImpl* GetImpl() const { return m_impl; }

		/// \brief 创建实际窗体对象
		void Implement();

		/// \brief 销毁实体窗体
		/// \param delAtOnce 是否立即销毁窗体
		void Release(bool delAtOnce);

		/// \brief 当前实体窗体是否存在
		bool IsValid() const { return m_impl != NULL; }

		/// \brief 当前实体窗体是否存在
		bool IsShown() const;

	private:

		// 来自 wxTimer，作用为延迟删除实体菜单窗体
		virtual void Notify();

		// 响应实体窗体的销毁事件
		void OnImplDestroy(wxWindowDestroyEvent&);

		//////////////////////////////////////////////////////////////////////////

		VdkMenu* m_menu;
		VdkMenuImpl* m_impl;
	};

	VdkMenuPtr m_impl;

public:

	enum MenuStyle {

		MSOFFICE_2003,
		MSOFFICE_2007,
		TM2009,
		QQ2010,
	};

private:

	//////////////////////////////////////////////////////////////////////////
	// \brief 菜单显示风格定义
	class StaticStyle {
	public:

		int rowHeight; ///< 菜单项高
		int borderWeight; ///< 边框宽度
		int padding; ///< 高亮条与菜单边框之间的距离
		int textPaddingCount; ///< 文本与左边位图区域之间的距离（以字符数计）
		int bitmapRegion; ///< 左边显示位图的区域的宽度
		int rightArrowSize; ///< 代表子菜单的右边箭头宽度
		int subMainGap; ///< 子菜单与父菜单之间的间隙宽度
		/// \brief 菜单文本各部分之间的间隙宽度，如右箭头与加速键之间
		int componentPaddingCount;

		wxColour bgColor; ///< 菜单背景颜色
		wxBrush bitmapRegionBrush; ///< 左边显示位图的区域所用的画刷
		wxBrush hilightBrush; ///< 菜单项高亮时所用的画刷
		wxBrush checkedBrush; ///< 选中项普通状态下绘制时所用的画刷
		wxBrush checkedHilightBrush; ///< 选中项高亮时绘制所用的画刷
		wxPen hilightBorderPen; ///< 菜单项高亮时的边框颜色
		wxPen borderPen; ///< 菜单边框颜色

		wxFont menuFont; ///< 菜单所用字体
		// 菜单已选时表现所用的的对勾，wxWidgets 绘制时会添加白色的背景
		wxBitmap checkMark;

	} * m_sstyle;

	//////////////////////////////////////////////////////////////////////////
	// \brief 菜单显示风格定义工厂
	class StyleFactory
	{
	public:

		/// \brief 获取指定风格的定义
		static StaticStyle* GetStyle(MenuStyle);

	private:

		/// \brief 设置风格的共同属性
		static void InitStyle(StaticStyle& sstyle);

		/// \brief 获取 MS Office 2003 风格的定义
		static StaticStyle* GetOffice2003Style();

		/// \brief 获取 TM2009 风格的定义
		static StaticStyle* GetTM2009Style();
	};

public:

	/// \brief 选择菜单绘制时所用的风格
	bool SelectStaticStyle(MenuStyle style);

	/// \brief 获取左边显示位图的区域的宽度
	int GetBitmapRegionWidth() const { return m_sstyle->bitmapRegion; }

	/// \brief 获取菜单项高
	int GetRowHeight() const { return m_sstyle->rowHeight; }

	/// \brief 获取菜单文本与菜单左边距的距离
	/// \return 不包括菜单边框
	int GetTextPaddingToLeft() const ;

	/// \brief 获取菜单文本与菜单右边距之间的距离
	///
	/// 与右边距或左边色条右侧的距离。
	int GetTextPaddingToRight() const ;

	/// \brief 获取菜单边框宽度
	int GetBorderWeight() const { return m_sstyle->borderWeight; }

	/// \brief 获取子菜单的右边箭头宽度
	int GetRightArrowSize() const { return m_sstyle->rightArrowSize; }

	/// \brief 获取左边显示位图的区域的画刷
	const wxBrush& GetBitmapRegionBrush() const {
		return m_sstyle->bitmapRegionBrush;
	}

	/// \brief 获取高亮项所用的画刷
	const wxBrush& GetHilightBrush() const { return m_sstyle->hilightBrush; }

	/// \brief 获取选中项普通状态下绘制时所用的画刷
	const wxBrush& GetCheckedBrush() const { return m_sstyle->checkedBrush; }

	/// \brief 获取选中项高亮时绘制所用的画刷
	const wxBrush& GetCheckedHilightBrush() const {
		return m_sstyle->checkedHilightBrush;
	}

	/// \brief 获取菜单项高亮时的边框颜色
	wxPen GetHilightBorderPen() const { return m_sstyle->hilightBorderPen; }

	/// \brief 获取菜单边框颜色
	wxPen GetBorderPen() const { return m_sstyle->borderPen; }

	/// \brief 获取子菜单与父菜单之间的间隙宽度
	int GetSubMainGap() const { return m_sstyle->subMainGap; }

	/// \brief 绘制菜单项选中时的对勾
	void DrawCheckMark(wxDC& dc, int x, int y) const ;
};

//////////////////////////////////////////////////////////////////////////

/// \brief 菜单项的风格属性集
enum VdkMenuItemStyle {

	VMIS_CHECKABLE		= 1 << 0, ///< 允许选中/不选中
};

/// \brief 菜单项的运行时状态属性集
enum VdkMenuItemState {

	VMIST_CHECKED		= 1 << 0, ///< 是否已被选中
	VMIST_DISABLED		= 1 << 1, ///< 是否已被禁用
};

//////////////////////////////////////////////////////////////////////////
/// \brief 菜单的一项
///
/// 包括分隔条。
class VdkMenuItem : public VdkClientDataOwner,
					public VdkStyleAndStateOwner
{
public:

	/// \brief 构造函数
	VdkMenuItem(VdkMenuType type = MIT_NORMAL)
		: nType( type ),
		  nID( -1 ),
		  pSubMenu( NULL ),
		  nStartY( 0 ),
		  nAcceStrWidth( 0 )
	{

	}

	/// \brief 析构函数
	~VdkMenuItem();

	/// \brief 获取菜单项的类型
	VdkMenuType type() const { return nType; }

	/// \brief 获取菜单项 ID
	VdkCtrlId id() const { return nID; }

	/// \brief 设置菜单项 ID
	VdkMenuItem& id(VdkCtrlId i) { nID = i; return *this; }

	/// \brief 获取菜单项的标题文本
	const wxString& caption() const { return strCaption; }

	/// \brief 设置菜单项的标题文本
	VdkMenuItem& caption(const wxString& s) {
		strCaption = s; return *this;
	}

	/// \brief 获取菜单项加速键描述文本
	const wxString& accelString() const { return strAccel; }

	/// \brief 设置菜单项加速键描述文本
	VdkMenuItem& accelString(const wxString& s) {
		strAccel = s; return *this;
	}

	/// \brief 获取菜单项加速键描述文本的宽度
	int accelStringWidth() { return nAcceStrWidth; }

	/// \brief 是否设置了左侧位图
	bool isBitmapOk() const { return BitmapId.IsOk(); }

	/// \brief 获取位图 ID
	const VdkBitmapArrayId& bmpArrayID() const { return BitmapId; }

	/// \brief 设置位图 ID
	VdkMenuItem& bmpArrayID(const VdkBitmapArrayId& i) {
		BitmapId = i; return *this;
	}

	/// \brief 获取子菜单句柄
	VdkMenu* subMenu() const { return pSubMenu; }

	/// \brief 设置子菜单句柄
	VdkMenuItem& subMenu(VdkMenu* sm) { pSubMenu = sm; return *this; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief 设置菜单项当前是否允许被选中
	VdkMenuItem& checkable(bool b);

	/// \brief 测试菜单项当前是否允许被选中
	bool checkable() const  { return TestStyle( VMIS_CHECKABLE ); }

	/// \brief 设置菜单项当前是否已被选中
	VdkMenuItem& checked(bool b);

	/// \brief 测试菜单项当前是否已被选中
	bool checked() const  { return TestState( VMIST_CHECKED ); }

	/// \brief 禁用菜单项
	VdkMenuItem& disabled(bool d);

	/// \brief 测试菜单项是否已被禁用
	bool disabled() const { return TestState( VMIST_DISABLED ); }

private:

	VdkMenuType				nType;
	VdkCtrlId				nID;
	wxString				strCaption;
	wxString				strAccel;

	VdkBitmapArrayId		BitmapId;
	VdkMenu*				pSubMenu;

	int						nStartY;
	int						nAcceStrWidth;

	friend class VdkMenu;
};

/// \brief 便利函数，在堆上新创建一个菜单项
inline VdkMenuItem* NewMenuItem() { return new VdkMenuItem(); }

//////////////////////////////////////////////////////////////////////////

/// \brief 允许在 VdkMenu 上显示普通 VdkControl 的适配器类
class VdkMenuCtrlHandler
{
public:

	/// \brief 默认构造函数
	VdkMenuCtrlHandler()
		: m_pCtrl( NULL ),
		  m_id( VdkGetUniqueId() ),
		  m_paddingLeft( 0 ),
		  m_paddingTop( 0 ),
		  m_paddingRight( 0 ),
		  m_paddingBottom( 0 )
	{

	}

	/// \brief 虚析构函数
	virtual ~VdkMenuCtrlHandler() {}

	/// \brief 实例化要附加到 VdkMenu 上的控件
	virtual VdkControl* Implement(VdkWindow* menuWin, const wxPoint& pos) = 0;

	/// \brief 获取控件句柄
	VdkControl* GetCtrl() const { return m_pCtrl; }

	/// \brief 获取控件与菜单之间的内边距
	void GetPaddings(int* l, int* t, int* r, int* b) {
		if( l ) *l = m_paddingLeft;
		if( t ) *t = m_paddingTop;
		if( r ) *r = m_paddingRight;
		if( b ) *b = m_paddingBottom;
	}

	/// \brief 重置控件句柄
	void Reset();

protected:

	/// \brief 保存控件状态
	virtual void SaveState() {}

	/// \brief 还原控件状态
	virtual void RestoreState() {}

	//////////////////////////////////////////////////////////////////////////

	VdkControl* m_pCtrl; ///< 控件句柄
	VdkCtrlId m_id; ///< 控件 ID

	int m_paddingLeft; ///< 控件与菜单左边界之间的距离
	int m_paddingTop; ///< 控件与菜单顶部内边界之间的距离
	int m_paddingRight; ///< 控件与菜单右边界之间的距离
	int m_paddingBottom; ///< 控件与菜单底部内边界之间的距离
};

/// \brief 结合 XRC 机制的一个简易绑定类
class VdkMenuXrcCtrlWrapper : public VdkMenuCtrlHandler
{
public:

	/// \brief 构造函数
	VdkMenuXrcCtrlWrapper(const wxString& strXmlData)
		: m_strXmlData( strXmlData )
	{
		wxASSERT( !strXmlData.empty() );
	}

	/// \brief 实例化要附加到 VdkMenu 上的控件
	virtual VdkControl* Implement(VdkWindow* menuWin, const wxPoint& pos);

private:

	wxString m_strXmlData;
};

/// \brief 结合控件克隆机制的一个简易绑定类
class VdkMenuCloneableCtrlWrapper : public VdkMenuCtrlHandler
{
public:

	/// \brief 构造函数
	VdkMenuCloneableCtrlWrapper(VdkWindow* parent, const wxString& strName)
		: m_Window( parent ), m_strName( strName )
	{
		wxASSERT( parent && !strName.IsEmpty() );
	}

	/// \brief 实例化要附加到 VdkMenu 上的控件
	virtual VdkControl* Implement(VdkWindow* menuWin, const wxPoint& pos);

private:

	VdkWindow* m_Window;
	wxString m_strName;
};

//////////////////////////////////////////////////////////////////////////

#include <wx/hashmap.h>
WX_DECLARE_HASH_MAP( int, wxString, wxIntegerHash, wxIntegerEqual, MenuGenItemMap );

/// \brief 从一个“ID-字符串”对应图中创建整套菜单
struct MenuGenerator {

	/// \brief 构造函数
	MenuGenerator(int menuID, VdkMenu* p = NULL)
		: mid( menuID ), parent( p ) 
	{

	}

	/// \brief 清空所有留存信息，为下次创建做清理工作
	void Clear();

	/// \brief “ID-字符串”对应图
	MenuGenItemMap itemMap;
	int mid; ///< 菜单 ID
	VdkMenu* parent; ///< 父菜单
};

/// \brief 从一个包含关系树的字符串创建整套菜单
///
/// 示例：字符串“CMCC WLAN;<SEP>;华南理工大学图书馆”将生成两个菜单项
/// 和一个将两者隔开的分隔条。
/// \attention 本函数不支持层叠菜单。
VdkMenu* GenerateMenu(VdkWindow* parent, const MenuGenerator& menuTree);

/// \brief 从一个包含关系树的字符串创建整套菜单
///
/// 示例：字符串“CMCC WLAN;<SEP>;华南理工大学图书馆”将生成两个菜单项
/// 和一个将两者隔开的分隔条。
/// \attention 本函数不支持层叠菜单。
VdkMenu* GenerateMenu(int mid, VdkWindow* parent, const wxString& strMenuTree);
