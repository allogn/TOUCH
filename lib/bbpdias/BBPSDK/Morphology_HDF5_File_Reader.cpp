/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2006-2007. All rights reserved.

                Authors: Juan Hernando Vieites
*/
#include <boost/filesystem.hpp>

#include "BBP/Model/Microcircuit/Containers/Morphologies.h"

#include "File/Morphology_HDF5_File_Reader.h"
#include "Parsers/Morphology_HDF5_File_Parser.h"
#include "Parsers/Morphology_HDF5_v2_File_Parser.h"
#include "Parsers/MVD_File_Parser.h"

namespace bbp {


// ----------------------------------------------------------------------------

void Morphology_HDF5_File_Reader::open()
{
    // Checking the morphology path
    if (!boost::filesystem::is_directory(_morphology_path))
    {
        throw_exception(
            IO_Error("Morphology_HDF5_File_Reader::open: Bad morphology"
                     " path: '" + _morphology_path.string() + "'"),
            FATAL_LEVEL, __FILE__, __LINE__);
    }        
}

// ----------------------------------------------------------------------------
void Morphology_HDF5_File_Reader::load(Morphologies & final_morphologies, 
                                       const std::set<Label> & labels,
                                       H5File_Mode mode)
{
    Morphologies morphologies;
    for (std::set<Label>::const_iterator label = labels.begin();
         label != labels.end();
         ++label) {
        if (final_morphologies.find(*label) == final_morphologies.end())
        {
            Morphology_Ptr morphology = read(*label,mode);
            morphologies.insert(morphology);
        }
    }

    // Final insertion from local container to result
    for (Morphologies::iterator i = morphologies.begin();
         i != morphologies.end();
         ++i)
        final_morphologies.insert(i.ptr());
}

// ----------------------------------------------------------------------------

void Morphology_HDF5_File_Reader::load(Morphologies & final_morphologies,
                                       const Cell_Target & cells, 
                                       const URI & circuit_source)
{
    Morphologies morphologies;

    boost::filesystem::path mvd_file = uri_to_filename(circuit_source);
    if (mvd_file == "" || boost::filesystem::extension(mvd_file) != ".mvd2")
    {
        throw_exception(
            Bad_Data_Source("Loading morphologies: circuit_source '" + 
                            circuit_source + "' "), 
            FATAL_LEVEL, __FILE__, __LINE__);
    }
    MVD_File_Parser mvd_parser(mvd_file);

    mvd_parser.parse_file(cells);

    for (Cell_Target::iterator cell_gid = cells.begin(); 
         cell_gid != cells.end();
         ++cell_gid)
    {
        MVD_Cell_Index index = mvd_parser.cell_index(*cell_gid);
        if (index == UNDEFINED_MVD_CELL_INDEX)
        {
            std::stringstream msg;
            msg << "Loading morphologies: bad cell target, neuron gid " 
                << *cell_gid << " not in mvd file '" << mvd_file << "'";
            throw_exception(Bad_Data(msg.str()), 
                            FATAL_LEVEL, __FILE__, __LINE__);
        }

        // Loading morphology if not already in target container
        const std::string & label = mvd_parser.morphology_names()[index];
        if (morphologies.find(label) == morphologies.end() &&
            final_morphologies.find(label) == final_morphologies.end())
        {
            Morphology_Ptr morphology = read(label);
            origin(*morphology) = mvd_parser.origins()[index];
            type(*morphology) = mvd_parser.morphology_types()[index];

            morphologies.insert(morphology);
        }
    }

    // Final insertion from local container to result
    for (Morphologies::iterator i = morphologies.begin();
         i != morphologies.end();
         ++i)
        final_morphologies.insert(i.ptr());
}

// ----------------------------------------------------------------------------

Morphology_Ptr Morphology_HDF5_File_Reader::read(const Label & label,
                                                 H5File_Mode mode) 
    /* throw IO_Error */
{
    log_message("Loading morphology " + label, INFORMATIVE_LEVEL);
    boost::filesystem::path morphology_filename(_morphology_path / 
                                                (label + ".h5"));

    Morphology_Ptr morphology_ptr(new Morphology());
    Morphology &morphology = *morphology_ptr.get();

    this->label(morphology) = label;
    
    if (mode==HDF5_V1)
    {
       Morphology_HDF5_File_Parser parser;
       parser.read_morphology(morphology_filename,
                              point_count(morphology),
                              section_count(morphology),
                              section_start_points(morphology),
                              section_types(morphology),
                              section_parent_sections(morphology),
                              point_positions(morphology),
                              point_diameters(morphology));
    }
    else
    {
       Morphology_HDF5_v2_File_Parser parser;
       parser.read_morphology(morphology_filename,
                              point_count(morphology),
                              section_count(morphology),
                              section_start_points(morphology),
                              section_types(morphology),
                              section_parent_sections(morphology),
                              point_positions(morphology),
                              point_diameters(morphology),
                              mode);
    }
    // Computing additional information: section type offsets, section lengths
    compute_points_per_section_and_offsets(morphology);
    compute_extents_and_lengths(morphology);
    compute_children(morphology);
    compute_branch_orders(morphology);
    
    // Hmm... We cannot know the type and origin without the MVD file
    origin(morphology) = UNDEFINED_MORPHOLOGY_RECONSTRUCTION_ORIGIN;
	//type(morphology) = UNDEFINED_MORPHOLOGY_TYPE;

    return morphology_ptr;
}

}
