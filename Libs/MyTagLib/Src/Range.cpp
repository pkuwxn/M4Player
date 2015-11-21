/***************************************************************
 * Name:      Range.cpp
 * Purpose:   Represent a continuing part of the music file for rewriting.
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-21
 **************************************************************/
#include "StdAfx.h"
#include "Range.h"
#include "Tag.h" // TODO: Remove it

namespace MyTagLib {

Range::Range(int start, size_t length)
    : m_tag(NULL), m_start(start), m_length(length),
      m_deltaX(0), m_deltaLength(0),
      m_order(ORDER_WHATEVER) {

}

void Range::assign(int start, size_t length) {
    m_start = start;
    m_length = length;
    m_deltaX = 0;
    m_deltaLength = 0;
    // Do not change m_order here.

    assert(isOk());
}

void Range::reset() {
    m_tag = NULL;
    m_start = POS_INVALID;
    m_length = 0;

    m_deltaX = m_deltaLength = 0;
    m_order = ORDER_WHATEVER;
}

void Range::setStart(int start) {
    m_start = start;

    assert(isOk());
}

int Range::getEnd() const {
    assert(m_start != POS_LATER_BOUND);

    int ret;

    if (m_length == LENGTH_LATER_BOUND) {
        // Must not add the delta length here, this is only for
        // relocate the following peers.
        ret = m_start/* + m_deltaLength*/;
    } else {
        ret = m_start + m_length;
    }

    assert(ret >= 0);
    return ret;
}

void Range::move(int delta) {
    assert(isOk());

    m_deltaX += delta;
    assert((m_start == POS_LATER_BOUND) ||
           (m_start + m_deltaX >= 0));
}

void Range::resize(int delta) {
    assert(isOk());

    m_deltaLength += delta;
    assert((m_length == LENGTH_LATER_BOUND) ||
           (m_length + m_deltaLength > 0));
}

void Range::setOrder(int order) {
    assert(order >= ORDER_WHATEVER);
    m_order = order;
}

int Range::getOrder() const {
    return m_order;
}

bool Range::updateStreamState(std::iostream &strm) {
    assert(isOk());
    assert(m_deltaX >= 0);
    assert(m_deltaLength >= 0);

    Tag::WriteResult wr;

    if (m_tag) {
        strm.seekp(m_start + m_deltaX);
        if (!strm) {
            return false;
        }

        wr = m_tag->save(strm);
    } else { // We will handle it ourselves
        // This range is just an unknown section, writing it back directly
        if (m_deltaX == 0) {
            assert(m_deltaLength == 0);
            return true; // TODO: TRUE of FALSE?
        }

        //--------------------------------------------

        char *buf = new char[m_deltaX];
        int pos = getEnd() - m_deltaX;

        while (pos >= m_start) {
            strm.seekg(pos);
            strm.read(buf, m_deltaX);

            strm.seekp(pos + m_deltaX);
            strm.write(buf, m_deltaX);

            pos -= m_deltaX;
        }

        size_t rest = m_length % m_deltaX;
        if (rest) {
            strm.seekg(m_start);
            strm.read(buf, rest);

            strm.seekp(m_start + m_deltaX);
            strm.write(buf, rest);
        }

        delete [] buf;

        //------------------------------------------------

        wr = Tag::WR_OK;
    }

    m_start += m_deltaX;
    if (m_length == LENGTH_LATER_BOUND) {
        m_length = m_deltaLength;
    } else {
        m_length += m_deltaLength;
    }

#ifdef _DEBUG
    if (m_tag && wr == Tag::WR_OK) {
        /** MUST NOT WRITE OUTSIDE THE RANGE **/
        assert(((int) strm.tellp() - m_start) == m_length);
    }
#endif

    // Clear deltas
    m_deltaX = 0;
    m_deltaLength = 0;

    return wr != Tag::WR_FAILED;
}

bool Range::isOk() const {
    return m_start >= 0 && m_length > 0;
}

bool Range::isLaterBound() const {
    return m_start == POS_LATER_BOUND && m_length == LENGTH_LATER_BOUND;
}

bool Range::operator < (const Range &rhs) const {
    assert(isOk() && rhs.isOk());
    return m_start < rhs.m_start;
}

}
