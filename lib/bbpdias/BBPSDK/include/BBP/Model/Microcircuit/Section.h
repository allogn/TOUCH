/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible author:     Thomas Traenkler
        Contributing authors:   Nikolai Chapochnikov
                                Juan Hernando Vieites
*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SECTION_H
#define BBP_SECTION_H

#include "Types.h"
#include "BBP/Common/Math/Geometry/Vector_3D.h"
#include "BBP/Common/Math/Geometry/Rotation.h"

namespace bbp {

class Microcircuit;
class Morphology;
class Segments;
class Segment;
class Compartments;
class Compartment;
class Neuron;
class Sections;

// ----------------------------------------------------------------------------

//! Interface to a neuron section.
/*! 
    A section is a cylindrical, unbranched cable volume.
    In this implementation a section is considered either a neurite
    (i.e. a dendrite or axon part) or a soma. This is
    like the NEURON section (there the Soma is also a section).
    \ingroup Microcircuit
*/
class Section
{
    friend class Sections;
    friend class _Global_Section_Storage_Adapter;
    friend class _Section_Storage_Adapter;
    friend class Morphology;
    friend class Neuron;
    friend class Soma;
    friend std::ostream & operator<< (std::ostream & lhs, const Section & rhs);

protected:
    //! This is reserved for future extensions of modification support. (TT)
    inline Section() {}

    //! Creates a new section relative to the center of the morphology.
    inline Section(const Morphology * morphology, Section_ID id);
    //! Creates a new section relative to the microcircuit center.
    /*!
        Creates a new morphology section, and applies the neuron translation
        and rotation in the microcircuit.
    */
    inline Section(const Morphology     * morphology, 
                   const Neuron         * neuron, 
                   const Microcircuit   * microcircuit, 
                         Section_ID       id);
    //! Creates a new section relative to the microcircuit center.
    /*!
        Creates a new morphology section, and applies the neuron translation
        and rotation in the microcircuit.
        This version receives a precomputed rotation object for speeding-up.
    */
    inline Section(const Morphology     * morphology, 
                   const Neuron         * neuron, 
                   const Microcircuit   * microcircuit, 
                   const Rotation_3D<Micron>       & neuron_rotation, 
                         Section_ID       id);

public:
    //! Copy constructor
    inline Section(const Section & section);

    //! Assignment operator
    inline Section & operator = (const Section & section);

    //! Get the local section identifier (inside a neuron morphology).
    inline Section_ID id() const;

    //! Get the parent of this section.
    /*! 
        Asking for the parent of the Soma Section will throw an exception.
        As a stop condition you can either catch that exception 
        (Section_Not_Found) or use 
        \code
        Section_ID id = 23;
        Sections sections = neuron.sections();
        Sections::iterator current_section = sections.find(id);
        if (current_section != sections.end()
        {
            while (current_section->parent().type() != SOMA)
            {
                ...
            }
        }
        \endcode
    */
    inline const Section parent() const;

    //// Get the parent of this section.
    ///* 
    //    \return Returns an iterator to the parent section of this section.
    //    If the parent section is the soma, the Sections::end() iterator is
    //    returned to indicate this condition. The soma cannot be returned since
    //    it is currently not considered a section in the Blue Brain Project,
    //    in contrast to how the NEURON simulator handles it.
    //    \todo Should return soma Section if it can inherit from Section. (TT)
    //*/
    //inline Sections::const_iterator parent() const
    //{
    //    ...
    //    two cases, one is global, one is local
    //    first find the section in the neurites() container
    //    
    //    Section_ID parent = _morphology->parent_section(_id);
    //    if (parent == 0)
    //        return *this;
    //    else
    //        return Section(_morphology, _neuron, _microcircuit, _neuron_rotation,
    //                       parent);
    //}


    //! Get the children of this section
    /*! When this section is the soma it will return the first order sections
        connected to it */
    inline const Sections children() const;
    
    //! Get a specified segment.
    /*!
       @param id A segment number between 0 and the segment count of this 
                 section.
        \sa Segment
    */
    inline const Segment segment(Segment_ID id) const;

