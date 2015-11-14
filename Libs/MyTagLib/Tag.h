/***************************************************************
 * Name:      Tag.h
 * Purpose:   The base interface of all tagging schemas.
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-18
 **************************************************************/
#pragma once
#include "Types.h"
#include "Genre.h"
#include "Range.h"

namespace MyTagLib
{
	/// The base interface of all tagging schemas.
	///
	/// A tag should never shrink itself. If you want to delete a field from
	/// the tag, leaving it empty would be enuough. 
	class Tag
	{
	public:

		/// Default constructor
		Tag();

		/// Virtual destructor
		virtual ~Tag() {}

		/// Load tags from an input stream.
		///
		/// In general, this function will use as much the state information 
		/// held by @a istrm as possible, such as the current file pointer
		/// position, etc.
		bool load(std::istream& istrm);

		/*! The result code of writing operation. */
		enum WriteResult {
			/*! No changes have been made to the tags, and no need 
			 *! to write back. */
			WR_NO_CHANGE,
			WR_OK, /*! The operation has done and no error accurred. */
			WR_FAILED, /*! Error encounted. */
		};

		/// Write tags back to an output stream.
		///
		/// @a ostrm The output stream, which should seek to the right position 
		/// before calling this function.
		WriteResult save(std::ostream& ostrm);

		/// Initialize the tag as a whole new one
		///
		/// This function does not intent to be called by a initialized
		/// tag (isOk() returns true).
		bool init();

		/// Reset the parser, and clear all the resource.
		void clear();

		/// Test whether this tag is empty or not.
		virtual bool isEmpty() const {
			return getArtist().empty() && getTitle().empty();
		}

		/// Get range of the tag in the music file.
		const Range& getRange() const { return m_range; }

		/// Get range of the tag in the music file.
		Range& getRange() { return m_range; }

		/// Returns true if the tag has been modified (but not commited).
		bool isModified() const { return m_changed; }

	private:

		virtual bool doLoad(std::istream& istrm) = 0;
		virtual WriteResult doSave(std::ostream& ostrm) = 0;
		// The overrider should set the initial size of the tag by
		// Range::resize( XXX ).
		virtual bool doInit() = 0;
		virtual void doClear() = 0;

		// The data has been changed.
		void setModified() { m_changed = true; }

	public:

		/// Returns true if the tag has been read successfully.
		virtual bool isOk() const = 0;

#define DECLARE_ACCESSORS( name, varType ) \
	varType get ## name() const; \
	bool set ## name(const varType& name);

		DECLARE_ACCESSORS( Artist, String )
		DECLARE_ACCESSORS( Title, String )
		DECLARE_ACCESSORS( Album, String )
		DECLARE_ACCESSORS( Year, String )
		DECLARE_ACCESSORS( Comment, String )
		DECLARE_ACCESSORS( Lyric, String )

		int getTrackNumber() const;
		/// Set the track number for the song
		/// @param TrackNumber The valid range is [1, 255].
		bool setTrackNumber(int TrackNumber);
		Genre getGenre() const;
		bool setGenre(const Genre& genre);

#undef DECLARE_ACCESSORS

	private:

#define DECLARE_PV_ACCESSORS( name, varType ) \
	virtual varType doGet ## name() const = 0; \
	virtual bool doSet ## name(const varType& name) = 0;

		DECLARE_PV_ACCESSORS( Artist, String )
		DECLARE_PV_ACCESSORS( Title, String )
		DECLARE_PV_ACCESSORS( Album, String )
		DECLARE_PV_ACCESSORS( Year, String )
		DECLARE_PV_ACCESSORS( Comment, String )
		DECLARE_PV_ACCESSORS( Lyric, String )

		virtual int doGetTrackNumber() const = 0;
		virtual bool doSetTrackNumber(int TrackNumber) = 0;

		virtual Genre doGetGenre() const = 0;
		virtual bool doSetGenre(const Genre& genre) = 0;

#undef DECLARE_PV_ACCESSORS

	private:

		// Disable copy
		Tag(const Tag&) {}
		Tag& operator = (const Tag&) { return *this; }

	private:

		bool m_changed; // Indicates the modification to any filed
		Range m_range;
	};
}

//////////////////////////////////////////////////////////////////////////
// Getters

#define OVERRIDE_ACCESSORS( name, varType ) \
	virtual varType doGet ## name() const; \
	virtual bool doSet ## name(const varType& name);

#define OVERRIDE_ALL_ACCESSORS() \
private: \
	OVERRIDE_ACCESSORS( Artist, String ) \
	OVERRIDE_ACCESSORS( Title, String ) \
	OVERRIDE_ACCESSORS( Album, String ) \
	OVERRIDE_ACCESSORS( Year, String ) \
	OVERRIDE_ACCESSORS( Comment, String ) \
	OVERRIDE_ACCESSORS( Lyric, String ) \
	virtual int doGetTrackNumber() const; \
	virtual bool doSetTrackNumber(int TrackNumber); \
	virtual Genre doGetGenre() const; \
	virtual bool doSetGenre(const Genre& genre)

//////////////////////////////////////////////////////////////////////////
// An example

/*

String ID3v2::doGetArtist() const
{
	return EmptyString;
}

bool ID3v2::doSetArtist(const String& Artist)
{
	return false;
}

String ID3v2::doGetTitle() const
{
	return EmptyString;
}

bool ID3v2::doSetTitle(const String& Title)
{
	return false;
}

String ID3v2::doGetAlbum() const
{
	return EmptyString;
}

bool ID3v2::doSetAlbum(const String& Album)
{
	return false;
}

String ID3v2::doGetYear() const
{
	return EmptyString;
}

bool ID3v2::doSetYear(const String& Year)
{
	return false;
}

String ID3v2::doGetComment() const
{
	return EmptyString;
}

bool ID3v2::doSetComment(const String& Comment)
{
	return false;
}

String ID3v2::doGetLyric() const
{
	return EmptyString;
}

bool ID3v2::doSetLyric(const String& Lyric)
{
	return false;
}

int ID3v2::doGetTrackNumber() const
{
	return TRACK_NUMBER_NOT_SET;
}

bool ID3v2::doSetTrackNumber(int TrackNumber)
{
	return false;
}

Genre ID3v2::doGetGenre() const
{
	return Genre::GENRE_NOT_SET;
}

bool ID3v2::doSetGenre(const Genre& genre)
{
	return false;
}

*/
