/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2007-2008. All rights reserved.

        Responsible authors:    Juan Hernando Vieites
                                Thomas Traenkler

*/
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_STRING_H
#define BBP_STRING_H

#include <string>
#include <cctype>
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace bbp {

// ----------------------------------------------------------------------------

//! Replace a string prefix with another one.
/*!
    @param original_string the original string that is to be modified
    @param original_prefix the string prefix that is to be replaced
    @param replacement_prefix the string prefix that is to be placed instead
    @return the new string with the replaced prefix
*/
inline std::string replace_prefix(  const std::string & original_string, 
                                    std::string original_prefix, 
                                    std::string replacement_prefix)
{
    std::string new_string;
    std::string::size_type result;

    result = original_string.find(original_prefix);
    if (result == 0)
    {
        new_string = replacement_prefix 
            + original_string.substr(original_prefix.size());
#ifndef NDEBUG 
        std::cout << "Corrected \"" << original_string << "\" to \"" 
            << new_string << "\"" << std::endl;
#endif
        return new_string;
    }
    else
    {
        std::string error= std::string("Replacement of prefix ")
            + original_prefix + " in "
            + original_string + " with "
            + replacement_prefix + " failed.";
        std::cerr << error << std::endl;
        throw std::logic_error(error);
    }
}

// ----------------------------------------------------------------------------

//! removes spaces from the beginning of a string
inline std::string trim_left_end (const std::string & s) 
{
    std::string result = s;

    std::string::iterator to = result.begin();
    while (to != result.end() && isspace(*to))
        ++to;
    result.erase(result.begin(), to);

    return result;
}

// ----------------------------------------------------------------------------

//! removes spaces from the end of a string
/*!
    \bug Seems to not remove spaces from end as seen in "StopTime 750 ".
    Check if the fix below works correctly. (TT)
*/
inline std::string trim_right_end (const std::string & s) 
{
    std::string result;
    size_t cutoff = 0;

    std::string::const_reverse_iterator from = s.rbegin();
    while (from != s.rend() && isspace(*from))
    {
        ++from;
        ++cutoff;
    }
    std::string::const_iterator i = from.base();
    
    //--i;
    //! \todo WRONG! (Code fixed, but inspect commented out code later). (TT)
    //++i;  
    //s.erase(i, s.end());

    result.resize(s.size() - cutoff);
    std::copy<std::string::const_iterator,
        std::string::iterator>(s.begin(), i, result.begin());

    return result;
}

// ----------------------------------------------------------------------------

//! removes spaces from beginning and end of a string
inline std::string trim_ends (const std::string & s) 
{ 
    return trim_left_end(trim_right_end(s));
}

// ----------------------------------------------------------------------------

}
#endif
