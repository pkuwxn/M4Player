#pragma once
#include "VdkControl.h"
#include "VdkBitmapArray.h"

/// \brief VdkButton ������Լ�
enum VdkButtonStyle {

	/// \brief ��������ʼ����λ
	VBS_USER					= VCS_USER + 5,
	/// \brief �ܷ�ı��С
	VBS_RESIZEABLE				= 1 << ( VCS_USER + 0 ),
	/// \brief ƽ�̻������죿
	VBS_RESIZE_TYPE_TILE		= 1 << ( VCS_USER + 1 ),
	/// \brief �Ƿ�ʹ�� wxRegion ��Ϊ�������ʶ��
	VBS_REGION					= 1 << ( VCS_USER + 2 ),
};

/// \brief VdkButton ����ʱ���Լ�
enum VdkButtonState {

	/// \brief ��������ʼ����λ
	VBST_USER					= VCST_USER + 5,
	/// \brief ������ʹ�� wxRegion ��Ϊ�������ʶ��
	///
	/// ���� wxRegion ʧ�ܡ�
	VBST_REGION_FAILED			= 1 << ( VCST_USER + 0 ),
};

//////////////////////////////////////////////////////////////////////////

/// \brief Ϊ VdkButton ���ó�ʼ����Ϣ
class VdkButtonInitializer : public 
	VdkCtrlInitializer< VdkButtonInitializer >
{
public:

	typedef VdkButtonInitializer Button;

	/// \brief ���캯��
	VdkButtonInitializer();

	/// \brief ���ð�ť���õı���λͼ
	/// \see fileName
	Button& bitmap(const wxBitmap& bm) { BkGnd = bm; return *this; }

	/// \brief ���ð�ť���õı���λͼ�ļ��ڴ����ϵ�λ��
	Button& fileName(const wxString& s) { FileName = s; return *this; }

	/// \brief ���ð�ť�ı����ı�
	Button& caption(const wxString& s) { Caption = s; return *this; }

	/// \brief ���ð�ť�ı����ı���������
	Button& textAlign(align_type a) { TextAlign = a; return *this; }

	/// \brief ���ð�ť�Ƿ���Ըı��С
	Button& resizeable(bool b) { Resizeable = b; return *this; }

	/// \brief ���ð�ť�ɸı��Сʱ����λͼ�����췽ʽ
	Button& tileType(VdkResizeableBitmapType type) {
		TileType = type; return *this;
	}

	/// \brief ���ð�ť�ɸı��Сʱ����λͼ�ĵ��ų���
	///
	/// �����м���һ����չ�ĳ�/��ȡ�
	Button& tileLen(unsigned len) { TileLen = len; return *this; }

	/// \brief ���ð�ťҪ�ָ�ɵķ���
	///
	/// Ҫ�󽫱��ְ�ť�ĸ���״̬��λͼ�ϲ���һ��λͼ�
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

/// \brief ��ť�������GUI���
///
/// һ����ť������״̬��ƽ�������������¡�ʧ���
/// �û������õ������һ�δӶ�������״̬֮���л���
class VdkButton : public VdkControl
{
public:

	/// \brief ���캯��
	VdkButton();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ��������
	///
	/// �����Զ���λͼ����Ҫ����Ϊ����ĳЩ׼��׼�ؼ����� VdkCheckBox ��\n
	/// ����Ҫ���û��Լ�ָ��λͼ��������Ҫ�� XRC �ļ��ж�̬������\n
	/// ������λͼֱ����ϵͳ���ɻ��ߴ� XPM �ļ����롣
	/// \attention ��Ҫ�� VdkControl �ڲ����ã��������� VdkControl \n
	/// �е� VdkButton ��
	VdkControl* Create(wxXmlNode* node,
					   const wxBitmap& bm,
					   int nExpl);

	/// \brief �ڴ�ִ����Ҫ��������
	void Create(const VdkButtonInitializer& init_data);

	/// \brief ������ʶ wxRegion
	///
	/// ����ؼ��������� wxRegion ����ʶ��������ñ�������������ʶ wxRegion��\n
	/// ������ VdkButton �� wxRect ����ʶ������
	void CreateRegion();

	/// \brief ��ʶ wxRegion �Ƿ���Ȼ����ȷ����
	bool IsRegionOk() const { return TestStyle( VBS_REGION ); }

	/// \brief ���� wxRegion �������Ƿ�ʧ����
	bool IsRegionCreationFailed() const {
		return !TestState( VBST_REGION_FAILED );
	}

	/// \brief �ж�\a mousePos �Ƿ�λ�ڰ�ť����������
	virtual bool HitTest(const wxPoint& mousePos) const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief ģ��һ����굥���¼�
	/// \attention ��Ҫ�ٵ���ԭ���Ļص�����!
	void EmuClick(wxDC& dc);

	/// \brief ��ť������״̬
	enum State {
		NORMAL,
		HOVERING,
		PUSHED,
		DISABLED,
	};

	/// \brief ����ť�ĵ�ǰ״̬����Ϊ\a nState
	virtual void Update(State nState, wxDC* pDC);

	/// \brief �õ���ť�ϴδ��ڵ�״̬
	State GetLastState() const { return GetButtonState(); }

	/// \brief ��ť�Ƿ�����ά���Ľ���״̬
	///
	/// ע�⣬�������״̬�� VDK ԭ�����û��Ʋ�ͬ��
	bool IsDisabled() const { return m_nState == DISABLED; }

	//////////////////////////////////////////////////////////////////////////

	/// \brief �õ� VdkButton �Ĵ�С
	///
	/// �Ա���ͼƬ�Ĵ�СΪ׼�����ܲ�ͬ��\link GetRect()\endlink�õ��Ĵ�С��
	void GetSize(int* w, int* h) const;

	/// \brief �õ� VdkButton ����С��С
	///
	/// ������ͼƬ�Ĵ�С��
	void GetMinSize(int* w, int* h) const;

	/// \brief ��ȡ��ť����
	wxString GetCaption() { return m_strCaption; }

	/// \brief ���ð�ť����
	void SetCaption(const wxString& strCaption, wxDC* pDC);

	/// \brief �ı� VdkButton �Ĵ�С
	/// \return true: �����ɹ���false: ����ʧ�ܡ�
	bool Resize(int w, int h);

	/// \brief ���ݰ�ť�ı����Զ�������С����ȣ�
	void Fit(wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡԭʼλͼ
	///
	/// ��������һ����Ҫ��;�Ǹ���һ����ť��
	const VdkBitmapArray& GetPrimaryBitmap() const;

protected:

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

	/// \brief ���ð�ť״̬
	void SetButtonState(State state) { m_nState = state; }

	/// \brief ��ȡ��ť״̬
	State GetButtonState() const { return m_nState; }

	/// \brief ���ݶ�������ı�λ��
	void CalcTextPos();

	//////////////////////////////////////////////////////////////////////////

	/// \brief �ı��Сʱƽ��ͼƬ�������������֮��
	int m_nTile;

	/// \brief �ı�����
	align_type m_TextAlign;

	/// \brief ����״̬�ı���ͼƬ
	VdkBitmapArray m_bmArray;
	/// \brief ԭʼ��λͼ�����顱
	/// 
	/// �����ڸı䰴ť�Ĵ�С����������ֱ��ĸ߶ȣ��ȡ�
	VdkBitmapArray m_bmPrimArray;

	/// \brief ��ť�ı����ܵĿհ�
	int m_nPaddingX, m_nPaddingY;

	/// \brief ��ť�ı�
	wxString m_strCaption;
	/// \brief �ı���ɫ
	wxColour m_TextColor;

	/// \brief ʹ�� wxRegion ��ʶ������ؼ�������������
	wxRegion m_Region;

private:

	// ��ť��ǰ״̬
	State m_nState;

	DECLARE_CLONEABLE_VOBJECT( VdkButton )
};
