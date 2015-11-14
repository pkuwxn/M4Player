/***************************************************************
 * Name:      OOPProgressDlg.cpp
 * Purpose:   “文件属性”对话框
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-8-19
 **************************************************************/
#include "StdAfx.h"
#include "OOPProgressDlg.h"

#include "VdkCtrlParserInfo.h"
#include "VdkLabel.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT( EVT_UPDATE_TEXT, wxCommandEvent );

OOPProgressDlg::OOPProgressDlg(wxWindow* parent, 
							   const wxString& title,
							   const wxString& msg,
							   int maximum)
	: VdkDialog( parent, title, 0, VWS_DRAGGABLE ),
	  m_label( NULL ), m_maximum( maximum ), m_value( 0 ),
	  m_skipped( false ), m_canceled( false )
{
	FromXrc( L"../../App/progress_dlg.xml", NULL );

	m_label = (VdkLabel *) FindCtrl( L"showText" );
	m_label->SetCaption( msg, NULL );

	VdkCtrlId cancel = VdkGetUniqueId();
	FindCtrl( L"cancel" )->SetID( cancel );
	Bind( wxEVT_VOBJ, &OOPProgressDlg::OnCancel, this, cancel );

	Bind( wxEVT_CLOSE_WINDOW, &OOPProgressDlg::OnClose, this );
}

OOPProgressDlg::~OOPProgressDlg()
{

}

bool OOPProgressDlg::Update(int v, const wxString& newmsg, bool* skip)
{
	if( m_canceled )
		return false;

	m_value = v;

	if( !newmsg.empty() )
	{
		VdkDC dc( this );
		m_label->SetCaption( newmsg, &dc );
	}

	if( skip )
	{
		*skip = m_skipped;
	}

	return v < m_maximum;
}

void OOPProgressDlg::ClearState()
{
	m_value = 0;
	m_skipped = false;
	m_canceled = false;
}

void OOPProgressDlg::OnCancel(VdkVObjEvent&)
{
	m_canceled = true;
}

void OOPProgressDlg::OnClose(wxCloseEvent&)
{
	m_canceled = true;
}

int OOPProgressDlg::GetMaximum() const
{
	return m_maximum;
}

int OOPProgressDlg::GetValue() const
{
	return m_value;
}
