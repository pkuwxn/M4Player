#pragma once

class VdkObject;

typedef VdkObject* ( *VdkObjectCreator )();
WX_DECLARE_STRING_HASH_MAP( VdkObjectCreator, OBJECT_MAP );

/// \brief ���п��Ա���̬��������Ļ���
///
/// �������һ����Ķ�̬��������
class VdkObject
{
public:

	/// \brief ��������ȡһ����Ӧ��Ķ���
	static VdkObject* CreateDynamicVObject(const wxString& strClassName);

protected:

	/// \brief ��֤������-��̬�������󹹽�������ͼ�ṹ��ʹ��ǰ�ѱ���ȷ��ʼ��
	static OBJECT_MAP& GetCreators();

	friend class AddToHashMap;
};

/// \brief ��̬�������󹹽�������Ϣ�Զ�������
///
/// �ڹ��캯���н�ָ���Ķ�̬�������󹹽�������ӵ�\n
/// ������-��̬�������󹹽�������ͼ�ṹ�С�
class AddToHashMap
{
public:

	/// \brief ���캯��
	/// \param strClassName ����
	/// \param Creator \a strClassName �����Ķ��󹹽�����
	AddToHashMap(const wxString& strClassName, VdkObjectCreator Creator);
};

#define DECLARE_DYNAMIC_VOBJECT \
	public: \
		static VdkObject* CreateVdkObject(); \
		static AddToHashMap	m_HashMapAdder;

#define DECLARE_CLONEABLE_VOBJECT( ClassName ) \
	DECLARE_DYNAMIC_VOBJECT \
		\
		virtual ClassName* GetImitation() const { return new ClassName; } \
		\
		virtual void Clone(VdkControl* o, VdkControl* parent); \
		\
	private:\
		\
		void Clone(ClassName* o);

#define IMPLEMENT_DYNAMIC_VOBJECT( ClassName ) \
	\
	AddToHashMap ClassName::m_HashMapAdder \
		( L ## #ClassName, (VdkObjectCreator) ClassName::CreateVdkObject ); \
	\
	VdkObject* ClassName::CreateVdkObject() { return new ClassName; }

#define IMPLEMENT_CLONEABLE_VOBJECT( ClassName, BaseClassName ) \
	\
	AddToHashMap ClassName::m_HashMapAdder( L ## #ClassName, \
		(VdkObjectCreator) ClassName::CreateVdkObject ); \
	\
	VdkObject* ClassName::CreateVdkObject() { return new ClassName; } \
	\
	void ClassName::Clone(VdkControl* o, VdkControl* parent) \
	{ \
		wxASSERT_MSG( VdkControl::CheckSourceCtrl< ClassName >( o ), \
					   L"����Ŀ�¡ʵ��" ); \
		\
		ClassName* oo = (ClassName *) o; \
		BaseClassName::Clone( o, parent ); \
		Clone( oo ); \
	}
