/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Juan Hernando Vieites
        Contributing authors:   Sebastien Lasserre
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MORPHOLOGY_H
#define BBP_MORPHOLOGY_H

#include "Types.h"
#include "BBP/Model/Microcircuit/Datasets/Morphology_Dataset.h"
#include "BBP/Model/Microcircuit/Datasets/Mesh_Dataset.h"

namespace bbp {

class Microcircuit_Reader;
class Sections;
class Section;
class Soma;
class Mesh;
typedef boost::shared_ptr<Mesh> Mesh_Ptr;

class Morphology;
typedef boost::shared_ptr<Morphology> Morphology_Ptr;
typedef boost::shared_ptr<const Morphology> Const_Morphology_Ptr;



// ----------------------------------------------------------------------------

//! Interface to the shape (morphology) of a neuron.
/*!
    Morphology class provides access to neuron specific information (e.g.
    position, electrophysiology and morphology type) as well as to parts of
    the neuron (e.g. soma, dendrite, section, compartment)

    \todo Functionality is top priority at the moment - at the cost of 
    performance. However this does not mean the library 
    should not be optimized later on when needed. 
    In general, precomputation and memory use 
    profiles would be beneficial to decide on each case. For now, this
    is a hard-coded decision due to limit developer resources. (TT)
	\bug Potential bug: Section 0 is no longer always necessarily soma
	if I remember correctly. Review code if this is considered. (TT)
    \todo Consider the checks in apical_dendrites() axon(), etc... to be either
    commented or centralized in a single location (e.g. insert_range()) so
    other developers get the purpose of the checks. (TT/JH)
    \ingroup Microcircuit
*/
class Morphology
    : public Morphology_Dataset
{
    friend class Microcircuit_Reader;
    friend class Mesh_File_Reader;
    friend class Neuron_Accessor;
    friend class Section;
    friend class Segment;
    friend class Soma;
    friend class bbp::rw::Morphology;

public:
    Morphology() {}
    inline Morphology(const Morphology & rhs);
    /*
    inline Morphology(const bbp::rw::Morphology & rhs)
    {
    _label = rhs._label;
    _origin = rhs._origin;
    _morphology_type = rhs._morphology_type;
    
    _point_count = ...
    _section_count = rhs.size();
    
    // soma points are soma membrane surface points
    // not points of cylindrical segments
    //! \bug Potential bug: What if there is no soma? (TT)
    _soma_offset = 0;
    //! \bug Potential bug: What if there is no axon? (TT)
    _axon_offset = 1;
    _basal_dendrite_offset = _axon_offset + rhs.axon().size();
    _apical_dendrite_offset = _basal_dendrite_offset + rhs.basal_dendrites().size();
    Box<Micron> bounding_box = rhs.bounding_box();
    _max_values = bounding_box.maximum();
    _min_values = bounding_box.minimum();

    _section_start_points.reset(new Morphology_Point_ID[_section_count]);

    _section_point_counts.reset(new Morphology_Point_ID[_section_count]);
    _section_parent_sections.reset(new Morphology_Point_ID[_section_count]);
    _section_children_sections.reset(new Morphology_Point_ID[_section_count * MAX_CHILDREN]);
    for (size_t i = 0; i < _section_count * MAX_CHILDREN; ++i)
    {
        _section_children_sections[i] = UNDEFINED_SECTION_ID;
    }


    _first_order_section_count = rhs.soma().children().size();
    _first_order_sections.reset(new Section_ID [_first_order_section_count];
    size_t j = 0;
    for (Sections::const_iterator i = rhs.soma().children().begin(); i != rhs.soma().children().end(); ++i)
    {
        _first_order_sections[j] = i->id();
        ++j;
    }
    
    _section_branch_orders = ...
    _section_types = ...
    _section_lengths = ...

    _section_start_points = ...
    _section_point_counts = ...
    _section_parent_sections = ...
    _section_children_sections = ...
    _first_order_sections = ...
    _first_order_section_count = ...
    _section_branch_orders = ...
    _section_types = ...
    _section_lengths = ...

    _point_positions = ...
    _point_diameters = ...
    _point_relative_distances = ...



    _point_positions = ...
    _point_diameters = ...
    _point_relative_distances = ...

    _mesh = rhs._mesh;
//#ifndef NDEBUG
//    std::cout << "Copy construction of bbp::rw::Morphology from "
//        "bbp::Morphology finished." << std::endl;
//#endif
    }*/

    // IDENTITY ---------------------------------------------------------------

    //! returns the name of the morphology
    inline const Label                  & label() const;

    // CLASSIFICATION ---------------------------------------------------------

    //! Get the morphology type (m-type) of this cell.
    /*!
        The morphology (geometrical form) of the cell is classified into 
        different phenomenological classes (m-types):  pyramidal cell, 
        small basket cell, ...)
        \return m-type of the neuron
    */
    inline Morphology_Type                    type() const;
    //! Origin of the reconstructed morphology model (cloned, repaired, ...)
    inline Morphology_Reconstruction_Origin   origin() const;
    
    // CELL PARTS -------------------------------------------------------------

    //! Get a single section of the morphology specified by its ID.
    inline const Section           section(Section_ID section_id) const;
    //! Get cell body of this neuron.
    inline const Soma              soma() const;
    //! Get all dendrite and axon sections of this neuron (excluding soma).
    /*
        \bug The returned container should be allocated by default so
        there is one less pitfall for users with using iterators of
        two different temporary containers. Same for other temporary
        Sections containers. (TT)
    */
    inline const Sections          neurites() const;
    //! Get axon sections of this neuron.
    inline const Sections          axon() const;
    //! Get all dendrite sections of this neuron.
    inline const Sections          dendrites() const;
    //! Get all basal dendrite sections of this neuron.
    /*!
        Note dendrites are by default considered basal dendrites.
    */
    inline const Sections          basal_dendrites() const;
    //! Get all apical dendrite sections of this neuron.
    /*! 
        An empty container is returned if no apical dendrites are present 
    */
    inline const Sections          apical_dendrites() const;
    //! Get all sections of this neuron.
    /*!
        \bug Potential bug: Make sure the soma is in these. (TT)
    */
    inline const Sections          all_sections() const;
    //! Get first order sections
    /*!
        \deprecated Will be removed in future builds and replaced by
        const Sections Soma::children() const.
    */
    inline const Sections          first_order_sections() const;
    //! Get the parent section of the current section.
    /*!
        \section the ID of the section, may be soma section ID 0 also
        \return the ID of the parent section of the specified section, if
        soma section 0 has been passed, UNDEFINED_SECTION_ID is returned
        \bug Potential bug: Changed behavior to return UNDEFINED_SECTION_ID
        if soma section id 0 is passed.
    */
    inline Section_ID              parent_section(Section_ID section) const;
    //! Get the parent section of the current section.
    inline std::vector<Section_ID> children_sections(Section_ID section) const;

    // GEOMETRY ---------------------------------------------------------------

    //! Returns an axis-aligned box geometry that cointains the morphology.
    inline void  bounding_box(Vector_3D<Micron> &min_corner,
                              Vector_3D<Micron> &max_corner)
        const;

    //! soma relative position of a location on the morphology
    inline const Vector_3D<Micron> & position(
        Morphology_Point_ID point_id) const;

    //! soma relative position of a location on the morphology
    inline Vector_3D<Micron> position(
        Section_ID section, Section_Normalized_Distance distance) const;
    //! soma relative position of a location on the morphology
    inline Vector_3D<Micron> position(
        Section_ID section, Segment_ID segment, 
        Segment_Normalized_Distance segment_distance) const;

    //! Get diameter of a point on the morphology.
    inline Micron diameter(Morphology_Point_ID point_id) const;
    //! Get diameter of a point on the morphology.
    inline Micron diameter(Section_ID section, 
                           Section_Normalized_Distance distance) const;
    //! Get diameter of a point on the morphology.
    inline Micron diameter(
        Section_ID section, Segment_ID segment, 
        Segment_Normalized_Distance segment_distance) const;


    //! return total number of points in this morphology
    inline Morphology_Point_ID number_of_points() const;
    //! return total number of sections in this morphology
    /*! The section count includes the section used to define the soma in
        the source dataset. */
    inline Section_ID          number_of_sections() const;
    //! return total number of sections of a given type in this morphology
    inline Section_ID          number_of_sections(enum Section_Type) const;
    //! return total number of segments of a given section
    /*!
        \bug Could be all of the segments of a neuron. Since the type 
        Segment_ID is section local, this could potentially overflow depending
        on the size of the type used! (TT)
    */
    inline Segment_ID          number_of_segments(Section_ID id) const;

    //! Get length of specified section.
    inline Micron              length(Section_ID section_id) const;
    //! Get length of specified segment.
    inline Micron              length(Section_ID section_id, 
                                      Segment_ID segment_id) const;
    /*!
        \brief Get normalized distance of a segment from section start.
        
        If middle_point is true the distance will be reported for the middle
        point of the segment, otherwise it will be for the first segment point.
    */
    inline Section_Normalized_Distance section_distance(
         Section_ID section, Segment_ID segment, 
         bool middle_point = true) const;

    //! Mesh associated with this morphology.
    /*! Undefined behaviour when mesh hasn't been loaded. */
    inline const Mesh              & mesh() const;

    //! Returns whether there is a mesh for this morphology loaded or not.
    inline bool                      mesh_loaded() const;

    //! Get length of path to soma from specified point on a neurite.
    /*!
        \param section the index of the section of the destination point.
        \param distance the normalized distance inside the section 
               between 0.0 and 1.0 from the start of that section.
        \return Will return the distance along the branch path to the soma
        surface. If the section passed is the soma itself (section 0), then
        the distance returned is zero.
    */
    inline Micron path_to_soma
        (Section_ID section, Section_Normalized_Distance distance = 0.0);

    // STATUS INFORMATION -----------------------------------------------------

    //! print neuron status information
    inline void print() const;

protected:
    //! Get the ID of the point on the morphology of the segments begin.
    Morphology_Point_ID segment_begin_point(Section_ID section_id, 
                                    Segment_ID segment_id) const
    {
        bbp_assert (section_id < this->number_of_sections() );
        bbp_assert (segment_id < this->number_of_segments(section_id) );
        return this->section_start_points()[section_id] + segment_id;
    }

    //! Mesh for the morphology
    Mesh_Ptr _mesh;
};

inline std::ostream & operator << (std::ostream & lhs, const Morphology & rhs);

}


