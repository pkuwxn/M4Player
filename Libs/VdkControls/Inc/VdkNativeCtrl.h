#pragma once
#include "VdkControl.h"

/// \brief 原生控件的包装类，使之能对某些意义相同的操作做出正确响应
template< class D >
class VdkNativeCtrl : public VdkControl
{
public:

	/// \brief 构造函数
	VdkNativeCtrl()	{ m_align |= ALIGN_NATIVE_CTRL;	}

	/// \brief 获取原生句柄
	const D* GetNativeHandle() const {
		return static_cast< const  D* >( this );
	}

	/// \brief 获取原生句柄(非const)
	D* GetNativeHandle() {
		return static_cast< D* >( this );
	}

	/// \brief 统一处理改变大小事件
	virtual void UpdateSize() {
		static_cast< D* >( this )->wxWindow::
			SetSize( GetAbsoluteRect() );
	}

protected:

	/// \brief 接收、处理通知信息
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
