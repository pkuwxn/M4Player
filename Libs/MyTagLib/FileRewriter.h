/***************************************************************
 * Name:      FileRewriter.h
 * Purpose:   Hanlde music file rewriting.
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-21
 **************************************************************/
#pragma once
#include <vector>

namespace MyTagLib
{
	class Range;

	/// Hanlde music file rewriting.
	class FileRewriter
	{
	public:

		/// Default constructor
		FileRewriter();

		/// Destructor
		~FileRewriter();

		/// Clear all ranges.
		void clear();

		/// Add a new range (a logical part of the file)
		void addRange(Range& range);

		/// Add (insert) a brand new range
		/// @param range The range must be associated with some tag.
		void addWholeNewRange(Range& range);

		/// Commit change.
		/// @param strm The stream cannot be entirely empty, which means that
		/// MyTagLib does not support write pure tag data without the concreate 
		/// audio data (the main part).
		bool commit(std::iostream& strm);

		/// Debug output
		void print() const;

	private:

		// Validate all ranges to determine they don't overlap each other.
		bool validateRanges() const;

		// Check the last part of the file. If it's missing, we should 
		// make it up.
		void checkLastPart(std::iostream &strm);

		// Fill out all holds.
		void fillHoles();

		// Insert the delay-inserted whole new ranges to proper positions
		// according to their orders.
		void insertWholeNewRanges();

		// Relocate the ranges whose positions have been changed and 
		// their followers affected.
		void relocateRanges();

		// Commit all range changes. (Update stream state)
		bool commitRangeChanges(std::iostream& strm);

	private:

		typedef std::vector< Range* > RangeVec;

		// Relocate a range's followers (assign new delta value)
		void relocateFollowers(RangeVec::const_iterator iter, int delta);

		// Find a right seat for the new range in the stream.
		RangeVec::iterator insertWholeNewRange(Range* newRange);

	private:

		RangeVec m_ranges;
		RangeVec m_wholeNewRanges;
	};

}
