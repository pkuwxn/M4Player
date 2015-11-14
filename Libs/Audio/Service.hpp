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
// ���ṩ�ġ�����
//
////////////////////////////////////////////////////////////
class ServiceBase {
protected :

    /// Default constructor
    ServiceBase() {}

public :

    /// Virtual destructor
    virtual ~ServiceBase() {}

    /// ��ȡ��Ӧ�Ķ��󹤳�ָ��
    FactoryBase *getFactory() const {
        assert(m_factory);
        return m_factory;
    }

    /// �����Ƿ����ĳ������
    bool testAbility(const char *ability);

private :

    FactoryBase *m_factory;
    friend class FactoryBase;

private :

    ServiceBase(const ServiceBase &);
    const ServiceBase &operator = (const ServiceBase &);
};

////////////////////////////////////////////////////////////
// ������󹤳��Ļ���
//
////////////////////////////////////////////////////////////
class FactoryBase {
protected :

    //! Constructor
    //! @param ability �������Ʒ��������ʶ�ַ��������� Decoder/MP3; (��������)��
    //! Decoder/WMA;Decoder/RA; (�������)
    //! @attention ÿ��������ʶ�Ӵ���������һ���ֺ�����������
    //! @attention @a ability ������Ӧ�ó������������ڶ�������Ч��
    FactoryBase(const char *ability)
        : __internal__next(0), m_ability(ability) {
        addToList(this);
    }

    //! Virtual destructor
    virtual ~FactoryBase() {}

public :

    //! ��һ���µĶ��󹤳�ָ����ӵ��ڲ��б���
    static void addToList(FactoryBase *factory);

public :

    //! ��ȡ�����������������ṩ�ķ����������ʶ��
    const char *getAbility() const {
        return m_ability;
    }

    //! ���Թ����������������ṩ�ķ����Ƿ����ĳ������
    bool testAbility(const char *ability);

    //! ���Ҿ���ָ�������Ķ��󹤳���ָ��
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
    //! ϵͳ�Ƿ��Ѿ���ʼ�����
    static bool __internal__sysInitialized;

private:

    const char *m_ability;
};

////////////////////////////////////////////////////////////
// �����������ⲿ���޷�����
//
////////////////////////////////////////////////////////////
class library_not_loaded {};

////////////////////////////////////////////////////////////
// ����һ���򵥶��󹤳�
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
// \brief һ��ͨ�õġ���������
//
////////////////////////////////////////////////////////////
template<class ServiceT>
class ServiceMgr {
public:

    /// һ����������
    typedef ServiceT service_t;

    /// ��������
    ///
    /// \TODO �Ƿ���Ҫ������Ϊһ���麯����
    ~ServiceMgr();

    /// ��ȡ��ǰ��Ľ�����
    ServiceT *get() const;

    /// ����ָ�������
    ServiceT *operator->() const {
        return get();
    }

    /// ���� bool ����ת��������
    operator bool () const;

    /// ��ǰ�ָ���Ƿ���Ч
    bool isOk() const;

    /// ���ڲ�ָ��ָ���һ�����õĽ�����
    void reset() {
        m_active = m_candidates.begin();
    }

    /// ���ڲ�ָ��ָ����һ�����õĽ�����
    bool next();

protected :

    // ���к�ѡ�ġ����񡱶���
    typedef std::vector<ServiceT *> ServiceVec;
    ServiceVec m_candidates;

    // ��ǰ���ڻ�ġ����񡱶���
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
