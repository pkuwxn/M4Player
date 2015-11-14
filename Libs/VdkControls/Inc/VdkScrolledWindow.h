#pragma once

class VdkWindow;
class VdkScrollBar;
#include "VdkControl.h"

/*!\brief �������ڵ�ʵ�ֻ���
 * 
 * �μ�����������ṩ���¼���֪ͨ��Ϣ������¼���
 * 
 * ֪ͨ��Ϣ��
 * VCN_CREATE
 * VCN_SIZE_CHANGED
 * VCN_WINDOW_RESIZED
 * 
 * ����¼���
 * SCROLLING_UP
 * SCROLLING_DOWN
 * WHEEL_UP
 * WHEEL_DOWN
**/
class VdkScrolled
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkScrolled();

	/// \brief ��\a o ��¡
	void Clone(VdkScrolled* o, VdkScrollBar* scrollbar);

	/// \brief ˢ�¹�������״̬
	///
	/// ��Ҫ�ǹ�������ʱ����Ҫ���¹�����λ�á�
	void RefreshState(wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief �������⻭���Ĵ�С
	void SetVirtualSize(int x, int y, wxDC* pDC = NULL);

	/// \brief һ���ݶȵĴ�С
	///
	/// �������һ�ι������⻭�����������ݶȣ���ô���⻭�����Ƶ�������:
	/// 3 * ��Ӧ��������ݶ�
	void SetScrollRate(int xstep, int ystep);

	/// \brief ��ȡ�����������������ݶȵĴ�С
	/// \see SetScrollRate
	void GetScrollRate(int* xstep, int* ystep) const;

	/// \brief ÿ�������ֻ���ʱ����������
	void SetLinesUpDown(int lines) { m_linesUpDown = lines; }

	/// \brief ÿ�������ֻ���ʱ����������
	int GetLinesUpDown() const { return m_linesUpDown; }

	/// \brief ������ʼ�к�
	void SetViewStart(int x, int y, wxDC* pDC);

	/// \brief ��ȡ��ʼ�к�
	///
	/// �к� * yStep = ������ʼ���ꡣ
	/// \see GetViewStartCoord
	virtual void GetViewStart(int* x, int* y) const;

	/// \brief ��ȡ�����ܵ���ʼ���
	virtual void GetMaxViewStart(int* x, int* y) const;

	/// \brief ��ȡ���⻭���Ĵ�С
	void GetVirtualSize(int* w, int* h) const;

	/// \brief ��ȡ��ʼ����
	/// \see GetViewStart
	void GetViewStartCoord(int* x, int* y) const;

	/// \brief ��ȡ�����ʼ����
	/// \see GetMaxViewStart
	void GetMaxViewStartCoord(int* x, int* y) const;

	/// \brief ��ȡ��������ֻ�����
	VdkScrollBar* GetScrollBar() const { return m_pScrollBar; }

	/// \brief ������ָ���ٷֱ�
	/// \param progress 0 <= \a progress <= 1
	void GoTo(double progress, wxDC* pDC);

	/// \brief ��ȡ��ǰ����λ�õİٷֱ�
	virtual double GetProgress() const;

public:

	//////////////////////////////////////////////////////////////////////////
	// ���������������ڲ����ã������㲻֪���Լ��ڸ�ʲô����Ҫ�������ǣ�

	/// \brief ��ͼǰ��׼������
	///
	/// ������ȷ���� DC ����ʼ�㡣
	void PrepareDC(wxDC& dc);

	/// \brief ��ͼ���������ԭ����
	void RestoreDC(wxDC& dc);

protected:

	//////////////////////////////////////////////////////////////////////////
	// ���������

	/// \brief ���¹�����״̬
	void CalcScrollBarSize(wxDC* pDC);

	/// \brief �������ķ�����Զ�����
	struct ScrollBarStyle {

		wxString* strFileNames; ///< ���������õ�λͼ�ļ�������
		int nExpl; ///< �������ֱ�����λͼ�ķָ����
		VdkResizeableBitmapType nThumbTileType; ///< ���ù������ֱ�����λͼ�����췽ʽ
		int nThumbTile; ///< �������ֱ�����λͼ��������ų���
	};

	/// \brief ���Ĺ�����Ƥ��
	virtual void SetScrollBarStyle(ScrollBarStyle& style);

private:

	/// \brief �������������ʼ����
	void CalcMaxViewStart();

protected:

	//////////////////////////////////////////////////////////////////////////
	// �ɴμ�������ַ����¼�

	/// \brief �ڴ˳�ʼ�� DC ��׼������
	///
	/// ������ʵ��\link OnDraw \endlink ���ɡ�
	void Paint(wxDC& dc);

	/// \brief ��������¼�
	/// \warning VdkMouseEvent �а��������λ��Ӧ���Թ����������Ͻ�Ϊ��ʼ�㡣
	void MouseEvent(VdkMouseEvent& e);

	/// \brief ������̰����¼�
	void KeyEvent(VdkKeyEvent& e);

	/// \brief ���ա�����֪ͨ��Ϣ
	void Notify(const VdkNotify& notice);

private:

	//////////////////////////////////////////////////////////////////////////
	// �μ���������븲д

	/// \brief ��ȡ�������ĸ�����
	virtual VdkWindow* ParentWindow() const = 0;

	/// \brief �������ĸ��ؼ�(������)
	virtual VdkControl* ScrollBarParent() const = 0;

	/// \brief �����������λ��
	virtual wxRect ScrollBarRect() const = 0;

	/// \brief ��ȡ��������Ӧ�� DC
	virtual wxDC* GetScrollBarDC(wxDC* dc) const = 0;

	/// \brief ���ٴ�\link GetScrollBarDC \endlink ��õ� DC
	/// \see GetScrollBarDC
	virtual void DestroyScrollBarDC(wxDC* dc) const = 0;

	/// \brief ��ȡ�������ڵ�����
	virtual const wxString& Name() const = 0;

	/// \brief ��ȡ/���ù������ڵķ�����Լ�
	virtual long Style() const = 0;

	/// \brief ��ȡ/���ù�����������ڸ��ؼ������λ��
	/// 
	/// ����û�и��ؼ����򷵻�ֵ��\link AbsoluteRect() \endlink ��ͬ��
	/// \see AbsoluteRect
	virtual wxRect& RelativeRect() = 0;

	/// \brief ��ȡ/���ù�����������ڶ��� VdkWindow �ľ���λ��
	virtual wxRect AbsoluteRect() const = 0;

	/// \brief �ڴ˸���/���ƹ�������
	virtual void UpdateUI(wxDC& dc) = 0;

	/// \brief ����Ƿ����� XRC �ļ��ж��岢��������Ҫ�Ĺ�����
	///
	/// ���ڹ������ڴ���ȱʡ������ǰ��
	virtual VdkScrollBar* ScrollBarExists() = 0;

	/// \brief ��Ӧ��������ʾ/�����¼�֪ͨ
	virtual void OnScrollBarShowHide(const VdkScrollBar* sb) = 0;

	//////////////////////////////////////////////////////////////////////////
	// �μ���������Ը�д

	/// \brief ��ͼǰ��׼������
	///
	/// ������ȷ���� DC ����ʼ�㡣
	/// \warning ��ʱ DC ����ʼ��Ϊ (0, 0) ��
	virtual void DoPrepareDC(wxDC& dc) {}

	/// \brief ��ͼ���������ԭ����
	virtual void DoRestoreDC(wxDC& dc) {}

	//////////////////////////////////////////////////////////////////////////
	// �δμ���������Ը�д

	/// \brief ˢ�¹�������
	virtual void DoRefreshState(wxDC& dc) {}

protected: // ������Щ�¼�����������Ҫʵ����������������

	/// \brief �ڴ˻�ͼ
	virtual void OnDraw(wxDC& dc) {}

	/// \brief ��Ӧ����¼�
	virtual void OnMouseEvent(VdkMouseEvent& e) {}

	/// \brief ��Ӧ�����¼�
	virtual void OnKeyEvent(VdkKeyEvent& e) {}

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void OnNotify(const VdkNotify& notice) {}

private:

	int					m_xStep;
	int					m_yStep;

	int					m_xVirtualSize;
	int					m_yVirtualSize;

	int					m_xViewStart;
	int					m_yViewStart;
	int					m_yViewStartMax;

	int					m_linesUpDown;		///< ÿ�������ֻ���ʱ����������

	//////////////////////////////////////////////////////////////////////////

	VdkScrollBar*		m_pScrollBar;
};

