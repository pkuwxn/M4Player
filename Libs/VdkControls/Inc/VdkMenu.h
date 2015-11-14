/***************************************************************
 * Name:      VdkMenuImplBase.h
 * Purpose:   һ������ VdkWindow �ĸ���ʽ�Ի�˵�
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-02-26
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkDefs.h"
#include "VdkBitmapArray.h"
#include "VdkUtil.h"
#include "VdkCtrlId.h"

/// \brief �˵��������
enum VdkMenuType {

	MIT_NORMAL,	///< ��ͨ��
	MIT_SEPERATOR, ///< �ָ���
	MIT_USER_DEFINED, ///< �û��Զ���ؼ�
};

//////////////////////////////////////////////////////////////////////////

enum VdkMenuStyle {

	/// \brief �Ƿ�ʹ����ߵ�Сλͼ
	VMS_BITMAP						= 1 << 0,
	/// \brief �Ƿ�Ҫ����κζ���Ŀհ�
	///
	/// ��Ҫʹ�ò˵��ϵ��û��Զ���ؼ��ܹ���ȷ���Ʋ˵�ʵ�崰��Ĵ�С��
	VMS_NO_EXTRA_SPACE				= 1 << 1,
	/// \brief ��Ҫ�ӳ�һ��ʱ���ɾ���˵�ʵ�崰��
	VMS_NO_AUTO_DELETE				= 1 << 2,
	/// \brief �ػ�ǰ����һ������
	VMS_ERASE_ALL					= 1 << 3,
	/// \brief �Ƿ���Ҫ���Ͳ˵�Ԥ������Ϣ
	///
	/// �����ڴ���ǰ�����˵��
	VMS_SEND_PRECREATE_MSG			= 1 << 7,
	/// \brief �Ƿ���Ҫ���Ͳ˵�Ԥ��ʾ��Ϣ
	///
	/// �����ڲ˵�ÿ����ʾǰ�����˵��
	VMS_SEND_PRESHOW_MSG			= 1 << 8,
	/// \brief ���Ͳ˵��ѡ���¼�(���˵���û�о���ID��Ϣʱ)
	VMS_SEND_GLOBAL_SELECTED_MSG	= 1 << 9,
};

/// \brief �˵�������״̬
enum VdkMenuState {

	/// \brief �Ƿ���Ȼ��ʼ��
	///
	/// ���һ���ǿ��ǵ����ܳ�ʼ����һ�����ü��ػ���
	/// �� VDK �����״̬��δ��ʼ���������ٴγ�ʼ��
	VMST_INITIALIZED				= 1 << 0,
};

class VdkWindow;
class VdkMenuImpl;
class VdkMenuItem;
class VdkMenuItemImpl;
class VdkMenuCtrlHandler;

class VdkControl;
class VdkVObjEvent;

/// \brief �����Ĳ˵�
class VdkMenu : public VdkClientDataOwner,
				public VdkStyleAndStateOwner
{
public:

	/// \brief ���캯��
	VdkMenu(int mid, VdkWindow* win, VdkMenu* parent);

	/// \brief ��������
	~VdkMenu();

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ĩβ���һ��
	void AppendItem(VdkMenuItem* item);

	/// \brief ��ĩβ���һ��
	void AppendItem(const wxString& strCaption);

	/// \brief ��ĩβ���һ��
	void AppendItem(VdkMenuItem& item) { AppendItem( &item ); }

	/// \brief ���˵�ĩβ���һ���ָ���
	void AppendSeperator();

	/// \brief ��ȡ�˵��ܹ�������
	size_t GetItemCount() const;

	/// \brief �˵��Ƿ�Ϊ��(δ��ʼ��)
	bool IsEmpty() const;

	/// \brief ��ȡָ���˵���
	VdkMenuItem* GetItem(size_t i);

	/// \brief ��ȡ����ָ�� ID �Ĳ˵���
	VdkMenuItem* FindItem(VdkCtrlId id);

	/// \brief ��ȡ����ָ�� ID �Ĳ˵���
	VdkMenuItem* FindCheckedItem();

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ָ���ؼ��������˵���
	void AttachCtrl(VdkMenuCtrlHandler* hdler);

	/// \brief �������б���ɾ��ָ���ؼ�
	void DeAttachCtrl(VdkMenuCtrlHandler* hdler);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��<b>**����Ļ����**</b>��λ��(\a x, \a y)����ʾ�˵�
	/// \param pCtrl ��˵�������Ŀؼ������Ϊ������\a window �����
	void ShowContext(int x, int y, VdkControl* pCtrl);

    /**
        @overload
    */
	void ShowContext(const wxPoint& pos, VdkControl* pCtrl) {
		ShowContext( pos.x, pos.y, pCtrl );
	}

	/// \brief ���ز˵�
	void Return();

	/// \brief ��ȡ�˵�������
	/// \return �޸��˵�ʱ��������
	VdkMenu* GetRootMenu();

	/// \brief ��ȡ���˵�
	VdkMenu* GetParentMenu() const { return m_parent; }

	/// \brief ��ȡʵ�崰��
	VdkMenuImpl* GetImpl() const { return m_impl.GetImpl(); }

	/// \brief ��ȡʵ�崰���Ӧ�� VdkWindow
	VdkWindow* GetVdkWindowImpl() const;

	/// \brief ��ȡ������
	VdkWindow* GetVdkWindow() const { return m_window; }

	/// \brief ��ȡ��ǰ������ʾ�����һ����ε��Ӳ˵�
	VdkMenu* GetLastShownMenu() const;

	/// \brief �˵���ǰ�Ƿ����ڴ��ڻ״̬
	bool IsShown() const { return m_impl.IsShown(); }

	/// \brief ���� VdkMenuItemImpl
	void ImplementMenu();

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���ָ���Ƿ�λ�ڲ˵�������
	///
	/// �����ж�ָ���Ƿ�ת�����Ӳ˵��������ڡ�
	bool ContainsPointerGlobally() const ;

	/// \brief ���ָ���Ƿ�λ�ڲ˵�������
	///
	/// �����ж�ָ���Ƿ�ת�����Ӳ˵��������ڡ�
	bool ContainsPointer(const wxPoint& mousePos) const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���ò˵�����ѿ��
	///
	/// ���˵�����������С��\a w ʱ���˵���Ȼ����չ��\a w �ĳ��ȡ�
	void SetBestWidth(int w);

	/// \brief �Ƿ���Ȼ��ʼ��
	bool IsInitialized() const { return TestState( VMST_INITIALIZED ); }

	/// \brief ��Ӧʵ�崰��������¼�
	void OnMenuImplDestroy();

