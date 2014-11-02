/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Konstantinos Sfyrakis
                                
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_ENDIAN_SWAP_H
#define BBP_ENDIAN_SWAP_H

#include <utility>
#include <algorithm>
#include <limits>
#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

namespace bbp {

// ----------------------------------------------------------------------------

//! Swap byte ordering (endianness) of variable memory representation.
/*!
    http://en.wikipedia.org/wiki/Endianness

    War of the Worlds: Endianness (Marius Bancila)
    http://www.codeguru.com/cpp/cpp/algorithms/math/article.php/c10243/

    In case you wonder where the names come from, the answer may surprise you:
    from Jonathan Swift's Gulliver's Travels book. In the first part of the 
    book, Gulliver, an English sailor, awakes after a shipwreck as a prisoner 
    of a six-inch high people, called Lilliputians. In Chapter 4 of the book, 
    a secretary of the emperor of Lilliput tells him about the war with the 
    people of Blefuscu, a rival empire, who offered protection for the 
    Big-Endians in the civil war between the Big- and Little-Endians. It was 
    the primitive way in Lilliput that the eggs were broken from the larger 
    end before being eaten. But, when the son of an emperor (that later become 
    emperor himself) cut his finger breaking an egg, his father, the emperor, 
    published an edict, commanding everyone, under great penalties, to break 
    the eggs from the little end. And that edict led to a great civil war 
    between the followers of the new way (the Little-Endians) and those who 
    remained committed to the old way (the Big-Endians). In the torment of the 
    conflict, the Big-Endians found protection in the Kingdom of Blefuscu and 
    a war was started between the mighty Kingdoms of Lilliput and Blefuscu.
*/
template <typename T>
inline void endian_swap(T & x);

// ----------------------------------------------------------------------------

template <>
inline void endian_swap<boost::uint16_t>(boost::uint16_t & x)
{
    x =    ((x & 0xff) << 8)
        |  ((x >> 8));
}


// ----------------------------------------------------------------------------

template <>
inline void endian_swap<boost::uint32_t>(boost::uint32_t & x)
{
    x =   ((x << 24))
        | ((x << 8)     & 0x00ff0000U) 
        | ((x >> 8)     & 0x0000ff00U)
        | ((x >> 24));
}

// ----------------------------------------------------------------------------

template <>
inline void endian_swap<boost::uint64_t>(boost::uint64_t & x)
{
#ifdef __GNUC__
    unsigned char * low  = (unsigned char *) & x;
    unsigned char * high = (unsigned char *) & x 
                           + sizeof(boost::uint64_t) - 1; 
    while(low < high)
    {
        std::swap(*low, *high);
        ++low;
        --high;
    }
#else    
    x =    ((x  << 56))
        |  ((x  << 40)  & 0x00ff000000000000UL)
        |  ((x  << 24)  & 0x0000ff0000000000UL)
        |  ((x  <<  8)  & 0x000000ff00000000UL)
        |  ((x  >>  8)  & 0x00000000ff000000UL)
        |  ((x  >> 24)  & 0x0000000000ff0000UL)
        |  ((x  >> 40)  & 0x000000000000ff00UL)
        |  ((x  >> 56));
#endif
}

// ----------------------------------------------------------------------------

template <>
inline void endian_swap<float>(float & x)
{
    // check if floating point number represenation conforms to IEEE 754-1985
    // http://en.wikipedia.org/wiki/IEEE_754-1985
    BOOST_STATIC_ASSERT(std::numeric_limits<float>::is_iec559);
    endian_swap(* (boost::uint32_t *) & x);
}

// ----------------------------------------------------------------------------

template <>
inline void endian_swap<double>(double & x)
{
    // check if floating point number represenation conforms to IEEE 754-1985
    // http://en.wikipedia.org/wiki/IEEE_754-1985
    BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);
    endian_swap(* (boost::uint64_t *) & x);
}

// ----------------------------------------------------------------------------

template <>
inline void endian_swap<boost::int16_t>(boost::int16_t & x)
{
    // check if negative number representation format is 2-complement
    // http://en.wikipedia.org/wiki/Signed_number_representations#Two.27s_complement
    BOOST_STATIC_ASSERT((boost::uint16_t)((boost::int16_t) -1) == 0xFFFF);
    endian_swap(* (boost::uint16_t *) & x);
}

// ----------------------------------------------------------------------------

template <>
inline void endian_swap<boost::int32_t>(boost::int32_t & x)
{
    // check if negative number representation format is 2-complement
    // http://en.wikipedia.org/wiki/Signed_number_representations#Two.27s_complement
    BOOST_STATIC_ASSERT((boost::uint32_t)((boost::int32_t) -1) == 0xFFFFFFFF);
    endian_swap(* (boost::uint32_t *) & x);
}

// ----------------------------------------------------------------------------

template <>
inline void endian_swap<boost::int64_t>(boost::int64_t & x)
{
    // check if negative number representation format is 2-complement
    // http://en.wikipedia.org/wiki/Signed_number_representations#Two.27s_complement
    BOOST_STATIC_ASSERT((boost::uint32_t)((boost::int32_t) -1) == 0xFFFFFFFF);
    endian_swap(* (boost::uint64_t *) & x);
}

// ----------------------------------------------------------------------------

}

#endif
