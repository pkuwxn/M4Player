/***************************************************************
 * Name:      Tag.cpp
 * Purpose:   The base interface of all tagging schemas.
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-3-18
 **************************************************************/
#include "StdAfx.h"
#include "Tag.h"

#include "Utils.h" // for trim()

namespace MyTagLib {
const wchar_t *EmptyString = L"";

Tag::Tag() : m_changed(false) {
    // Cannot placed in the initialization list
    m_range.setTag(this);
}

bool Tag::load(std::istream &istrm) {
    // Always clear the previously read tags
    clear();

    if (istrm.fail()) {
        istrm.clear();
    }

    if (!istrm) {
        return false;
    }

    bool ok = doLoad(istrm);
    if (ok) {
        assert(getRange().isOk());
    }

    return ok;
}

Tag::WriteResult Tag::save(std::ostream &ostrm) {
    assert(ostrm);

    if (!m_changed && getRange().getDeltaX() == 0) {
        return WR_NO_CHANGE;
    }

    WriteResult wr = doSave(ostrm);
    assert(wr != WR_NO_CHANGE);

    if (wr != WR_FAILED) {
        m_changed = false;
    }

    return wr;
}

bool Tag::init() {
    clear();
    assert(!isOk());

    m_changed = true; // TODO:
    m_range.assign(Range::POS_LATER_BOUND, Range::LENGTH_LATER_BOUND);

    return doInit();
}

void Tag::clear() {
    doClear();

    //========================================

    m_changed = false;

    // TODO: Range::setTag() 和 Range::setOrder() 的 API 设计要重新考虑
    int order = m_range.getOrder();
    m_range.reset();
    m_range.setTag(this);
    m_range.setOrder(order);
}

//////////////////////////////////////////////////////////////////////////

#define DEFINE_STRING_ACCESSORS(name) \
    String Tag::get ## name() const \
    { \
        if (!isOk()) { \
            return EmptyString; \
        } \
        \
        String name(doGet ## name()); \
        return trim(name); \
    } \
    \
    bool Tag::set ## name(const String& name) \
    { \
        if (name == get ## name()) \
            return false; \
        \
        if (doSet ## name(name)) { \
            setModified(); \
            return true; \
        } \
        \
        return false; \
    }

DEFINE_STRING_ACCESSORS(Artist)
DEFINE_STRING_ACCESSORS(Title)
DEFINE_STRING_ACCESSORS(Album)
DEFINE_STRING_ACCESSORS(Year)
DEFINE_STRING_ACCESSORS(Comment)
DEFINE_STRING_ACCESSORS(Lyric)

int Tag::getTrackNumber() const {
    if (!isOk()) {
        return TRACK_NUMBER_NOT_SET;
    }

    int trackNumber = doGetTrackNumber();
    if ((trackNumber > 0) && (trackNumber <= 255)) {
        return trackNumber;
    } else {
        return TRACK_NUMBER_NOT_SET;
    }
}

bool Tag::setTrackNumber(int TrackNumber) {
    if ((TrackNumber <= 0) || (TrackNumber <= 255)) {
        return false;
    }

    if (TrackNumber == getTrackNumber()) {
        return false;
    }

    if (doSetTrackNumber(TrackNumber)) {
        setModified();
        return true;
    }

    return false;
}

Genre Tag::getGenre() const {
    if (!isOk()) {
        return Genre::GENRE_NOT_SET;
    }

    return doGetGenre();
}

bool Tag::setGenre(const Genre &genre) {
    if (genre == getGenre()) {
        return false;
    }

    if (doSetGenre(genre)) {
        setModified();
        return true;
    }

    return false;
}

}