    ///* \brief access the segment inside this section at the specified 
    //           normalized distance (0.0-1.0) */
    //inline Segment segment(Section_Normalized_Distance normalized_distance);
    
    // // Get a specified compartment.
    // /*
    //     \todo Commented during the SWIG wrapping because not implmented (SL)
    //     \sa Compartment
    // */
    // inline const Compartment compartment(Compartment_ID id) const;

    //! Get a temporary container with all morphology segments of this section.
    /*!
        \sa Segment
    */
    inline const Segments segments() const;

    //! Returns the number of segments of this section
    inline Segment_ID number_of_segments() const;


    //! Returns whether this section has compartment data associated or not.
    inline bool has_compartments() const;

    //! Get container with all electrical compartments of this section.
    /*!
        \sa Compartment
    */
    inline const Compartments compartments() const;
    
    //! Returns the number of compartments of this section.
    inline Compartment_ID number_of_compartments() const;

    /*! \brief Access the compartment inside this section at the specified 
               normalized distance (0.0-1.0). 
        If the section doesn't have any compartments the behaviour is 
        undefined. */
    inline const Compartment compartment(Section_Normalized_Distance t) const;

    /*! \brief Returns the compartment corresponding to the midpoint of
               the given segment. 
        If the segment doesn't belong to this section the behaviour is 
        undefined. */
    inline const Compartment compartment(const Segment & segment) const;

    //! print section information
    inline void print() const;

    // GEOMETRY _______________________________________________________________

    /* \todo Commented during the SWIG wrapping because not implemented (SL) */
    // //! Get soma relative position of a section point.
    // inline const Vector_3D<Micron> & position(
    //     Morphology_Point_ID point_id = 0) const;

    //! Get length of the section cable.
    inline Micron length() const;


    //! Return the normalized distance of a segment wihtin this section.
    /*! If middle_point is true then the distance for the middle of the segment
      will be returned. Otherwise it is the distance of the segment start. */
    inline Section_Normalized_Distance section_distance
        (Segment_ID segment, bool middle_point = true) const;

    //! Overlodaded function provided for convenience.
    /*! Look at the documentation of the function above */
    inline Section_Normalized_Distance section_distance
        (const Segment & segment, bool middle_point = true) const;

    // CLASSIFICATION _________________________________________________________

    //! Get branch order of the section in the morphology.
    /*! 
        Tree branch order relative to the root section. Soma is 0. 
        \todo Check if soma is returned correctly as 0. (TT)
    */
    inline Section_Branch_Order     branch_order() const;

    //! Get type of the section.
    inline Section_Type             type() const;

    // DYNAMICS _______________________________________________________________

    //// Get the voltage of specified compartment.
    //inline Millivolt & voltage(Compartment_ID compartment = 0) const;

    // OPERATORS ______________________________________________________________

    inline bool operator == (const Section & rhs) const;

private:
    // The attributes could move in a global dataset and only the array index
    // stored here. (TT)
    //! \todo: This should be Neuron instead, Neuron : public Morphology.
    // This way attributes can be reduced and section dynamics supported.
    // Think about how to solve unique morphology storage issue first. 
    // E.g. morphology data could be outsourced in a global dataset. (TT)
    const Morphology       * _morphology;
    //! The neuron pointer for globally placed sections.
    const Neuron           * _neuron;
    const Microcircuit     * _microcircuit;
    //! Attribute stored only for speeding up global calculations.
    Rotation_3D<Micron>      _neuron_rotation;
    Section_ID               _id;
};


// ----------------------------------------------------------------------------

}
#include "BBP/Model/Microcircuit/Containers/Segments.h"
#include "BBP/Model/Microcircuit/Containers/Compartments.h"
#include "BBP/Model/Microcircuit/Morphology.h"
#include "BBP/Model/Microcircuit/Neuron.h"

