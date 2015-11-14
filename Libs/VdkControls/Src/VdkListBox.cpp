#include "StdAfx.h"
#include "VdkListBox.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_VOBJECT( VdkListBox );

//////////////////////////////////////////////////////////////////////////

void VdkListBox::Create(wxXmlNode* node)
{
	DoXrcCreate( node );
	Create( VdkListBoxInitializer().
			textAlign( GetXrcAlign( node, L"text-align" ) ) );
}

void VdkListBox::Create( const VdkListBoxInitializer& init_data )
{
	VdkControl::Create( init_data );
	VdkListCtrl::Create( m_Window, m_strName, m_Rect );

	InsertColumn( VdkLcColumnInitializer( this ).
				  percentage( 100 ).
				  leftPadding( 2 ).
				  width( m_Rect.width ).
				  textAlign( init_data.TextAlign) );
}
