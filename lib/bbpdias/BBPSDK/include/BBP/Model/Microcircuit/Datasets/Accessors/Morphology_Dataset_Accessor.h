/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2008. All rights reserved.

		Authors: Juan Hernando Vieites
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_DATASET_MORPHOLOGY_ACCESSOR_H
#define BBP_DATASET_MORPHOLOGY_ACCESSOR_H

#include "BBP/Model/Microcircuit/Datasets/Morphology_Dataset.h"

namespace bbp
{

// ----------------------------------------------------------------------------

//! Class with access to private attributes of Morphology_Dataset.
class Morphology_Dataset_Accessor
{
protected:
    static Label & label(Morphology_Dataset & morphology)
    {
        return morphology._label;
    }
    static Morphology_Reconstruction_Origin & origin
        (Morphology_Dataset & morphology)
    {
        return morphology._origin;
    }
    static Morphology_Type & type(Morphology_Dataset & morphology)
    {
        return morphology._morphology_type;
    }
    static Morphology_Point_ID & point_count(Morphology_Dataset & morphology)
    {
        return morphology._point_count;
    }
    static Section_ID & section_count(Morphology_Dataset & morphology)
    {
        return morphology._section_count;
    }
    static Section_ID & soma_offset(Morphology_Dataset & morphology)
    {
        return morphology._soma_offset;
    }
    static Section_ID & axon_offset(Morphology_Dataset & morphology)
    {
        return morphology._axon_offset;
    }
    static Section_ID & basal_dendrite_offset(Morphology_Dataset & morphology)
    {
        return morphology._basal_dendrite_offset;
    }
    static Section_ID & apical_dendrite_offset(Morphology_Dataset & morphology)
    {
        return morphology._apical_dendrite_offset;
    }
    static Morphology_Point_ID_Array & section_start_points
        (Morphology_Dataset & morphology)
    {
        return morphology._section_start_points;
    }
    static Morphology_Point_ID_Array & section_point_counts
        (Morphology_Dataset & morphology)
    {
        return morphology._section_point_counts;
    }
    static Section_ID_Array & section_parent_sections
        (Morphology_Dataset & morphology)
    {
        return morphology._section_parent_sections;
    }
    static Section_ID_Array & section_children_sections
        (Morphology_Dataset & morphology)
    {
        return morphology._section_children_sections;
    }
    static Section_ID_Array & first_order_sections
        (Morphology_Dataset & morphology)
    {
        return morphology._first_order_sections;
    }
    static Section_ID & first_order_section_count
        (Morphology_Dataset & morphology)
    {
        return morphology._first_order_section_count;
    }
    static Section_Branch_Order_Array & section_branch_orders
    (Morphology_Dataset & morphology)
    {
        return morphology._section_branch_orders;
    }
    static Section_Type_Array & section_types(Morphology_Dataset & morphology)
    {
        return morphology._section_types;
    }
    static Micron_Array & section_lengths(Morphology_Dataset & morphology)
    {
        return morphology._section_lengths;
    }
    static Vector_3D_Micron_Array & point_positions
        (Morphology_Dataset & morphology)
    {
        return morphology._point_positions;
    }
    static Micron_Array & point_diameters(Morphology_Dataset & morphology)
    {
        return morphology._point_diameters;
    }
    static Section_Normalized_Distance_Array & point_relative_distances
        (Morphology_Dataset & morphology)
    {
        return morphology._point_relative_distances;
    }
    static Vector_3D<Micron> & bound_max_values(Morphology_Dataset & morphology)
    {
        return morphology._max_values;
    }
    static Vector_3D<Micron> & bound_min_values(Morphology_Dataset & morphology)
    {
        return morphology._min_values;
    }

    // Const versions

    static const Label & label(const Morphology_Dataset & morphology)
    {
        return morphology._label;
    }
    static Morphology_Reconstruction_Origin origin
        (const Morphology_Dataset & morphology)
    {
        return morphology._origin;
    }
    static Morphology_Type type(const Morphology_Dataset & morphology)
    {
        return morphology._morphology_type;
    }
    static Morphology_Point_ID point_count(const Morphology_Dataset & morphology)
    {
        return morphology._point_count;
    }
    static Section_ID section_count(const Morphology_Dataset & morphology)
    {
        return morphology._section_count;
    }
    static Section_ID soma_offset(const Morphology_Dataset & morphology)
    {
        return morphology._soma_offset;
    }
    static Section_ID axon_offset(const Morphology_Dataset & morphology)
    {
        return morphology._axon_offset;
    }
    static Section_ID basal_dendrite_offset
        (const Morphology_Dataset & morphology)
    {
        return morphology._basal_dendrite_offset;
    }
    static Section_ID apical_dendrite_offset
        (const Morphology_Dataset & morphology)
    {
        return morphology._apical_dendrite_offset;
    }
    static const Morphology_Point_ID_Array & section_start_points
        (const Morphology_Dataset & morphology)
    {
        return morphology._section_start_points;
    }
    static const Morphology_Point_ID_Array & section_point_counts
        (const Morphology_Dataset & morphology)
    {
        return morphology._section_point_counts;
    }
    static const Section_ID_Array & section_parent_sections
        (const Morphology_Dataset & morphology)
    {
        return morphology._section_parent_sections;
    }
    static const Section_ID_Array & section_children_sections
        (const Morphology_Dataset & morphology)
    {
        return morphology._section_children_sections;
    }
    static const Section_ID_Array & first_order_sections
        (const Morphology_Dataset & morphology)
    {
        return morphology._first_order_sections;
    }
    static Section_ID first_order_section_count
        (const Morphology_Dataset & morphology)
    {
        return morphology._first_order_section_count;
    }
    static const Section_Branch_Order_Array & section_branch_orders
        (const Morphology_Dataset & morphology)
    {
        return morphology._section_branch_orders;
    }
    static const Section_Type_Array & section_types
        (const Morphology_Dataset & morphology)
    {
        return morphology._section_types;
    }
    static const Micron_Array & section_lengths
        (const Morphology_Dataset & morphology)
    {
        return morphology._section_lengths;
    }
    static const Vector_3D_Micron_Array & point_positions
        (const Morphology_Dataset & morphology)
    {
        return morphology._point_positions;
    }
    static const Micron_Array & point_diameters
        (const Morphology_Dataset & morphology)
    {
        return morphology._point_diameters;
    }
    static const Section_Normalized_Distance_Array & point_relative_distances
        (const Morphology_Dataset & morphology)
    {
        return morphology._point_relative_distances;
    }
    static const Vector_3D<Micron> & bound_max_values
        (const Morphology_Dataset & morphology)
    {
        return morphology._max_values;
    }
    static const Vector_3D<Micron> & bound_min_values
        (const Morphology_Dataset & morphology)
    {
        return morphology._min_values;
    }
};

// ----------------------------------------------------------------------------

}
#endif
