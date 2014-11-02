/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2007. All rights reserved.

        Responsible authors:	Sebastien Lasserre
								Juan Hernando Vieites
        Contributing authors:   Thomas Traenkler

        Modifications:

        - 18.12.07 : removed static variable sMorphologyCount (SL)
        - 18.12.07 : removed interface for serialization (SL)
        - 18.12.07 : computeSectionsInfos() => CAUTION: this method should be 
                     implemented in the reader !! (SL)
        - 18.12.07 : added section offset for each types of  (SL)

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_DATASET_MORPHOLOGY_H
#define BBP_DATASET_MORPHOLOGY_H

#include <boost/shared_ptr.hpp>
#include <cstdio>
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Morphology_Type.h"

namespace bbp 
{        

// ----------------------------------------------------------------------------

class Morphology_Dataset_Accessor;
class Morphology_Dataset ;

namespace rw 
{
    class Morphology;
}

typedef boost::shared_ptr<Morphology_Dataset> Dataset_Morphology_Ptr;

/** \cond */
// Implementation constant
//! \bug Potential bug, why is max children 4? (TT)
static const unsigned int MAX_CHILDREN = 4;
/** \endcond */

// ----------------------------------------------------------------------------

//! Internal memory dataset for storing morphology data.
/*!
    Neurolucida morphology for a type of neuron.
 
    This class stores the morphology data in a flat data structure from the
    HDF5 morphology file. 
*/
class Morphology_Dataset
{
    friend class Morphology_Dataset_Accessor;
    friend class bbp::rw::Morphology;

public:
    
    Morphology_Dataset() 
    {
    }
    
    Morphology_Dataset(const Morphology_Dataset & rhs)
    {
        // GLOBAL ATTRIBUTES ______________________________________________________
        
        _label = rhs._label;
        _origin = rhs._origin;
        _morphology_type = rhs._morphology_type;
        _point_count = rhs._point_count;
        _section_count = rhs._section_count;
        _soma_offset = rhs._soma_offset;
        _axon_offset = rhs._axon_offset;
        _basal_dendrite_offset = rhs._basal_dendrite_offset;
        _apical_dendrite_offset = rhs._apical_dendrite_offset;
        
        // EXTENTS ________________________________________________________________
        
        _max_values = rhs._max_values;
        _min_values = rhs._min_values;
        
        // SECTION ATTRIBUTES _____________________________________________________
        
        Morphology_Point_ID * section_start_points_temp = 
        new Morphology_Point_ID[_section_count];
        std::memcpy(section_start_points_temp, rhs._section_start_points.get(), 
                    sizeof(Morphology_Point_ID) * _section_count);
        _section_start_points = Morphology_Point_ID_Array(section_start_points_temp);
        
        Morphology_Point_ID * section_point_counts_temp = 
        new Morphology_Point_ID[_point_count];
        std::memcpy(section_point_counts_temp, rhs._section_point_counts.get(), 
                    sizeof(Morphology_Point_ID) * _point_count);
        _section_point_counts = Morphology_Point_ID_Array(section_point_counts_temp);

        Section_ID * _section_parent_sections_temp = 
            new Section_ID[_section_count];
        std::memcpy(_section_parent_sections_temp, rhs._section_parent_sections.get(), 
                    sizeof(Section_ID) * _section_count);
        _section_parent_sections = Section_ID_Array(_section_parent_sections_temp);

        Section_ID * _section_children_sections_temp = 
            new Section_ID[MAX_CHILDREN * _section_count];
        std::memcpy(_section_children_sections_temp, rhs._section_children_sections.get(), 
                    sizeof(Section_ID) * _section_count * MAX_CHILDREN);
        _section_children_sections = Section_ID_Array(_section_children_sections_temp);

        _first_order_section_count = rhs._first_order_section_count;        
        
        Section_ID * _first_order_sections_temp = 
            new Section_ID[_first_order_section_count];
        std::memcpy(_first_order_sections_temp, rhs._first_order_sections.get(), 
                    sizeof(Section_ID) * _first_order_section_count);
        _first_order_sections = Section_ID_Array(_first_order_sections_temp);
        
        Section_Branch_Order * _section_branch_orders_temp =
            new Section_Branch_Order[_section_count];
        std::memcpy(_section_branch_orders_temp, 
                    rhs._section_branch_orders.get(),
                    sizeof(Section_Branch_Order) * _section_count);
        _section_branch_orders = 
            Section_Branch_Order_Array(_section_branch_orders_temp);
        
        Section_Type * _section_types_temp =
            new Section_Type[_section_count];
        std::memcpy(_section_types_temp, rhs._section_types.get(),
                    sizeof(Section_Type) * _section_count);
        _section_types = Section_Type_Array(_section_types_temp);
        
        Micron * _section_lengths_temp = new Micron[_section_count];
        std::memcpy(_section_lengths_temp, rhs._section_lengths.get(),
                    sizeof(Micron) * _section_count);
        _section_lengths = Micron_Array(_section_lengths_temp);
        
        // POINT ATTRIBUTES _______________________________________________________
        
        Vector_3D<Micron> * _point_positions_temp =
            new Vector_3D<Micron>[_point_count];
        std::memcpy(_point_positions_temp, rhs._point_positions.get(),
                    sizeof (Vector_3D<Micron>) * _point_count);
        _point_positions = Vector_3D_Micron_Array(_point_positions_temp);
        
        Micron * _point_diameters_temp =
            new Micron[_point_count];
        std::memcpy(_point_diameters_temp, rhs._point_diameters.get(),
                    sizeof (Micron) * _point_count);
        _point_diameters = Micron_Array(_point_diameters_temp);
        
        Section_Normalized_Distance * _point_relative_distances_temp =
        new Section_Normalized_Distance[_point_count];
        std::memcpy(_point_relative_distances_temp, 
            rhs._point_relative_distances.get(),
                    sizeof (Section_Normalized_Distance) * _point_count);
        _point_relative_distances = 
            Section_Normalized_Distance_Array(_point_relative_distances_temp);
    }
    
