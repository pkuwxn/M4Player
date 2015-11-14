#pragma once
#include "VdkControl.h"
#include "VdkSlider.h"

/// \brief Ϊ VdkScrollBar ���ó�ʼ����Ϣ
class VdkScrollBarInitializer : public VdkSliderInitializer
{
public:

	/// \brief ���캯��
	VdkScrollBarInitializer(){}

	friend class VdkScrollBar;
};

//////////////////////////////////////////////////////////////////////////

/*!\brief �������� VDK ʵ��
 *
 * Ŀǰֻʵ���˴�ֱ��������
**/
class VdkScrollBar : public VdkControl
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkScrollBar(){ Init(); }

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);
	
	/// \brief �����ؼ�
	void Create(const VdkScrollBarInitializer& init_data);

	/// \brief ����λͼ����
	wxBitmap* GetBitmapArray(wxString strFileNames[3], int& nExpl);

	/////////////////////////////////////////////////////////////////////////
	
	/// \brief �õ���ǰ�����İٷֱ�
	double GetProgress();
	
	/// \brief �õ��ֱ��߶�
	int GetThumbHeight();

	/// \brief �ֱ���ǰ�Ƿ�ɼ�
	bool IsThumbOnShow() const;

	/// \brief ��ʾ/�����ֱ�
	void ShowThumb(bool bShow, wxDC* pDC);

	/// \brief �����ֱ��߶�
	void SetThumbHeight(int height, wxDC* pDC);

	/// \brief �ƶ���ָ���ٷֱ�
	void GoTo(double percentage, wxDC* pDC);

private:

	/// \brief ��ʼ���ؼ�
	void Init();
	
	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

	/////////////////////////////////////////////////////////////////////////

	int				m_nWidth;		///< ���������

	VdkSlider*		m_pSlider;		///< �������ܵ���Ҫʵ���� VdkSlider

	VdkButton*		m_pGoUp;		///< �����Ϲ�������ť
	VdkButton*		m_pGoDown;		///< �����¹�������ť

	/////////////////////////////////////////////////////////////////////////

	/// \brief �����¼��Ƿ���Դ�������ֻ����϶� VdkSlider
	///
	/// �ڹ������ϳ�����ס���Ҳ����ʹ�����������ƶ��������������һ�����⡣
	/// ���ǵ��������ֱ��ĸ߶��㹻С��ʹ֮����һ�ξʹ����ָ�����һ����Ծ����һ�࣬
	/// �����ҵ��㷨����ô��һ�����ڣ���ʹ�û���Ȼ�᲻�÷ſ���������
	/// ��������Ӧ�������û��ĳ��Է����෴�Ľ����������ֱ�����ԭ�ȵ�һ�ࡣ�����ѭ����
	/// �����Ҽ���һ���жϣ����ǰ�ס���ʱ���ֱ�ֻ����һ���̶��ķ���
	/// ��������˵�ġ����Է����ƶ����������ԣ�����������Ҳ��ʹ��������Է��򡱸ı䣬
	/// �����޷�������״̬�����㡱����Ϊ����һ�ι��־�ֻ��һ���¼����밴�°�����ͬ��
	/// �޷���ʱ�����¼�״̬��
	bool			m_bNotHoldOn;

	DECLARE_CLONEABLE_VOBJECT( VdkScrollBar )
};
