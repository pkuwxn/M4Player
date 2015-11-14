#pragma once
#include <wchar.h>

class wxFile;
class wxMemoryInputStream;
class wxOutputStream;
class wxXmlNode;

//////////////////////////////////////////////////////////////////////////

/// �Ƚ���������;
int CompareInts(int n1, int n2);

/// \brief �����ı�
bool CopyText(const wxString& strText);

/// \brief ���������Ƿ�����ı�
bool IsTextOnClipborad();

/// \brief �����������ɾ�����ڵ�ָ��
template< class T >
void ClearPtrContainer(T& container)
{
	typename T::const_iterator i( container.begin() );
	for( ; i != container.end(); ++i )
		delete *i;
}

/// ����������������
template< class Iter, class T >
Iter SearchContainer(Iter beg, Iter end, const T& data)
{
	Iter i( beg );

	while( i != end )
	{
		if( *i == data )
			break;
		++i;
	}

	return i;
}

/// \brief ���ͼ��������ɾ�����ڵ�ָ��
template< class T >
void ClearPtrMap(T& container)
{
	typename T::iterator i;
	for( i = container.begin(); i != container.end(); ++i )
		delete (*i).second;
}

/// \brief ��� wxRect
inline wxRect& ZeroRect(wxRect& rc)
{
	rc.x = rc.y = rc.width = rc.height = 0;
	return rc;
}

/// \brief ��� wxPoint
inline wxPoint& ZeroPoint(wxPoint& pt)
{
	pt.x = pt.y = 0;
	return pt;
}

/// \brief ��ȫ��������λͼ
inline wxBitmap DuplicateBitmap(wxBitmap& bmp)
{
	wxRect rc( 0, 0, bmp.GetWidth(), bmp.GetHeight() );
	return bmp.GetSubBitmap( rc );
}

/// \brief ��λͼ�ı���ɫ��������λͼ
///
/// ͸�������Ǿ��г�����ɫ֮�����ɫ������
/// \note �������Ὣ����ɫתΪ��ɫ������ת��Ϊ��ɫ��
void TransparentBmp(wxBitmap& bmp, const wxColour& bgColour);

/// \brief ��ȡͼ�������
wxBitmapType TestImageType(wxInputStream* is);
wxBitmapType TestImageType(char* data);

/// \brief ��һ�� wxDateTime ��ʽ��Ϊ ISO ��ʽ�ı�
///
/// ���� 2010-09-24T15:09:33
/// wxWidgets 2.9+ �� wxDateTime ��ԭ���ṩ֧��
wxString GetISODateTime(const wxDateTime& dt);

/// \brief ����һ�������
int Rand(int max);

/// \brief �õ�\a num ��λ��
int GetNumBitCount(int num);

//////////////////////////////////////////////////////////////////////////

/// \brief д����� UTF-8 ������ļ�ͷ��BOM��
void WriteUtf8BOM(wxFile& f);

/// \brief �ļ��Ƿ�ʹ�� UTF-8 ����
bool IsEncodedInUtf8(const char* header);

enum RA_FileEncoding {

	RA_ANSI,	/// < Windows Ĭ�ϱ��루����������GB2312��
	RA_UTF8,	/// < UTF-8
	RA_UNICODE	/// < Unicode
};

/// \brief ��ȡ�ı��ļ�����������
wxString ReadAll(const wxString& strFileName, RA_FileEncoding type);

/// \brief ʹ�� strcmp ���Ƚ����� Unicode �ַ���
///
/// ò�����ĵ� Unicode ���벻����ƴ��˳�������еġ�
int CompareUnicodeStrings(const wxString& s1, const wxString& s2);

//////////////////////////////////////////////////////////////////////////

/// \brief �����뷨�����Ƶ�ָ��λ��
///
/// λ��(\a x, \a y)��\a win ���Ͻ�Ϊ������ʼ�㡣
void MoveImeWindow(wxWindow* win, int x, int y);

/// @Override
inline void MoveImeWindow(wxWindow* win, const wxPoint& pos) {
	MoveImeWindow( win, pos.x, pos.y );
}

