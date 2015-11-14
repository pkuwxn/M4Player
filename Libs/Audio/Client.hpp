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

#ifndef SFML_CLIENT_HPP
#define SFML_CLIENT_HPP

namespace sf {
class FactoryBase;

////////////////////////////////////////////////////////////
// OOPlayer 的逻辑客户端（插件）
////////////////////////////////////////////////////////////
class Client {
public:

    /// 获取当前客户端提供的对象工厂列表
    virtual FactoryBase *getFactoryList() = 0;

    /// 设置是否已然初始化所有当前客户端提供的对象工厂
    virtual void setServicesInitialized(bool val) = 0;

protected:

    Client() {}
    virtual ~Client() {}
};

} // namespace sf


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Export.hpp"
#include "Service.hpp"


#define DEFINE_STD_OOPLUGIN(ClassName, Ability) \
namespace { \
\
class ClassName ## Factory : public SimpleFactory<ClassName> { \
public: \
    ClassName ## Factory() \
        : SimpleFactory<ClassName>(Ability) \
    {} \
}; \
\
\
ClassName ## Factory sFactory; \
\
\
class ClientImpl : public sf::Client { \
public: \
\
    virtual sf::FactoryBase* getFactoryList() { \
        return sf::FactoryBase::__internal__list; \
    } \
    \
    virtual void setServicesInitialized(bool val) { \
        sf::FactoryBase::__internal__sysInitialized = val; \
    } \
}; \
\
\
ClientImpl gs_Client; \
extern "C" SFML_API_EXPORT sf::Client* getInterface() { \
    return &gs_Client; \
} \
\
\
} // namespace

#endif // SFML_CLIENT_HPP
