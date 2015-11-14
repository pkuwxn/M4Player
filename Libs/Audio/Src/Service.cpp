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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "../Service.hpp"
#include <string>
#include <string.h>

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif


namespace sf {

FactoryBase *FactoryBase::__internal__list = NULL;
bool FactoryBase::__internal__sysInitialized = false;


bool ServiceBase::testAbility(const char *ability) {
    return m_factory->testAbility(ability);
}

bool FactoryBase::testAbility(const char *ability) {
    assert(ability);

    std::string ab(ability);
    if (ab[ab.length() - 1] != ';') {
        ab.push_back(';');
    }

    return strstr(m_ability, ab.c_str()) != NULL;
}

/*static*/
FactoryBase *FactoryBase::query(const char *ablity) {
    assert(ablity);

    std::string ab( ablity );
    if (ab[ab.length() - 1] != ';') {
        ab.push_back(';');
    }

    FactoryBase *factory = __internal__list;
    while (factory) {
        if (strstr(factory->m_ability, ab.c_str())) {
            return factory;
        }

        factory = factory->__internal__next;
    }

    return NULL;
}

/*static*/
void FactoryBase::addToList(FactoryBase *factory) {
    assert(!__internal__sysInitialized);

    FactoryBase *beg = factory;
    while (beg->__internal__next) {
        beg = beg->__internal__next;
    }

    beg->__internal__next = __internal__list;
    __internal__list = factory;
}


} // namespace sf
