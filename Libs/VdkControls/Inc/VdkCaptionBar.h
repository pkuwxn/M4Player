#pragma once
#include "VdkControl.h"

/*!\brief һ�������࣬���Լ򻯱�������ؿؼ��Ķ��塢��������
 * 
 * ��Ϊ�൱��� VdkWindow ����Ҫһ������������һ��������ͨ����һ��ͼ�ꡢ\n
 * һ�������ı��� VdkLabel �����������ڿ��ư�ť����С������󻯡��رգ��ȵȣ���\n
 * �����Ǿ��ǻ��������Ŀ��Ƕ����衣\n
 * ����
 \code
    <object class="VdkCaptionBar" name="captionbar">
        <padding-right>5</padding-right>
        <close>close_stats.bmp</close>
        <min>min.bmp</min>
        <object class="VdkLabel" name="caption">
            VdkControls All-in-One
            <rect>4, 9, 50, 9</rect>
            <bold>true</bold>
            <font-name>Arial</font-name>
            <font-color>#FFFFFF</font-color>
        </object>
    </object>
 \endcode
 * ע�����ж���Ŀؼ����� VdkCaptionBar ���ӿؼ���������������� VdkLabel ��\n
 * ���е���Щ���嶼����������ɾ��\n
 * �����ԣ�����ֻ������ XRC �����¡�
**/
class VdkCaptionBar : public VdkControl
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkCaptionBar() {}
	
	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief �رմ���
	static void ClosePanel(VdkVObjEvent&);

	/// \brief ��С������
	static void MaximizePanel(VdkVObjEvent&);

	/// \brief ��С������
	static void IconizePanel(VdkVObjEvent&);

private:

	/// \brief ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	DECLARE_DYNAMIC_VOBJECT
};