private:

	// ����˵����
	void CalcMenuWidth();

	// �����������˵��Ŀؼ�������ȣ���ȷ���˵��ĺ��ʿ��
	// ���ز˵��¿����ɿ��֮��Ĳ�ֵ��
	int MeasureOnMenuCtrls();

	// �����¼�����ת����
	static void OnSelect(VdkVObjEvent& e);

	// ���Ͳ˵�Ԥ������Ԥ��ʾ��Ϣ
	// �����߿��ڲ˵�ʵ�崰�崴��ǰ��ÿ����ʾǰִ�����������ò˵���Ȳ�����
	void SendPreOperateMessage(VdkMenuStyle style);

	// ����ѡ���¼�
	void FireEvent(VdkMenuItem* item);

private:

	VdkWindow* m_window;
	VdkMenu* m_parent;

	VdkCtrlId m_id; // �˵� ID
	static VdkCtrlId m_fakeId; // �ٲ˵��� ID �������¼���ת����

	int m_width;
	int m_height;
	int m_bestWidth; // ��ѿ�ȣ���Ӧ��С�ڸ�ֵ
	int	m_maxTextWidth;

	VdkControl* m_pRelativeCtrl; // ������Ŀؼ�������֪ͨ�ؼ��˵�����

private:

	VDK_DECLARE_WX_PTR_LIST_NO_FORWARD_DECL
		( VdkMenuItem, item_list, item_list_iterator )

	VDK_DECLARE_WX_PTR_LIST_NO_FORWARD_DECL
		( VdkMenuCtrlHandler, OnMenuCtrls, OnMenuCtrlIter )

