/***************************************************************
 * Name:      Charset.cpp
 * Purpose:   Text charset converter
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-18
 **************************************************************/
#include "StdAfx.h"
#include "Charset.h"

namespace MyTagLib
{
    Charset* Charset::ms_instance = NULL;

    Charset::Charset()
    {
        assert(ms_instance == NULL);
        ms_instance = this;
    }

    /*static*/
    Charset& Charset::getInstance()
    {
        assert(ms_instance);
        return *ms_instance;
    }

    /*static*/
    bool Charset::hasUtf8BOM(const char* data)
    {
        const static unsigned char utf8BOM[] = { 0xEF, 0xBB, 0xBF };
        return memcmp(data, utf8BOM, 3) == 0;
    }

    /*static*/
    bool Charset::hasUtf16LE_BOM(const char* data)
    {
        const static unsigned char utf16LE_BOM[] = { 0xFF, 0xFE };
        return memcmp(data, utf16LE_BOM, 2) == 0;
    }

    /*static*/
    bool Charset::isUtf8(const char*& data, size_t& len)
    {
        if (hasUtf8BOM(data)) {
            data += 3;
            len -= 3;

            return true;
        }
        else {
            // 始终还是不精确
            // TODO: 添加一个 testUtf8ByState 的单元测试
            //return testUtf8ByState(data, len);
            return false;
        }
    }

    // From Notepad2
    /*static*/
    bool Charset::testUtf8ByState(const char* data, size_t len)
    {
        static int byte_class_table[256] = {
            /*       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  */
            /* 00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            /* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            /* 20 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            /* 30 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            /* 40 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            /* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            /* 60 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            /* 70 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            /* 80 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            /* 90 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            /* A0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            /* B0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            /* C0 */ 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            /* D0 */ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            /* E0 */ 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 7, 7,
            /* F0 */ 9,10,10,10,11, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
            /*       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  */
        };

        /* state table */
        typedef enum {
            kSTART = 0, kA, kB, kC, kD, kE, kF, kG, kERROR, kNumOfStates
        } utf8_state;

        static utf8_state state_table[] = {
            /*                            kSTART, kA,     kB,     kC,     kD,     kE,     kF,     kG,     kERROR */
            /* 0x00-0x7F: 0            */ kSTART, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
            /* 0x80-0x8F: 1            */ kERROR, kSTART, kA,     kERROR, kA,     kB,     kERROR, kB,     kERROR,
            /* 0x90-0x9f: 2            */ kERROR, kSTART, kA,     kERROR, kA,     kB,     kB,     kERROR, kERROR,
            /* 0xa0-0xbf: 3            */ kERROR, kSTART, kA,     kA,     kERROR, kB,     kB,     kERROR, kERROR,
            /* 0xc0-0xc1, 0xf5-0xff: 4 */ kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
            /* 0xc2-0xdf: 5            */ kA,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
            /* 0xe0: 6                 */ kC,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
            /* 0xe1-0xec, 0xee-0xef: 7 */ kB,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
            /* 0xed: 8                 */ kD,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
            /* 0xf0: 9                 */ kF,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
            /* 0xf1-0xf3: 10           */ kE,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
            /* 0xf4: 11                */ kG,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR
        };

#        define BYTE_CLASS(b) (byte_class_table[(unsigned char) (b)])
#        define NEXT_STATE(b, cur) (state_table[(BYTE_CLASS(b) * kNumOfStates) + (cur)])

        utf8_state current = kSTART;
        const char* pt = data;

        for (size_t i = 0; (i < len) && *pt; i++, pt++) {
            current = NEXT_STATE(*pt, current);

            if (kERROR == current) {
                break;
            }
        }

        return (current == kSTART) ? true : false;
    }

}
