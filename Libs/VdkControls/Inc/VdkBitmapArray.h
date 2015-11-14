/***************************************************************
 * Name:      VdkBitmapArray.h
 * Purpose:   VdkBitmapArray ��ͷ�ļ�
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-01-10
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkDefs.h"

/// \brief λͼ����Ĺ��������
///
/// ��д�����Ŀ���Ǳ������ɹ���Сλͼ�Խ�ʡ��Դ��
class VdkBitmapArray
{
public:

	/// \brief ���캯��
	VdkBitmapArray();

	/// \brief ���캯��
	/// \param strPath λͼ���ڵĴ���·��
	/// \param cols �ָ�ķ���
	VdkBitmapArray(const wxString& strPath, unsigned cols, unsigned rows);

	/// \brief ���캯��
	/// \param bm ����λͼ
	/// \param cols �ָ�ķ���
	VdkBitmapArray(const wxBitmap& bm, unsigned cols, unsigned rows) {
		Set( bm, cols, rows );
	}

	/// \brief ���ƹ��캯��
	VdkBitmapArray(const VdkBitmapArray& other);

	/// \brief ���Ƹ�ֵ����
	VdkBitmapArray& operator = (const VdkBitmapArray& rhs);

	/// \brief ����һ��������
	void Assign(const VdkBitmapArray& rhs);

	/// \brief �����Ƿ���Ȼ��ʼ��/λͼ����
	bool IsOk() const { return m_bmp.IsOk(); }

	/// \brief ������������λͼ
	/// \param bm ����λͼ
	/// \param cols �ָ�ķ���
	void Set(const wxBitmap& bm, unsigned cols, unsigned rows);

	/// \brief ������������λͼ
	/// \param strPath λͼ���ڵĴ���·��
	/// \param cols �ָ�ķ���
	void Set(const wxString& strPath, unsigned cols, unsigned rows);

	/// \brief ��(\a row, \a col )����Сλͼ���Ƶ� DC ��(\a x, \a y)��
	void BlitTo(wxDC& dc, unsigned col, unsigned row, wxCoord x, wxCoord y);

	/// \brief ��������λͼ��С
	/// 
	/// ��ǰֻ֧�ָ���ֻ��һ�е�λͼ����Ĵ�С��
	bool Resize(int newwidth, int newheight, 
		VdkResizeableBitmapType type, const wxRect& rcTile);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡԭʼλͼ
	wxBitmap GetRawBitmap() const { return m_bmp; }

	/// \brief ��ȡ���������ϵ�λͼ����
	wxBitmap* Explode() const;

	/// \brief �õ�Сλͼ�Ŀ��
	unsigned GetWidth() const;

	/// \brief �õ�Сλͼ�ĸ߶�
	unsigned GetHeight() const;

	/// \brief �õ��ָ�ķ���
	unsigned GetExplNum() const { return m_cols; }

	/// \brief �õ�����
	unsigned GetRows() const { return m_rows; }

	/// \brief �õ�����
	unsigned GetColumns() const { return m_cols; }

private:

	wxBitmap m_bmp;
	unsigned m_cols, m_rows;
};

//////////////////////////////////////////////////////////////////////////

/// \brief λͼ�����ָ��Сλͼ��ʹ֮���� wxBitmap ����������
class VdkBitmapArrayId
{
public:

	/// \brief Ĭ�Ϲ��캯��������һ���հ�αλͼ
	VdkBitmapArrayId() : m_ba( NULL ), m_x( -1 ), m_y( -1 ) {}

	/// \brief ���캯��
	/// \param ba Сλͼ������λͼ����
	/// \param index Сλͼ�����
	VdkBitmapArrayId(VdkBitmapArray& ba, int x, int y)
		: m_ba( &ba ), m_x( x ), m_y( y ) {}

	/// \brief ���Ƹ�ֵ����
	VdkBitmapArrayId& operator = (const VdkBitmapArrayId& another)
	{
		m_ba = another.m_ba;
		m_x = another.m_x;
		m_y = another.m_y;

		return *this;
	}

	/// \brief ��Сλͼ���Ƶ�ָ�� DC ��(\a x, \a y)��
	void BlitTo(wxDC& dc, wxCoord x, wxCoord y) const;

	/// \brief Сλͼ�Ƿ���Ч�����Ա�����
	bool IsOk() const {

		return m_ba && 
			   m_ba->IsOk() &&
			   m_x >=0 && 
			   m_x < static_cast< int >( m_ba->GetColumns() ) &&
			   m_y >=0 && 
			   m_y < static_cast< int >( m_ba->GetRows() );
	}

private:

	VdkBitmapArray* m_ba;
	int m_x, m_y;
};
