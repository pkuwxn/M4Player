////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2012 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef SFML_SERVICE_HPP
#define SFML_SERVICE_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include <assert.h>

namespace sf {
class FactoryBase;

////////////////////////////////////////////////////////////
// 可提供的“服务”
//
////////////////////////////////////////////////////////////
class ServiceBase {
protected :

    /// Default constructor
    ServiceBase() {}

public :

    /// Virtual destructor
    virtual ~ServiceBase() {}

    /// 获取相应的对象工厂指针
    FactoryBase *getFactory() const {
        assert(m_factory);
        return m_factory;
    }

    /// 测试是否具有某种能力
    bool testAbility(const char *ability);

private :

    FactoryBase *m_factory;
    friend class FactoryBase;

private :

    ServiceBase(const ServiceBase &);
    const ServiceBase &operator = (const ServiceBase &);
};

////////////////////////////////////////////////////////////
// 定义对象工厂的基类
//
////////////////////////////////////////////////////////////
class FactoryBase {
protected :

    //! Constructor
    //! @param ability 工厂类产品的能力标识字符串，例如 Decoder/MP3; (单个能力)、
    //! Decoder/WMA;Decoder/RA; (多个能力)
    //! @attention 每个能力标识子串后必须跟着一个分号用以区隔。
    //! @attention @a ability 必须在应用程序生命周期内都保持有效！
    FactoryBase(const char *ability)
        : __internal__next(0), m_ability(ability) {
        addToList(this);
    }

    //! Virtual destructor
    virtual ~FactoryBase() {}

public :

    //! 将一个新的对象工厂指针添加到内部列表中
    static void addToList(FactoryBase *factory);

public :

    //! 获取工厂所能生产对象提供的服务的能力标识符
    const char *getAbility() const {
        return m_ability;
    }

    //! 测试工厂所能生产对象提供的服务是否具有某种能力
    bool testAbility(const char *ability);

    //! 查找具有指定能力的对象工厂的指针
    static FactoryBase *query(const char *ablity);

    //! Throws std::bad_alloc or another exception on failure.
    ServiceBase *create() {
        ServiceBase *instance = doCreate();
        if (instance) {
            instance->m_factory = this;
        }

        return instance;
    }

    //! Throws std::bad_alloc or another exception on failure.
    virtual void destory(ServiceBase *service) = 0;

private:

    virtual ServiceBase *doCreate() = 0;

public :

    //! FOR INTERNAL USE ONLY
    static FactoryBase *__internal__list;

    //! FOR INTERNAL USE ONLY
    FactoryBase *__internal__next;

    //! FOR INTERNAL USE ONLY
    //! 系统是否已经初始化完毕
    static bool __internal__sysInitialized;

private:

    const char *m_ability;
};

////////////////////////////////////////////////////////////
// 服务依赖的外部库无法载入
//
////////////////////////////////////////////////////////////
class library_not_loaded {};

////////////////////////////////////////////////////////////
// 定义一个简单对象工厂
//
////////////////////////////////////////////////////////////
template<class T>
class SimpleFactory : public FactoryBase {
public:

    //! Constructor
    SimpleFactory(const char *ability)
        : FactoryBase(ability) {

    }

    //! Throws std::bad_alloc or another exception on failure.
    virtual void destory(ServiceBase *service) {
        delete service;
    }

private:

    //! Throws std::bad_alloc or another exception on failure.
    virtual ServiceBase *doCreate() {
        return new T;
    }
};


////////////////////////////////////////////////////////////
// \brief 一个通用的“服务”容器
//
////////////////////////////////////////////////////////////
template<class ServiceT>
class ServiceMgr {
public:

    /// 一个便利定义
    typedef ServiceT service_t;

    /// 析构函数
    ///
    /// \TODO 是否需要将其作为一个虚函数？
    ~ServiceMgr();

    /// 获取当前活动的解码器
    ServiceT *get() const;

    /// 重载指针操作符
    ServiceT *operator->() const {
        return get();
    }

    /// 重载 bool 类型转换操作符
    operator bool () const;

    /// 当前活动指针是否有效
    bool isOk() const;

    /// 将内部指针指向第一个可用的解码器
    void reset() {
        m_active = m_candidates.begin();
    }

    /// 将内部指针指向下一个可用的解码器
    bool next();

protected :

    // 所有候选的“服务”对象
    typedef std::vector<ServiceT *> ServiceVec;
    ServiceVec m_candidates;

    // 当前正在活动的“服务”对象
    typename ServiceVec::iterator m_active;
};


////////////////////////////////////////////////////////////
template<class ServiceT>
ServiceMgr<ServiceT>::~ServiceMgr() {
    typename ServiceVec::iterator iter(m_candidates.begin());
    for (; iter != m_candidates.end(); ++iter) {
        (*iter)->getFactory()->destory(*iter);
    }
}


////////////////////////////////////////////////////////////
template<class ServiceT>
ServiceMgr<ServiceT>::operator bool() const {
    return isOk();
}


////////////////////////////////////////////////////////////
template<class ServiceT>
bool sf::ServiceMgr<ServiceT>::isOk() const {
    return !m_candidates.empty() && (m_active != m_candidates.end());
}


////////////////////////////////////////////////////////////
template<class ServiceT>
ServiceT *ServiceMgr<ServiceT>::get() const {
    if (*this) {
        return *m_active;
    }

    return 0;
}


////////////////////////////////////////////////////////////
template<class ServiceT>
bool ServiceMgr<ServiceT>::next() {
    if (*this) {
        ++m_active;
        return true;
    }

    return false;
}

} // namespace sf

#endif // SFML_SERVICE_HPP
