#pragma once
#include <wchar.h>

class wxFile;
class wxMemoryInputStream;
class wxOutputStream;
class wxXmlNode;

//////////////////////////////////////////////////////////////////////////

/// 比较两个整数;
int CompareInts(int n1, int n2);

/// \brief 复制文本
bool CopyText(const wxString& strText);

/// \brief 剪贴板内是否存有文本
bool IsTextOnClipborad();

/// \brief 清空容器，并删除其内的指针
template< class T >
void ClearPtrContainer(T& container)
{
	typename T::const_iterator i( container.begin() );
	for( ; i != container.end(); ++i )
		delete *i;
}

/// 遍历容器进行搜索
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

/// \brief 清空图容器，并删除其内的指针
template< class T >
void ClearPtrMap(T& container)
{
	typename T::iterator i;
	for( i = container.begin(); i != container.end(); ++i )
		delete (*i).second;
}

/// \brief 清空 wxRect
inline wxRect& ZeroRect(wxRect& rc)
{
	rc.x = rc.y = rc.width = rc.height = 0;
	return rc;
}

/// \brief 清空 wxPoint
inline wxPoint& ZeroPoint(wxPoint& pt)
{
	pt.x = pt.y = 0;
	return pt;
}

/// \brief 安全复制整个位图
inline wxBitmap DuplicateBitmap(wxBitmap& bmp)
{
	wxRect rc( 0, 0, bmp.GetWidth(), bmp.GetHeight() );
	return bmp.GetSubBitmap( rc );
}

/// \brief 从位图的背景色创建掩码位图
///
/// 透明区域将是具有除背景色之外的颜色的区域。
/// \note 本函数会将背景色转为白色，其余转换为黑色。
void TransparentBmp(wxBitmap& bmp, const wxColour& bgColour);

/// \brief 获取图像的类型
wxBitmapType TestImageType(wxInputStream* is);
wxBitmapType TestImageType(char* data);

/// \brief 将一个 wxDateTime 格式化为 ISO 格式文本
///
/// 形如 2010-09-24T15:09:33
/// wxWidgets 2.9+ 的 wxDateTime 已原生提供支持
wxString GetISODateTime(const wxDateTime& dt);

/// \brief 产生一个随机数
int Rand(int max);

/// \brief 得到\a num 的位数
int GetNumBitCount(int num);

//////////////////////////////////////////////////////////////////////////

/// \brief 写入代表 UTF-8 编码的文件头（BOM）
void WriteUtf8BOM(wxFile& f);

/// \brief 文件是否使用 UTF-8 编码
bool IsEncodedInUtf8(const char* header);

enum RA_FileEncoding {

	RA_ANSI,	/// < Windows 默认编码（对于中文是GB2312）
	RA_UTF8,	/// < UTF-8
	RA_UNICODE	/// < Unicode
};

/// \brief 读取文本文件的所有内容
wxString ReadAll(const wxString& strFileName, RA_FileEncoding type);

/// \brief 使用 strcmp 来比较两个 Unicode 字符串
///
/// 貌似中文的 Unicode 编码不是以拼音顺序来排列的。
int CompareUnicodeStrings(const wxString& s1, const wxString& s2);

//////////////////////////////////////////////////////////////////////////

/// \brief 将输入法窗口移到指定位置
///
/// 位置(\a x, \a y)以\a win 左上角为坐标起始点。
void MoveImeWindow(wxWindow* win, int x, int y);

/// @Override
inline void MoveImeWindow(wxWindow* win, const wxPoint& pos) {
	MoveImeWindow( win, pos.x, pos.y );
}

/// \brief 当前输入法窗口是否可见
bool IsImeWindowVisiable(wxWindow* win);

//////////////////////////////////////////////////////////////////////////

/// \brief 将窗口移到指定位置
///
/// 用处主要体现在 GTK+ 这种异步 GUI 模型下面。
void MoveWindowSynchronously(wxWindow* win, int x, int y);
/// @Override
inline void MoveWindowSynchronously(wxWindow* win, const wxPoint& pos) {
	MoveWindowSynchronously( win, pos.x, pos.y );
}

//////////////////////////////////////////////////////////////////////////

/// \brief 持久化读入、写入发生错误
class serialize_bad_format {};

/// \brief 往流里写入一个整型数
template< class V, class OutStream >
void WriteVar(OutStream& fout, V data)
{
	fout.Write( &data, sizeof( data ) );
}

/// \brief 从流里读入一个整型数
template< class V, class InStream >
V ReadVar(InStream& fin)
{
	V ret;
	fin.Read( &ret, sizeof( ret ) );

	return ret;
}

/// \brief 往流里写入一个字符串
///
/// 先写入字符串的长度，然后是字符串，没有字符串末尾的“0”
void WriteString(wxOutputStream& fout, const wxString& s);

/// \brief 从流中读入一个字符串
wxString ReadString(wxInputStream& fin);

/// \brief 从内存流中读入一个字符串
wxString ReadString(wxMemoryInputStream& min);

//////////////////////////////////////////////////////////////////////////

