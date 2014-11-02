/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2008. All rights reserved.

		Authors: Juan Hernando Vieites
*/

#include <boost/filesystem.hpp>

#include "BBP/Model/Microcircuit/Readers/Morphology_Reader.h"
#include "BBP/Model/Microcircuit/Morphology.h"
#include "BBP/Model/Microcircuit/Containers/Sections.h"

#include "File/Morphology_HDF5_File_Reader.h"

namespace bbp
{

// ----------------------------------------------------------------------------

boost::filesystem::path MORPHOLOGY_SUBPATH = "h5";

// ----------------------------------------------------------------------------

Morphology_Reader_Ptr 
Morphology_Reader::create_reader(const URI & morphology_source)
{
    using namespace boost;
    filesystem::path path = uri_to_filename(morphology_source);

    // Checking if path is a directory
    if (path != "" && filesystem::is_directory(path))
    {
        // Checking if there is any h5 file inside this directory
        for (filesystem::directory_iterator entry(path);
             entry != filesystem::directory_iterator(); 
             ++entry)
        {
            if (filesystem::extension(entry->path()) == ".h5" &&
                filesystem::is_regular(entry->status()))
            {
                return Morphology_Reader_Ptr
                    (new Morphology_HDF5_File_Reader(path));
            }
        }

        // Setting path as path/h5
        return Morphology_Reader_Ptr
            (new Morphology_HDF5_File_Reader(path / MORPHOLOGY_SUBPATH));
    }

    // No suitable reader found
    log_message("Morphology_Reader: No suitable reader for"
                " morphology source: " + morphology_source, SEVERE_LEVEL);
    return Morphology_Reader_Ptr();
}

// ----------------------------------------------------------------------------

void Morphology_Reader::compute_points_per_section_and_offsets
(Morphology & morphology)
{
    // Compute the number of points per section
    Section_ID num_sections = morphology.number_of_sections();
    Morphology_Point_ID_Array & point_counts =
        section_point_counts(morphology);
    point_counts.reset(new Morphology_Index[num_sections]);

    const Morphology_Point_ID * start_points = 
        morphology.section_start_points();
    for (Section_ID i = 0 ; i < num_sections - 1 ; ++i)
    {
        point_counts[i] = start_points[i + 1]  - start_points[i] ;
    }
    point_counts[num_sections - 1] = 
       morphology.number_of_points() - start_points[num_sections - 1];
    
    const Section_Type * types = morphology.section_types();
    bbp_assert(types[0] & SOMA);    
    soma_offset(morphology) = 0;
    bbp_assert(types[1] & AXON);
    axon_offset(morphology) = 1;

    Section_ID offset = morphology.axon_offset(); 
    while (offset < num_sections && types[offset] & AXON)
        ++offset;
    if (offset != num_sections && types[offset] & DENDRITE)
        basal_dendrite_offset(morphology) = offset;
    else
        basal_dendrite_offset(morphology) = UNDEFINED_SECTION_ID;

    while (offset < num_sections && types[offset] & DENDRITE)
        ++offset;
    if (offset != num_sections && types[offset] & APICAL_DENDRITE)
        apical_dendrite_offset(morphology) = offset;
    else
        apical_dendrite_offset(morphology) = UNDEFINED_SECTION_ID;
}    
    
// ----------------------------------------------------------------------------

void Morphology_Reader::compute_children
(Morphology & morphology)
{
    Section_ID_Array & children = section_children_sections(morphology);
    Section_ID_Array parents = section_parent_sections(morphology);
    size_t num_sections = morphology.number_of_sections();

    children.reset(new Section_ID[num_sections * MAX_CHILDREN]);
    for (Section_ID i = 0; i < num_sections * MAX_CHILDREN; ++i)
        children[i] = UNDEFINED_SECTION_ID;

    std::vector<Section_ID> first_order;
    for (Section_ID i = 1; i < num_sections; ++i)
    {
        Section_ID parent = parents[i];
        if (parent == 0) 
        {
            first_order.push_back(i);
        }
        else
        {    
            Section_ID *list = &children[parent * MAX_CHILDREN];
            
            /*! 
                \bug Potential bug, why is max children 4? Looks potentially
                problematic for yet unknown morphologies (e.g. cerebellum cells
                are very branched and might have more? Also I found
                that actually a few morphologies have at least 4 children,
                so the runtime error cannot be correct since it checks
                for 5. (TT)
            */
            for (size_t j = 0; j < 4; ++j)
            {
                if (j == MAX_CHILDREN)
                {
                    runtime_error exc(
                        "FATAL ERROR: Morphology " + morphology.label() + 
                        " was found to have 4 children in"
                        " section " + boost::lexical_cast<std::string>(i));
                    throw_exception(exc, FATAL_LEVEL);
                }
                if (list[j] == UNDEFINED_SECTION_ID)
                {
                    list[j] = i;
                    break;
                }
            }
        }
    }

    first_order_section_count(morphology) = first_order.size();
    if (first_order.size())
    {
        Section_ID_Array & sections = first_order_sections(morphology);
        sections.reset(new Section_ID[first_order.size()]);

        // Replaced due to security warning on Visual Studio. (TT)
        //std::copy(first_order.begin(), first_order.end(), &sections[0]);
        size_t index = 0;
        for (std::vector<Section_ID>::const_iterator i = first_order.begin();
            i != first_order.end(); ++i)
        {
            sections[index] = *i;
            ++index;
        }    
    }
}

// ----------------------------------------------------------------------------

void Morphology_Reader::compute_extents_and_lengths
(Morphology & morphology)
{
    Section_ID num_sections = morphology.number_of_sections();
    const Morphology_Point_ID * point_counts =
        morphology.section_point_counts();
    const Morphology_Point_ID * start_points = 
        morphology.section_start_points();

    Vector_3D<Micron> & max_values = bound_max_values(morphology);
    Vector_3D<Micron> & min_values = bound_min_values(morphology);
    const Soma & soma = morphology.soma();
    Micron max_radius = soma.max_radius();
    max_values.x() = max_values.y() = max_values.z() = max_radius;
    min_values.x() = min_values.y() = min_values.z() = -max_radius;

    // Compute the length of the sections and the relative distance of each 
    // point from the beginning of the section
    Micron_Array & lengths = section_lengths(morphology);
    lengths.reset(new Micron[num_sections]);
    memset(&lengths[0], 0, sizeof(Micron) * num_sections);
    
    Section_Normalized_Distance_Array & relative_distances = 
        point_relative_distances(morphology);
    relative_distances.reset
        (new Section_Normalized_Distance[morphology.number_of_points()]);

    const Vector_3D<Micron> * points = morphology.point_positions();
    const Micron * diameters = morphology.point_diameters();
    for (Section_ID i = 0 ; i < num_sections; ++i)
    {
        // compute length and relative distance
        for (Morphology_Point_ID j = start_points[i];
             j < start_points[i] + point_counts[i] - 1;
             ++j) 
        {
            lengths[i] += (points[j] - points[j + 1]).length();
        }
        
        // compute relative distance (for all points but last)
        size_t last_point_index = start_points[i] + point_counts[i] - 1;
        Micron accumulated_length = 0;
        for (Morphology_Point_ID j = start_points[i];
             j < last_point_index;
             ++j) 
        {
            // Updating maximum and mininum values found
            const Vector_3D<Micron> & point = points[j];
            Micron diameter = diameters[j];
            if (point.x() + diameter > max_values.x())
                max_values.x() = point.x() + diameter;
            if (point.x() - diameter < min_values.x())
                min_values.x() = point.x() - diameter;
            if (point.y() + diameter > max_values.y())
                max_values.y() = point.y() + diameter;
            if (point.y() - diameter < min_values.y())
                min_values.y() = point.y() - diameter;
            if (point.z() + diameter > max_values.z())
                max_values.z() = point.z() + diameter;
            if (point.z() - diameter < min_values.z())
                min_values.z() = point.z() - diameter;

            relative_distances[j] = 
                accumulated_length / lengths[i];
            accumulated_length += (point - points[j + 1]).length();
        }
        const Vector_3D<Micron> & point = points[last_point_index];
        Micron diameter = diameters[last_point_index];
            if (point.x() + diameter > max_values.x())
                max_values.x() = point.x() + diameter;
            if (point.x() - diameter < min_values.x())
                min_values.x() = point.x() - diameter;
            if (point.y() + diameter > max_values.y())
                max_values.y() = point.y() + diameter;
            if (point.y() - diameter < min_values.y())
                min_values.y() = point.y() - diameter;
            if (point.z() + diameter > max_values.z())
                max_values.z() = point.z() + diameter;
            if (point.z() - diameter < min_values.z())
                min_values.z() = point.z() - diameter;

        // Last section point relative distance
        relative_distances[last_point_index] =
            accumulated_length / lengths[i] ;
    }
}

// ----------------------------------------------------------------------------
    
void Morphology_Reader::compute_branch_orders(Morphology & morphology)
{
    Sections current_sections = morphology.soma().children();
    Section_Branch_Order_Array & branch_orders = 
        section_branch_orders(morphology);
    Section_ID num_sections = morphology.number_of_sections();   
    branch_orders.reset(new Section_Branch_Order[num_sections]);    
    
    /*! 
        \bug Potential bug, soma not necessarily 0 I think. We need to
        let Soma inherit from Section also to get rid of the need for
        first_order_sections() (but use soma.children() instead). (TT)
     */
    //Soma soma = morphology.soma();
    // soma.id()
    branch_orders[0] = 0;
    
    //! \todo Once soma is a section branch order should be 0. (TT)
    //Section_Branch_Order branch_order = 0;

    Section_Branch_Order branch_order = 1;
    this->traverse_children(morphology, current_sections, branch_order);
}

// ----------------------------------------------------------------------------
    
void Morphology_Reader::traverse_children(Morphology & morphology,
    Sections & sections, 
    Section_Branch_Order current_branch_order)
{
    Section_Branch_Order_Array & branch_orders = 
    section_branch_orders(morphology);

    for (Sections::const_iterator i = sections.begin();
         i != sections.end(); ++i)
    {
        branch_orders[i->id()] = current_branch_order; 
        Sections children = i->children();
        if (children.size() != 0)
            traverse_children(morphology, children, current_branch_order + 1);
        
    }
}
    
// ----------------------------------------------------------------------------

}