namespace bbp
{

// ----------------------------------------------------------------------------

inline std::ostream & operator << (std::ostream & lhs, const Section & rhs);

// ----------------------------------------------------------------------------

Section::Section(const Morphology * morphology, 
                       Section_ID   id) 
:
    _morphology(morphology), 
    _neuron(0),
    _microcircuit(0),
    _id(id)
{
}

// ----------------------------------------------------------------------------

Section::Section(const Morphology   * morphology,
                 const Neuron       * neuron, 
                 const Microcircuit * microcircuit,
                       Section_ID     id) 
:
    _morphology(morphology),
    _neuron(neuron),
    _microcircuit(microcircuit),
    _neuron_rotation(Rotation_3D<Micron>(neuron->orientation())),
    _id(id)
{
}

// ----------------------------------------------------------------------------

Section::Section(const Morphology   * morphology,
                 const Neuron       * neuron, 
                 const Microcircuit * microcircuit,
                 const Rotation_3D<Micron>     & neuron_rotation, 
                       Section_ID     id) 
:
    _morphology(morphology),
    _neuron(neuron),
    _microcircuit(microcircuit),
    _neuron_rotation(neuron_rotation),
    _id(id)
{
}

// ----------------------------------------------------------------------------

Section::Section(const Section & section) :
    _morphology(section._morphology),
    _neuron(section._neuron),
    _microcircuit(section._microcircuit),
    _id(section._id)
{
    if (_neuron)
    {
        // Only copying the rotation is needed this makes copies cheaper
        _neuron_rotation = section._neuron_rotation;
    }
}

// ----------------------------------------------------------------------------

Section & Section::operator = (const Section & other)
{
    _morphology = other._morphology;
    _neuron = other._neuron;
    _microcircuit = other._microcircuit;
    _id = other._id;
    if (_neuron)
    {
        // Only copying the rotation is needed this makes copies cheaper
        _neuron_rotation = other._neuron_rotation;
    }

    return *this;
}

// ----------------------------------------------------------------------------

const Segment Section::segment(Segment_ID id) const
{
    bbp_assert(_id != 0); // soma section has no segments
    bbp_assert(number_of_segments() > id);
    if (_neuron)
    {
        return Segment(_morphology, _neuron->position(), _neuron_rotation, 
                       _morphology->section_start_points()[_id] + id);
    }
    else
    {
        return Segment(_morphology, 
                       _morphology->section_start_points()[_id] + id);
    }
}

// ----------------------------------------------------------------------------

const Segments Section::segments() const
{
    /*!
        \todo Put this back in when soma has segments. (TT)
    */
    //bbp_assert(number_of_segments() > 0);
    if (_neuron)
    {
        if (this->type() != SOMA)
        {
            return Segments(_morphology, _neuron->position(), _neuron_rotation,
                            _morphology->section_start_points()[_id], 
                            _morphology->section_start_points()[_id] + 
                            _morphology->section_point_counts()[_id] - 1);
        }
        else // soma section has no segments
        {
            return Segments(_morphology, _neuron->position(), _neuron_rotation,
                            _morphology->section_start_points()[_id], 
                            _morphology->section_start_points()[_id]);
        }
    }
    else
    {
        if (this->type() != SOMA)
        {
            return Segments(_morphology,
                            _morphology->section_start_points()[_id], 
                            _morphology->section_start_points()[_id] + 
                            _morphology->section_point_counts()[_id] - 1);
        }
        else // soma section has no segments
        {
            return Segments(_morphology,
                            _morphology->section_start_points()[_id], 
                            _morphology->section_start_points()[_id]);
        }
    }
}
            
// ----------------------------------------------------------------------------

Segment_ID Section::number_of_segments() const
{
    if (this->type() != SOMA)
        return _morphology->section_point_counts()[_id] - 1;
    else
        return 0;
}

// ----------------------------------------------------------------------------

bool Section::has_compartments() const
{
    if (_neuron != 0 && _microcircuit != 0)
    {
        const Compartment_Report_Frame<Millivolt>::Context & context =
            _microcircuit->dataset().dynamics->compartment_voltages.context();
        Cell_Index cell_index = _neuron->index();
        return context.mapping()->number_of_compartments(cell_index, _id) != 0;
    }
    else
        return false;
}

// ----------------------------------------------------------------------------

const Compartments Section::compartments() const
{
    Compartments compartments(*_microcircuit);

    if (_neuron == 0 || _microcircuit == 0)
        return compartments;

    const Compartment_Report_Frame<Millivolt>::Context & context =
        _microcircuit->dataset().dynamics->compartment_voltages.context();
    Cell_Index cell_index = _neuron->index();
    Compartment_Count num_compartments =
        context.mapping()->number_of_compartments(cell_index, _id);


    if (num_compartments != 0)
    {        
        Report_Frame_Index start = 
            context.mapping()->section_offset(cell_index, _id);
        Report_Frame_Index end = start + num_compartments - 1;
        compartments.insert_range(Compartment_Index(start, cell_index),
                                  Compartment_Index(end, cell_index));
    }

    return compartments;
}

// ----------------------------------------------------------------------------
    
Section_Branch_Order Section::branch_order() const
{
    return _morphology->section_branch_orders()[this->id()];
}

// ----------------------------------------------------------------------------

Compartment_ID Section::number_of_compartments() const
{
    if (_neuron == 0 || _microcircuit == 0)
        return 0;
    const Compartment_Report_Frame<Millivolt>::Context & context =
        _microcircuit->dataset().dynamics->compartment_voltages.context();
    Cell_Index cell_index = _neuron->index();
    return context.mapping()->number_of_compartments(cell_index, _id);
}

// ----------------------------------------------------------------------------

const Compartment Section::compartment(Section_Normalized_Distance t) const
{
    bbp_assert(_neuron != 0 && _microcircuit != 0);
    const Compartment_Report_Frame<Millivolt>::Context & context =
        _microcircuit->dataset().dynamics->compartment_voltages.context();
    Cell_Index cell_index = _neuron->index();
    double num_compartments =
        context.mapping()->number_of_compartments(cell_index, _id);
    bbp_assert(num_compartments != 0);

    Compartment_Count index = 
        Compartment_Count(t == 1 ? num_compartments - 1 : 
                                   std::floor(num_compartments * t));
   
    return Compartment(_microcircuit, 
                       context.mapping()->section_offset(cell_index, _id) +
                       index, cell_index);
}

// ----------------------------------------------------------------------------

const Compartment Section::compartment(const Segment & segment) const
{
    const Section_Normalized_Distance * distances =
        _morphology->point_relative_distances();
    return compartment(0.5f * (distances[segment.begin_point_id()] +
                              distances[segment.begin_point_id() + 1]));
}

// ----------------------------------------------------------------------------

Section_ID Section::id() const
{
    return _id;
}

// ----------------------------------------------------------------------------

const Section Section::parent() const
{
    Section_ID parent = _morphology->parent_section(_id);
    if (parent == UNDEFINED_SECTION_ID)
        throw_exception(Section_Not_Found(), SEVERE_LEVEL, __FILE__, __LINE__);
    return Section(_morphology, _neuron, _microcircuit, _neuron_rotation,
                       parent);
}

// ----------------------------------------------------------------------------

const Sections Section::children() const
{
    // Make sure if this is or is not a soma section
    if (this->type() != SOMA)
    {
        typedef std::vector<Section_ID> Section_ID_List;
        const Section_ID_List & children =
            _morphology->children_sections(_id);
        Sections result(_morphology, _neuron, _microcircuit);
        for (Section_ID_List::const_iterator i = children.begin();
             i != children.end(); ++i)
        {
            result.insert(*i);
        }
        return result;
    }
    else // if soma section things work differently
    {
        // check whether this is a morphology or a neuron placed in a microcircuit
        if (_neuron == 0)
        {
            // get all first order sections from morphology
            Sections sections(_morphology);
            for (size_t i = 0; i < _morphology->_first_order_section_count; ++i)
            {
                sections.insert(_morphology->_first_order_sections[i]);
            }
            return sections;
        }
        else // in this case we have a real neuron, not a morphology class
        {
            Sections result(& _neuron->morphology(), 
                            _neuron, 
                            _neuron->_microcircuit.lock().get());

            // get all first order sections from morphology
            Sections sections(& _neuron->morphology());
            for (size_t i = 0; 
                i < _neuron->morphology()._first_order_section_count; ++i)
            {
                sections.insert(_neuron->morphology()._first_order_sections[i]);
            }

            // make them relative to a neuron placed in a microcircuit
            for (Sections::const_iterator i = sections.begin(); 
                 i != sections.end();
                 ++i)
            {
                result.insert(i->id());
            }
            return result;
        }
    }
}

// ----------------------------------------------------------------------------

Micron Section::length() const
{
    if (this->type() != SOMA)
        return _morphology->section_lengths()[_id];
    else
        return UNDEFINED_MICRON;
}

// ----------------------------------------------------------------------------

Section_Normalized_Distance Section::section_distance
(Segment_ID segment, bool middle_point) const
{
    Morphology_Point_ID point_id = 
        _morphology->section_start_points()[_id] + segment;
    if (middle_point)
        return ((_morphology->point_relative_distances()[point_id] + 
                 _morphology->point_relative_distances()[point_id + 1]) / 2);
    else
        return _morphology->point_relative_distances()[point_id];
}

// ----------------------------------------------------------------------------

Section_Normalized_Distance Section::section_distance
(const Segment &segment, bool middle_point) const
{
    Morphology_Point_ID point_id = segment.begin_point_id();
    if (middle_point)
        return ((_morphology->point_relative_distances()[point_id] + 
                 _morphology->point_relative_distances()[point_id + 1]) / 2);
    else
        return _morphology->point_relative_distances()[point_id];
}

// ----------------------------------------------------------------------------

Section_Type Section::type() const
{
    return _morphology->section_types()[_id];
}

// ----------------------------------------------------------------------------

std::ostream & operator << (std::ostream & lhs, const Section & rhs)
{
    lhs << "Section (id = " << rhs.id();
    // section type
    lhs << ", type = ";
    if (rhs.type() == SOMA)
        lhs << "soma";
    else if (rhs.type() == AXON)
        lhs << "axon";
    else if (rhs.type() == DENDRITE)
        lhs << "dendrite";
    else if (rhs.type() == APICAL_DENDRITE)
        lhs << "apical dendrite";
    else
        lhs << "undefined";

    lhs << ", branch order = " << rhs.branch_order();
    Section_ID parent = rhs._morphology->parent_section(rhs._id);
    if (parent == UNDEFINED_SECTION_ID)
        lhs << ", root";
    else
        lhs << ", parent = " << parent;

    lhs << ") = \n{\n";
    Segments segments = rhs.segments();
    for (Segments::const_iterator segment = segments.begin(); 
        segment != segments.end(); ++segment)
    {
        Segments::const_iterator temp = segment;
        ++temp;
        lhs << "\tcenter = " << segment->begin().center() 
            << ", \tdiameter = " << segment->begin().diameter();
        if (temp != segments.end())
            lhs << ",\n";
        else
            lhs << ",\n"
                << "\tcenter = " << segment->end().center() 
                << ", \tdiameter = " << segment->end().diameter()
                << "\n";
    }
    lhs << "}\n\n";

//     // compartments
//     lhs << "Compartments: " << rhs.compartments.size() << std::endl;
//     for (Count i = 0; i < rhs.compartments.size(); ++i)
//     {
//         lhs << rhs.compartment(i);
//     }
    return lhs;
}

// ----------------------------------------------------------------------------

//Compartment Section::compartment(Compartment_ID id)
//{
//    return Compartment(cell_gid,
//        section_compartments_offset(cell_gid, section_id) + 
//        id);
//}

// ----------------------------------------------------------------------------

//Millivolt & Section::voltage(Compartment_ID compartment = 0)
//{
//    return (* compartment_voltage)(cell_gid, 
//        section_compartments_offset(cell_gid, section_id) +
//        compartment);
//}

// ----------------------------------------------------------------------------

bool Section::operator == (const Section & rhs) const
{
    return (_id == rhs._id &&
            _morphology == rhs._morphology &&
            _neuron == rhs._neuron &&
            _microcircuit == rhs._microcircuit);
}

// ----------------------------------------------------------------------------

void Section::print() const
{
    std::cout << *this;
}

// ----------------------------------------------------------------------------

}
#endif
