/***************************************************************
 * Name:      VdkWindowImpl.h
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-13
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkWindow.h"
#include "VdkControl.h"

#include <wx/display.h>
#include <wx/xml/xml.h>

/// \brief VdkWindow ��������
///
/// ʹ VdkWindow ��������ִ������ͣ��� wxDialog��wxWindow��wxPanel �ȡ�
template< class T >
class VdkWindowImpl: public T, public VdkWindow
{
public:

	/// \brief ���캯��
	VdkWindowImpl(long style = 0)
		: VdkWindow( this, style )
	{

	}
};

//////////////////////////////////////////////////////////////////////////

/// \brief VdkWindow ���������п�����Ϊ���㴰�ڵ�������
///
/// ���㴰��ֱ���봰�ڹ��������н�������������С������󻯵ȡ�
template< class T >
class VdkTopLevelWindow : public VdkWindowImpl< T >
{
public:

	/// \brief ���캯��
	VdkTopLevelWindow(wxWindow* parent, 
					  const wxString& title, 
					  long style, 
					  long vstyle);

	/// \brief XRC ��̬�����ı�������
	virtual bool FromXrc(const wxString& xrcFileName, MapOfCtrlIdInfo* ids = NULL);

	/// \brief ���봰������С��Ϊ����
	virtual wxSize GetMaxSize() const;

private:

	// ����������ڴ˶Դ��� XRC ������һЩ���⴦��
	virtual void HackXrc(wxXmlNode* winRoot) {}

	// �ı䴰�ڴ�С
	virtual void DoResize(int w, int h, int width, int height, 
						  int sizeFlags = wxSIZE_AUTO);

	// ��С��
	virtual void DoMinimize() { this->Iconize( true ); }
};

template< class T >
VdkTopLevelWindow< T >::VdkTopLevelWindow(wxWindow* parent,
										  const wxString& title,
										  long style,
										  long vstyle)
					  : VdkWindowImpl< T >( vstyle )
{
	T::Create( parent, wxID_ANY, title, wxDefaultPosition,
			   wxDefaultSize,
			   style |
			   wxFULL_REPAINT_ON_RESIZE |
			   wxSYSTEM_MENU |
			   wxMAXIMIZE_BOX |
			   wxMINIMIZE_BOX |
			   wxCLOSE_BOX |
			   wxFRAME_SHAPED |
			   wxBORDER_NONE |
			   wxCLIP_CHILDREN/* |
			   wxCLIP_SIBLINGS */); // �����������ֱ߿�

#ifdef __WXDEBUG__
	this->SetDebugCaption(title);
#endif // __WXDEBUG__
}

template< class T >
bool VdkTopLevelWindow< T >::FromXrc(const wxString& xrcFileName, 
									 MapOfCtrlIdInfo* ids)
{
	wxString xrcPath( VdkControl::GetFilePath( xrcFileName ) );
	wxXmlDocument docSkin( xrcPath );
	wxXmlNode* root = docSkin.GetRoot();
	if( (root == NULL) || (root->GetName() != L"resource") )
	{
		wxLogError( L"Ƥ�� XML �ļ������ڻ��߸�ʽ����:\r\n%s", xrcPath );
		return false;
	}

	//-----------------------------------------------

	wxXmlNode* winRoot = root->GetChildren();

	HackXrc( winRoot );
	VdkWindow::Create( winRoot, ids );

	return true;
}

template< class T >
wxSize VdkTopLevelWindow< T >::GetMaxSize() const
{
	return wxDisplay().GetClientArea().GetSize();
}

template< class T >
void VdkTopLevelWindow<T>::DoResize(int x, int y, int w, int h, int sizeFlags)
{
	// ��������ͬ�Ĵ�С��ֱ�� SetShape
	this->SetSize( x, y, w, h, sizeFlags );

	if( this->IsMaximized() )
	{
		// TODO: �������ʱ�����
		this->SetShape( wxRegion() );
	}
	else
	{
		this->m_bkCanvas.WindowSetShape
			( this, VdkControl::GetMaskColour(), VdkScalableBitmap::WSM_LAYERED );
	}
}

//////////////////////////////////////////////////////////////////////////

#define DEFINE_USABLE_WINDOW_CLASS( T ) \
class Vdk ## T : public VdkTopLevelWindow< wx ## T > \
{ \
public: \
\
	Vdk ## T(wxWindow* parent, \
			 const wxString& title, \
			 long style = 0, \
			 long vstyle = VWS_DRAGGABLE) \
		: VdkTopLevelWindow< wx ## T >( parent, title, style, vstyle ) \
	{} \
};

DEFINE_USABLE_WINDOW_CLASS( Dialog )
DEFINE_USABLE_WINDOW_CLASS( Frame )
