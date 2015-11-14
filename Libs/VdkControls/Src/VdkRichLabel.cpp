#include "StdAfx.h"
#include "VdkRichLabel.h"

#include "VdkWindow.h"
#include "wxUtil.h"

#ifdef __WXMSW__
#    include <wx/msw/msvcrt.h>
#endif


IMPLEMENT_DYNAMIC_VOBJECT(VdkRichLabel);

// 两行文本之间的间隔距离，用于多行 VdkRichLabel
static const int gTextInternal = 5;

//////////////////////////////////////////////////////////////////////////

VdkRichLabel::VdkRichLabel() : m_UrlColor( *wxBLUE )
{
	RemoveStyle( VCS_ONESHOT_HOVERING | VCS_IGNORE_ALL_EVENTS );
}

void VdkRichLabel::Create(wxXmlNode* node)
{
	VdkLabel::Create(node);
	PrepareRect();
}

void VdkRichLabel::PrepareRect()
{
	m_Urls.clear();
	m_strBreaks.clear();

	if( m_strCaption.IsEmpty() )
		return;

	int line = 0, maxWidth = 0, w;

	wxMemoryDC mdc;
	mdc.SetFont(m_Font);

	//////////////////////////////////////////////////////////////////////////
	// 首先分析要着色的文本
	/*
	ColorTextNode ctn;
	wxString strText, strAttriName, strAttriValue;
	while( GetUbbNode(m_strCaption, strAttriName, strAttriValue, strText) )
	{
		mdc.GetTextExtent(strText, &ctn.width, NULL);
	}*/

	//////////////////////////////////////////////////////////////////////////

	wxString strBreak;
	wxString strCaption(m_strCaption);
	wxString::iterator p, base;

	int max(m_Rect.GetWidth());
	wxASSERT_MSG( max >= mdc.GetCharWidth()*2, L"m_Rect的宽度太小。" );

	const wchar_t* alphas =   L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	const wchar_t* urlchars = L"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             L"abcdefghijklmnopqrstuvwxyz/,:.&?#=;-";
	const wchar_t* symbols =  L"!),.:;?]}¨·ˇˉ―‖’”…∶、。〃々〉》」』】〕"
                             L"〗！＂＇），．：；？］｀｜｝～￠";

	////////////////////////////////////
	// 解释、分解 URL

	wxString::size_type beg, end;
	beg = strCaption.find( L"http://" );

	while( beg != wxString::npos )
	{
		p = strCaption.begin() + beg;

		wxString::size_type i = beg,
							len = strCaption.length();
		const wchar_t* szCaption = strCaption.c_str();

		while( i < len && wcschr( urlchars, szCaption[i++] ) );
		if( i < len )
			i--;

		p = strCaption.begin() + i;
		end = p - strCaption.begin();

		Url aUrl = { strCaption.substr(beg, end-beg) };
		aUrl.strUrlDisplay = aUrl.strUrl;
		m_Urls.push_back(aUrl);
		Url& url = m_Urls[m_Urls.size()-1];

		if( end - beg > 21 )
		{
			url.strUrlDisplay =
				url.strUrl.substr(0, 9) + L"..." +
				url.strUrl.substr(url.strUrl.length()-9, 9);

			GetRandomString(url.strDummy, 21);
		}
		else
			GetRandomString(url.strDummy, end-beg);

		strCaption.replace(beg, end-beg, url.strDummy);

		beg = strCaption.find(L"http://", beg + 1);
	}

	////////////////////////////////////

	wxArrayInt exts;
	mdc.GetPartialTextExtents(strCaption, exts);

	int i = 0,
		size = exts.Count(),
		last = 0,
		curr = 0;				// 当前已处理的长度

	for( ; i < size; ++i )
	{
		if( exts[i] - curr < max && strCaption[i] != L'\n' )
		{
			continue;
		}
		else
		{
			if( strCaption[i] != L'\n' )
			{
				p = strCaption.begin() + i;
				base = strCaption.begin() + last;

				// 不能截断完整的英文单词
				if( wcschr(alphas, *p) )
				{
					p--;
					while( wcschr(alphas, *p) && p >= base )
						p--;

					if( p != base ) // 太长无法分词
						i = p - strCaption.begin();
				}

				// 不能放在一句话开头的标点
				if( wcschr(symbols, *p) )
				{
					p--;
					i--;
				}

				if( i >= size )
					i = size - 1;
			}

			strBreak = strCaption.Mid(last, i-last).Trim(false);
			mdc.GetTextExtent(strBreak, &w, NULL);
			if( w > maxWidth )
				maxWidth = w;

			m_strBreaks.push_back(strBreak);
			last = i;
			line++;
			curr = exts[i];
		}
	}

	if( i != last )
	{
		strBreak = strCaption.Mid(last, i-last).Trim(false);
		mdc.GetTextExtent(strBreak, &w, NULL);
		if( w > maxWidth )
			maxWidth = w;

		m_strBreaks.push_back(strBreak);
		line++;
	}

	m_Rect.height = line*mdc.GetCharHeight() + gTextInternal*(line-1);
}

