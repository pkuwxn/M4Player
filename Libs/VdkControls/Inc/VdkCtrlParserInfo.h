/***************************************************************
 * Name:      VdkCtrlParserInfo.h
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-23
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkCtrlId.h"
#include <stdio.h>

class VdkWindow;
class VdkControl;
class VdkCtrlParserCallback;

class VdkCtrlIdInfo;
typedef VdkCtrlIdInfo* VdkCtrlIdInfoPtr;

WX_DECLARE_STRING_HASH_MAP( VdkCtrlIdInfoPtr, MapOfCtrlIdInfo );
typedef MapOfCtrlIdInfo::iterator IdInfoIter;

/// \brief XRC ���͹��̵ĵ�����Ϣ
class VdkCtrlParserInfo
{
public:

	/// \brief ���캯��
    VdkCtrlParserInfo();

	/// \brief �����¼��ؼ������ĸ�����
	VdkCtrlParserInfo& window(VdkWindow* win) { Window = win; return *this; }

	/// \brief �����¼��ؼ������ĸ��ؼ�
	VdkCtrlParserInfo& parent(VdkControl* p) { Parent = p; return *this; }

	/// \brief ���� XRC ���ڵ�
	///
	/// ����ֱ���ӽڵ�ӦΪ��Ч�� XRC ���壬����ֱ���ṩ��һ�����ӿؼ��� XRC ���塣
	VdkCtrlParserInfo& node(wxXmlNode* n) { Node = n; return *this; }

	/// \brief ���ý������̵Ļص�����
	VdkCtrlParserInfo& callback(VdkCtrlParserCallback* c) {
		Callback = c; return *this;
	}

	/// \brief ���ÿؼ� ID - ���� ��Ӧͼ
	VdkCtrlParserInfo& ids(MapOfCtrlIdInfo* ids_) { IDs = ids_; return *this; }

private:

	VdkWindow* Window;
	VdkControl* Parent;
	wxXmlNode* Node;
	VdkCtrlParserCallback* Callback;
	MapOfCtrlIdInfo* IDs;

	friend class VdkControl;
};

/// \brief ���� XRC �ļ�ʱ��ÿ���ؼ��Ķ�����Ϣ
///
/// ��ؼ� ID ������ȡ�
class VdkCtrlIdInfo
{
public:

	/// \brief ���캯��
    VdkCtrlIdInfo(VdkCtrlId id, VdkControl** ptr = NULL);

	/// \brief ���ÿؼ� ID
	VdkCtrlIdInfo& id(VdkCtrlId id) { Id = id; return *this; }

	/// \brief ��ȡ�ؼ� ID
	///
	/// VdkWindow ��������ָ�����á�
	VdkCtrlId id() const { return Id; }

	/// \brief ��ȡ�ؼ�����ĵ�ַ
	///
	/// VdkWindow ��������ָ�����á�
	VdkControl** ptr() const { return Ptr; }

	/// \brief ���ÿؼ�����ĵ�ַ
	VdkCtrlIdInfo& ptr(VdkControl** ptr_) { Ptr = ptr_; return *this; }

	/// \brief ��ȡ�ؼ����
	template< class T >
	T* ptr() const { return (T*) *Ptr; }

private:

	VdkCtrlId Id;
	VdkControl** Ptr;

	friend class VdkControl;
};

//////////////////////////////////////////////////////////////////////////
// TODO: ���ע��

#define BindCtrl( name, id, ptr ) \
	BindCtrl_Named( ids, name, id, ptr )

#define BindCtrl_Named( map_, name, id, ptr ) \
	map_[(name)] = new VdkCtrlIdInfo( (id), (VdkControl**) &(ptr) )

#define BindCtrlID( name, id ) BindCtrlID_Named( ids, name, id )
#define BindCtrlID_Named( map_, name, id ) \
	map_[(name)] = new VdkCtrlIdInfo( (id) )
