/***************************************************************
 * Name:      Range.h
 * Purpose:   Represent a continuing part of the music file for rewriting.
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-03-21
 **************************************************************/
#pragma once

namespace MyTagLib {
class Tag;

/// Represent a continuing part of the music file for rewriting.
class Range {
public:

    enum {
        POS_INVALID = -1, ///< Invalid range starting or ending position.
        /*! To a whole new range, the position information is unknown
         *   until committing the changes.
         */
        POS_LATER_BOUND = unsigned(~0) >> 1,
        /*! @see POS_LATER_BOUND */
        LENGTH_LATER_BOUND = POS_LATER_BOUND,
    };

    /// Constructor
    Range(int start = POS_INVALID, size_t length = 0);

    /// Set the associated tag.
    void setTag(Tag *tag) {
        m_tag = tag;
    }

    /// Get the associated tag.
    Tag *getTag() const {
        return m_tag;
    }

    /// Assign new values.
    ///
    /// The range in math language is [start, start + length).
    void assign(int start, size_t length);

    /// Reset the range to make it invalid.
    void reset();

    enum {
        ORDER_WHATEVER = -1, ///< Invalid order, not order-sensitive.
        ORDER_FILE_BEG = 0, ///< Guarantee to be at the very beginning.
        /*! Guarantee to be at the end of file. */
        ORDER_FILE_END = unsigned(~0) >> 1,
    };

    /// Set order for the range, which can define where a brand new range
    /// should locate at.
    /// @param order If not set (value is ORDER_WHATEVER), this range
    /// will not participate in the seat-seeking for a brand new range,
    /// which probablly means that it's order-insensitive.
    void setOrder(int order);

    /// Get order of the range
    int getOrder() const;

    /// Test whether the range valid.
    bool isOk() const;

    /// Returns true if the range is a later bound one.
    /// If the range is whole new, it's later bound.
    bool isLaterBound() const;

    /// Override for comparing
    bool operator < (const Range &rhs) const;

    /// Returns the starting position of the range.
    int getStart() const {
        return m_start;
    }

    /// Sets the starting position of the range.
    void setStart(int start);

    /// Returns the ending position of the range.
    int getEnd() const;

    /// Returns the length of the range.
    int getLength() const {
        return m_length;
    }

    /// Get the position changing for the next writing in diff with
    /// the current range.
    int getDeltaX() const {
        return m_deltaX;
    }

    /// Get the length changing for the next writing in diff with
    /// the current range.
    int getDeltaLength() const {
        return m_deltaLength;
    }

    /// Move to new position by @a delta bytes in the stream.
    void move(int delta);

    /// Resize (increase or decrease) the range length by @a delta bytes.
    /// @param delta The delta value will accumulate.
    void resize(int delta);

    /// Commit all changes - update range position and size in the stream.
    bool updateStreamState(std::iostream &strm);

private:

    // The tag located at this range, can be null, which indicates
    // that this range hold some unknown data.
    Tag *m_tag;

    int m_start;
    size_t m_length;

    int m_deltaX;
    int m_deltaLength;

    // Define the permutation between ranges of various types,
    // and where a brand new range should locate at.
    int m_order;
};
}
