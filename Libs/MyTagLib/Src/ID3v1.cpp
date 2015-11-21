/***************************************************************
 * Name:      ID3v1.cpp
 * Purpose:   Implement the ID3 v1/v1.1 tagging format
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-19
 **************************************************************/
#include "StdAfx.h"
#include "ID3v1.h"

#include "Charset.h"

namespace MyTagLib
{

    ID3v1::ID3v1()
    {
        getRange().setOrder(Range::ORDER_FILE_END);
    }

    void ID3v1::doClear()
    {
        memset(m_data, 0, TAG_DATA_SIZE);

        // The serial number 0 is of meaning.
        doSetGenre(Genre::GENRE_NOT_SET);
    }

    bool ID3v1::doLoad(std::istream& istrm)
    {
        istrm.seekg(- TAG_DATA_SIZE, std::ios::end);
        istrm.read(m_data, TAG_DATA_SIZE);

        if (!isOk()) {
            return false;
        }

        getRange().assign((int) istrm.tellg() - TAG_DATA_SIZE, TAG_DATA_SIZE);
        return true;
    }

    Tag::WriteResult ID3v1::doSave(std::ostream& ostrm)
    {
        ostrm.write(m_data, TAG_DATA_SIZE);
        return WR_OK;
    }

    bool ID3v1::doInit()
    {
        memcpy(m_data, "TAG", 3);

        getRange().resize(128);
        return true;
    }

    /// Returns true if the tag has been read successfully.
    bool ID3v1::isOk() const
    {
        return strncmp(m_data, "TAG", 3) == 0;
    }

    // 有时中文会被截断(2字节只保存了1字节，最后半个双字节是无效的)
    size_t strip(const char* s, int maxlen)
    {
        bool consistent = true;
        const char* p = s;
        while (((p - s) < maxlen) && *p) {

            if(*p < 0) {
                consistent = !consistent;
            }

            p++;
        }

        return consistent ? (p - s) : (p - s - 1);
    }

    String ID3v1::getString(int fieldOffset, size_t maxFiledSize) const
    {
        Charset& stringcvt = Charset::getInstance();
        const char* dataPtr = m_data + fieldOffset;
        
        // Sometimes tags will be encoded in UTF-8 (very rarely).
        if (Charset::isUtf8(dataPtr, maxFiledSize)) {
            return stringcvt.utf8ToWide(dataPtr, maxFiledSize);
        }
        else if (Charset::hasUtf16LE_BOM(dataPtr)) {
            return stringcvt.utf16ToWide(dataPtr, maxFiledSize);
        }
        else {
            return stringcvt.multibyteToWide(dataPtr, maxFiledSize);
        }
    }

    bool ID3v1::setString(int fieldOffset, int maxFiledSize, const String& val)
    {
        Charset& stringcvt = Charset::getInstance();
        char* mb = stringcvt.wideToMultibyte(val);
        
        // Charset conversion failed.
        if (!mb) {
            return false;
        }

        memset(m_data + fieldOffset, 0, maxFiledSize);
        // TODO: 这里第三个参数是否不能是 maxFiledSize ？
        // 可能有些内存区域可能没有读权限
        // wxn 20130306
        memcpy(m_data + fieldOffset, mb, strlen(mb));

        delete [] mb;
        return true;
    }

    // Define the ID3v1 format
    enum
    {
        OFFSET_TITLE = 3,
        TITLE_MAX_SIZE = 30,

        OFFSET_ARTIST = OFFSET_TITLE + TITLE_MAX_SIZE,
        ARTIST_MAX_SIZE = 30,

        OFFSET_ALBUM = OFFSET_ARTIST + ARTIST_MAX_SIZE,
        ALBUM_MAX_SIZE = 30,

        OFFSET_YEAR = OFFSET_ALBUM + ALBUM_MAX_SIZE,
        YEAR_MAX_SIZE = 4,

        OFFSET_COMMENT = OFFSET_YEAR + YEAR_MAX_SIZE,
        COMMENT_MAX_SIZE = 30,

        // ID3 v1.1 only
        OFFSET_TRACK_NUMBER = OFFSET_COMMENT + COMMENT_MAX_SIZE - 1,

        OFFSET_GENRE = OFFSET_TRACK_NUMBER + 1,
    };

    String ID3v1::doGetArtist() const
    {
        return getString(OFFSET_ARTIST, ARTIST_MAX_SIZE);
    }

    bool ID3v1::doSetArtist(const String& Artist)
    {
        return setString(OFFSET_ARTIST, ARTIST_MAX_SIZE, Artist);
    }

    String ID3v1::doGetTitle() const
    {
        return getString(OFFSET_TITLE, TITLE_MAX_SIZE);
    }

    bool ID3v1::doSetTitle(const String& Title)
    {
        return setString(OFFSET_TITLE, TITLE_MAX_SIZE, Title);
    }

    String ID3v1::doGetAlbum() const
    {
        return getString(OFFSET_ALBUM, ALBUM_MAX_SIZE);
    }

    bool ID3v1::doSetAlbum(const String& Album)
    {
        return setString(OFFSET_ALBUM, ALBUM_MAX_SIZE, Album);
    }

    String ID3v1::doGetYear() const
    {
        return getString(OFFSET_YEAR, YEAR_MAX_SIZE);
    }

    bool ID3v1::doSetYear(const String& Year)
    {
        return setString(OFFSET_YEAR, YEAR_MAX_SIZE, Year);
    }

    String ID3v1::doGetComment() const
    {
        return getString(OFFSET_COMMENT, COMMENT_MAX_SIZE);
    }

    bool ID3v1::doSetComment(const String& Comment)
    {
        // We always uses ID3 v1.1
        if (setString(OFFSET_COMMENT, COMMENT_MAX_SIZE - 2, Comment)) {
            m_data[OFFSET_COMMENT + COMMENT_MAX_SIZE - 1] = 0;
            return true;
        }

        return false;
    }

    String ID3v1::doGetLyric() const
    {
        return EmptyString;
    }

    bool ID3v1::doSetLyric(const String& Lyric)
    {
        return false;
    }

    int ID3v1::doGetTrackNumber() const
    {
        // ID3 v1.1 specified
        if (m_data[OFFSET_TRACK_NUMBER - 1] == 0) {
            return m_data[OFFSET_TRACK_NUMBER];
        }
        else {
            // ID3 v1.0 doesn't have a track number field.
            return TRACK_NUMBER_NOT_SET;
        }
    }

    bool ID3v1::doSetTrackNumber(int TrackNumber)
    {
        if (m_data[OFFSET_TRACK_NUMBER - 1] == 0) {
            m_data[OFFSET_TRACK_NUMBER] = TrackNumber;
            return true;
        }
        else {
            return false;
        }
    }

    Genre ID3v1::doGetGenre() const
    {
        return (int) m_data[OFFSET_GENRE];
    }

    bool ID3v1::doSetGenre(const Genre& genre)
    {
        m_data[OFFSET_GENRE] = (char) genre.getSN();
        return true;
    }

}
