/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Sebastien Lasserre
                                Juan Hernando Vieites
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_VERSION_H
#define BBP_VERSION_H

#include "Types.h"
#include <string>
//#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

namespace bbp {

// ----------------------------------------------------------------------------

//! The major revision number, e.g. the "1" in 1.3.2.
const Count BBP_SDK_MAJOR_REVISION_NUMBER           = 0;
//! The minor revision number, e.g. the "3" in 1.3.2.
const Count BBP_SDK_MINOR_REVISION_NUMBER           = 9;
//! The release number, e.g. the "2" in 1.3.2.
const Count BBP_SDK_RELEASE_NUMBER                  = 40;

//! The type of the release, e.g. "beta", "alpha", "RC1", "CTP"
const char * const BBP_RELEASE_TYPE                 = "beta";

// ----------------------------------------------------------------------------

//! Version information.
class Version
{
public:
    Version(Count major_revision_number,
            Count minor_revision_number,
            Count release_number,
            const std::string & release_type = std::string(),
            Count build_number = UNDEFINED_COUNT)
    : _major(major_revision_number),
      _minor(minor_revision_number),
      _release(release_number),
      _build(build_number),
      _type(release_type)
    {
    }

    Version(Count major_revision_number,
            Count minor_revision_number,
            Count release_number,
            const char * release_type,
            Count build_number = UNDEFINED_COUNT)
    : _major(major_revision_number),
      _minor(minor_revision_number),
      _release(release_number),
      _build(build_number),
      _type(release_type)
    {
    }

    //! Get the major revision number, e.g. the "1" in 1.3.2.
    Count major_revision() const
    {
        return _major;
    }

    //! Get the minor revision number, e.g. the "3" in 1.3.2.
    Count minor_revision() const
    {
        return _minor;
    }

    //! Get the release number, e.g. the "2" in 1.3.2.
    Count release_revision() const
    {
        return _release;
    }

    //! Get the build revision number, e.g. 25
    Count build_revision() const
    {
        return _build;
    }

    //! Get the release type, e.g. "beta".
    std::string type() const
    {
        return _type;
    }

    //! Get a string containing the version (e.g. "1.3.2 beta build 25").
    std::string string() const
    {
        std::string result;

        if (_type.empty() != true)
        {
            result = std::string(
            boost::lexical_cast<std::string>(_major) + "." +
            boost::lexical_cast<std::string>(_minor) + "." +
            boost::lexical_cast<std::string>(_release) + " " +
            _type);
        }
        else 
        {
            result = std::string(
            boost::lexical_cast<std::string>(_major) + "." +
            boost::lexical_cast<std::string>(_minor) + "." +
            boost::lexical_cast<std::string>(_release));
        }

        if (this->build_revision() != UNDEFINED_COUNT)
        {
            result += std::string(" ") 
                   + boost::lexical_cast<std::string>(this->build_revision());
        }
        return result;
    }

private:
    Count           _major, 
                    _minor, 
                    _release,
                    _build;
    std::string     _type;
};

// ----------------------------------------------------------------------------

//! Get the version number of the Blue Brain SDK.
inline Version version() 
{
    return Version( BBP_SDK_MAJOR_REVISION_NUMBER,
                    BBP_SDK_MINOR_REVISION_NUMBER,
                    BBP_SDK_RELEASE_NUMBER,
                    BBP_RELEASE_TYPE);
}

// ----------------------------------------------------------------------------

/*
// Get the repository revision number of the Blue Brain SDK.
inline Count repository_revision()
{
    // extract number from revision tag, e.g. $Revision: 3697 $
    static const boost::regex e("\\$Revision: (\\d*) \\$");
    boost::cmatch matches;
    if (boost::regex_match("$Revision: 3697 $", matches, e))
    {
        // matches[0] contains the original string.  matches[n]
        // contains a sub_match object for each matching
        // subexpression
        std::string revision_number;
        for (size_t i = 1; i < matches.size(); i++)
        {
            // sub_match::first and sub_match::second are iterators that
            // refer to the first and one past the last chars of the
            // matching subexpression
            revision_number = std::string(matches[i].first, matches[i].second);
        }
        
        return boost::lexical_cast<Count>(revision_number);
    }
    else
    {
        return 0;
    }
}
*/

// ----------------------------------------------------------------------------

} // bbp

#endif