public:

	typedef item_list_iterator_Const ItemIter_Const;

	/// \brief ���ز˵��������������ʼ
	ItemIter_Const begin() const { return m_items.begin(); }

	/// \brief ���ز˵��������������ֹ
	ItemIter_Const end() const { return m_items.end(); }

	typedef item_list_iterator ItemIter;

	/// \brief ���ز˵������������ʼ
	ItemIter begin() { return m_items.begin(); }

	/// \brief ���ز˵������������ֹ
	ItemIter end() { return m_items.end(); }

private:

	item_list m_items;
	OnMenuCtrls m_attached; // ���������ϵ��û��Զ���ؼ�

	//////////////////////////////////////////////////////////////////////////
	/// \brief ʹ�ö���Բ˵�ʵ�崰����������ڽ�����Ч����
	///
	/// ����Ҫ��ʾ�˵�ʱ����ʵ�崰�壬�˵�������/ѡ�к���������ʵ�崰�塣
	class VdkMenuPtr : public wxTimer
	{
	public:

		/// \brief ���캯��
		VdkMenuPtr(VdkMenu* menu);

		/// \brief ��������
		~VdkMenuPtr();

		/// \brief ʹ����������ָͨ��һ������
		VdkMenuImpl* operator -> ();

		/// \brief ��ȡʵ�ʴ������
		VdkMenuImpl* GetImpl() const { return m_impl; }

		/// \brief ����ʵ�ʴ������
		void Implement();

		/// \brief ����ʵ�崰��
		/// \param delAtOnce �Ƿ��������ٴ���
		void Release(bool delAtOnce);

		/// \brief ��ǰʵ�崰���Ƿ����
		bool IsValid() const { return m_impl != NULL; }

		/// \brief ��ǰʵ�崰���Ƿ����
		bool IsShown() const;

	private:

		// ���� wxTimer������Ϊ�ӳ�ɾ��ʵ��˵�����
		virtual void Notify();

		// ��Ӧʵ�崰��������¼�
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
	// \brief �˵���ʾ�����
	class StaticStyle {
	public:

		int rowHeight; ///< �˵����
		int borderWeight; ///< �߿���
		int padding; ///< ��������˵��߿�֮��ľ���
		int textPaddingCount; ///< �ı������λͼ����֮��ľ��루���ַ����ƣ�
		int bitmapRegion; ///< �����ʾλͼ������Ŀ��
		int rightArrowSize; ///< �����Ӳ˵����ұ߼�ͷ���
		int subMainGap; ///< �Ӳ˵��븸�˵�֮��ļ�϶���
		/// \brief �˵��ı�������֮��ļ�϶��ȣ����Ҽ�ͷ����ټ�֮��
		int componentPaddingCount;

		wxColour bgColor; ///< �˵�������ɫ
		wxBrush bitmapRegionBrush; ///< �����ʾλͼ���������õĻ�ˢ
		wxBrush hilightBrush; ///< �˵������ʱ���õĻ�ˢ
		wxBrush checkedBrush; ///< ѡ������ͨ״̬�»���ʱ���õĻ�ˢ
		wxBrush checkedHilightBrush; ///< ѡ�������ʱ�������õĻ�ˢ
		wxPen hilightBorderPen; ///< �˵������ʱ�ı߿���ɫ
		wxPen borderPen; ///< �˵��߿���ɫ

		wxFont menuFont; ///< �˵���������
		// �˵���ѡʱ�������õĵĶԹ���wxWidgets ����ʱ����Ӱ�ɫ�ı���
		wxBitmap checkMark;

	} * m_sstyle;

	//////////////////////////////////////////////////////////////////////////
	// \brief �˵���ʾ����幤��
	class StyleFactory
	{
	public:

		/// \brief ��ȡָ�����Ķ���
		static StaticStyle* GetStyle(MenuStyle);

	private:

		/// \brief ���÷��Ĺ�ͬ����
		static void InitStyle(StaticStyle& sstyle);

		/// \brief ��ȡ MS Office 2003 ���Ķ���
		static StaticStyle* GetOffice2003Style();

		/// \brief ��ȡ TM2009 ���Ķ���
		static StaticStyle* GetTM2009Style();
	};

public:

	/// \brief ѡ��˵�����ʱ���õķ��
	bool SelectStaticStyle(MenuStyle style);

	/// \brief ��ȡ�����ʾλͼ������Ŀ��
	int GetBitmapRegionWidth() const { return m_sstyle->bitmapRegion; }

	/// \brief ��ȡ�˵����
	int GetRowHeight() const { return m_sstyle->rowHeight; }

	/// \brief ��ȡ�˵��ı���˵���߾�ľ���
	/// \return �������˵��߿�
	int GetTextPaddingToLeft() const ;

	/// \brief ��ȡ�˵��ı���˵��ұ߾�֮��ľ���
	///
	/// ���ұ߾�����ɫ���Ҳ�ľ��롣
	int GetTextPaddingToRight() const ;

	/// \brief ��ȡ�˵��߿���
	int GetBorderWeight() const { return m_sstyle->borderWeight; }

	/// \brief ��ȡ�Ӳ˵����ұ߼�ͷ���
	int GetRightArrowSize() const { return m_sstyle->rightArrowSize; }

	/// \brief ��ȡ�����ʾλͼ������Ļ�ˢ
	const wxBrush& GetBitmapRegionBrush() const {
		return m_sstyle->bitmapRegionBrush;
	}

	/// \brief ��ȡ���������õĻ�ˢ
	const wxBrush& GetHilightBrush() const { return m_sstyle->hilightBrush; }

	/// \brief ��ȡѡ������ͨ״̬�»���ʱ���õĻ�ˢ
	const wxBrush& GetCheckedBrush() const { return m_sstyle->checkedBrush; }

	/// \brief ��ȡѡ�������ʱ�������õĻ�ˢ
	const wxBrush& GetCheckedHilightBrush() const {
		return m_sstyle->checkedHilightBrush;
	}

	/// \brief ��ȡ�˵������ʱ�ı߿���ɫ
	wxPen GetHilightBorderPen() const { return m_sstyle->hilightBorderPen; }

	/// \brief ��ȡ�˵��߿���ɫ
	wxPen GetBorderPen() const { return m_sstyle->borderPen; }

	/// \brief ��ȡ�Ӳ˵��븸�˵�֮��ļ�϶���
	int GetSubMainGap() const { return m_sstyle->subMainGap; }

	/// \brief ���Ʋ˵���ѡ��ʱ�ĶԹ�
	void DrawCheckMark(wxDC& dc, int x, int y) const ;
};

