/***************************************************************
 * Name:      VdkUtil.h
 * Purpose:   VdkUtil ��ͷ�ļ�
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-02-15
 * Copyright: vanxining
 **************************************************************/
#pragma once

class wxXmlNode;

//////////////////////////////////////////////////////////////////////////

typedef unsigned long align_type;

/// \brief ����һ���������� wxRect ���ַ������� Windows ��ʽ���� wxWidgets ��ʽ�����ֵ�
enum RectType {

	PRT_POSITION,	///< Windows ��ʽ���ĸ����ֵĺ���λ�����˾��ε����½ǵ������
	PRT_RECTANGLE	///< wxWidgets ��ʽ���ĸ����ֵĺ���λ�����˾��εĳ��Ϳ�
};

/// \brief ��һ�� XRC ��������ȡ�ı�����
struct TextInfo {

	wxFont			font;			///< ����
	wxColour		background;		///< �ı�ǰ��ɫ
	wxColour		foreground;		///< �ı�����ɫ

	int				xPadding;		///< �ı�����������ߵľ���
	int				yPadding;		///< �ı����������ұߵľ���
};

/// \brief ��һ�� XRC ��������ȡ��������
///
/// ��ǰ��ҪӦ���� VdkPanel �ı���ɫ�ͱ߿�Ķ���
struct BackgroundInfo {

	wxColour		bgColor;
	wxPen			borderPen;
};

/// \brief ��һ�� XRC ��������ȡ����ɫ����������
///
/// ��ǰ��ҪӦ���� VdkPanel �ı���ɫ
struct GradientBgInfo {

	wxColour		beg;			///< ��ʼɫ
	wxColour		end;			///< ��ֹɫ
	wxDirection		direction;		///< ���򣨶��������е�һ����
};

//////////////////////////////////////////////////////////////////////////
/// \brief �����ౣ��ķ�����Լ�������ʱ���Լ�
class VdkStyleAndStateOwner
{
public:

	/// \brief ���캯��
	VdkStyleAndStateOwner(long style = 0, long state = 0)
		: m_style( style ), m_state( state )
	{

	}

	/// \brief ��ȡ��ǰ�ؼ������ԣ���񣩼�
	long GetStyle() const { return m_style; }

	/// \brief ���õ�ǰ�ؼ������ԣ���񣩼�
	void SetStyle(long style) { m_style = style; }

	/// \brief ��ָ��ֵ�뵱ǰ������Լ����
	void SetAddinStyle(long style) { m_style |= style; }

	/// \brief �Ƴ���ǰ������Լ���ָ������\a style
	void RemoveStyle(long style) { m_style &= ~style; }

	/// \brief ȡ����ǰ������Լ��е�ָ������\a style
	void ReverseStyle(long style) { m_style ^= style; }

	/// \brief ���Ե�ǰ������Լ����Ƿ�������ָ������\a style
	bool TestStyle(long style) const { return (m_style & style) != 0; }

	/// \brief ��ȡ��ǰ���ڵ�����ʱ״̬���Լ�
	long GetState() const { return m_state; }

	/// \brief ���õ�ǰ���ڵ�����ʱ״̬���Լ�
	void SetState(long state) { m_state = state; }

	/// \brief ��ָ��ֵ�뵱ǰ����ʱ״̬���Լ����
	void SetAddinState(long state) { m_state |= state; }

	/// \brief �Ƴ���ǰ����ʱ״̬���Լ��е�ָ������\a state
	void RemoveState(long state) { m_state &= ~state; }

	/// \brief ȡ����ǰ����ʱ״̬���Լ��е�ָ������\a state
	void ReverseState(long state) { m_state ^= state; }

	/// \brief ���Ե�ǰ����ʱ״̬���Լ����Ƿ�������ָ������\a state
	///
	/// \param state ������ X | Y ���ֻ�ֵ��������ΪֻҪ X �� Y ������һλ��
	/// ���ü������档
	bool TestState(long state) const { return (m_state & state) != 0; }

private:

	long			m_style; ///< �ؼ����
	long			m_state; ///< �ؼ�����ʱ���Լ����������ʹ�ò�������
};

//////////////////////////////////////////////////////////////////////////
/// \brief �����ౣ���һ��������صġ�VDK ��֤������֮���õ�ָ��
class VdkClientDataOwner
{
public:

	/// \brief ���캯��
	VdkClientDataOwner() : m_clientData( NULL ) {}

	/// \brief ��ȡ�����һ���û�ָ���Ŀ�ָ��
	void* GetClientData() const { return m_clientData; }