/// \brief 画一个矩形
///
/// 不填充，只是画边框。
void DrawRectangle(wxDC& dc, const wxRect& rc);

/// \brief 在一个字符串中找出那个有快捷键的字符，如“&N”，
/// 然后只在这个字符下面画一条下划线
void DrawHotKeyCaption( const wxString& strText,
						wxDC& dc,
						int x,
						int y );

/// \brief 得到系统相关的 RamDisk 的路径
///
/// Windows 下为 Z:\ ，Linux 下为 /dev/shm/ 。
wxString GetRamDiskPath(const wchar_t* szFileName);

/// \brief 将指定 wxBitmap 保存到内存盘上
void RandSave(const wxBitmap& bm, const wchar_t* szPostfix = NULL);

/// \brief 将 DC 中指定区域的内容保存到内存盘上
void RandSaveDC(wxDC& dc, const wxRect& rt, const wchar_t* szPostfix = NULL);

//////////////////////////////////////////////////////////////////////////

/// \brief 直接获取指定名字子结点的布尔值
bool XmlGetContentOfBoolean(wxXmlNode* xmlNode, bool defv = false);

/// \brief 直接获取指定名字子结点的数值
int XmlGetContentOfNum(wxXmlNode* xmlNode, int defaultValue = 0);

/// \brief 直接获取指定名字子结点的文本
wxString XmlGetChildContent(wxXmlNode* node, const wxString& strChdName,
							const wxString& defaultValue = wxEmptyString);

/// \brief 查找名为\a strName 的子结点
wxXmlNode* FindChildNode(wxXmlNode* xmlNode, const wxString& strName);

/// \brief 查找同名的兄弟结点
wxXmlNode* FindSibling(wxXmlNode* node);

/// \brief 设置 XML 结点的文本内容
void XmlSetContent(wxXmlNode* node, const wxString& strContent);

/// \brief 插入一个新的结点
wxXmlNode* XmlInsertChild(wxXmlNode* parent, const wxString& strName,
						  const wxString& strContent);

//////////////////////////////////////////////////////////////////////////

/// \brief 是否已然设置开机启动
bool wxIsAutoStart(const wxString& strAppName);

/// \brief 设置开机启动
bool wxAutoStart(const wxString& strAppName,
				 const wxString& strAppPath,
				 bool bAdd = true);

/// \brief 一个便利的字体创建函数
wxFont wxEasyCreatFont(const wxString& strFaceName = wxEmptyString,
					   int nPointSize = 9,
					   bool bBold = false);

/// \brief 得到字体的精确高度
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

/// \brief 返回位于矩形的右部第一个点(这个点并不属于该矩形)
inline int RightOf (const wxRect& rc) { return rc.x + rc.width; }

/// \brief 返回位于矩形的底部第一个点(这个点并不属于该矩形)
inline int BottomOf(const wxRect& rc) { return rc.y + rc.height; }

/// \brief 将矩形对象表示为“x, y, width, height”的格式
///
/// 不会出现“(”“)”。
wxString RectToString(const wxRect& rc);

/// \brief 打印一个矩形对象
void Print(const wxRect& rc, const wxString& desc = wxEmptyString);

/// \brief 将点对象表示为“x, y”的格式
///
/// 不会出现“(”“)”。
wxString PointToString(const wxPoint& point);

/// \brief 打印一个点对象
void Print(const wxPoint& point, const wxString& desc = wxEmptyString);

/// \brief 将方向属性转换成相应的文本形式（VdkDirection）
wxString DirectionToString(unsigned d);

/// \brief 将对齐属性转换成相应的文本形式（VdkAlignment）
wxString AlignmentToString(unsigned align);

//////////////////////////////////////////////////////////////////////////

/// \brief 获取键盘按键标识符的字符串表示
wxString GetKeyCodeName(int keycode);

#ifdef __WXDEBUG__

	/// \brief 获取 VDK 事件（event）标识符的字符串表示
	const wxChar* GetEventName(int evtCode);
	/// \brief 获取 VDK 通知（notice）标识符的字符串表示
	const wxChar* GetNoticeName(int notice);

#	ifdef __WXMSW__
		/// \brief 获取 Win32 SDK 消息的字符串表示
		const wxChar* wxGetMessageName(int message);
#	endif // __WXMSW__

#endif // __WXDEBUG__

//////////////////////////////////////////////////////////////////////////

/// \brief 多行渐变色文本中的一行
struct GtmTextLine {

	wxString s;
	int x;
};

WX_DECLARE_OBJARRAY( GtmTextLine, ArrayOfGtmTextLines );

/// \brief 创建渐变色文本类
///
/// 用于限制命名空间，避免污染。
struct GradientTextMethod
{
	/// \brief 创建渐变色文本
	static void Draw(wxDC& dc,
					 const wxFont& font,
					 const wxBrush& BgBrush,
					 const wxColour& TextColor,
					 const wxRect& rc,
					 int rowHeight,
					 wxDirection direction,
					 const ArrayOfGtmTextLines& lines);


private:

	// 执行实际绘图操作
	static void DrawText(wxDC& dc, int rowHeight, int y, 
						 const ArrayOfGtmTextLines& lines);
};
