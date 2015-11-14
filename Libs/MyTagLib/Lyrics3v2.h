/***************************************************************
 * Name:      Lyrics3v2.h
 * Purpose:   Implement the Lyrics3 v2 tagging format
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-19
 **************************************************************/
#pragma once
#include "Tag.h"

namespace MyTagLib
{
	/// Implement the Lyrics3 v2 tagging format
	class Lyrics3v2 : public Tag
	{
	public:

		/// Default constructor
		Lyrics3v2();

		/// Returns true if the tag has been read successfully.
		virtual bool isOk() const;

	private:

		OVERRIDE_ALL_ACCESSORS();

		virtual bool doLoad(std::istream& istrm);
		virtual WriteResult doSave(std::ostream& ostrm);
		virtual bool doInit();
		virtual void doClear();

		// Read the tag data.
		bool readData(std::istream& istrm);

		// Stip all fields except the 'LYR' one.
		const char* strip(const char* buf, size_t maxLen, size_t& outLen);

	private:

		bool m_exists; // Is the tag initialized.
		String m_data;
		size_t m_paddings;
	};
}
