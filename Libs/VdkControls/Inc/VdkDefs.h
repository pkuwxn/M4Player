/***************************************************************
 * Name:      VdkDefs.h
 * Author:    vanxining (vanxining@139.com)
 * Created:   2010-02-01
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkCommon.h"

/// \brief VDK ����¼�
enum VdkMouseEventType {

	NORMAL,						///< ����״̬
	HOVERING = 1,				///< �����������

	LEFT_DOWN,					///< ����������
	LEFT_UP,					///< �������ͷ�
	DLEFT_DOWN,					///< ������˫��
	RIGHT_UP,					///< ����Ҽ�����
	DRAGGING,					///< ����϶�

	WHEEL_UP,					///< ���������¹���
	WHEEL_DOWN,					///< ���������Ϲ���
	SCROLLING_UP,				///< ��ֱ VdkSlider �����϶�
	SCROLLING_DOWN,				///< ��ֱ VdkSlider �����϶�

	MOUSE_LEAVE,				///< ����뿪�ؼ�������
	MOUSE_LEAVE_WINDOW,			///< ����뿪 VdkWindow ������

	MOUSE_HOLD_ON_RELEASED,		///< �ͷų�����ס��������
};

/// \brief VDK �����¼�
enum VdkKeyEventType {

	KEY_DOWN,	///< ����������
	KEY_UP,	///< ��������
	KEY_CHAR,	///< �ѷ���İ���
};

/// \brief VDK ֪ͨ��Ϣ
enum VdkCtrlNotifyCode {

	//////////////////////////////////////////////////////////////////////////
	// ����������

	VCN_CREATE,
	VCN_DESTROY,
	VCN_BIND_TO_ANOTHER_PARENT, ///< �󶨵�����ĸ�������

	VCN_ID_CHANGED, ///< �ؼ� ID �����ı�

	//////////////////////////////////////////////////////////////////////////
	// ����״̬

	VCN_FONT_CHANGED,
	VCN_SHOW,
	VCN_HIDE,
	VCN_THAW,
	VCN_FREEZE,
	VCN_ENABLED,
	VCN_DISABLED,

	VCN_PARENT_SHOW, /*! ��������ʾ */
	VCN_PARENT_HID, /*! ���������� */

	//////////////////////////////////////////////////////////////////////////
	// ����

	VCN_FOCUS, ///< ��ý���
	VCN_LOST_FOCUS, ///< ʧȥ����

	VCN_MENU_HID, ///< �����ĵ����˵���������

	//////////////////////////////////////////////////////////////////////////
	// �ߴ�

	VCN_POSITION_CHANGED,   ///< SetRect �еĿؼ�λ�øı�
	VCN_SIZE_CHANGED,		///< SetRect �е������С�ı�
	/// \brief VdkWindow ��С�ı�
	/// \note �� VCN_SIZE_CHANGED ��������Ҫ�Ǳ��¼����ܻ��ٴθ��Ŀؼ��Ĵ�С��
	/// ����ظ�
	VCN_WINDOW_RESIZED,
	/// \brief ���ڿ�����ʼ��ı��ˣ���Ϣ����������Ϊ�µĿ�����ʼ��
	/// \note ע�ⲻ��ԭ�㣬���ǻ�ͼ DC ��������ʼ���ꡣ
	VCN_WINDOW_ORIGIN_CHANGED, 
};

/// �ɸı��С��λͼ�Ժ��ַ�ʽ����ƽ�̻������죩
enum VdkResizeableBitmapType {

	RESIZE_TYPE_TILE, ///< ƽ��
	RESIZE_TYPE_STRETCH ///< ����
};

// ��������
enum VdkAlignment {

	vdkALIGN_INVALID		= 0,

	//--------

	vdkALIGN_LEFT			= 1 << 0,
	vdkALIGN_RIGHT			= 1 << 1,
	vdkALIGN_TOP			= 1 << 2,
	vdkALIGN_BOTTOM			= 1 << 3,

	//--------

	vdkALIGN_ALL			= vdkALIGN_LEFT |
							  vdkALIGN_RIGHT |
							  vdkALIGN_TOP |
							  vdkALIGN_BOTTOM,
};

// ��������
enum VdkDirection {

	vdkDIRECTION_INVALID	= 0,

	//--------

	vdkEAST					= 1 << 0,
	vdkSOUTH				= 1 << 1,
	vdkWEST					= 1 << 2,
	vdkNORTH				= 1 << 3,

	//--------

	vdkDIRECTION_ALL		= vdkEAST |
							  vdkSOUTH |
							  vdkWEST |
							  vdkNORTH,
};

/// \brief �ؼ���������
enum AlignType {

	ALIGN_LEFT				= 0,
	ALIGN_RIGHT				= 1 << 0,
	ALIGN_TOP				= 1 << 1,
	ALIGN_BOTTOM			= 1 << 2,
	ALIGN_CENTER_X_Y		= 1 << 3,
	ALIGN_CENTER_X			= 1 << 4,
	ALIGN_CENTER_Y			= 1 << 5,
	/// \brief ��������Ĵ�С�ı�ͬ������ͬ������Ȳ���
	ALIGN_SYNC_X_Y			= 1 << 6,
	ALIGN_SYNC_X			= 1 << 7,
	ALIGN_SYNC_Y			= 1 << 8,

	/// \brief ԭ���ؼ�ר��
	///
	/// ����һ�� VdkNativeCtrl û���κζ�����Ϣ�Ļ������޷������Լ���λ�õġ�\n
	/// ��Ϊû���κζ�����Ϣ���������\link VdkControl::HandleResize \endlink 
	/// ��û���κ����õġ���ˣ��������ֻ��һ�������ֵ��ʹ�� VDK 
	/// ��Ϊ�����ǵ�ԭ���ؼ��Ǳ�Ҫ�ġ�������VdkNativeCtrl ģ�������˳������һ��
	/// VdkNativeCtrl ����� VdkWindow ���Ͻǵľ���λ�á�
	ALIGN_NATIVE_CTRL		= 1 << 9,
};

/// \brief �ı���������
enum VdkTextAlign {

	TEXT_ALIGN_LEFT			= ALIGN_LEFT, ///< �����
	TEXT_ALIGN_RIGHT		= ALIGN_RIGHT,
	TEXT_ALIGN_TOP			= ALIGN_TOP,
	TEXT_ALIGN_BOTTOM		= ALIGN_BOTTOM,
	TEXT_ALIGN_CENTER_X_Y	= ALIGN_CENTER_X_Y,
	TEXT_ALIGN_CENTER_X		= ALIGN_CENTER_X,
	TEXT_ALIGN_CENTER_Y		= ALIGN_CENTER_Y,
};

//////////////////////////////////////////////////////////////////////////

VDK_DECLARE_WX_PTR_LIST( VdkControl, VdkCtrlList, VdkCtrlIter )
VDK_DECLARE_WX_PTR_LIST( VdkWindow, VdkWindowList, VdkWindowIter )