// ----------------------------------------------------------------------------

#include "BBP/Model/Microcircuit/Containers/Sections.h"
#include "BBP/Model/Microcircuit/Soma.h"
#include "BBP/Model/Microcircuit/Mesh.h"

namespace bbp
{

// ----------------------------------------------------------------------------

inline Morphology::Morphology(const Morphology & rhs)
    : Morphology_Dataset( (Morphology_Dataset) rhs)
{
    if (_mesh.get() != 0)
        _mesh.reset(new Mesh(*(rhs._mesh)));
}

// ----------------------------------------------------------------------------

inline const Label & Morphology::label() const
{
    return _label;
}

// ----------------------------------------------------------------------------

inline Morphology_Type Morphology::type() const
{
    return _morphology_type;
}

// ----------------------------------------------------------------------------

inline Morphology_Reconstruction_Origin Morphology::origin() const
{
    return _origin;
}

// ----------------------------------------------------------------------------

inline Vector_3D<Micron> Morphology::position(
    Section_ID section_id, Section_Normalized_Distance section_distance) const
{
    bbp_assert(section_distance <= 1.0f && section_distance >= 0.0f);
    // Local variable used for brevity.
    Section_Normalized_Distance *distances = _point_relative_distances.get();
    Morphology_Point_ID section_start = _section_start_points[section_id];
    Morphology_Point_ID section_end = _section_start_points[section_id] + 
        _section_point_counts[section_id] - 1;
    // Checking 0 length sections.
    bbp_assert(section_start != section_end);
 
    Morphology_Point_ID start_point = section_start + ((Morphology_Point_ID) 
        ((_section_point_counts[section_id] - 1) * section_distance));
    // Decreasing start_point by 1 if it equals the section_end.
    start_point -= (start_point == section_end ? 1 : 0);
    Morphology_Point_ID end_point = start_point + 1;
 
    if (section_distance > distances[end_point])
    {
        while ((section_distance > distances[end_point]) && 
               (end_point < section_end))
        {
            ++end_point;
        }
        start_point = end_point - 1;
    }
    else if (section_distance < distances[start_point])
    {
        while ((section_distance < distances[start_point]) &&
               (start_point > section_start ))
        {
            --start_point;
        }
        end_point = start_point + 1;
    }

    double t = (section_distance - distances[start_point]) /
               (distances[end_point] - distances[start_point]);
    return (_point_positions[start_point] * Micron(1.0 - t) +
            _point_positions[end_point] * (Micron) t);
}

// ----------------------------------------------------------------------------

inline const Vector_3D<Micron> & Morphology::
    position(Morphology_Point_ID point_id) const
{
    return _point_positions[point_id];
}

// ----------------------------------------------------------------------------

inline Vector_3D<Micron> Morphology::position(
    Section_ID section, Segment_ID segment, 
    Segment_Normalized_Distance segment_distance) const
{
    Morphology_Point_ID section_start = _section_start_points[section];
    Vector_3D<Micron> segment_begin = 
        _point_positions[section_start + segment];
    Vector_3D<Micron> segment_end = 
        _point_positions[section_start + segment + 1];

    return segment_begin + (segment_end - segment_begin) * 
        (Micron) segment_distance;
}

// ----------------------------------------------------------------------------

//! Get diameter of a point on the morphology.
inline Micron Morphology::diameter(
        Section_ID section, Segment_ID segment, 
        Segment_Normalized_Distance segment_distance) const
{
#ifdef BBP_SAFETY_MODE
    if (segment_distance < 0.0f && segment_distance > 1.0f + 10e-7)
    {
        log_message("Segment distance value out of bounds: " + 
                    boost::lexical_cast<std::string>(segment_distance), 
                    SEVERE_LEVEL, __FILE__, __LINE__);
        if (segment_distance < 0.0f)
            segment_distance = 0;
        if (segment_distance > 1.0)
            segment_distance = 1.0;
    }
#else
    bbp_assert(segment_distance >= 0.0f && segment_distance <= 1.0f + 10e-7 );
#endif
    Morphology_Point_ID section_start = _section_start_points[section];
    Micron segment_begin = 
        _point_diameters[section_start + segment];
    Micron segment_end = 
        _point_diameters[section_start + segment + 1];

    return segment_begin + (segment_end - segment_begin) * 
        (Micron) segment_distance;
}

// ----------------------------------------------------------------------------

inline Micron Morphology::diameter(Morphology_Point_ID point_id) const
{
    return _point_diameters[point_id];
}

// ----------------------------------------------------------------------------

inline Micron Morphology::diameter(
    Section_ID section_id, Section_Normalized_Distance section_distance) const
{
#ifdef BBP_SAFETY_MODE
    if (section_distance < 0.0f && section_distance > 1.0f + 10e-7)
    {
        log_message("Section distance value out of bounds: " + 
                    boost::lexical_cast<std::string>(section_distance), 
                    SEVERE_LEVEL, __FILE__, __LINE__);
        if (section_distance < 0.0f)
            section_distance = 0;
        if (section_distance > 1.0)
            section_distance = 1.0;
    }
#else
    bbp_assert(section_distance >= 0.0f && section_distance <= 1.0f + 10e-7 );
#endif
    // Local variable used for brevity.
    Section_Normalized_Distance *distances = _point_relative_distances.get();
    Morphology_Point_ID section_start = _section_start_points[section_id];
    Morphology_Point_ID section_end = _section_start_points[section_id] + 
        _section_point_counts[section_id] - 1;
    // Checking 0 length sections.
    bbp_assert(section_start != section_end);

    Morphology_Point_ID start_point = section_start + ((Morphology_Point_ID) 
        ((_section_point_counts[section_id] - 1) * section_distance));
    // Decreasing start_point by 1 if it equals the section_end.
    start_point -= (start_point == section_end ? 1 : 0);
    Morphology_Point_ID end_point = start_point + 1;

    if (section_distance > distances[end_point])
    {
        while ((section_distance > distances[end_point]) && 
               (end_point < section_end))
        {
            ++end_point;
        }
        start_point = end_point - 1;
    }
    else if (section_distance < distances[start_point])
    {
        while ((section_distance < distances[start_point]) &&
               (start_point > section_start ))
        {
            --start_point;
        }
        end_point = start_point + 1;
    }

    Section_Normalized_Distance t = 
               (section_distance - distances[start_point]) /
               (distances[end_point] - distances[start_point]);

    return (Micron)(_point_diameters[start_point] * (1 - t) +
                    _point_diameters[end_point] * t);
}

// ----------------------------------------------------------------------------

inline Section_Normalized_Distance 
Morphology::section_distance(Section_ID section, Segment_ID segment, 
                             bool middle_point) const
{
    Morphology_Point_ID point_id = _section_start_points[section] + segment;
    if (middle_point)
        return ((_point_relative_distances[point_id] + 
                 _point_relative_distances[point_id + 1]) / 2);
    else
        return _point_relative_distances[point_id];
}

// ----------------------------------------------------------------------------

inline Morphology_Point_ID Morphology::number_of_points() const
{
    return _point_count;
}

// ----------------------------------------------------------------------------

inline Section_ID Morphology::number_of_sections() const
{
    return _section_count;
}

// ----------------------------------------------------------------------------

inline Section_ID Morphology::number_of_sections(Section_Type type) const
{
    switch(type)
    {
    case SOMA: 
        bbp_assert(axon_offset() == 1); 
        return 1;
    case AXON:
        bbp_assert(axon_offset() < basal_dendrite_offset()); 
        return basal_dendrite_offset() - axon_offset();
    case DENDRITE:
        if (basal_dendrite_offset() == UNDEFINED_SECTION_ID)
            return 0;
        else if (apical_dendrite_offset() == UNDEFINED_SECTION_ID) {
            bbp_assert(basal_dendrite_offset() < apical_dendrite_offset());
            return _section_count - basal_dendrite_offset();
        } else
            return apical_dendrite_offset() - basal_dendrite_offset();
    case APICAL_DENDRITE:
        if (apical_dendrite_offset() == UNDEFINED_SECTION_ID)
            return 0;
        else
            return _section_count - apical_dendrite_offset();
    default:
        return UNDEFINED_SECTION_ID;
    }
}

// ----------------------------------------------------------------------------

inline Segment_ID Morphology::number_of_segments(Section_ID section) const
{
    return _section_point_counts[section] - 1;
}

// ----------------------------------------------------------------------------

inline Micron Morphology::length(Section_ID section_id) const
{
    // assures the section is not the soma or invalid.
    bbp_assert(section_id > 0 && section_id < number_of_sections());
    return _section_lengths[section_id];
}

// ----------------------------------------------------------------------------

inline Micron Morphology::length(Section_ID section_id,
                                 Segment_ID segment_id) const
{
    // assures the section is not invalid.
    //! \todo segment parameter check (TT)
//#ifndef NDEBUG
//    std::cout << "Length of section " << section_id << ", segment "
//        << segment_id << " with number of sections " << number_of_sections()
//        << std::endl;
//#endif

//! \bug Check what happens with 9 length segments in some other parts of
// the code.
// Segments of length 0 exist, we cannot abort if we find one of those.
//    bbp_assert((_point_positions[_section_start_points[section_id] + 
//                             segment_id + 1] -
//            _point_positions[_section_start_points[section_id] + 
//                             segment_id]) !=
//           Vector_3D<Micron>(0.0, 0.0, 0.0));

    bbp_assert(section_id < number_of_sections());
    return (_point_positions[
                _section_start_points[section_id] + segment_id + 1] -
            _point_positions[
                _section_start_points[section_id] + segment_id]).length();
}

// ----------------------------------------------------------------------------

Section_ID Morphology::parent_section(Section_ID section) const
{
    bbp_assert(section < number_of_sections());
    if (section == 0)
        return UNDEFINED_SECTION_ID;
    else
        return _section_parent_sections[section];
}

// ----------------------------------------------------------------------------

std::vector<Section_ID> Morphology::children_sections(Section_ID section) const
{
    bbp_assert(section < number_of_sections());
    std::vector<Section_ID> result;

    Section_ID *child = &_section_children_sections[section * MAX_CHILDREN];
    for (Count count = 0;
         *child != UNDEFINED_SECTION_ID && count < MAX_CHILDREN;
         ++child, ++count)
    {
        result.push_back(*child);
    }
    return result;
}

// ----------------------------------------------------------------------------

const Sections Morphology::first_order_sections() const
{
    std::cout << "Morphology::first_order_sections() is deprecated. This "
        "function will be removed from BBP-SDK in one of the next builds. "
        "Please adapt your code to use Soma::children() instead." << std::endl;

    Sections sections(this);
    for (size_t i = 0; i < _first_order_section_count; ++i)
        sections.insert(_first_order_sections[i]);
    return sections;
}


// ----------------------------------------------------------------------------

void Morphology::bounding_box(Vector_3D<Micron> & min_corner,
                              Vector_3D<Micron> & max_corner) const
{
    min_corner = _min_values;
    max_corner = _max_values;
}

// ----------------------------------------------------------------------------

inline Micron Morphology::path_to_soma(
    Section_ID                  section, 
    Section_Normalized_Distance distance)
{
    Section_ID current_section = section;

    // Assert in debug mode that the section is not the 
    // soma or invalid and distance is normalized.
    bbp_assert( section > 0 &&  section < number_of_sections() &&
        distance >= 0.0 && distance <= 1.0 );

    if (section == 0)
        return 0;

    // length inside last section
    Micron path_length = static_cast<Micron>(
        length(current_section) * distance);
    current_section = parent_section(current_section);

    // lengths of sections to soma
    while (current_section != 0)
    {
        path_length += length(current_section);
        current_section = parent_section(current_section);
    }

    return path_length;
}

// ----------------------------------------------------------------------------

inline const Section Morphology::section(Section_ID id) const
{
    return Section(this, id);
}

// ----------------------------------------------------------------------------

inline const Soma Morphology::soma() const
{
    return Soma(* this);
}
    
// ----------------------------------------------------------------------------

inline const Sections Morphology::neurites() const
{
    Sections neurites(this);
    
    if (_axon_offset < _section_count - 1)
    {
        neurites.insert_range(_axon_offset, _section_count - 1);
    }
    else if ((_section_count - _axon_offset) == 1)
    {
        neurites.insert(_axon_offset);
    }

    return neurites;
}

// ----------------------------------------------------------------------------

inline const Sections Morphology::axon() const
{
    Sections axon(this);
    if (_axon_offset < _basal_dendrite_offset - 1)
    {
        axon.insert_range(_axon_offset, _basal_dendrite_offset - 1);
    }
    else if ((_basal_dendrite_offset - _axon_offset) == 1)
    {
        axon.insert(_axon_offset);
    }

    return axon;
}

// ----------------------------------------------------------------------------

inline const Sections Morphology::dendrites() const
{
    Sections dendrites(this);
    if (_basal_dendrite_offset < _section_count - 1)
    {
        dendrites.insert_range(_basal_dendrite_offset, _section_count - 1);
    }
    else if ((_section_count - _basal_dendrite_offset) == 1)
    {
        dendrites.insert(_basal_dendrite_offset);
    }

    return dendrites;
}

// ----------------------------------------------------------------------------

inline const Sections Morphology::basal_dendrites() const
{
    Sections basal(this);
    if (_apical_dendrite_offset == UNDEFINED_SECTION_ID)
    {
        if (_basal_dendrite_offset < _section_count - 1)
        {
            basal.insert_range(_basal_dendrite_offset, 
                               _section_count - 1);
        }
        else if ((_section_count - _basal_dendrite_offset) == 1)
        {
            basal.insert(_basal_dendrite_offset);
        }
    }
    else
    {
        if (_basal_dendrite_offset < _apical_dendrite_offset - 1)
        {
            basal.insert_range(_basal_dendrite_offset, 
                               _apical_dendrite_offset - 1);
        }
        else if ((_apical_dendrite_offset - _basal_dendrite_offset) == 1)
        {
            basal.insert(_basal_dendrite_offset);
        }
    }
    return basal;
}

// ----------------------------------------------------------------------------

inline const Sections Morphology::apical_dendrites() const
{
    Sections apical(this);

    if (_apical_dendrite_offset != UNDEFINED_SECTION_ID)
    {
        if (_apical_dendrite_offset < _section_count - 1)
        {
            apical.insert_range(_apical_dendrite_offset, _section_count - 1);
        }
        else if ((_section_count - _apical_dendrite_offset) == 1)
        {
            apical.insert(_apical_dendrite_offset);
        }
    }
    else
    {
        // do nothing
    }

    return apical;
}

// ----------------------------------------------------------------------------

inline const Sections Morphology::all_sections() const
{
    Sections sections(this);
    if (0 < _section_count - 1)
    {
        sections.insert_range(0, _section_count - 1);
    }
    else if (_section_count == 1)
    {
        sections.insert(0);
    }
    
    return sections;
}

// ----------------------------------------------------------------------------

inline const Mesh & Morphology::mesh() const
{
    bbp_assert(_mesh.get() != 0);
    return *_mesh.get();
}

// ----------------------------------------------------------------------------

inline bool Morphology::mesh_loaded() const
{
    return _mesh.get() != 0;
}

// ----------------------------------------------------------------------------

inline std::ostream & operator << (std::ostream & lhs, const Morphology & rhs)
{
    lhs << "Morphology (" << rhs.label() << ") = { sections: " 
        << rhs.number_of_sections() << " } \n\n";
    Sections all_sections = rhs.all_sections();
    for (Sections::const_iterator i = all_sections.begin();
        i != all_sections.end(); ++i)
    {
        lhs << *i;
    }
    return lhs;
}

// ----------------------------------------------------------------------------

void Morphology::print() const
{
    std::cout << *this << std::endl;
}

// ----------------------------------------------------------------------------

}
#endif