void VdkRichLabel::DoDraw(wxDC& dc)
{
	if( m_strCaption.IsEmpty() )
		return;

	dc.SetFont(m_Font);
	dc.SetTextForeground(m_TextColor);

	int size = m_strBreaks.size(),
		h = dc.GetCharHeight(),
		UrlIndex = 0;

	for(int i=0;i<size;i++)
	{
		if( m_strBreaks[i].find(L"LRUPTTH") == wxString::npos )
		{
			dc.DrawText( m_strBreaks[i],
						 m_Rect.x + m_xFix,
						 m_Rect.y + ( gTextInternal + h ) * i );
		}
		else
		{
			DrawUrl( UrlIndex,
					 m_strBreaks[i], dc,
					 m_Rect.x + m_xFix,
					 m_Rect.y + ( gTextInternal + h ) * i );
		}
	}
}

void VdkRichLabel::DrawUrl(int& UrlIndex,
	const wxString& strUrlAll, wxDC& dc, int x, int y)
{
	const wxPen& oldPen = dc.GetPen();
	int w, h = dc.GetCharHeight();

	wxString::size_type beg, end = 0, beg2;
	beg = strUrlAll.find(L"LRUPTTH");
	beg2 = strUrlAll.find(L"LRUPTTH", beg+1);

	wxString::size_type len;

	while( beg != wxString::npos )
	{
		wxString strLeft = strUrlAll.substr( end, beg - end );

		end = beg + wcslen( m_Urls[ UrlIndex ].strDummy );
		if(!strLeft.IsEmpty())
		{
			dc.DrawText( strLeft, x, y );
			dc.GetTextExtent( strLeft, &w, NULL );
			x += w;
		}

		dc.SetPen( wxPen( m_UrlColor ) );
		dc.SetTextForeground( m_UrlColor );
		dc.DrawText( m_Urls[ UrlIndex ].strUrlDisplay, x, y );

		dc.GetTextExtent( m_Urls[ UrlIndex ].strUrlDisplay, &w, NULL );
		dc.DrawLine( x, y + h, x + w, y + h );

		if( m_Urls[ UrlIndex ].Rect.IsEmpty() )
			m_Urls[ UrlIndex ].Rect = wxRect( x - m_Rect.x, y - m_Rect.y, w, h );

		x += w;

		dc.SetPen( oldPen );
		dc.SetTextForeground( m_TextColor );

		// URL的右边部分
		len = strUrlAll.Len();
		if( beg2 <= len )
			len = beg2 - end;

		wxString strRight = strUrlAll.substr(end, len);
		if(!strRight.IsEmpty())
		{
			dc.DrawText(strRight, x, y);
			dc.GetTextExtent(strRight, &w, NULL);
			x += w;
		}

		// 是否超出在非常长度
		len = strUrlAll.Len();
		if( end >= len )
			end = len - 1;

		beg = strUrlAll.find( L"LRUPTTH", end );
		if( beg != wxString::npos )
			beg2 = strUrlAll.find( L"LRUPTTH", beg + 1 );

		UrlIndex++;
	}
}

