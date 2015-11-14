#include "StdAfx.h"
#include "VdkFileSelector.h"
#include "VdkButton.h"
#include "VdkEdit.h"
#include "VdkWindow.h"
#include "VdkCtrlParserInfo.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


IMPLEMENT_DYNAMIC_VOBJECT( VdkFileSelector );

//////////////////////////////////////////////////////////////////////////

VdkFileSelector::VdkFileSelector()
	: m_edit( NULL ), m_type( ST_DIR ), m_alertMsg( L"请选择路径:" )
{

}

void VdkFileSelector::Create(wxXmlNode* node)
{
	ParseObjects( VdkCtrlParserInfo().
				  window( m_Window ).
				  node( node ).
				  parent( this ) );

	if( m_firstChild && dynamic_cast< VdkButton* >( m_firstChild ) != NULL )
	{
		Create( m_Window, GetXrcName( node ), GetXrcRect( node ) );
	}
	else
	{
		// 原因是必须有一个明确定义的 VdkButton 背景图像
		wxASSERT_MSG( false, 
			wxString::Format( L"VdkFileSelector 应该有一个排在第一位的"
							  L" VdkButton 类型的子控件。(%s)", 
							  GetXrcName( node ).t_str() )
			);
	}
}

void VdkFileSelector::Create(VdkWindow* window, const wxString& name, const wxRect& rc)
{
	wxASSERT( m_firstChild );
	VdkButton* btn = (VdkButton *) m_firstChild;

	m_strName = name;
	m_Rect = rc;
	SetVdkWindow( window );

	//////////////////////////////////////////////////////////////////////////

	int btnw, btnh;
	btn->GetSize( &btnw, &btnh );

	if( btn->GetRect().IsEmpty() )
	{
		wxRect rcButton( m_Rect.width - btnw, (m_Rect.height - btnh) / 2, btnw, btnh );
		btn->SetRect( rcButton, NULL );
	}

	btn->SetID( VdkGetUniqueId() );
	m_WindowImpl->Bind( wxEVT_VOBJ, &VdkFileSelector::OnSelectorRed, 
						this, btn->GetID() );
	m_WindowImpl->Bind( wxEVT_REDIRECT_CMD, &VdkFileSelector::OnSelector, 
						this, btn->GetID() );

	//////////////////////////////////////////////////////////////////////////

	wxRect rcEdit( m_Rect );
	rcEdit.x = rcEdit.y = 0;
	rcEdit.width -= btnw;
	rcEdit.width -= 5; // 5 为间隔

	long s = GetStyle() | VCS_BORDER_SIMPLE;
	s &= ~VCS_HANDLER;

	m_edit = new VdkEdit;
	m_edit->Create( VdkEditInitializer().
					window( m_Window ).
					addToWindow( true ).
					name( m_strName + L"_Edit" ).
					parent( this ).
					style( s ).
					align( m_align ).
					rect( rcEdit ) );
}

void VdkFileSelector::SetExt(const wxString& strDefaultFileName, 
							 const wxString& strExtPrompt)
{
	if( m_type != ST_FILE )
		return;

	m_extPrompt = strExtPrompt;
	m_defaultFileName = strDefaultFileName;
}

void VdkFileSelector::OnSelectorRed(VdkVObjEvent& e)
{
	VdkButton* btn = reinterpret_cast< VdkButton* >( m_firstChild );
	btn->Update( VdkButton::NORMAL, e.GetVObjDC() );

	//======================================

	wxCommandEvent rde( e );
	rde.SetEventType( wxEVT_REDIRECT_CMD );

	wxPostEvent( m_WindowImpl, rde );
}

void VdkFileSelector::OnSelector(wxCommandEvent&)
{
	wxString strPath( m_edit->GetValue() );
	strPath = m_type == ST_DIR ? wxDirSelector( m_alertMsg, 
												strPath, 
												0, 
												wxDefaultPosition,
												m_WindowImpl )

							  : wxFileSelector( m_alertMsg, 
												strPath, 
												m_defaultFileName,
												wxEmptyString,
												m_extPrompt,
												0,
												m_WindowImpl );

	if( !strPath.IsEmpty() )
	{
		m_edit->SetValue( strPath );
		m_edit->SetInsertionPointEnd();

		m_Window->QueueRedrawEvent();
	}
}

wxString VdkFileSelector::GetPath() const
{
	return m_edit->GetValue();
}

void VdkFileSelector::SetPath(const wxString& strPath)
{
	m_edit->SetValue( strPath );
}