//////////////////////////////////////////////////////////////////////////

/// \brief �˵���ķ�����Լ�
enum VdkMenuItemStyle {

	VMIS_CHECKABLE		= 1 << 0, ///< ����ѡ��/��ѡ��
};

/// \brief �˵��������ʱ״̬���Լ�
enum VdkMenuItemState {

	VMIST_CHECKED		= 1 << 0, ///< �Ƿ��ѱ�ѡ��
	VMIST_DISABLED		= 1 << 1, ///< �Ƿ��ѱ�����
};

//////////////////////////////////////////////////////////////////////////
/// \brief �˵���һ��
///
/// �����ָ�����
class VdkMenuItem : public VdkClientDataOwner,
					public VdkStyleAndStateOwner
{
public:

	/// \brief ���캯��
	VdkMenuItem(VdkMenuType type = MIT_NORMAL)
		: nType( type ),
		  nID( -1 ),
		  pSubMenu( NULL ),
		  nStartY( 0 ),
		  nAcceStrWidth( 0 )
	{

	}

	/// \brief ��������
	~VdkMenuItem();

	/// \brief ��ȡ�˵��������
	VdkMenuType type() const { return nType; }

	/// \brief ��ȡ�˵��� ID
	VdkCtrlId id() const { return nID; }

	/// \brief ���ò˵��� ID
	VdkMenuItem& id(VdkCtrlId i) { nID = i; return *this; }

	/// \brief ��ȡ�˵���ı����ı�
	const wxString& caption() const { return strCaption; }

	/// \brief ���ò˵���ı����ı�
	VdkMenuItem& caption(const wxString& s) {
		strCaption = s; return *this;
	}

	/// \brief ��ȡ�˵�����ټ������ı�
	const wxString& accelString() const { return strAccel; }

	/// \brief ���ò˵�����ټ������ı�
	VdkMenuItem& accelString(const wxString& s) {
		strAccel = s; return *this;
	}

	/// \brief ��ȡ�˵�����ټ������ı��Ŀ��
	int accelStringWidth() { return nAcceStrWidth; }

	/// \brief �Ƿ����������λͼ
	bool isBitmapOk() const { return BitmapId.IsOk(); }

	/// \brief ��ȡλͼ ID
	const VdkBitmapArrayId& bmpArrayID() const { return BitmapId; }

	/// \brief ����λͼ ID
	VdkMenuItem& bmpArrayID(const VdkBitmapArrayId& i) {
		BitmapId = i; return *this;
	}

	/// \brief ��ȡ�Ӳ˵����
	VdkMenu* subMenu() const { return pSubMenu; }

	/// \brief �����Ӳ˵����
	VdkMenuItem& subMenu(VdkMenu* sm) { pSubMenu = sm; return *this; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���ò˵��ǰ�Ƿ�����ѡ��
	VdkMenuItem& checkable(bool b);

	/// \brief ���Բ˵��ǰ�Ƿ�����ѡ��
	bool checkable() const  { return TestStyle( VMIS_CHECKABLE ); }

	/// \brief ���ò˵��ǰ�Ƿ��ѱ�ѡ��
	VdkMenuItem& checked(bool b);

	/// \brief ���Բ˵��ǰ�Ƿ��ѱ�ѡ��
	bool checked() const  { return TestState( VMIST_CHECKED ); }

	/// \brief ���ò˵���
	VdkMenuItem& disabled(bool d);

	/// \brief ���Բ˵����Ƿ��ѱ�����
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

/// \brief �����������ڶ����´���һ���˵���
inline VdkMenuItem* NewMenuItem() { return new VdkMenuItem(); }

//////////////////////////////////////////////////////////////////////////

/// \brief ������ VdkMenu ����ʾ��ͨ VdkControl ����������
class VdkMenuCtrlHandler
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkMenuCtrlHandler()
		: m_pCtrl( NULL ),
		  m_id( VdkGetUniqueId() ),
		  m_paddingLeft( 0 ),
		  m_paddingTop( 0 ),
		  m_paddingRight( 0 ),
		  m_paddingBottom( 0 )
	{

	}

	/// \brief ����������
	virtual ~VdkMenuCtrlHandler() {}

	/// \brief ʵ����Ҫ���ӵ� VdkMenu �ϵĿؼ�
	virtual VdkControl* Implement(VdkWindow* menuWin, const wxPoint& pos) = 0;

	/// \brief ��ȡ�ؼ����
	VdkControl* GetCtrl() const { return m_pCtrl; }

	/// \brief ��ȡ�ؼ���˵�֮����ڱ߾�
	void GetPaddings(int* l, int* t, int* r, int* b) {
		if( l ) *l = m_paddingLeft;
		if( t ) *t = m_paddingTop;
		if( r ) *r = m_paddingRight;
		if( b ) *b = m_paddingBottom;
	}

	/// \brief ���ÿؼ����
	void Reset();

protected:

	/// \brief ����ؼ�״̬
	virtual void SaveState() {}

	/// \brief ��ԭ�ؼ�״̬
	virtual void RestoreState() {}

	//////////////////////////////////////////////////////////////////////////

	VdkControl* m_pCtrl; ///< �ؼ����
	VdkCtrlId m_id; ///< �ؼ� ID

	int m_paddingLeft; ///< �ؼ���˵���߽�֮��ľ���
	int m_paddingTop; ///< �ؼ���˵������ڱ߽�֮��ľ���
	int m_paddingRight; ///< �ؼ���˵��ұ߽�֮��ľ���
	int m_paddingBottom; ///< �ؼ���˵��ײ��ڱ߽�֮��ľ���
};

/// \brief ��� XRC ���Ƶ�һ�����װ���
class VdkMenuXrcCtrlWrapper : public VdkMenuCtrlHandler
{
public:

	/// \brief ���캯��
	VdkMenuXrcCtrlWrapper(const wxString& strXmlData)
		: m_strXmlData( strXmlData )
	{
		wxASSERT( !strXmlData.empty() );
	}

	/// \brief ʵ����Ҫ���ӵ� VdkMenu �ϵĿؼ�
	virtual VdkControl* Implement(VdkWindow* menuWin, const wxPoint& pos);

private:

	wxString m_strXmlData;
};

/// \brief ��Ͽؼ���¡���Ƶ�һ�����װ���
class VdkMenuCloneableCtrlWrapper : public VdkMenuCtrlHandler
{
public:

	/// \brief ���캯��
	VdkMenuCloneableCtrlWrapper(VdkWindow* parent, const wxString& strName)
		: m_Window( parent ), m_strName( strName )
	{
		wxASSERT( parent && !strName.IsEmpty() );
	}

	/// \brief ʵ����Ҫ���ӵ� VdkMenu �ϵĿؼ�
	virtual VdkControl* Implement(VdkWindow* menuWin, const wxPoint& pos);

private:

	VdkWindow* m_Window;
	wxString m_strName;
};

//////////////////////////////////////////////////////////////////////////

#include <wx/hashmap.h>
WX_DECLARE_HASH_MAP( int, wxString, wxIntegerHash, wxIntegerEqual, MenuGenItemMap );

/// \brief ��һ����ID-�ַ�������Ӧͼ�д������ײ˵�
struct MenuGenerator {

	/// \brief ���캯��
	MenuGenerator(int menuID, VdkMenu* p = NULL)
		: mid( menuID ), parent( p ) 
	{

	}

	/// \brief �������������Ϣ��Ϊ�´δ�����������
	void Clear();

	/// \brief ��ID-�ַ�������Ӧͼ
	MenuGenItemMap itemMap;
	int mid; ///< �˵� ID
	VdkMenu* parent; ///< ���˵�
};

/// \brief ��һ��������ϵ�����ַ����������ײ˵�
///
/// ʾ�����ַ�����CMCC WLAN;<SEP>;��������ѧͼ��ݡ������������˵���
/// ��һ�������߸����ķָ�����
/// \attention ��������֧�ֲ���˵���
VdkMenu* GenerateMenu(VdkWindow* parent, const MenuGenerator& menuTree);

/// \brief ��һ��������ϵ�����ַ����������ײ˵�
///
/// ʾ�����ַ�����CMCC WLAN;<SEP>;��������ѧͼ��ݡ������������˵���
/// ��һ�������߸����ķָ�����
/// \attention ��������֧�ֲ���˵���
VdkMenu* GenerateMenu(int mid, VdkWindow* parent, const wxString& strMenuTree);