    //! Virtual destructor.
    virtual ~Morphology_Dataset() {}
    
    bool operator == (const Morphology_Dataset & rhs) const
    {
        bool result = true;

        // GLOBAL ATTRIBUTES ______________________________________________________

        result  = result  && (_label == rhs._label);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "label not equal" << std::endl;
            return result;
        }

        result  = result  && (_origin == rhs._origin);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "origin not equal" << std::endl;
            return result;
        }

        result  = result  && (_morphology_type == rhs._morphology_type);
                if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "morphology type not equal" << std::endl;
            return result;
        }

        result  = result  && (_point_count == rhs._point_count);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "point count not equal" << std::endl;
            return result;
        }

        result  = result  && (_section_count == rhs._section_count);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "section count not equal" << std::endl;
            return result;
        }

        result  = result  && (_soma_offset == rhs._soma_offset);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "soma offset not equal" << std::endl;
            return result;
        }

        result  = result  && (_axon_offset == rhs._axon_offset);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "axon offset not equal" << std::endl;
            return result;
        }

        result  = result  && (_basal_dendrite_offset == 
                              rhs._basal_dendrite_offset);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "basal dendrite offset not equal" << std::endl;
            return result;
        }

        result  = result  && (_apical_dendrite_offset == 
                              rhs._apical_dendrite_offset);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "apical dendrite offset not equal" << std::endl;
            return result;
        }
        
        // EXTENTS ________________________________________________________________

        result  = result  && (_max_values == rhs._max_values);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "max values not equal" << std::endl;
            return result;
        }

        result  = result  && (_min_values == rhs._min_values);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "min values not equal" << std::endl;
            return result;
        }

        // SECTION ATTRIBUTES _____________________________________________________
        
        for (size_t i = 0; i < _section_count; ++i)
        {
            result = result && (_section_start_points[i] == 
                                rhs._section_start_points[i]);
            return result;
        }
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "section start points not equal" << std::endl;
            return result;
        }

        for (size_t i = 0; i < _section_count; ++i)
        {
            result = result && (_section_point_counts[i] == 
                                rhs._section_point_counts[i]);
            return result;
        }
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "section point counts not equal" << std::endl;
        }
        
        for (size_t i = 0; i < _section_count; ++i)
        {
            result = result && (_section_parent_sections[i] == 
                                rhs._section_parent_sections[i]);
            return result;
        }
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "section parent sections not equal" << std::endl;
            return result;
        }

        for (size_t i = 0; i < _section_count * MAX_CHILDREN; ++i)
        {
            result = result && (_section_children_sections[i] == 
                                rhs._section_children_sections[i]);
            if (result == false)
            {
                std::cout << "Morphology_Dataset::operator ==() - "
                             "section children sections not equal at position " 
                           << i << std::endl;
                return result;
            }
        }


        result = result && (_first_order_section_count == 
                            rhs._first_order_section_count);
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "first order section count not equal" << std::endl;
        }


        for (size_t i = 0; i < _first_order_section_count; ++i)
        {
            result = result && (_first_order_sections[i] == 
                                rhs._first_order_sections[i]);
        }
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "first order sections not equal" << std::endl;
        }

        for (size_t i = 0; i < _section_count; ++i)
        {
            result = result && (_section_branch_orders[i] == 
                                rhs._section_branch_orders[i]);
        }
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "section branch orders not equal" << std::endl;
        }
        
        for (size_t i = 0; i < _section_count; ++i)
        {
            result = result && (_section_types[i] == 
                                rhs._section_types[i]);
        }
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "section types not equal" << std::endl;
        }

        for (size_t i = 0; i < _section_count; ++i)
        {
            result = result && (_section_lengths[i] == 
                                rhs._section_lengths[i]);
        }
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "section lengths not equal" << std::endl;
        }
        
        // POINT ATTRIBUTES _______________________________________________________
        
        for (size_t i = 0; i < _point_count; ++i)
        {
            result = result && (_point_positions[i] == 
                                rhs._point_positions[i]);
        }
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "point positions not equal" << std::endl;
        }

        for (size_t i = 0; i < _point_count; ++i)
        {
            result = result && (_point_diameters[i] == 
                                rhs._point_diameters[i]);
        }
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "point diameters not equal" << std::endl;
        }

        for (size_t i = 0; i < _point_count; ++i)
        {
            result = result && (_point_relative_distances[i] == 
                                rhs._point_relative_distances[i]);
        }
        if (result == false)
        {
            std::cout << "Morphology_Dataset::operator ==() - "
                         "point relative distances not equal" << std::endl;
        }

        return result;
    }

    bool operator != (const Morphology_Dataset & rhs) const
    {
        return !(* this == rhs);
    }


    //! Get section offset of the soma.
    Section_ID                      soma_offset() const;
    //! Get section offset of the axon.
    Section_ID                      axon_offset() const;
    //! Get section offsets of the basal dendrites.
    Section_ID                      basal_dendrite_offset() const;
    //! Get section offsets of the apical dendrites.
    Section_ID                      apical_dendrite_offset() const;
    
    //! Get start point of the sections.
    const Morphology_Point_ID *     section_start_points() const;
    //! Get number of points in each neuron morphology section.
    const Morphology_Point_ID *     section_point_counts() const;
    
    //! Get array of parent sections for each neuron morphology section.
    const Section_ID *              section_parent_sections() const;
    //! Get array of children sections for each neuron morphology section.
    /** The array length is MAX_CHILDREN * section_count(). 
        Given section s, its children
        are located sequentially starting at 
        _section_children_sections[s * MAX_CHILDREN], 
        when an UNDEFINED_SECTION_ID value is found, then the section has
        no more children.
    */
    const Section_ID *              section_children_sections() const;
    const Section_Branch_Order *    section_branch_orders() const;
    //! Get the array of sections connected to the soma
    const Section_ID *              first_order_sections() const;
    //! Get the number of sections connected to the soma
    Section_ID                      first_order_section_count() const;
    //! Get array of types of each neuron morphology section.
    const Section_Type *            section_types() const;
    //! Get array of lengths in micron of each neuron morphology section.
    const Micron *                  section_lengths() const;
    //! Get the number of raw points
    Morphology_Point_ID             point_count() const;
    //! Get array of 3D positions of the morphology points.
    const Vector_3D<Micron> *       point_positions() const;
    //! Get plain array of 3D positions of the morphology points.
    const Micron *                  raw_point_positions() const;
    //! Get the diameter for each sample point. 
    /*! size = number of points */
    const Micron *                  point_diameters() const;
    //! Get distance of points within a section.
    /*!
        The distance is computed normalized (0.0-1.0) from the start
        of the section. The distance is used to map the segments to
        electrical compartments to map simulation data on to the
        morphology.
    */
    const Section_Normalized_Distance *     point_relative_distances() const;
  
