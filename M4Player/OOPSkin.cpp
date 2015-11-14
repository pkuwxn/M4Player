/***************************************************************
 * Name:      OOPSkin.cpp
 * Purpose:   OOPSkin 类的实现文件
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-18
 **************************************************************/
#include "StdAfx.h"
#include "OOPSkin.h"
#include "OOPFileSystem.h"

#include "VdkUtil.h"
#include "wxUtil.h"

#include <wx/filename.h> // for wxFileName::GetPathSeparator()
#include <wx/dir.h>
#include <wx/xml/xml.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

void OOPSkin::wxListOfSkinInfoNode::DeleteData()
{
	delete (_WX_LIST_ITEM_TYPE_ListOfSkinInfo *) GetData();
}

OOPSkin::~OOPSkin()
{
	m_skins.DeleteContents( true );
	m_skins.Clear();
}

void OOPSkin::GetAllSkins()
{
	if( !m_skins.empty() )
		return;

	wxString strSkinRootDir( OOPFileSystem::GetSkinRootDir() );
	wxDir dir( strSkinRootDir );

	if( !dir.IsOpened() )
		return;

	wxString name;
	bool cont = dir.GetFirst( &name, wxEmptyString, wxDIR_DIRS );

	while( cont )
	{
		wxString skinRoot( strSkinRootDir + name );
		skinRoot += wxFileName::GetPathSeparator();

		if( wxFileExists( skinRoot + L"Skin_OOP.xml" ) )
		{
			SkinInfo* skin = new SkinInfo( name, skinRoot );
			m_skins.push_back( skin );
		}

		cont = dir.GetNext( &name );
	}
}

OOPSkin::SkinInfo* OOPSkin::GetSkin(int index)
{
	return m_skins[index];
}

//////////////////////////////////////////////////////////////////////////

OOPSkin::SkinInfo::SkinInfo(const wxString& strName, const wxString& strRootPath)
	: m_strRootPath( strRootPath ),
	  m_maskColor( 255, 0, 255 )
{
	wxXmlDocument doc( m_strRootPath + L"Skin_OOP.xml" );
	wxXmlNode* root = doc.GetRoot();
	wxASSERT( root );

	m_strName = XmlGetChildContent( root, L"name", strName );
	m_author = XmlGetChildContent( root, L"author" );
	m_url = XmlGetChildContent( root, L"url" );
	m_email = XmlGetChildContent( root, L"email" );

	wxString maskColor( XmlGetChildContent( root, L"transparent_color" ) );
	if( !maskColor.IsEmpty() )
		m_maskColor.Set( maskColor );
}

wxString OOPSkin::SkinInfo::folderName() const
{
	wxUniChar pathSeperator( wxFileName::GetPathSeparator() );

	wxString::const_iterator i( m_strRootPath.end() - 2 );
	while( *i != pathSeperator )
		--i;

	wxString::size_type first( i - m_strRootPath.begin() + 1 );
	return m_strRootPath.substr( first, m_strRootPath.length() - 1 - first );
}

wxBitmap OOPSkin::SkinInfo::thumbNail(const wxSize& sz) const
{
	int w, h;
	if( m_bmpThumbnail.IsOk() )
    {
        w = m_bmpThumbnail.GetWidth();
        h = m_bmpThumbnail.GetHeight();

        if( w == sz.x && h == sz.y )
            return m_bmpThumbnail;
    }

	wxString fileName( m_strRootPath );
	wxString fileData( ReadAll( fileName + L"player_window.xml", RA_ANSI ) );

	wxString::size_type first = fileData.find( L"<image>" );
	if( first == wxString::npos )
		return wxNullBitmap;

	first += 7;
	wxString::size_type last = fileData.find( L"</image>", first );

	fileName += fileData.substr( first, last - first );
	VdkUtil::ImRead( m_bmpThumbnail, fileName );

	if( !m_bmpThumbnail.IsOk() )
		return wxNullBitmap;

	w = m_bmpThumbnail.GetWidth();
	h = m_bmpThumbnail.GetHeight();
	if( w > sz.x || h > sz.y )
	{
		wxRect rcsub( 0, 0, (w > sz.x) ? sz.x : w, (h > sz.y) ? sz.y : h );
		m_bmpThumbnail = m_bmpThumbnail.GetSubBitmap( rcsub );
		VdkUtil::CreateMask( m_bmpThumbnail );

		return m_bmpThumbnail;
	}

	return m_bmpThumbnail;
}

wxString OOPSkin::SkinInfo::GetNode(const wxString& xmlData, const wxString& nodeName)
{
	wxString::size_type first = xmlData.find( L"<image>" );
	if( first == wxString::npos )
		return wxEmptyString;

	first += 7;
	wxString::size_type last = xmlData.find( L"</image>", first );

	return xmlData.substr( first, last - first );
}
