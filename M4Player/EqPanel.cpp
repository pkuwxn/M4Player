/***************************************************************
 * Name:      EqPanel.cpp
 * Purpose:   均衡器窗口
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-24
 **************************************************************/
#include "StdAfx.h"
#include "EqPanel.h"

#include "OOPApp.h"
#include "VdkSlider.h"
#include "VdkToggleButton.h"

#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

extern void AssignToolTip(VdkControl* pCtrl, const wxString& strToolTip);
extern OOPlayerApp* g_app;

EqPanel::EqPanel(wxWindow* parent)
	: OOPWindow( parent, L"0F146699-FA80-4120-85CD-057E09FED8C8", L"EqPanel" ),
	  m_balance( NULL ), m_surround( NULL ), m_preamp( NULL )
{
	memset( m_eqfactors, 0, sizeof( m_eqfactors ) );

	SetSkin();
}

void EqPanel::DoSetSkin()
{
	const wxChar* xrcFileName = L"equalizer_window.xml";
	FromXrc( xrcFileName );
	BindCloseBtnEvent();

	m_enableAll = (VdkToggleButton *) FindCtrl( L"enabled" );
	m_profile = (VdkButton *) FindCtrl( L"profile" );
	m_reset = (VdkButton *) FindCtrl( L"reset" );

	AssignToolTip( FindCtrl( L"close" ), L"关闭" );
	AssignToolTip( m_enableAll, L"均衡开关" );
	AssignToolTip( m_profile, L"配置文件" );
	AssignToolTip( m_reset, L"重设均衡器" );

	m_balance = (VdkSlider *) FindCtrl( L"balance" );
	m_surround = (VdkSlider *) FindCtrl( L"surround" );
	m_preamp = (VdkSlider *) FindCtrl( L"preamp" );

	m_balance->GoTo( 0.5, NULL, false );
	m_surround->GoTo( 0.5, NULL, false );
	m_preamp->GoTo( 0.5, NULL, false );
	m_preamp->SetAddinStyle( VSS_VERTICAL_BOTTOM_UP );

	AssignToolTip( m_balance, L"平衡" );
	AssignToolTip( m_surround, L"环绕" );
	AssignToolTip( m_preamp, wxEmptyString );

	//=========================================================
	// 创建其它拖动条

	wxXmlDocument docSkin( VdkControl::GetFilePath( xrcFileName ) );
	wxXmlNode* nodeWindowRoot = docSkin.GetRoot()->GetChildren();
	wxXmlNode* nodeInterval = FindChildNode( nodeWindowRoot, L"eq_interval" );
	if( !nodeInterval )
		return;

	size_t interval = XmlGetContentOfNum( nodeInterval, 5 );

	wxXmlNode* nodeBase = FindEqfactor( nodeWindowRoot );
	if( !nodeBase )
		return;

	wxString SliderFileNames[] = {
		XmlGetChildContent( nodeBase, L"bar_image" ),
		XmlGetChildContent( nodeBase, L"fill_image" ),
		XmlGetChildContent( nodeBase, L"thumb_image" ),
	};

	for( int i = 0; i < 3 ; i++ )
	{
		if( !SliderFileNames[i].Trim( true ).Trim( false ).IsEmpty() )
		{
			SliderFileNames[i] = VdkUtil::GetFilePath( SliderFileNames[i] );
		}
	}

	wxBitmap* bmArray = VdkSlider::GetBitmaps( SliderFileNames );

	//=========================================================

	m_eqfactors[0] = (VdkSlider *) FindCtrl( L"eqfactor" );
	m_eqfactors[0]->GoTo( 0.5, NULL, false );
	m_eqfactors[0]->SetAddinStyle( VSS_VERTICAL_BOTTOM_UP );
	AssignToolTip( m_eqfactors[0], wxEmptyString );

	wxRect rcBase( m_eqfactors[0]->GetRect() );

	for( int i = 1; i < 10; i++ )
	{
		rcBase.x = rcBase.x + rcBase.width + interval;

		VdkSlider* bar = new VdkSlider;
		bar->Create( VdkSliderInitializer().
					 bitmapArray( bmArray, false ).
					 vertical( m_eqfactors[0]->IsVertical() ).
					 thumbExpl( 4 ).
					 parent( NULL ).
					 window( this ).
					 name( wxString::Format( L"eqfactor%d", i ) ).
					 rect( rcBase ).
					 addToWindow( true ) );

		AssignToolTip( bar, wxEmptyString );
		bar->GoTo( 0.5, NULL, false );
		bar->SetAddinStyle( VSS_VERTICAL_BOTTOM_UP );

		m_eqfactors[i] = bar;
	}

	wxDELETEA( bmArray );

	//------------------------------------------------
	// TODO

	EnableAll( false, NULL );
}

wxXmlNode* EqPanel::FindEqfactor(wxXmlNode* windowRoot) const
{
	wxXmlNode* child = windowRoot->GetChildren();
	while( child )
	{
		if( VdkUtil::GetXrcName( child ) == L"eqfactor" )
		{
			return child;
		}

		child = child->GetNext();
	}

	return NULL;
}

void EqPanel::EnableAll(bool enable, wxDC* pDC)
{
	m_enableAll->Enable( enable, pDC );
	m_profile->Enable( enable, pDC );
	m_reset->Enable( enable, pDC );
	m_balance->Enable( enable, pDC );
	m_surround->Enable( enable, pDC );
	m_preamp->Enable( enable, pDC );

	for( int i = 0; i < NUM_EQFACTORS; i++ )
	{
		m_eqfactors[i]->Enable( enable, pDC );
	}
}
