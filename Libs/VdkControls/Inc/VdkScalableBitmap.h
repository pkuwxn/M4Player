/***************************************************************
 * Name:      VdkScalableBitmap.h
 * Purpose:   VdkScalableBitmap ��������
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-03-11
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkDefs.h" // for VdkResizeableBitmapType

/// \brief ������λͼ��ģ��ʵ��
class VdkScalableBitmap
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkScalableBitmap();

	/// \brief ���캯��
	VdkScalableBitmap(const wxBitmap& bm, 
			  	      VdkResizeableBitmapType resizeType, 
				      const wxRect& rcTile);

	/// \brief ����ͼ������
	void Set(const wxBitmap& bm, 
			 VdkResizeableBitmapType resizeType, 
			 const wxRect& rcTile);

	/// \brief ɾ��λͼ
	void Reset();

	/// \brief ������������Ƿ��ܱ�ԭʼλͼ�����������������ı�λͼ��С
	bool CanResize() const;

	/// \brief λͼ�Ƿ���Ч
	bool IsOk() const { return m_bitmap.IsOk(); }

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��λͼ������ָ���Ĵ�С
	bool Rescale(const wxSize& newsize);

	/// \brief ��ȡλͼ���ź�Ŀ��
	int GetWidth() const { return m_size.x; }

	/// \brief ��ȡλͼ���ź�ĸ߶�
	int GetHeight() const { return m_size.y; }

	/// \brief ��ȡԭʼλͼ�Ĵ�С
	wxSize GetMinSize() const {
		return wxSize( m_bitmap.GetWidth(), m_bitmap.GetHeight() );
	}

	/// \brief ��ȡλͼ���ź�Ĵ�С
	const wxSize& GetSize() const { return m_size; }

	/// \brief ��ȡλͼ��������
	const wxRect& GetTileArea() const { return m_rcTile; }

	/// \brief ��λͼ���Ƶ�ָ�� DC ��(\a x, \a y)��
	void Blit(wxDC& dc, wxCoord x, wxCoord y);

	/// \brief ��λͼָ�������Ƶ�ָ�� DC ��(\a rc.x, \a rc.y)��
	void BlitRect(wxDC& dc, const wxRect& rc);

	//! ����͸������ķ�ʽ
	enum WindowShapeMode {
		WSM_LAYERED, ///< ʹ�ò�δ��ڴ���͸�������� Windows �¿���
		WSM_REGION, ///< ʹ�� wxRegion ����͸������
	};

	/// \brief ʹ�õ�ǰλͼ����ǰ��С����ָ���ǹ��崰�ڵ�������
	void WindowSetShape(wxTopLevelWindow* win, 
						const wxColour& maskColor,
						WindowShapeMode mode) const;

private:

	// ��������λͼ
	void SetBitmap(const wxBitmap& bm);

private:

	wxBitmap m_bitmap; // ԭʼλͼ
	wxBitmap m_rescaled; // ��ǰλͼ

	VdkResizeableBitmapType m_resizeType; // λͼ���ŷ�ʽ
	wxRect m_rcTile; // ��������
	wxRect m_rcTile_o; // ƽ������ʱ�����Ż��Ŀ���������

	wxSize m_size; // ��ǰλͼ��С
};
