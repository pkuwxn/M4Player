/***************************************************************
 * Name:      Genre.h
 * Purpose:   Format genre information
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-20
 **************************************************************/
#pragma once
#include "Types.h"

namespace MyTagLib
{
	/// Genre information formatter
	class Genre
	{
	public:
	
		/// Construct using serial number.
		Genre(unsigned int sn);

		/// Construct using literal name.
		Genre(const String& name);

		/// Access genre serial number.
		unsigned int getSN() const { return m_sn; }
		void setSN(unsigned int sn);

		/// Access genre literal name.
		String getName() const { return m_name; }
		void setName(const String& name);

		enum {
			GENRE_NOT_SET = 255, /*! The genre field is not set or invalid. */
		};

		/// Returns the genres count of ID3v1 standard.
		static size_t getGenresCount();

		/// Implicitly convert to String
		operator const String::value_type*() const { return m_name.c_str(); }

		/// Test equality
		bool operator == (const Genre& rhs) const;

	private:

		void sn2Name();
		void name2SN();

	private:

		unsigned int m_sn;
		String m_name;
	};

}