	/// \brief ���ÿؼ������һ���û��Զ���Ŀ�ָ��
	VdkClientDataOwner* SetClientData(void* clientData) {
		m_clientData = clientData;
		return this;
	}

	/// \brief ǿ��ת��Ϊ������
	template< class DD >
	DD& castTo() { return static_cast< DD& >( *this ); }

private:

	void* m_clientData;
};

//////////////////////////////////////////////////////////////////////////
/// \brief һ����̬�����࣬����������������
class VdkUtil
{
public:

	/// \brief ��ָ�� DC ��ĳ������ĳˢ�����
	static void ClrBkGnd(wxDC& dc, const wxBrush& bgBrush, 
						 const wxRect& rc);

	/// \brief ��ָ�� DC ��ĳ���򱣴浽������
	static void SaveDC(wxDC& dc, const wxRect& rc, 
					   const wxChar* szSubfix = NULL);

	/// \brief �����¼���ʶ���Ƿ��������������¼�
	static bool IsLeftButtonEvent(int evtCode);

	//---------------------------------------------------

	/// \brief ��ȡ��ǰ���̿ռ�Ŀؼ�Ƥ������ļ��ĸ�Ŀ¼
	static wxString GetFilePath(const wxString& strFileName);

    /// \brief �� XRC �ļ��н��� <position> ���
    /// \see GetXrcRect
	static wxRect ParseRect(wxXmlNode* xmlNode);

	/// \brief ֱ�Ӵ��ַ���\a strRect �н�����һ�� wxRect
	/// \param nType ����ȡֵ: \n PRT_RECTANGLE wxWidgets ��ʽ�� wxRect \n
	/// PRT_POSITION Windows SDK ��ʽ
	/// \see RectType
	static wxRect ParseRect(const wxString& strRect, RectType nType);

    /// \brief ��λͼƽ���ָ��\a nExpl ����
	static wxBitmap* ExplodeBitmap(const wxBitmap& bmOld, int nExpl);

	/// \brief ��֪��Ϊʲô��ʱ�� wxBitmap::GetSubBitmap ��ò�����ȷ�Ľ��
	static wxBitmap GetSubBitmapWithoutMask
		(const wxBitmap& bmp, const wxRect& subRegion);

	/// \brief ���ļ�·��\a path ������λͼ
	///
	/// ����λͼ�����һЩ��Ҫ�ĳ�ʼ��������������͸��ɫ�ȡ�
	/// ������Ӧ��֤\a path �ַ���ǰ�󲻺��ո�
	static bool LoadMaskBitmap(wxBitmap& bitmap, const wxString& path);

	/// \brief Ϊλͼ��������λͼ
	static void CreateMask(wxBitmap& bitmap, const wxColour* pMaskColor = NULL);

	/// \brief ���ļ�·��\a path ��������ָ�ʽ��ͼƬ
	///
	/// ֧�ֵ��ļ���ʽ�� .BMP��.PNG��.JPG/.JPEG��.GIF��.ICO �ȡ�
	/// ������Ӧ��֤\a path �ַ���ǰ�󲻺��ո�
	/// ���� .ICO (Windows ͼ��) �ļ���ֻ������ 16 * 16 ��С����ͼ��
	/// ���� .BMP (Windows λͼ) �ļ��������Զ�����͸��λͼ��
	static bool ImRead(wxBitmap& bm, const wxString& path);

	/// \brief �Ӷ����ƴ��������� PNG ͼƬ
	static wxBitmap LoadPngFromRawData(const unsigned char* raw, int size);

	/// \brief ƽ��λͼ
	///
	/// ����ͬʱָ���µĸ߶ȺͿ�ȡ�Ҳ��ָֻ������һ����������һ��ʵ����Ϊ 0 ��
	/// \param mask �Ƿ�ʹ������λͼ���и���
	/// \see StretchBitmap
	static wxBitmap TileBitmap(wxBitmap& bm,
							   int w, int h,
							   const wxRect& rcTile,
							   bool mask = true);

	/// \brief ƽ�̻���λͼ
	///
	/// ����ͬʱָ���µĸ߶ȺͿ�ȡ�Ҳ��ָֻ������һ����������һ��ʵ����Ϊ 0 ��
	/// ��������Ϊ�˲�����̫�಻��Ҫ��λͼ������������
	/// \see StretchBitmap, TileBitmap
	static void TileDrawBitmap(wxDC& dc,
							   wxBitmap& bm,
							   int x,
							   int y,
							   int w,
							   int h,
							   const wxRect& rcTile);