protected:

    // GLOBAL ATTRIBUTES ______________________________________________________

    //! label of the morphology, e.g. R-C010306G
    Label                               _label;
    //! origin of the morphology (clone or repaired)
    Morphology_Reconstruction_Origin    _origin;
    //! type of the morphology (pyramidal cell, small basket cell, ...)
    Morphology_Type                     _morphology_type;
    //! total number of points in this morphology
    Morphology_Point_ID                 _point_count;
    //! total number of sections in this morphology
    Section_ID                          _section_count;
    //! stores the section offsets for soma (should be always 0)
    Section_ID                          _soma_offset;
    //! stores the section offsets for axon
    Section_ID                          _axon_offset;
    //! stores the section offsets for basal dendrite
    Section_ID                          _basal_dendrite_offset;
    //! stores the section offsets for apical dendrite
    Section_ID                          _apical_dendrite_offset;
    
    // EXTENTS ________________________________________________________________
    
    //! Maximum x, y, z, values of the bounding volume.
    Vector_3D<Micron>                   _max_values;
    //! Minimum x, y, z, values of the bounding volume.
    Vector_3D<Micron>                   _min_values;

    // SECTION ATTRIBUTES _____________________________________________________
    
    //! the start point of the sections
    Morphology_Point_ID_Array           _section_start_points;
    //! number of morphological points in each section of the morphology
    Morphology_Point_ID_Array           _section_point_counts;
    //! the section parent for each section of the morphology
    Section_ID_Array                    _section_parent_sections;
    //! the children sections for each section of the morphology
    Section_ID_Array                    _section_children_sections;
    //! the first order sections
    Section_ID_Array                    _first_order_sections;
    //! the first order section
    Section_ID                          _first_order_section_count;
    //! section branch_order
    Section_Branch_Order_Array          _section_branch_orders;
    //! the section type for each section of the morphology
    Section_Type_Array                  _section_types;
    //! the total morphological length for each section of the morphology
    Micron_Array                        _section_lengths;
    
    // POINT ATTRIBUTES _______________________________________________________
    
    //! position of each morphological point describing the section. 
    Vector_3D_Micron_Array              _point_positions;
    //! diameter of each morphological points.
    Micron_Array                        _point_diameters;
    /*!
        normalized distance of each morphology point from the start (0.0) 
        of the section.  This is used to map the simulation data.
    */
    Section_Normalized_Distance_Array   _point_relative_distances;

};


