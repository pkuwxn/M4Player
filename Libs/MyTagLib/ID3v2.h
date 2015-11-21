/***************************************************************
 * Name:      ID3v2.h
 * Purpose:   Implement the ID3 v2.3 tagging format
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-18
 **************************************************************/
#pragma once
#include "Tag.h"

#include <vector>

namespace MyTagLib
{
    /// Implement the ID3 v2.3 tagging format
    class ID3v2 : public Tag
    {
    public:

        /// Default constructor
        ID3v2();

        /// Destructor
        virtual ~ID3v2();

        /// Returns true if the tag has been read successfully.
        virtual bool isOk() const;

    private:

        OVERRIDE_ALL_ACCESSORS();

        virtual bool doLoad(std::istream& istrm);
        virtual WriteResult doSave(std::ostream& ostrm);
        virtual bool doInit();
        virtual void doClear();

        // Load all frames.
        bool loadFrames(std::istream& istrm, int body_size);

        // Read a single frame.
        bool readFrame(std::istream& istrm);

        // A frame should not shrink itself.
        void handleFrameResize(size_t deltaSize);

    private:

        // Find a specified text frame.
        template< class FrameT >
        FrameT* findFrame(const char* name) const;

        // Get a specified text frame's content.
        String getFrameText(const char* name) const;

        // Assign a new string for the frame.
        bool setFrameText(const char* name, const String& val);

    public:

        /// For internal use only
        struct Frame;

    private:

        struct Header;
        Header* m_header;

        typedef std::vector< Frame* > FrameVec;
        FrameVec m_frames;

        // Number of padding bytes
        size_t m_paddings;
    };
}
