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

#ifndef BBP_TYPES_H
#define BBP_TYPES_H

#include <string>
#include <limits>
#include <boost/cstdint.hpp>
#include <boost/shared_array.hpp>

#include "BBP/Common/Math/Geometry/Vector_3D.h"

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MATH_DEFINES
#define BBP_MATH_DEFINES

#ifdef SWIG
#define const %constant
#endif
//! the number PI describing the relationship between radius and circumference
const double              PI = 3.14159265358979323846264338327;
const double              DEGREES_PER_RADIANT __attribute__((unused)) = 180.0 / PI ;
const double              RADIANT_PER_DEGREE = PI / 180.0;
#ifdef SWIG
#undef const
#endif

#endif

namespace bbp {

// workaround for Windows 32 bit platform
#ifdef WIN32
#undef min
#undef max
#endif


// BASIC TYPES _______________________________________________________________

//! alias for size of byte
#ifdef SWIG
#define const %constant
#endif
//! single byte (8 bits)
typedef boost::uint8_t          Byte;
//! non-negative integer for counting numbers (32 bits)
typedef size_t                  Count;
 const Count UNDEFINED_COUNT  __attribute__((unused)) =
    std::numeric_limits<Count>::max();
#ifdef SWIG
#undef const
#endif
//! array index (32 bits at least on 32 bit machine)
typedef size_t                  Index;
//! normalized value (floating point value between 0.0 and 1.0)
typedef float                   Normalized_Real_Number;
//! probability (floating point value between 0.0 and 1.0)
typedef Normalized_Real_Number  Probability;
//! Floating point value between 0.0 and 100.0 percent.
typedef float                   Percentage;
//! Resolution is an unsigned integer value (e.g. 800 for pixels)
typedef Count                   Resolution;
//! Multiplicative factor relative to normal (e.g. speed)
/*! e.g. "three times the speed"    = 3.0 */
typedef double                  Times;    
//! Unitless double precision floating point coordinate (64 bits on Win32/x86)
typedef double                  Coordinate;
//! Length unit on the unit circle
typedef double                  Radiant;
//! Revision counter e.g. for subversion or format versioning.
/*!
    This value is an unsigned integer, but encoded as a string currently.
*/
typedef std::string             Revision;

//! Direction in 3D space.
typedef Vector_3D<Coordinate>   Direction;

////! Position in 3D space.
//template <typename Length_Unit>
//typedef Vector_3D<Length_Unit> Position<Length_Unit>;
//! Angle in degrees.
typedef float                   Degree;

//! \todo This should go outside types in Math. (TT)
//! An interval is a range between two values in an ordered dimension.
/*!
    \todo Fix all files that are affected by new type before changing it
    definetely (JH)
    \ingroup Math
*/
template <typename T>
struct Interval
{
    T begin;
    T end;
         