// ----------------------------------------------------------------------------

inline Section_ID Morphology_Dataset::soma_offset() const 
{
    return _soma_offset;
}

// ----------------------------------------------------------------------------

inline Section_ID Morphology_Dataset::axon_offset() const 
{
    return _axon_offset;
}

// ----------------------------------------------------------------------------

inline Section_ID Morphology_Dataset::basal_dendrite_offset() const 
{
    return _basal_dendrite_offset;
}

// ----------------------------------------------------------------------------

inline Section_ID Morphology_Dataset::apical_dendrite_offset() const 
{
    return _apical_dendrite_offset;
}

// ----------------------------------------------------------------------------

inline const Morphology_Point_ID * Morphology_Dataset::
    section_start_points() const 
{
    return _section_start_points.get();
}

// ----------------------------------------------------------------------------

inline const Morphology_Point_ID * Morphology_Dataset::
    section_point_counts() const 
{
    return _section_point_counts.get();
}

// ----------------------------------------------------------------------------

inline const Section_ID * Morphology_Dataset::section_parent_sections() const 
{
    return _section_parent_sections.get();
}
    
// ----------------------------------------------------------------------------

inline const Section_Branch_Order * Morphology_Dataset::
    section_branch_orders() const 
{
    return _section_branch_orders.get();
}

// ----------------------------------------------------------------------------

inline const Section_Type * Morphology_Dataset::section_types() const 
{
    return _section_types.get();
}

// ----------------------------------------------------------------------------

inline const Micron * Morphology_Dataset::section_lengths() const 
{
    return _section_lengths.get();
}

// ----------------------------------------------------------------------------

inline const Micron * Morphology_Dataset::raw_point_positions() const
{
    return reinterpret_cast<Micron*>(_point_positions.get());
}

// ----------------------------------------------------------------------------

inline Morphology_Point_ID Morphology_Dataset::point_count() const
{
    return _point_count;
}

// ----------------------------------------------------------------------------

inline const Vector_3D<Micron> * Morphology_Dataset::point_positions() const
{
    return _point_positions.get();
}

// ----------------------------------------------------------------------------

inline const Micron * Morphology_Dataset::point_diameters() const 
{
    return _point_diameters.get();
}

// ----------------------------------------------------------------------------

inline const Section_Normalized_Distance * 
    Morphology_Dataset::point_relative_distances() const 
{
    return _point_relative_distances.get();
}

// ----------------------------------------------------------------------------



}
#endif // BBP_DATASET_MORPHOLOGY_H
