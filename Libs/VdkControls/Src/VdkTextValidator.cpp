/***************************************************************
 * Name:      VdkTextValidator.cpp
 * Purpose:   定义 VdkTextValidator 类
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-7-31
 **************************************************************/
#include "StdAfx.h"
#include "VdkTextValidator.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

VdkTextValidator::VdkTextValidator(const VdkTextValidator& v)
	: wxTextValidator( v )
{

}

VdkTextValidator::VdkTextValidator(long style, wxString* valPtr)
	: wxTextValidator( style, valPtr )
{

}

bool VdkTextValidator::Validate(const wxString& val, wxWindow* parent)
{
	wxString errormsg;
	if( HasFlag( wxFILTER_EMPTY ) && val.empty() )
	{
		errormsg = _("Required information entry is empty.");
	}
	else if( !(errormsg = IsValid(val)).empty() )
	{
		// NB: this format string should always contain exactly one '%s'
		wxString buf;
		buf.Printf(errormsg, val.c_str());
		errormsg = buf;
	}

	if ( !errormsg.empty() )
	{
		m_validatorWindow->SetFocus();
		wxMessageBox( errormsg, _("Validation conflict"),
					  wxOK | wxICON_EXCLAMATION, 
					  parent );

		return false;
	}

	return true;
}

wxObject* VdkTextValidator::Clone() const
{
	return new VdkTextValidator( *this );
}

bool VdkTextValidator::Copy(const VdkTextValidator& v)
{
	return wxTextValidator::Copy( v );
}
