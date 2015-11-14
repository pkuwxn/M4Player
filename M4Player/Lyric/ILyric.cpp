/***************************************************************
 * Name:      ILyric.cpp
 * Purpose:   歌词秀的抽象接口
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-06-05
 **************************************************************/
#include "StdAfx.h"
#include "ILyric.h"

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

ILyric::ILyric()
	: m_stopWatch( NULL )
{

}

void ILyric::SetDefualtInteractiveOutput(const wxString& msg)
{
	m_defaultInteractiveOutput = msg;
}

void ILyric::SetInteractiveOutput(const wxString& msg, wxDC* pDC)
{
	m_interactiveOutput = msg;

	//------------------------------------------------

	DoSetInteractiveOutput( pDC );
}

void ILyric::ResetInteractiveOutput(wxDC* pDC)
{
	SetInteractiveOutput( wxEmptyString, pDC );
}

wxString ILyric::GetInteractiveOutput() const
{
	return m_interactiveOutput.empty() ? 
				m_defaultInteractiveOutput :
				m_interactiveOutput;
}
