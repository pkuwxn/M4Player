#pragma once
#include "VdkControl.h"

class VdkButton;
class VdkWindow;
class VdkScrollBar;

/// \brief VdkSlider �������
enum VdkSliderStyle {

	/// \brief ��������λ�������ʼλ���
	VSS_USER					= ( VCS_USER + 5 ),
	/// \brief �ڻ������������Ƿ���Ҫ�����������
	VSS_CLICK_TO_GO				= 1 << ( VCS_USER + 0 ),
	/// \brief ��ֱ����������ʼ���� y ����ߴ����͹���������Ϊ�պ��෴
	VSS_VERTICAL_BOTTOM_UP		= 1 << ( VCS_USER + 1 ),
};

/// \brief VdkSlider ������ʱ״̬���Լ�
enum VdkSliderState {

	/// \brief �����������ʱ״̬�������ʼλ���
	VSST_USER					= ( VCS_USER + 5 ),
	/// \brief ������飬�ͷ����ʱ��Ҫ�����¼�
	VSST_DONT_FIRE_EVENT		= 1 << ( VCST_USER + 0 ),
	/// \brief �յ� NORMAL �¼�ʱ������Դ������㼴��ͷ�������϶�����
	/// �ͷ������յ��Ĳ�ͬ�汾�¼�
	VSST_LAST_CLICK_TO_GO		= 1 << ( VCST_USER + 1 ),
};

//////////////////////////////////////////////////////////////////////////

/// \brief Ϊ VdkSlider ���ó�ʼ����Ϣ
class VdkSliderInitializer : public 
	VdkCtrlInitializer< VdkSliderInitializer >
{
public:

	typedef VdkSliderInitializer Slider;

	/// \brief ���캯��
	VdkSliderInitializer();

	/// \brief ��������
	~VdkSliderInitializer();

	/// \brief �������õ�λͼ����
	///
	/// \param hold ��Ϊ�棬���ӹ�\a ba �Ŀ���Ȩ����������������ʱ�����ڴ�
	Slider& bitmapArray(wxBitmap* ba, bool hold)
	{
		BitmapArray = ba;
		HoldBitmapArrayPointer = hold;

		return *this;
	}

	/// \brief ���ù������Ĺ�������
	Slider& vertical(bool b) { Vertical = b; return *this; }

	/// \brief ���ù������ֱ�����λͼ�ķָ����
	Slider& thumbExpl(int nExpl) { Expl = nExpl; return *this; }

	/// \brief ���ù������ֱ�����λͼ��������ų���
	Slider& thumbTileLen(int len) { ThumbTileLen = len; return *this; }

	/// \brief ���ù������ֱ�����λͼ�����췽ʽ
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

/*!\brief �϶��ֱ�����������
**/
class VdkSlider : public VdkControl
{
public:

	/// \brief ���캯��
	VdkSlider() { Init(); }

	/// \brief ��һ�׶ι�������
	///
	/// ��һ���ļ��������л�ȡһ��λͼ���顣
	/// \param strFileNames ��������Ӧ�� 3 ��( 3 ���ļ���)
	static wxBitmap* GetBitmaps(wxString* strFileNames);

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ʵ�ʹ�������
	void Create(const VdkSliderInitializer& init_data);
	
	/////////////////////////////////////////////////////////////////////////

	/// \brief �ƶ���ָ���ٷֱ�
	void GoTo(double percentage, wxDC* pDC, bool fireCallback = false);

	/// \brief �õ� VdkSlider ��ǰ�İٷֱ�
	/// \param prt ��Ҫ��ȡ�ֱ�������ʱ����ָ����ָ��
	double GetProgress(wxRect* prt = NULL) const;

	/// \brief �ϴι��������ĸ������϶���
	enum SliderDirection {

		SD_NONE,
		SD_LESS,
		SD_MORE
	};

	/// \brief �ṩһ����õ����ĸ��������
	/// \param fLastDirection ��ȡ�ϴι����ķ���
	SliderDirection GetDirection(int y, SliderDirection& lastDirection) const;

	/// \brief �����϶�����
	void ResetDirection();

	/// \brief ���ò�������ʱ���õĻ�ˢ
	void SetBackground(const wxBrush& brush) { m_bgBrush = brush; }

	/// \brief �������ɹ����ķ����Ƿ�Ϊ��ֱ����
	bool IsVertical() const { return m_bVertical; }

	/// \brief ��ȡ�ֱ����
	/// \warning ���������޸��ֱ��ĸ������ԣ�
	const VdkButton* GetThumb() const { return m_pThumb; }

	/// \brief �����ֱ�״̬
	///
	/// ͨ��ĳЩ�����Եķ�ʽ�Ķ����ֱ��ĵ�ǰλ�õ���Ϣʱ��\n
	/// ���Ե��ñ��������Ķ��������ֳ�����
	void UpdateThumbState(wxDC* pDC);

private:

	/// \brief ��ʼ���ؼ�
	void Init();

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief detect width and height for dc.Blit
	void DectectWH(int& w, int& h);

	/// \brief ����\a rc ָ���ľ���
	/// \param rc �������Ϊ���ؼ������Ͻ�
	virtual void DoEraseBackground(wxDC& dc, const wxRect& rc);

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ťλͼ������ʵ��
	struct Bitmap {

		int work_side;
		int non_work_side;
	};

	const static int INVALID_MOUSE_ON_THUMB = -1;

	struct SliderImp {

		int curr; // �����Ϣ����������ֱ�Ӧ�ô��ڵ�λ��
		int ptr; // ����λ��

		/// \brief �����϶� VdkSlider ʱ�ϴ������ķ���
		///
		/// ��ǰ��ҪӦ�������� VdkScrollBar �ĳ�����ס����������ʵ����
		SliderDirection direction;
		double percent;

		Bitmap bmFull;
		Bitmap bmHandle;

		//////////////////////////////////////////////////////////////////////////

		/// \brief ���Ƹ�ֵ����
		SliderImp& operator = (const SliderImp& rhs);

		/// \brief ��ʼ��
		void Init();

		/// \brief �����϶�
		void HandleDrag(int mousePos);

		/// \brief �ƶ���ָ���ٷֱ�
		void MoveTo(double percent_);

		/// \brief �ͷ����ָ��
		void Release(){	ptr = INVALID_MOUSE_ON_THUMB; }

		/// \brief ���ָ���Ƿ�λ���ֱ��ϣ���ʱ�����϶�
		bool IsOk() const { return ptr != INVALID_MOUSE_ON_THUMB; }
	};

	SliderImp					m_imp;

	//////////////////////////////////////////////////////////////////////////

	bool						m_bVertical;
	VdkButton*					m_pThumb;

	// �ֱ�����С�߶ȣ���λͼԴ�ļ��Ĵ�С
	int							m_nMinHeight;
	// �ı��Сʱ�ֱ��ı�����ô�������������ƽ��
	VdkResizeableBitmapType		m_nThumbTileType;
	// �ı��Сʱ�ֱ��ı�������������������ĳ���֮��
	int							m_nThumbTile;

	//////////////////////////////////////////////////////////////////////////

	// ����������λͼ
	wxBitmap					m_bmBar;
	// ���米��λͼ����ͬ�ģ�ֱ��ʹ�ô�ɫ����������
	wxBrush						m_bgBrush; 
	// ������ʾ
	wxBitmap					m_bmFull;
	// ���ڻص��������϶�����С���40ms��
	wxMilliClock_t				m_dragTimeStamp;

	DECLARE_CLONEABLE_VOBJECT( VdkSlider )
};