/// \brief ��ǰ���뷨�����Ƿ�ɼ�
bool IsImeWindowVisiable(wxWindow* win);

//////////////////////////////////////////////////////////////////////////

/// \brief �������Ƶ�ָ��λ��
///
/// �ô���Ҫ������ GTK+ �����첽 GUI ģ�����档
void MoveWindowSynchronously(wxWindow* win, int x, int y);
/// @Override
inline void MoveWindowSynchronously(wxWindow* win, const wxPoint& pos) {
	MoveWindowSynchronously( win, pos.x, pos.y );
}

//////////////////////////////////////////////////////////////////////////

/// \brief �־û����롢д�뷢������
class serialize_bad_format {};

/// \brief ������д��һ��������
template< class V, class OutStream >
void WriteVar(OutStream& fout, V data)
{
	fout.Write( &data, sizeof( data ) );
}

/// \brief ���������һ��������
template< class V, class InStream >
V ReadVar(InStream& fin)
{
	V ret;
	fin.Read( &ret, sizeof( ret ) );

	return ret;
}

/// \brief ������д��һ���ַ���
///
/// ��д���ַ����ĳ��ȣ�Ȼ�����ַ�����û���ַ���ĩβ�ġ�0��
void WriteString(wxOutputStream& fout, const wxString& s);

/// \brief �����ж���һ���ַ���
wxString ReadString(wxInputStream& fin);

/// \brief ���ڴ����ж���һ���ַ���
wxString ReadString(wxMemoryInputStream& min);

//////////////////////////////////////////////////////////////////////////

/// \brief ��һ������
///
/// ����䣬ֻ�ǻ��߿�
void DrawRectangle(wxDC& dc, const wxRect& rc);

/// \brief ��һ���ַ������ҳ��Ǹ��п�ݼ����ַ����硰&N����
/// Ȼ��ֻ������ַ����滭һ���»���
void DrawHotKeyCaption( const wxString& strText,
						wxDC& dc,
						int x,
						int y );

/// \brief �õ�ϵͳ��ص� RamDisk ��·��
///
/// Windows ��Ϊ Z:\ ��Linux ��Ϊ /dev/shm/ ��
wxString GetRamDiskPath(const wchar_t* szFileName);

/// \brief ��ָ�� wxBitmap ���浽�ڴ�����
void RandSave(const wxBitmap& bm, const wchar_t* szPostfix = NULL);

/// \brief �� DC ��ָ����������ݱ��浽�ڴ�����
void RandSaveDC(wxDC& dc, const wxRect& rt, const wchar_t* szPostfix = NULL);

//////////////////////////////////////////////////////////////////////////

/// \brief ֱ�ӻ�ȡָ�������ӽ��Ĳ���ֵ
bool XmlGetContentOfBoolean(wxXmlNode* xmlNode, bool defv = false);

/// \brief ֱ�ӻ�ȡָ�������ӽ�����ֵ
int XmlGetContentOfNum(wxXmlNode* xmlNode, int defaultValue = 0);

/// \brief ֱ�ӻ�ȡָ�������ӽ����ı�
wxString XmlGetChildContent(wxXmlNode* node, const wxString& strChdName,
							const wxString& defaultValue = wxEmptyString);

/// \brief ������Ϊ\a strName ���ӽ��
wxXmlNode* FindChildNode(wxXmlNode* xmlNode, const wxString& strName);

/// \brief ����ͬ�����ֵܽ��
wxXmlNode* FindSibling(wxXmlNode* node);

/// \brief ���� XML �����ı�����
void XmlSetContent(wxXmlNode* node, const wxString& strContent);

/// \brief ����һ���µĽ��
wxXmlNode* XmlInsertChild(wxXmlNode* parent, const wxString& strName,
						  const wxString& strContent);

//////////////////////////////////////////////////////////////////////////

/// \brief �Ƿ���Ȼ���ÿ�������
bool wxIsAutoStart(const wxString& strAppName);

