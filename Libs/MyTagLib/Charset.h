/***************************************************************
 * Name:      Charset.h
 * Purpose:   Text charset converter
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-18
 **************************************************************/
#pragma once
#include "Types.h"

namespace MyTagLib
{

    /// Text charset converter
    class Charset
    {
    public:

        /// Get the singleton object
        static Charset& getInstance();

        /// Virtual dtor
        virtual ~Charset() {}

        /// ANSI -> Unicode
        virtual String multibyteToWide(const char* mb, size_t len) = 0;

        /// UTF16LE -> Native wchar_t string
        virtual String utf16ToWide(const char* utf16, size_t numBytes) = 0;

        /// UTF8 -> Native wchar_t string
        virtual String utf8ToWide(const char* utf8, size_t numBytes) = 0;

        /// Native wchar_t string -> UTF16LE
        ///
        /// The caller should `delete []` the returned pointer.
        /// @return Including the terminated '\0'.
        virtual char* wideToUtf16(const String& wide) = 0;

        /// Native wchar_t string -> UTF8
        ///
        /// The caller should `delete []` the returned pointer.
        /// @return Including the terminated '\0'.
        virtual char* wideToUtf8(const String& wide) = 0;

        /// Native wchar_t string -> ANSI
        ///
        /// The caller should `delete []` the returned pointer.
        /// @return Including the terminated '\0'.
        virtual char* wideToMultibyte(const String& wide) = 0;

    public:

        /// Returns ture if @a data begins with 0xFFFE.
        static bool hasUtf16LE_BOM(const char* data);

        /// Returns ture if @a data begins with 0xEFBBBF.
        static bool hasUtf8BOM(const char* data);

        /// Returns ture if @a data is encoded in UTF-8.
        /// 
        /// @param data The data to test, may begins with the UTF-8 BOM.
        /// @param len The length of the @a data.
        static bool isUtf8(const char*& data, size_t& len);

    private:

        // Returns ture if @a data is encoded in UTF-8.
        // 
        // This function will not test the UTF-8 BOM.
        static bool testUtf8ByState(const char* data, size_t len);
        
    protected:

        /// Can only construct from sub class.
        Charset();

    private:

        Charset(const Charset&) {}
        Charset& operator = (const Charset&) { return *this; }
        Charset* operator & () { return this; }

    private:

        static Charset* ms_instance;
    };

}