	/// \brief ����λͼ
	/// \param dcsrc ԭʼ DC
	/// \param dcdest Ŀ�� DC
	/// \param src ԭʼ����
	/// \param dest Ŀ������ָ�����ʹ�С��
	/// \param bmTmp �����η�����ʱλͼ�Ŀ���
	static void StretchBlit(wxDC& dcdest,
							const wxRect& src,
							wxDC& dcsrc,
							const wxRect& dest,
							wxBitmap& bmTmp);

	/// \brief ���м���չλͼ
	///
	/// ����ͬʱָ���µĸ߶ȺͿ�ȡ����������ڵ�����������λͼ��
	/// ʹ�߽粻���Σ����� VdkWindow �ı���λͼ��Ϊ��Ҫ����
	/// \see InflateDrawByCenter
	static wxBitmap InflateBitmapByCenter(wxBitmap& bm,
									      const wxRect& center,
									      const wxSize& szDest);

	/// \brief ���м���չλͼ
	///
	/// ����ͬʱָ���µĸ߶ȺͿ�ȡ����������ڵ�����������λͼ��
	/// ʹ�߽粻���Σ����� VdkWindow �ı���λͼ��Ϊ��Ҫ����
	/// \see StretchBitmap
	static void InflateDrawByCenter(wxBitmap& bm,
									wxDC& dc, 
									const wxSize& newsize, 
									const wxRect& center);

    /// \brief �� XRC �ļ��н��� <align> ���(������ָ������Ϊ\a strNodeName 
    /// �Ľ��)
	static align_type GetXrcAlign(wxXmlNode* xmlNode,
								  wxString strNodeName = wxEmptyString);

    /// \brief �� XRC �ļ��н��� <text-align> ���
	static align_type GetXrcTextAlign(wxXmlNode* xmlNode)
	{
		return GetXrcAlign( xmlNode, L"text-align" );
	}

	/// \brief �� XRC �ļ��н��� <rect> ���
	///
	/// ������ֻ�ܽ��� wxWidgets ��ʽ�� <rect> ��㡣
	/// ����Ҫ���� Windows ��ʽ��ʾ�Ľ�㣨ͬʱ�����ӦΪ position ����
	/// ��ʹ��\link ParseRect \endlink ������
	/// \see ParseRect
	static wxRect GetXrcRect(wxXmlNode* xmlNode);

	/// \brief �� XRC �ļ��н��� <name> ���
	static wxString GetXrcName(wxXmlNode* xmlNode);

	/// \brief �� XRC �ļ��н���������\a xmlNode ���׸����ı��ӽ��
	///
	/// ����ڵ������һ�����ı���㡣VDK ��ȥ���õ��ı�����β�ո�
	static wxString GetXrcTextBlock(wxXmlNode* xmlNode);

	/// \brief �� XRC �ļ��н��� <image> ���
	///
	/// ��ʱ <image> ���ָ�����ļ���ʽ������ .BMP��.PNG��.JPG/.JPEG��
	/// .GIF ���ָ�ʽ��
	/// ����� VdkControl �ڱ����͸��ɫ������Ӧ��λͼ�������
	static bool GetXrcImage(wxXmlNode* xmlNode, wxBitmap& bm);

	/// \brief �� XRC �ļ��н������ı�������ص����ԡ����ֺš����塢��ɫ�ȵȡ�
	/// \param window ���� XRC �ļ������������Ϣ����ô��ڵ������Ϣ�ɹ��ο�
	static TextInfo GetXrcTextInfo(wxXmlNode* xmlNode, 
								   wxWindow* window = NULL);

	/// \brief �� XRC �ļ��н��� <gradient-bg> ��㣨����ɫ������
	static GradientBgInfo* GetXrcGradientBgInfo(wxXmlNode* xmlNode);

	/// \brief �� XRC �ļ��н��� <bg> ��㣨�������壩
	static BackgroundInfo* GetXrcBgInfo(wxXmlNode* xmlNode);

private:

	/// \brief �� X �᷽��ƽ�̻���
	static void TileDraw_X(const wxRect& rcSrc, 
						   wxDC& dcSrc, 
						   const wxRect& rcDest, 
						   wxDC& dcDest, 
						   bool mask = false);

	/// \brief �� Y �᷽��ƽ�̻���
	static void TileDraw_Y(const wxRect& rcSrc, 
						   wxDC& dcSrc, 
						   const wxRect& rcDest, 
						   wxDC& dcDest, 
						   bool mask = false);
};

#define VdkLoadPngFromRawData( RawData ) \
	VdkUtil::LoadPngFromRawData( RawData, sizeof( RawData ) )
