/***************************************************************
 * Name:      Lyrics3v2.cpp
 * Purpose:   Implement the Lyrics3 v2 tagging format
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-19
 **************************************************************/
#include "StdAfx.h"
#include "Lyrics3v2.h"
#include "Charset.h"

#include <stdlib.h> // for atoi()
#include <stdio.h> // for sprintf

namespace MyTagLib {

Lyrics3v2::Lyrics3v2()
    : m_exists(false), m_paddings(0) {
    getRange().setOrder(Range::ORDER_FILE_END - 1);
}

void Lyrics3v2::doClear() {
    m_exists = false;
    m_data.clear();
    m_paddings = 0;
}

bool Lyrics3v2::doLoad(std::istream &istrm) {
    char identifier[10] = { 0 }; // #LYRICS200 == 9
    istrm.seekg(-9, std::ios::end);
    istrm.read(identifier, 9);

    if (strcmp(identifier, "LYRICS200")) {
        istrm.seekg(-(128 + 9), std::ios::end);
        istrm.read(identifier, 9);

        if (strcmp(identifier, "LYRICS200")) {
            return false;
        }
    }

    m_exists = readData(istrm);
    return isOk();
}

bool Lyrics3v2::readData(std::istream &istrm) {
    istrm.seekg(-15, std::ios::cur);
    if (!istrm) {
        return false;
    }

    char bodySizeVec[7] = { 0 };
    istrm.read(bodySizeVec, 6);
    if (!istrm) {
        return false;
    }

    int bodySize = atoi(bodySizeVec);
    if (bodySize < 11) { // #LYRICSBEGIN == 11
        return false;
    }

    // Go to the very tag data begin
    istrm.seekg(-(bodySize + 6), std::ios::cur);
    if (!istrm) {
        return false;
    }

    int tagDataBeginnigPos = (int) istrm.tellg();

    char identifierAtBegin[12] = { 0 };
    istrm.read(identifierAtBegin, 11); // #LYRICSBEGIN == 11
    if (strcmp(identifierAtBegin, "LYRICSBEGIN")) {
        return false;
    }

    //==========================================================

    size_t textSize = bodySize - 11; // #LYRICSBEGIN == 11
    char *buf = new char[textSize + 1];
    istrm.read(buf, textSize);
    buf[textSize] = 0;

    size_t lyrSize;
    const char *lyr = strip(buf, textSize, lyrSize);
    if (lyrSize && lyr) {
        m_data = Charset::getInstance().multibyteToWide(lyr, lyrSize);
    }

    // Set stream pointer to the right position
    istrm.seekg(tagDataBeginnigPos);

    // 15 == The length of footer size mark and indentifier.
    getRange().assign(tagDataBeginnigPos, bodySize + 15);

    delete [] buf;

    // Always returns true no matter lyric text is empty or not.
    return true;
}

const char *Lyrics3v2::strip(const char *buf, size_t maxLen, size_t &outLen) {
    const char *p = buf;
    char sizeVec[6] = { 0 };

    while (size_t(p - buf) < maxLen) {
        bool lyrFiled = strncmp(p, "LYR", 3) == 0;
        outLen = atoi((const char *) memcpy(sizeVec, p + 3, 5));

        if (lyrFiled) {
            return p + 3 + 5;
        }

        p += 3 + 5 + outLen;
    }

    return NULL;
}

Tag::WriteResult Lyrics3v2::doSave(std::ostream &ostrm) {
    int beginWrite = (int) ostrm.tellp();

    const char stdHeader[] = "LYRICSBEGININD00003110";
    ostrm.write(stdHeader, sizeof(stdHeader) - 1);

    char *mb = Charset::getInstance().wideToMultibyte(m_data);

    // Charset conversion failed.
    if (!mb) {
        return WR_FAILED;
    }

    size_t mbLen = strlen(mb);
    {
        char buf[9] = { 0 };
        sprintf(buf, "LYR%05d", mbLen + m_paddings);
        ostrm.write(buf, sizeof(buf) - 1);
    }

    ostrm.write(mb, mbLen);
    delete [] mb;

    // Add paddings to avoid shrinking
    for (size_t i = 0; i < m_paddings; i++) {
        ostrm.write("\n", 1);
    }

    {
        char buf[16] = { 0 };
        sprintf(buf, "%06dLYRICS200", (int) ostrm.tellp() - beginWrite);
        ostrm.write(buf, sizeof(buf) - 1);
    }

    return WR_OK;
}

bool Lyrics3v2::doInit() {
    // 22 == #("LYRICSBEGININD00003110")
    // 15 == #("000000LYRICS200")
    //  8 == #("LYR00000")
    size_t minimalSize = 22 + 15 + 8;
    getRange().resize(minimalSize);

    m_exists = true;
    return isOk();
}

bool Lyrics3v2::isOk() const {
    return m_exists;
}

String Lyrics3v2::doGetLyric() const {
    return m_data;
}

bool Lyrics3v2::doSetLyric(const String &Lyric) {
    if (m_data == Lyric) {
        return false;
    }

    //===========================================
    // Calculate delta width

    Charset &stringcvt = Charset::getInstance();

    size_t oldLen = 0;
    if (!m_data.empty()) {
        char *mb = stringcvt.wideToMultibyte(m_data);

        // Charset conversion failed.
        if (!mb) {
            return false;
        }

        oldLen = strlen(mb);
        delete [] mb;
    }

    size_t newLen = 0;
    if (!Lyric.empty()) {
        char *mb = stringcvt.wideToMultibyte(Lyric);

        // Charset conversion failed.
        if (!mb) {
            return false;
        }

        newLen = strlen(mb);
        delete [] mb;
    }

    int delta = newLen - oldLen;
    // Add paddings
    if (delta < 0) {
        m_paddings = - delta;
    } else {
        m_paddings = 0;
        getRange().resize(delta);
    }

    //===========================================

    m_data = Lyric;
    return true;
}

String Lyrics3v2::doGetArtist() const {
    return EmptyString;
}
String Lyrics3v2::doGetTitle() const {
    return EmptyString;
}
String Lyrics3v2::doGetAlbum() const {
    return EmptyString;
}
String Lyrics3v2::doGetYear() const {
    return EmptyString;
}
String Lyrics3v2::doGetComment() const {
    return EmptyString;
}
int Lyrics3v2::doGetTrackNumber() const {
    return TRACK_NUMBER_NOT_SET;
}
Genre Lyrics3v2::doGetGenre() const {
    return Genre::GENRE_NOT_SET;
}

bool Lyrics3v2::doSetArtist(const String &Artist) {
    return false;
}
bool Lyrics3v2::doSetTitle(const String &Title) {
    return false;
}
bool Lyrics3v2::doSetAlbum(const String &String) {
    return false;
}
bool Lyrics3v2::doSetYear(const String &Year) {
    return false;
}
bool Lyrics3v2::doSetComment(const String &Comment) {
    return false;
}
bool Lyrics3v2::doSetTrackNumber(int TrackNumber) {
    return false;
}
bool Lyrics3v2::doSetGenre(const Genre &genre) {
    return false;
}

}
