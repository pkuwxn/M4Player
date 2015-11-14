#include "StdAfx.h"
#include "VdkToggleButton.h"

#include "VdkWindow.h"
#include "VdkSlider.h"
#include "VdkDC.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


IMPLEMENT_DYNAMIC_VOBJECT( VdkToggleButton );

//////////////////////////////////////////////////////////////////////////

VdkToggleButton::VdkToggleButton()
{

}

void VdkToggleButton::PushButton(bool bPushed, wxDC* pDC)
{
	SetAddinState( VTBST_TO_BE_PUSHED );

	if( !bPushed )
	{
		RemoveState( VTBST_TO_BE_PUSHED | 
					 VTBST_PUSHED | 
					 VTBST_TOGGLED );

		if( pDC )
		{
			State state = NORMAL;
			if( ContainsPointerGlobally() )
				state = HOVERING;

			Update( state, pDC );
		}
	}
}

void VdkToggleButton::Toggle(bool bToggled, wxDC* pDC, bool bFireCallback)
{
	if( bToggled == IsToggled() )
		return;
	
	if( bToggled )
	{
		SetAddinState( VTBST_TOGGLED );
		SetButtonState( PUSHED );

		if( bFireCallback )
			FireEvent( pDC, NULL );
	}
	else
	{
		RemoveState( VTBST_TOGGLED );
		SetButtonState( NORMAL );
	}

	if( pDC )
		Draw( *pDC );
}

void VdkToggleButton::DoHandleMouseEvent(VdkMouseEvent& e)
{
	// ��ʱ������ռ��ť
	if( IsPushed() )
		return;

	bool skip = false;
	int evtCode = e.evtCode;
	switch( evtCode )
	{
		case NORMAL:
		case HOVERING:
			// ToggleButton �������ʱ��Ӧ�� Hovering ��״̬
			if( IsToggled() )
				return;

			break;

		case LEFT_DOWN:
		case DLEFT_DOWN:

			SetAddinState( VTBST_LAST_LEFT_DOWN );

			break;

		case LEFT_UP:
		{
			if( TestState( VTBST_LAST_LEFT_DOWN ) )
			{
				RemoveState( VTBST_LAST_LEFT_DOWN );

				if( !IsOwnerControl() )
					ReverseState( VTBST_TOGGLED ); // !!!ȡ��!!!����״̬

				// ToggleButton �������ʱ��Ӧ�� Hovering ��״̬
				// ͬ�� PushedButton
				if( IsToggled() )
				{
					if( IsToBePushed() )
					{
						SetAddinState( VTBST_PUSHED );
						RemoveState( VTBST_TO_BE_PUSHED );
					}

					// û������ʱ�¼��� VdkButton ����
					if( IsReadyForEvent() )
						FireEvent( &e.dc, NULL );

					return;
				}
			}
			// ������Ŀؼ����϶�ʱ���� VdkToggledButton �Ϸſ�ָ�룬
			// ��ʱ��Ҫ�ı� VdkToggledButton ���κ�״̬
			else if( IsToggled() )
				return;

			evtCode = HOVERING;
			break;
		}

		default:
			
			skip = true;
			break;
	}

	VdkButton::DoHandleMouseEvent( e );

	// �������߶�Ҫ�������¼�
	if( !skip )
		e.Skip( false );
}
