/***************************************************************
 * Name:      VdkArtProvider.h
 * Purpose:   Code for VdkArtProvider declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-04-07
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include <wx/artprov.h>
#include "VdkBitmapArray.h"

/// \brief �Ի����ı���ؼ�
///
/// ��ܼ����鷳���Ի��ؼ���ԭ���ؼ����ʹ��ʱ�Ľ���ת�����⡣
class VdkArtProvider : public wxArtProvider
{
public:

	/// \brief ���캯��
	VdkArtProvider();

	/// \brief ��ȡָ��λ���ϵ�Сλͼ
	VdkBitmapArrayId GetBitmaplet(int x, int y);

protected:

	/// \brief ��ȡָ��λͼ
	wxBitmap CreateBitmap(const wxArtID& id,
						  const wxArtClient& client,
						  const wxSize& size);

private:

	VdkBitmapArray m_ba;
};