/// \brief ���ÿ�������
bool wxAutoStart(const wxString& strAppName,
				 const wxString& strAppPath,
				 bool bAdd = true);

/// \brief һ�����������崴������
wxFont wxEasyCreatFont(const wxString& strFaceName = wxEmptyString,
					   int nPointSize = 9,
					   bool bBold = false);

/// \brief �õ�����ľ�ȷ�߶�
int wxGetFontHeight(const wxFont& font, wxWindow* win);

//////////////////////////////////////////////////////////////////////////

#ifndef dprintf
#	ifdef __WXDEBUG__
#		if defined( __WXGTK__ )
#           define dprintf(...) wprintf(__VA_ARGS__);
#		elif defined( __WXMSW__ )
#           define dprintf(...)\
            {\
                wchar_t* szMessage = new wchar_t[ 1024 ]; \
                wsprintf( szMessage, __VA_ARGS__ ); \
				wcscat( szMessage, L"\n" ); \
                OutputDebugString( szMessage ); \
                delete szMessage; \
            }
#	    endif
#	else
#       define dprintf(...) ;
#	endif // __WXDEBUG__
#endif // !dprintf

//////////////////////////////////////////////////////////////////////////

/// \brief ����λ�ھ��ε��Ҳ���һ����(����㲢�����ڸþ���)
inline int RightOf (const wxRect& rc) { return rc.x + rc.width; }

/// \brief ����λ�ھ��εĵײ���һ����(����㲢�����ڸþ���)
inline int BottomOf(const wxRect& rc) { return rc.y + rc.height; }

/// \brief �����ζ����ʾΪ��x, y, width, height���ĸ�ʽ
///
/// ������֡�(����)����
wxString RectToString(const wxRect& rc);

/// \brief ��ӡһ�����ζ���
void Print(const wxRect& rc, const wxString& desc = wxEmptyString);

/// \brief ��������ʾΪ��x, y���ĸ�ʽ
///
/// ������֡�(����)����
wxString PointToString(const wxPoint& point);

/// \brief ��ӡһ�������
void Print(const wxPoint& point, const wxString& desc = wxEmptyString);

/// \brief ����������ת������Ӧ���ı���ʽ��VdkDirection��
wxString DirectionToString(unsigned d);

/// \brief ����������ת������Ӧ���ı���ʽ��VdkAlignment��
wxString AlignmentToString(unsigned align);

//////////////////////////////////////////////////////////////////////////

/// \brief ��ȡ���̰�����ʶ�����ַ�����ʾ
wxString GetKeyCodeName(int keycode);

#ifdef __WXDEBUG__

	/// \brief ��ȡ VDK �¼���event����ʶ�����ַ�����ʾ
	const wxChar* GetEventName(int evtCode);
	/// \brief ��ȡ VDK ֪ͨ��notice����ʶ�����ַ�����ʾ
	const wxChar* GetNoticeName(int notice);

#	ifdef __WXMSW__
		/// \brief ��ȡ Win32 SDK ��Ϣ���ַ�����ʾ
		const wxChar* wxGetMessageName(int message);
#	endif // __WXMSW__

#endif // __WXDEBUG__

//////////////////////////////////////////////////////////////////////////

/// \brief ���н���ɫ�ı��е�һ��
struct GtmTextLine {

	wxString s;
	int x;
};

WX_DECLARE_OBJARRAY( GtmTextLine, ArrayOfGtmTextLines );

/// \brief ��������ɫ�ı���
///
/// �������������ռ䣬������Ⱦ��
struct GradientTextMethod
{
	/// \brief ��������ɫ�ı�
	static void Draw(wxDC& dc,
					 const wxFont& font,
					 const wxBrush& BgBrush,
					 const wxColour& TextColor,
					 const wxRect& rc,
					 int rowHeight,
					 wxDirection direction,
					 const ArrayOfGtmTextLines& lines);


private:

	// ִ��ʵ�ʻ�ͼ����
	static void DrawText(wxDC& dc, int rowHeight, int y, 
						 const ArrayOfGtmTextLines& lines);
};
