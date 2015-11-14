#pragma once

class VdkWindow;

/// \brief ȫ���ػ�ʹ��˫����
///
/// �Ҿ�����һ�� VdkControl �Ļص������������ʹ�� wxClientDC ������һ���ܴ�ı׶�:
/// �����ػ�ʱ��Ϊڸ������˸������д������ࡣ�Զ�����ʵ��˫���壬�����Լ�������
/// һ�� wxMemoryDC �ˡ�\n
/// ʹ�÷���: �����ڶ��ϴ��������ָ�룬���������ص��������⡣
/// #define __VDK_NOT_USE_DOUBLE_BUFFERING__
#ifndef __VDK_NOT_USE_DOUBLE_BUFFERING__
class VdkDC : public wxMemoryDC
{
public:

	/// \brief ���캯��
	/// \param pdc �ṩһ��ԭʼ�豸��������ΪĿ���豸������
	VdkDC(VdkWindow* Window, const wxRect& rc, wxDC* pdc = NULL);

	/// \brief ���캯��
	///
	/// ��ʱĬ�ϵļ�������Ϊ�������ڡ�
	VdkDC(VdkWindow* Window);

	/// \brief ��������
	~VdkDC();

private:

	// ʵ�ʹ�������
	void Construct(VdkWindow* Window, const wxRect& rc, wxDC* pdc);

	// ����ǰ���������ύ��ʵ�� DC ��
	void Flush(wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	VdkWindow*					m_Window;
	wxRect						m_Rect;

	unsigned long				m_timeStamp;
	static unsigned long		ms_timeStamp;
	static VdkWindow*			ms_window;

	wxDC*						m_pdc;

	wxDECLARE_NO_COPY_CLASS( VdkDC );
};
#else
class VdkDC : public wxClientDC
{
public:

	/// \brief ���캯��
	/// \param pdc �ṩһ��ԭʼ�豸��������ΪĿ���豸������
	VdkDC(VdkWindow* Window, const wxRect& rc, wxDC* pdc = NULL);

	/// \brief ���캯��
	///
	/// ��ʱĬ�ϵļ�������Ϊ�������ڡ�
	VdkDC(VdkWindow* Window);

	wxDECLARE_NO_COPY_CLASS( VdkDC );
};
#endif // __VDK_NOT_USE_DOUBLE_BUFFERING__

/// \brief ����ȷ��ʹ�� wxDC* pDC ָ�봫������ DC ָ��һ������
///
/// �е���ְɣ���Ҫ��Ȼ����һ��ָ�����Ϊ����ʱ����Բ��ش���һ�� wxClientDC ,
/// ��Ϊʲô��Ҫȷ������Ч�أ�\n
/// ������Ҫ���� GUI ʱ�������� bRedraw = true ��������Ҫȷ��ָ�����Ч�Ե�
class VdkValidDC
{
public:

	/// \brief ���캯��
	VdkValidDC(wxDC* pDC, VdkWindow* window, wxRect* prc);
	/// \brief ��������
	~VdkValidDC();

	/// \brief �Զ�ת���� wxDC
	operator wxDC*() { return m_pDC; }

	/// \brief ��ȡ wxDC ����
	wxDC& operator*() { return *m_pDC; }

	/// \brief ��ȡ wxDC ���
	wxDC* get() { return m_pDC; }

private:

	wxDC* m_pDC;
	bool m_bOwnDC;
};

/// \brief �豸�����ĵ��豸��ʼ�����Զ��ָ���
class VdkDcDeviceOriginSaver
{
public:

	/// \brief ���캯��
	VdkDcDeviceOriginSaver(wxDC& dc);
	/// \brief ��������
	~VdkDcDeviceOriginSaver();

	/// \brief �ֶ��ָ��豸�����ĵ��豸��ʼ����
	void Destroy();

private:

	wxDC&						m_dc;
	int							m_x, m_y;
};

/// \brief DC �ü������Զ��ָ���
class VdkDcClippingRegionDestroyer
{
public:

	/// \brief ���캯��
	/// \param dc Ҫ�������豸������
	/// \param region Ҫ�ü��ɵ�����
	VdkDcClippingRegionDestroyer(wxDC& dc, const wxRect& region);
	/// \brief ��������
	~VdkDcClippingRegionDestroyer();

private:

	VdkDcDeviceOriginSaver		m_saver;
	wxRect						m_clpbox0;
	wxDC&						m_dc;
};

/// \brief �� VdkDC �ύ���ĵĵ����һ�������޸���������
class VdkDcPostListener
{
public:

	/// \brief ִ�л�ͼ����
	virtual void OnPost(wxDC& dc) = 0;

	/// \brief ����������
	void RemoveFromWindow(VdkWindow* win);
};

/// \brief ��\a DC ���豸��ʼ����Ļ���������\a dX ��\a dY
void VdkDcDeviceOriginAdder(wxDC& dc, int dX, int dY);
