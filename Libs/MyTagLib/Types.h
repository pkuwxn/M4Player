/***************************************************************
 * Name:      Types.h
 * Purpose:   Common types declaration
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-3-18
 **************************************************************/
#pragma once
#include <string>

namespace MyTagLib {
/// We use unicode string everywhere.
typedef std::wstring String;
/// For better readability
extern const wchar_t *EmptyString;
/// Some predifined constants
enum {
    TRACK_NUMBER_NOT_SET = 0, ///< The track number is invalid.
    _1KB = 1 * 1024, ///< A constant for convinience.
    _2KB = 2 * 1024, ///< The default padding size.
};
}
