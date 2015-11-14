/***************************************************************
* Name:      VdkScrolledPanel.h
* Purpose:   ������壨����Ϊ�����ؼ����ؼ��Ĺ������������ࣩ����
* Author:    vanxining (vanxining@139.com)
* Created:   2011-01-04
* Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkNativeCtrl.h"
#include "VdkWindowImpl.h"
#include "VdkScrolledWindow.h"
#include "VdkScrollBar.h"

/// \brief ��������ʵ�ֻ���
///
/// �����ѹ�������ԼӶ���һ��̳С�
class VdkScrolledPanelBase : public VdkWindowImpl< wxPanel >
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkScrolledPanelBase() {}

	/// \brief ���캯��
	VdkScrolledPanelBase(wxWindow* parent, long style = 0)
		 : VdkWindowImpl< wxPanel >( style )
	{
		Create( parent, style );
	}

	/// \brief �е�ʵ�ʹ�������
	void Create(wxWindow* parent, long style = 0);

private:

	virtual void DoResize(int x, int y, int width, int height, 
						  int sizeFlags = wxSIZE_AUTO) {
		SetSize( x, y, width, height, sizeFlags );
	}

	virtual void DoMinimize() {}
};

//////////////////////////////////////////////////////////////////////////

/// \brief ����Ϊһ�� VdkWindow �ӿؼ��Ĺ������
class VdkScrolledPanel : public VdkScrolledPanelBase, 
						 public VdkNativeCtrl< VdkScrolledPanel >,
						 public VdkScrolled
{
public:

	/// \brief ���캯��
	VdkScrolledPanel();

	/// \brief ��������
	~VdkScrolledPanel();

	/// \brief XRC ��̬����
	///
	/// һ�� VdkControl �����������Ҫʵ�ֶ�̬��������Ӧ��ʵ�����������
	/// \todo ���ؼ�
	virtual void Create(wxXmlNode* node);

	/// \brief ���ݿؼ������������⻭���Ĵ�С
	void CalcVirtualSize();

	/// \brief �����ѻ����˫�����ͼ�����ĵ�ĳЩ��ʼ����
	virtual void PrepareCachedDC();

	/// \brief �ӿؼ��Ƿ��ڿ���������
	bool IsChildVisible(VdkControl* chd) const;

protected:

	//////////////////////////////////////////////////////////////////////////
	// �μ���������븲д

	// ��ȡ�������ĸ�����
	virtual VdkWindow* ParentWindow() const {
		return dynamic_cast< VdkWindow * >( wxWindow::GetParent() );
	}

	// �������ĸ��ؼ�(������)
	virtual VdkControl* ScrollBarParent() const { return (VdkControl *) this; }

	// �����������λ��
	virtual wxRect ScrollBarRect() const { return Rect00(); }

	// ��ȡ��������Ӧ�� DC
	virtual wxDC* GetScrollBarDC(wxDC* dc) const ;

	// ���ٴ�\link GetScrollBarDC \endlink ��õ� DC
	virtual void DestroyScrollBarDC(wxDC* dc) const;/* { delete dc; }*/

	// ��ȡ�������ڵ�����
	virtual const wxString& Name() const { return m_strName; }

	// ��ȡ/���ù������ڵķ�����Լ�
	virtual long Style() const { return VdkWindow::GetStyle(); }

	// ��ȡ/���ù�����������ڸ��ؼ������λ��
	/// 
	/// ����û�и��ؼ����򷵻�ֵ��\link AbsoluteRect() \endlink ��ͬ��
	/// \see AbsoluteRect
	virtual wxRect& RelativeRect() { return m_Rect; }

	// ��ȡ/���ù�����������ڶ��� VdkWindow �ľ���λ��
	virtual wxRect AbsoluteRect() const { return Rect00(); }

	// �ڴ˸���/���ƹ�������
	virtual void UpdateUI(wxDC& dc);

	// ����Ƿ����� XRC �ļ��ж��岢��������Ҫ�Ĺ�����
	///
	/// ���ڹ������ڴ���ȱʡ������ǰ��
	virtual VdkScrollBar* ScrollBarExists() { return NULL; }

	// ��Ӧ��������ʾ/�����¼�֪ͨ
	virtual void OnScrollBarShowHide(const VdkScrollBar* sb);

	//////////////////////////////////////////////////////////////////////////
	// �μ���������Ը�д

	// ������Ҫ��ʾ/���ص��ӿؼ���
	virtual void DoRefreshState(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////
	// �̳��� VdkControl

	// ���Ʊ߿�
	virtual void DoDraw(wxDC& dc);

	// �����϶� VdkSlider ������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	// ���տؼ���֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

	//////////////////////////////////////////////////////////////////////////
	// �̳��� VdkScrolled

	// �����ֵ��¼����ᷢ�͵��˵��ϵĹ������
	// ͬʱ�������ش���ָ���ƶ��¼�
	virtual void OnMouseEvent(VdkMouseEvent& e);

	//////////////////////////////////////////////////////////////////////////
	// �̳��� VdkWindow

	// �� VdkWindow ��������¼�֮ǰ������ʼ����
	virtual bool FilterEventBefore(wxMouseEvent& evt, int evtCode);

	// ������δ�������ؼ�����Ĺ����¼�
	virtual bool FilterEventAfter(wxMouseEvent& evt, int evtCode);

	// �������أ�**�Ǽ̳�**
	bool FilterEventAfter(VdkMouseEvent& evt);

	// ����ȫ�ػ��¼����������
	//
	// VdkWindow ���ڹ��������Ǻ��޸���ģ�������֪���Լ���
	// EraseBackground �����в����Ĳ�����һ��ʼ��һ����
	virtual void DoPaint(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	// ��һ�����ܻᵼ�»�ȡ������ר�� DC ʱ����ֲ�������
	// ���������ڴ��ڵ� VdkDC
	// �����¼����Ƿ��͵���ǰ��˵����ڵģ�������˵�ʵ�崰�崫���Ĺ�����
	// ������һ�Σ���������ػ�ʱ�ֻ�����һ��
	mutable bool m_bUsingCachedSbDC;

	DECLARE_DYNAMIC_VOBJECT
};