//////////////////////////////////////////////////////////////////////////

enum VdkScrolledWindowStyle {

	/// \brief �����������Ե���ʼ ID
	VSWS_USER					= ( VCS_USER + 10 ),
	/// \brief ������������
	VSWS_NO_SCROLLBAR			= ( 1 << ( VCS_USER + 0 ) ),
	/// \brief ʼ����ʾ������������һ��ʱ���ù��������������أ�
	VSWS_ALWAYS_SHOW_SCROLLBAR	= ( 1 << ( VCS_USER + 1 ) ),
};

/// \brief VdkScrolledWindow ������ʱ���Լ�λ����
enum VdkScrolledWindowState {

	/// \brief �����������Ե���ʼ ID
	VSWST_USER				= ( VCST_USER + 10 ),
};

/*!\brief �������ڵ� VDK ʵ��
 *
 * ��ǰ�����������ڵľ޴����壬��ɵ������ʹ�÷�ԭ������������ģ������Ч����
 * ��ʵ֤��������Ŭ��̫û���壡ƽ���޹ʵؼ���������˻��ڹ������ڵĿؼ���ʵ�ָ����ԡ�\n
 * Ѫ��Ľ�ѵ��
 *
 * \attention �������� m_physicRect���������޸��� m_Rect ������£�
**/
class VdkScrolledWindow : public VdkControl, public VdkScrolled
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkScrolledWindow(long style = 0);

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ��\a o ��¡
	void Clone(VdkScrolledWindow* o);

	/// \brief ��ȡ������������������
	///
	/// m_Rect Ĭ�ϲ�������������������
	wxRect GetLogicalRect() const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���ñ߽���ɫ
	void SetBorderColour(const wxColour& color){ m_borderColor = color; }

	/// \brief ��ȡ�߽���ɫ
	wxColour GetBorderColour() const { return m_borderColor; }

