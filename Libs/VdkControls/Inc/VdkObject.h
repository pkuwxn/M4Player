#pragma once

class VdkObject;

typedef VdkObject* ( *VdkObjectCreator )();
WX_DECLARE_STRING_HASH_MAP( VdkObjectCreator, OBJECT_MAP );

/// \brief 所有可以被动态创建的类的基类
///
/// 本类代表一个类的动态创建属性
class VdkObject
{
public:

	/// \brief 从类名获取一个对应类的对象
	static VdkObject* CreateDynamicVObject(const wxString& strClassName);

protected:

	/// \brief 保证“类名-动态创建对象构建函数”图结构在使用前已被正确初始化
	static OBJECT_MAP& GetCreators();

	friend class AddToHashMap;
};

/// \brief 动态创建对象构建函数信息自动化对象
///
/// 在构造函数中将指定的动态创建对象构建函数添加到\n
/// “类名-动态创建对象构建函数”图结构中。
class AddToHashMap
{
public:

	/// \brief 构造函数
	/// \param strClassName 类名
	/// \param Creator \a strClassName 所属的对象构建函数
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
					   L"错误的克隆实体" ); \
		\
		ClassName* oo = (ClassName *) o; \
		BaseClassName::Clone( o, parent ); \
		Clone( oo ); \
	}