void VdkRichLabel::GetRandomString(wxString& strOut, int len)
{
	strOut = L"LRUPTTH";
	wchar_t str[53] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	srand( (unsigned int) time( (time_t* ) NULL ) );
	for( int i = 6; i < len - 1; i++ )
		strOut += str[ ( rand() %52 ) + 1 ];
}


bool VdkRichLabel::GetUbbNode(wxString& strUbb,
							  wxString& strName,
							  wxString& strAttriValue,
							  wxString& strText)
{
	wxString::size_type beg(strUbb.find('[')), end(strUbb.find(']', beg));
	if( beg != wxString::npos && end  != wxString::npos )
	{
		wxString::size_type comma(strUbb.find(':', beg));
		if( comma != wxString::npos )
		{
			if( comma != wxString::npos )
			{
				// 应该是一个UBB属性值了
				// e.g. [color:red]%d[/color]
				strName = strUbb.Mid(beg, comma-beg);
				strAttriValue = strUbb.Mid(comma+1, end-comma-1);

				// 再进行判断，看属性字符串是否闭合
				// cbeg == close_beg
				wxString strCloseTag;
				strCloseTag.Printf(L"[/%s]", strName.c_str());
				wxString::size_type cbeg(strUbb.find(strCloseTag, end)), cend;
				if( cbeg != wxString::npos )
				{
					strText = strUbb.Mid(end+1, cbeg-end-1);

					cend = cbeg + strCloseTag.Length();
					strUbb.Remove(beg, end-beg);
					strUbb.Remove(cbeg, cend-cbeg);

					return true;
				}
				else // 并不配对
				{
					strName = wxEmptyString;
					strAttriValue = wxEmptyString;
				}
			}
		}
	}

	return false;
}

wxString VdkRichLabel::GetUrl(unsigned int id)
{
	if( id >= m_Urls.size() )
		return wxEmptyString;

	return m_Urls.at(id).strUrl;
}

void VdkRichLabel::DoHandleMouseEvent(VdkMouseEvent& e)
{
	switch( e.evtCode )
	{
	case HOVERING:
	case LEFT_DOWN:
	{
		int size = m_Urls.size();

		for( int j = 0; j < size; ++j )
		{
			if( m_Urls[j].Rect.Contains( DeTranslatePoint( e.mousePos ) ) )
			{
				if( e.evtCode == LEFT_DOWN )
				{
#ifdef __WXDEBUG__
					wxMessageBox( m_Urls[j].strUrl );
#else
					wxLaunchDefaultBrowser( m_Urls[j].strUrl );
#endif // __WXDEBUG__
				}
				else
				{
					m_Window->AssignCursor( wxCursor( wxCURSOR_HAND ) );
				}

				return;
			}
		}

		// 找不到符合要求的 URL, 恢复鼠标指针
	}

	case NORMAL:

		m_Window->ResetCursor();

		break;

	default:

		break;
	}

	return;
}

void VdkRichLabel::DoHandleNotify(const VdkNotify& notice)
{
	switch( notice.GetNotifyCode() )
	{
	case VCN_POSITION_CHANGED:
		{
			int size = m_Urls.size();
			int dX = (int) notice.GetWparam();
			int dY = (int) notice.GetLparam();

			for( int i = 0; i < size; i++ )
			{
				if( m_Urls[i].Rect.IsEmpty() )
					continue;

				m_Urls[i].Rect.x += dX;
				m_Urls[i].Rect.y += dY;
			}

			break;
		}

	default:

		break;
	}
}