    Interval() {}
    Interval(const T & begin, const T & end) :
        begin(begin), end(end) {}
    ~Interval() {}
};
//template <typename T>
//class Interval
//{
//public:
//  //! Create an uninitialized interval.
//    Interval() {}
//    //! Create and initialize interval (defaults to include the limit values).
//  Interval(const T & lower, const T & upper)
//      : lower_limit(lower), upper_limit(upper), 
//        lower_limit_included(true), upper_limit_included(true)
//  {}
//    ~Interval() {}
//
//public:
//  //! Limits of the interval range.   
//  T       lower_limit, 
//          upper_limit;
//  //! Closedness of the interval limits (if they are included).
//  bool    lower_limit_included, 
//          upper_limit_included;
//
//public:
//  //! \todo Verify this with a test case. (TT)
//  //! Check if value is inside the interval.
//  bool contains(const T & value) const
//  {
//      //! Test for open or closed interval limits on both sides.
//      if (    (((lower_limit_included == true) && (value >= lower_limit))
//              || 
//              ((lower_limit_included == false) && (value > lower_limit)))
//          &&
//              (((upper_limit_included == true) && (value <= upper_limit))
//              ||
//              ((upper_limit_included == false) && (value < upper_limit)))
//          )
//      {
//          return true;
//      }
//      else
//      {
//          return false;
//      }
//  }
//};

#ifdef SWIG
#define const %constant
#endif

//! undefined array index value
 const Index                     UNDEFINED_INDEX __attribute__((unused)) =
    std::numeric_limits<Index>::max();
//! Undefined revision number.
const Revision                  UNDEFINED_REVISION = "";
//! undefined value with micron length unit
const Probability               UNDEFINED_PROBABILITY =
    std::numeric_limits<Probability>::max();
//! undefined value with micron length unit
const Percentage               UNDEFINED_PERCENTAGE __attribute__((unused)) =
    std::numeric_limits<Percentage>::min();

//! frame counter
typedef boost::uint32_t   Frame_Number;
//! undefined frame value
const Frame_Number        UNDEFINED_FRAME_NUMBER =
    std::numeric_limits<Frame_Number>::max();

#ifdef SWIG
#undefdefine const
#endif

// PHYSICAL UNITS _____________________________________________________________

//! standard length unit for coordinates of microcircuit components
typedef float             Micron;
//! standard time unit for simulations
typedef float             Millisecond;
//! standard time unit for unsigned integer time constants (e.g. depression).
typedef boost::uint16_t   Millisecond_Time_Constant;
//! standard voltage unit for neuron membrane potentials
typedef float             Millivolt;
//! standard conductance unit for ion channels (e.g. for synapses)
typedef float             Nanosiemens;    
//! standard current unit for ion channels (e.g. for synapses)
typedef float             Nanoampere;
//! standard frequency unit for action potentials (APs per second)
typedef float             Hertz;

//! standard volume unit in cubic micron
typedef Micron            Micron3;        


#ifdef SWIG
#define const %constant
#endif
//! undefined value with micron length unit
const Micron              UNDEFINED_MICRON =
                          std::numeric_limits<Micron>::max();
//! undefined value with nanosiemens unit
const Nanosiemens         UNDEFINED_NANOSIEMENS =
                          std::numeric_limits<Nanosiemens>::max();
#ifndef SWIG
//! undefined vector with micron length unit
const Vector_3D<Micron>   UNDEFINED_VECTOR_3D_MICRON =
    Vector_3D<Micron>(UNDEFINED_MICRON, UNDEFINED_MICRON, UNDEFINED_MICRON);
#endif
//! undefined value in millisecond time unit
const Millisecond         UNDEFINED_MILLISECOND =
    std::numeric_limits<Millisecond>::max();
//! undefined value in millivolt voltage unit
const Millivolt           UNDEFINED_MILLIVOLT =
    std::numeric_limits<Millivolt>::max();
//! undefined millisecond time constant
const Millisecond_Time_Constant UNDEFINED_MILLISECOND_TIME_CONSTANT =
    std::numeric_limits<Millisecond_Time_Constant>::max();
#ifdef SWIG
#undef const
#endif
// TEXT _______________________________________________________________________

//! brief text string used to name objects (no whitespaces allowed)
typedef std::string       Label;
//! A word is a letter string without format, whitespaces or 
//! non letters.
typedef std::string       Word;
//! A single sentence in form of a letter string without format.
typedef std::string       Sentence;
//! A text without format.
typedef std::string       Text;


// RESOURCE IDENTIFIER ________________________________________________________

//! unified resource identifier (URI) in ASCII encoding.
typedef std::string       URI;
//! unified resource locator (URL) in ASCII encoding.
typedef std::string       URL;

//! subversion repository link
struct Subversion_Link
{
    // This get/set methods, despite ugly, provide a better interface for
    // wrapping inside Java and Python.
    const URL &      url() const { return _url; }
    const Revision & revision() const { return _revision; }
    void url(const URL & url) { _url = url; }
    void revision(const Revision & revision) { _revision = revision; }
protected:
    URL                     _url;
    Revision                _revision;
};

//! date reference (should we switch to boost date type ?)
typedef Label             Date;
//! time reference (should we switch to boost time type ?)
typedef Label             Time;

// ----------------------------------------------------------------------------

#ifndef SWIGJAVA
typedef boost::shared_array<boost::uint16_t>    Uint16_Array;
typedef boost::shared_array<float>              Float_Array;
typedef boost::shared_array<Micron>             Micron_Array;
typedef boost::shared_array<Micron>             Millisecond_Array;
typedef boost::shared_array<Probability>        Probability_Array;
typedef boost::shared_array<Nanosiemens>        Nanosiemens_Array;
typedef boost::shared_array<Vector_3D<Micron> > Vector_3D_Micron_Array;
typedef boost::shared_array<Millisecond_Time_Constant>   
    Millisecond_Time_Constant_Array;
#endif

// ----------------------------------------------------------------------------

}
#endif
