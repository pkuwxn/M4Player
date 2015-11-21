/***************************************************************
 * Name:      Utils.h
 * Purpose:   Common userful functions declactions
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-23
 **************************************************************/
#pragma once

namespace MyTagLib
{

    template< class T >
    void clearPointerContainer(T& cont)
    {
        typename T::const_iterator iter(cont.begin());
        for (; iter != cont.end(); ++iter) {
            delete *iter;
        }

        cont.clear();
    }

    /// Returns true if @a ch is space, tabs, form feed, newline and carriage 
    /// return.
    inline bool isSpace(int ch)
    {
        return (ch == ' ')  || (ch == '\t') || (ch == 0x0C) ||
               (ch == '\r') || (ch == '\n');
    }

    /// Removes white-space (space, tabs, form feed, newline and carriage 
    /// return) from the left or from the right end of the string.
    template< class StrT >
    StrT& trim(StrT& s)
    {
        unsigned i = 0;
        unsigned len = s.length();
        
        while (i < len && isSpace(s[i])) {
            i++;
        }

        if (i) {
            s.erase(0, i);
            len -= i;
        }

        if (s.empty()) {
            return s;
        }

        //-------------------------------------

        i = len - 1;
        while (i >= 0 && isSpace(s[i])) {
            i--;
        }

        if (i < len - 1) {
            s.erase(i + 1);
        }        

        return s;
    }
}

#ifdef _MSC_VER
#    define WCSICMP _wcsicmp
#   define STRICMP _stricmp
#else
#    define WCSICMP wcscasecmp
#    define STRICMP strcasecmp
#endif
