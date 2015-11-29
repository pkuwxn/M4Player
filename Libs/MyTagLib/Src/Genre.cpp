/***************************************************************
 * Name:      Genre.cpp
 * Purpose:   Format genre information
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-3-20
 **************************************************************/
#include "StdAfx.h"
#include "Genre.h"

#include "Utils.h" // for trim(), WCSICMP()

namespace MyTagLib {
Genre::Genre(unsigned int sn)
    : m_sn(GENRE_NOT_SET) {
    setSN(sn);
}

Genre::Genre(const String &name)
    : m_sn(GENRE_NOT_SET) {
    setName(name);
}

void Genre::setSN(unsigned int sn) {
    m_sn = sn;

    sn2Name();
}

void Genre::setName(const String &name) {
    m_name = name;
    trim(m_name);

    name2SN();
}

bool Genre::operator == (const Genre &rhs) const {
    return (m_sn == rhs.m_sn) && (m_name == rhs.m_name);
}

static const wchar_t *gs_genres[] = {
    L"Blues",
    L"Classic Rock",
    L"Country",
    L"Dance",
    L"Disco",
    L"Funk",
    L"Grunge",
    L"Hip-Hop",
    L"Jazz",
    L"Metal",
    L"New Age",
    L"Oldies",
    L"Other",
    L"Pop",
    L"R&B",
    L"Rap",
    L"Reggae",
    L"Rock",
    L"Techno",
    L"Industrial",
    L"Alternative",
    L"Ska",
    L"Death Metal",
    L"Pranks",
    L"Soundtrack",
    L"Euro-Techno",
    L"Ambient",
    L"Trip-Hop",
    L"Vocal",
    L"Jazz+Funk",
    L"Fusion",
    L"Trance",
    L"Classical",
    L"Instrumental",
    L"Acid",
    L"House",
    L"Game",
    L"Sound Clip",
    L"Gospel",
    L"Noise",
    L"Alternative Rock",
    L"Bass",
    L"Soul",
    L"Punk",
    L"Space",
    L"Meditative",
    L"Instrumental Pop",
    L"Instrumental Rock",
    L"Ethnic",
    L"Gothic",
    L"Darkwave",
    L"Techno-Industrial",
    L"Electronic",
    L"Pop-Folk",
    L"Eurodance",
    L"Dream",
    L"Southern Rock",
    L"Comedy",
    L"Cult",
    L"Gangsta",
    L"Top 40",
    L"Christian Rap",
    L"Pop/Funk",
    L"Jungle",
    L"Native American",
    L"Cabaret",
    L"New Wave",
    L"Psychedelic",
    L"Rave",
    L"Showtunes",
    L"Trailer",
    L"Lo-Fi",
    L"Tribal",
    L"Acid Punk",
    L"Acid Jazz",
    L"Polka",
    L"Retro",
    L"Musical",
    L"Rock & Roll",
    L"Hard Rock",
    L"Folk",
    L"Folk/Rock",
    L"National Folk",
    L"Swing",
    L"Fusion",
    L"Bebob",
    L"Latin",
    L"Revival",
    L"Celtic",
    L"Bluegrass",
    L"Avantgarde",
    L"Gothic Rock",
    L"Progressive Rock",
    L"Psychedelic Rock",
    L"Symphonic Rock",
    L"Slow Rock",
    L"Big Band",
    L"Chorus",
    L"Easy Listening",
    L"Acoustic",
    L"Humour",
    L"Speech",
    L"Chanson",
    L"Opera",
    L"Chamber Music",
    L"Sonata",
    L"Symphony",
    L"Booty Bass",
    L"Primus",
    L"Porn Groove",
    L"Satire",
    L"Slow Jam",
    L"Club",
    L"Tango",
    L"Samba",
    L"Folklore",
    L"Ballad",
    L"Power Ballad",
    L"Rhythmic Soul",
    L"Freestyle",
    L"Duet",
    L"Punk Rock",
    L"Drum Solo",
    L"A Cappella",
    L"Euro-House",
    L"Dance Hall",
    L"Goa",
    L"Drum & Bass",
    L"Club-House",
    L"Hardcore",
    L"Terror",
    L"Indie",
    L"BritPop",
    L"Negerpunk",
    L"Polsk Punk",
    L"Beat",
    L"Christian Gangsta Rap",
    L"Heavy Metal",
    L"Black Metal",
    L"Crossover",
    L"Contemporary Christian",
    L"Christian Rock",
    L"Merengue",
    L"Salsa",
    L"Thrash Metal",
    L"Anime",
    L"Jpop",
    L"Synthpop"
};

/*static*/
size_t Genre::getGenresCount() {
    return sizeof(gs_genres) / sizeof(gs_genres[0]);
}

void Genre::sn2Name() {
    const size_t numGenres = getGenresCount();

    if (m_sn < numGenres) {
        m_name.assign(gs_genres[m_sn]);
    } else {
        m_sn = GENRE_NOT_SET;
        m_name.clear();
    }
}

void Genre::name2SN() {
    m_sn = GENRE_NOT_SET;

    if (m_name.length() >= 3) {

        // An informal format: (13)
        if ((m_name[0] == L'(') && (m_name[m_name.length() - 1] == L')')) {
            m_sn = wcstol(m_name.c_str() + 1, NULL, 10);
        } else {
            const size_t numGenres = getGenresCount();

            for (size_t i = 0; i < numGenres; i++) {
                if (WCSICMP(m_name.c_str(), gs_genres[i]) == 0) {
                    m_sn = i;
                    break;
                }
            }

            // User-defined genre name
            if (m_sn == GENRE_NOT_SET) {
                return;
            }
        }
    }

    sn2Name();
}
}
