#pragma once

#if defined(_DEBUG) && defined(_MSC_VER)

#   define _CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   ifndef _CRTBLD
#       define _CRTBLD // Need when builded with pure MS SDK
#   endif
#   include <crtdbg.h>

#   ifndef DEBUG_NEW
#       define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#       define new  DEBUG_NEW
#   endif // DEBUG_NEW

#   define BEGIN_DEBUG \
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#   define ENDD \
    _CrtDumpMemoryLeaks();

#   define MY_ACE_NEW_RETURN(POINTER, CONSTRUCTOR, RET_VAL) \
    do { try { POINTER = new CONSTRUCTOR; } \
    catch (ACE_bad_alloc) { ACE_del_bad_alloc errno = ENOMEM; \
        POINTER = 0; return RET_VAL; } \
    } while (0)

#else // !using VC CRT

#   define BEGIN_DEBUG
#   define ENDD

#endif
