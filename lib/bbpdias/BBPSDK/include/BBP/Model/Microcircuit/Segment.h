/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:	Thomas Traenkler
                                Nikolai Chapochnikov

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SEGMENT_H
#define BBP_SEGMENT_H

#include <vector>
#include <iostream>
#include <boost/shared_ptr.hpp>

#include "Types.h"
#include "BBP/Common/Math/Geometry/Rotation.h"
#include "BBP/Common/Math/Geometry/Circle.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Morphology;
class Segments;

// ----------------------------------------------------------------------------

//! Interface to a neuron segment (a cylindric part of a morphology section).
/*!
    A neuron segment is a part of a neuron section and 
    consists of two points and the cable diameter in these points.
    See Section and Neuron class for more information. These
    objects are used in multi-compartment neuron models like the
    ones used in the NEURON simulator.
    \ingroup Microcircuit
*/
class Segment 
	: private Interval<Segment_Cross_Section>
{
    friend class Segments;
    friend class Section;

protected:
    //! Empty constructor needed by implementation
    Segment()
    : Interval<Segment_Cross_Section>
              (Segment_Cross_Section(UNDEFINED_MICRON, UNDEFINED_MICRON, 
                                     UNDEFINED_MICRON, UNDEFINED_MICRON), 
               Segment_Cross_Section(UNDEFINED_MICRON, UNDEFINED_MICRON, 
                                     UNDEFINED_MICRON, UNDEFINED_MICRON))
    {}

    //! Creates a new segment and initializes it with global coordinates
    inline Segment(const Morphology *        morphology, 
                   const Vector_3D<Micron> & soma_position, 
                   const Rotation_3D<Micron> &          neuron_rotation,
                   Morphology_Point_ID       begin_point_id);

    /*!
		Creates a new segment and initializes it with local coordinates 
		in the morphology reference frame.
    */
    inline Segment(const Morphology  *       morphology, 
                   Morphology_Point_ID       begin_point_id);

public:
	//! Length (or height) of the segment cylinder.
	Micron length() const
	{
		return (begin().center() - end().center()).length();
	}
	//! Access a cable cross section at the begin of the segment.
    /*!
        \todo Check if this should not instead return a reference. (TT)
    */
    const Segment_Cross_Section & begin() const
    {
        return Interval<Segment_Cross_Section>::begin;
    }

	//! Access a cable cross section at the end of the segment.
    /*!
        \todo Check if this should not instead return a reference. (TT)
    */
    const Segment_Cross_Section & end() const
    {
        return Interval<Segment_Cross_Section>::end;
    }

	//! Get a cross section of the segment at a specific point.
	/*!
		@param norm_distance Normalized distance from begin of segment 0.0-1.0
		@return a cross section with position and diameter at specified point
	*/
	Segment_Cross_Section cross_section(
		Segment_Normalized_Distance norm_distance) const
	{
		bbp_assert(norm_distance >= 0.0f && norm_distance <= 1.0f);
		return this->interpolate(
			Interval<Segment_Cross_Section>::begin,
			Interval<Segment_Cross_Section>::end,
			norm_distance);
	}

    //! Get local identifier of segment inside a section.
    /*!
        DISCLAIMER: This will be computed on demand from the morphology point 
        ID, so it is not fast or even constant lookup. This decision has been 
        taken to minimize computation if this function is not used.
    */
    Segment_ID id() const;

    //! Comparison for equality.
    inline bool operator == (const Segment & rhs) const
    {
        return _begin_point_id == rhs._begin_point_id;
    }
    // Note: operator != cannot be added, otherwise 
    // Segments::const_iterator i, j; i != j; won't compile

    inline void print() const;

private:
    //! ID  of the morphology point of the segment start point.
    /*!
        This ID is local/relative to the neuron morphology.
    */
    Morphology_Point_ID begin_point_id() const
    {
        return _begin_point_id;
    }

    Segment_Cross_Section interpolate (
		const Segment_Cross_Section & begin, 
		const Segment_Cross_Section & end, 
		Segment_Normalized_Distance norm_distance) const
	{
		return Segment_Cross_Section(
			begin.center() + 
            (end.center() - begin.center()) * (Micron) norm_distance,
			begin.diameter() * (1.0f - (Micron) norm_distance) + 
            end.diameter()   * (Micron)norm_distance);
	}

    const Morphology   * _morphology;
    Morphology_Point_ID  _begin_point_id;
};

//! outputs the begin and end points of the segment to the output stream
inline std::ostream & operator << (std::ostream & lhs, const Segment & rhs)
{
    lhs << "Segment point " << ": coordinates (begin=" 
        << rhs.begin().center() << ", end=" << rhs.end().center() << "), "
        << "diameter (begin=" << rhs.begin().diameter() << ", end="
        << rhs.end().diameter() << ")" << std::endl; 
    return lhs;
}

// ----------------------------------------------------------------------------
}
#include "Morphology.h"

namespace bbp
{

// ----------------------------------------------------------------------------

Segment::Segment(const Morphology        * morphology, 
                 const Vector_3D<Micron> & soma_position, 
                 const Rotation_3D<Micron>          & neuron_rotation, 
                 Morphology_Point_ID       begin_point_id) 
:
    _morphology(morphology),
    _begin_point_id(begin_point_id)
{
    bbp_assert (morphology != 0);

    Interval<Segment_Cross_Section>::begin.center(soma_position 
        + (neuron_rotation 
        * morphology->position(_begin_point_id)));
    Interval<Segment_Cross_Section>::end.center(soma_position 
        + (neuron_rotation 
        * morphology->position(_begin_point_id + 1)));
    Interval<Segment_Cross_Section>::begin.diameter(
        morphology->diameter(_begin_point_id));
    Interval<Segment_Cross_Section>::end.diameter(
        morphology->diameter(_begin_point_id + 1));
}

// ----------------------------------------------------------------------------

Segment::Segment(const Morphology        * morphology, 
                 Morphology_Point_ID       begin_point_id) 
:
    _morphology(morphology),
    _begin_point_id(begin_point_id)
{
    bbp_assert (morphology != 0);

    Interval<Segment_Cross_Section>::begin.center(
        morphology->position(_begin_point_id ));
    Interval<Segment_Cross_Section>::end.center(
        morphology->position(_begin_point_id  + 1));
    Interval<Segment_Cross_Section>::begin.diameter(
        morphology->diameter(_begin_point_id ));
    Interval<Segment_Cross_Section>::end.diameter(
        morphology->diameter(_begin_point_id + 1));
}

// ----------------------------------------------------------------------------

void Segment::print() const
{
    std::cout << * this;
}

// ----------------------------------------------------------------------------

}
#endif
