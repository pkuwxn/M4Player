#pragma once
#include "VdkControl.h"

/// \brief ԭ���ؼ��İ�װ�࣬ʹ֮�ܶ�ĳЩ������ͬ�Ĳ���������ȷ��Ӧ
template< class D >
class VdkNativeCtrl : public VdkControl
{
public:

	/// \brief ���캯��
	VdkNativeCtrl()	{ m_align |= ALIGN_NATIVE_CTRL;	}

	/// \brief ��ȡԭ�����
	const D* GetNativeHandle() const {
		return static_cast< const  D* >( this );
	}

	/// \brief ��ȡԭ�����(��const)
	D* GetNativeHandle() {
		return static_cast< D* >( this );
	}

	/// \brief ͳһ����ı��С�¼�
	virtual void UpdateSize() {
		static_cast< D* >( this )->wxWindow::
			SetSize( GetAbsoluteRect() );
	}

protected:

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice)
	{
		switch( notice.GetNotifyCode() )
		{
		case VCN_WINDOW_RESIZED:
		case VCN_SIZE_CHANGED:

			UpdateSize();
			break;

		case VCN_SHOW:
		case VCN_HIDE:

			static_cast< D* >( this )->wxWindow::Show( IsOnShow() );
			break;

		case VCN_THAW:
		case VCN_FREEZE:

			static_cast< D* >( this )->wxWindow::Show( !IsFreezed() );
			break;

		case VCN_ENABLED:
		case VCN_DISABLED:

			static_cast< D* >( this )->wxWindow::Enable( IsEnabled() );
			break;

		case VCN_WINDOW_ORIGIN_CHANGED:
			{
				wxRect rc( GetAbsoluteRect() );
				static_cast< D* >( this )->wxWindow::Move
					( rc.x - notice.GetWparam(), rc.y - notice.GetLparam() );

				break;
			}

		default:

			break;
		}

	}

};