private:

	//////////////////////////////////////////////////////////////////////////
	// �μ���������븲д

	/// \brief ��ȡ�������ĸ�����
	virtual VdkWindow* ParentWindow() const { return m_Window; }

	/// \brief �������ĸ��ؼ�(������)
	virtual VdkControl* ScrollBarParent() const {
		return (VdkControl *)( this );
	}

	/// \brief �����������λ��
	virtual wxRect ScrollBarRect() const {
		return wxRect( 0, 0, m_Rect.width, m_Rect.height );
	}

	/// \brief ��ȡ��������Ӧ�� DC
	virtual wxDC* GetScrollBarDC(wxDC* dc) const { return dc; }

	/// \brief ���ٴ�\link GetScrollBarDC \endlink ��õ� DC
	virtual void DestroyScrollBarDC(wxDC* dc) const {}

	/// \brief ��ȡ�������ڵ�����
	virtual const wxString& Name() const { return m_strName; }

	/// \brief ��ȡ/���ù������ڵķ�����Լ�
	virtual long Style() const { return GetStyle(); }

protected:

	/// \brief ��ȡ/���ù�����������ڸ��ؼ������λ��
	/// 
	/// ����û�и��ؼ����򷵻�ֵ��\link AbsoluteRect() \endlink ��ͬ��
	/// \see AbsoluteRect
	virtual wxRect& RelativeRect() { return m_Rect; }

	/// \brief ��ȡ/���ù�����������ڶ��� VdkWindow �ľ���λ��
	virtual wxRect AbsoluteRect() const { return GetAbsoluteRect(); }

private:

	/// \brief XRC ��̬����
	virtual void OnXrcCreate(wxXmlNode* node) {}

	/// \brief �ڴ˸���/���ƹ�������
	virtual void UpdateUI(wxDC& dc) { Draw( dc ); }

	/// \brief ����Ƿ����� XRC �ļ��ж��岢��������Ҫ�Ĺ�����
	///
	/// ���ڹ������ڴ���ȱʡ������ǰ��
	virtual VdkScrollBar* ScrollBarExists();

	/// \brief ��Ӧ��������ʾ/�����¼�֪ͨ
	virtual void OnScrollBarShowHide(const VdkScrollBar* sb);

	//////////////////////////////////////////////////////////////////////////
	// �μ���������Ը�д

	/// \brief ��ͼǰ��׼������
	///
	/// ������ȷ���� DC ����ʼ�㡣
	/// \warning ��ʱ DC ����ʼ��Ϊ (0, 0) ��
	virtual void DoPrepareDC(wxDC& dc);

	/// \brief ��ͼ���������ԭ����
	virtual void DoRestoreDC(wxDC& dc);

	/// \brief ˢ�¹�������
	virtual void DoRefreshState(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////
	// �̳��� VdkControl

	/// \brief �ڴ˻���ȫ������
	virtual void DoDraw(wxDC& dc);

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief ������̰����¼�
	virtual void DoHandleKeyEvent(VdkKeyEvent& e) { KeyEvent( e ); }

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice) {
		Notify( notice );
	}

private:

	wxColour m_borderColor;
};
