/***************************************************************
 * Name:      FileRewriter.cpp
 * Purpose:   Hanlde music file rewriting.
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-21
 **************************************************************/
#include "StdAfx.h"
#include "FileRewriter.h"
#include "Range.h"

#include <stdlib.h> // for qsort()

namespace MyTagLib
{

	FileRewriter::FileRewriter()
	{

	}

	FileRewriter::~FileRewriter()
	{
		clear();
	}
	
	void FileRewriter::clear()
	{
		RangeVec::const_iterator iter( m_ranges.begin() );
		for (; iter != m_ranges.end(); ++iter) {
			if (!(*iter)->getTag())
				delete *iter;
		}

		m_ranges.clear();

		//--------------------------------------

		m_wholeNewRanges.clear();
	}

	void FileRewriter::addRange(Range& range)
	{
		assert( range.isOk() );
		assert( !range.isLaterBound() );

		m_ranges.push_back( &range );
	}

	void FileRewriter::addWholeNewRange(Range& range)
	{
		assert( range.isLaterBound() );
		assert( range.getTag() );

		m_wholeNewRanges.push_back( &range );
	}

	static int compareRanges(const void* range1, const void* range2)
	{
		// 这里指针的指针害死人啊，至少浪费了 30 分钟在这里！
		return (*((Range **) range1))->getStart() - 
			   (*((Range **) range2))->getStart();
	}

	bool FileRewriter::commit(std::iostream& strm)
	{
		if (m_ranges.empty() && m_wholeNewRanges.empty()) {
			return false;
		}

		if (!m_ranges.empty()) {
			if (m_ranges.size() > 1) {
				Range** base = &m_ranges[0];
				qsort( base, m_ranges.size(), sizeof( Range* ), compareRanges );

				if (!validateRanges()) {
					return false;
				}
			}
		}

		// Clear possible failbit (such as read after the EOF)
		if (strm.fail()) {
			strm.clear();
		}

		checkLastPart( strm );
		fillHoles();
		insertWholeNewRanges();
		relocateRanges();

		return commitRangeChanges( strm );
	}

	void FileRewriter::checkLastPart(std::iostream& strm)
	{
		strm.seekg( 0, std::ios::end );
		int fileEnd = (int) strm.tellg();
		int last = m_ranges.empty() ? 0 : m_ranges.back()->getEnd();
		if (last != fileEnd) {
			assert( last < fileEnd ); // TODO
			Range* missingPart = new Range( last, fileEnd - last );
			m_ranges.push_back( missingPart );
		}
	}

	bool FileRewriter::validateRanges() const
	{
		assert( !m_ranges.empty() );

		if (m_ranges.size() < 2)
			return true;

		RangeVec::const_iterator iter1( m_ranges.begin() );
		RangeVec::const_iterator iter2( iter1 );

		for (++iter2; iter2 != m_ranges.end(); ++iter1, ++iter2) {
			if ((*iter1)->getEnd() > (*iter2)->getStart())
				return false;
		}

		return true;
	}

	void FileRewriter::fillHoles()
	{
		int currPos = 0;

		RangeVec::iterator iter( m_ranges.begin() );
		for (; iter != m_ranges.end(); ++iter) {
			int nextRange = (*iter)->getStart();
			// encounters a hole
			if (currPos < nextRange) {
				Range* hold = new Range( currPos, nextRange - currPos );
				iter = m_ranges.insert( iter, hold ); // 不能为 const_iterator?

				// Skip the newly added range
				++iter;
			}

			currPos = (*iter)->getEnd();
		}
	}

	void FileRewriter::insertWholeNewRanges()
	{
		assert( !m_ranges.empty() );

		RangeVec::const_iterator iter( m_wholeNewRanges.begin() );
		for (; iter != m_wholeNewRanges.end(); ++iter) {
			insertWholeNewRange( *iter );
		}

		// Now update the starting postion of the whole new ranges
		RangeVec::iterator updateIter( m_ranges.begin() );
		for (; updateIter != m_ranges.end(); ++updateIter) {
			if ((*updateIter)->getStart() == Range::POS_LATER_BOUND) {
				if (updateIter == m_ranges.begin()) {
					(*updateIter)->setStart( 0 );
				}
				else {
					(*updateIter)->setStart( (*(updateIter - 1))->getEnd() );
				}
			}
		}/*

		print();*/

		m_wholeNewRanges.clear();
	}

	FileRewriter::RangeVec::iterator FileRewriter::insertWholeNewRange(Range* newRange)
	{
		// The very beginning of file.
		if (newRange->getOrder() == Range::ORDER_FILE_BEG) {
			return m_ranges.insert( m_ranges.begin(), newRange );
		}

		RangeVec::iterator before( m_ranges.begin() );
		while (before != m_ranges.end() && 
			// TODO: What if equal?
			(*before)->getOrder() < newRange->getOrder())
		{
			++before;
		}

		// If (before == m_ranges.end()), no range should locate before it, 
		// so push it back.
		// If not, we have found a correct position, and insert newRange at
		// that position.
		return m_ranges.insert( before, newRange );
	}

	void FileRewriter::relocateRanges()
	{
		assert( !m_ranges.empty() );

		RangeVec::const_iterator iter( m_ranges.begin() );
		for (; iter != m_ranges.end(); ++iter) {
			int deltaLength = (*iter)->getDeltaLength();
			if (deltaLength) {
				relocateFollowers( iter, deltaLength );
			}
		}
	}

	void FileRewriter::relocateFollowers(RangeVec::const_iterator iter, int delta)
	{
		RangeVec::iterator follower
			( m_ranges.begin() + (iter - m_ranges.begin() + 1));

		for (; follower != m_ranges.end(); ++follower) {
			(*follower)->move( delta );
		}
	}

	bool FileRewriter::commitRangeChanges(std::iostream& strm)
	{
		RangeVec::reverse_iterator iter( m_ranges.rbegin() );
		for (; iter != m_ranges.rend(); ++iter) {
			if (!(*iter)->updateStreamState( strm )) {
				return false;
			}
		}

		return true;
	}

	void FileRewriter::print() const
	{
		RangeVec::const_iterator iter( m_ranges.begin() );
		for (; iter != m_ranges.end(); ++iter) {
			const Range* range = *iter;

			int end = range->getEnd();
			int dstEnd = range->getEnd() + range->getDeltaX() + 
						 range->getDeltaLength();

			if (range->getLength() == Range::LENGTH_LATER_BOUND) {
				end = range->getStart();
				dstEnd = range->getStart() + range->getDeltaX() + 
						 range->getDeltaLength();
			}

			wprintf( L"FileRewriter: [%08d, %08d)\t\tDest: [%08d, %08d)\n",
					 range->getStart(), end,
					 range->getStart() + range->getDeltaX(),
					 dstEnd );
		}

		wprintf( L"\n" );
	}

}
