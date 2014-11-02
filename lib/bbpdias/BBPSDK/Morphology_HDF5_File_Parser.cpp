/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors: Sebastien Lasserre
                 Juan Hernando Vieites

*/

#define H5Dopen_vers 1
#include <hdf5.h>
#include <boost/scoped_array.hpp>
#include <sstream>

#include "BBP/Common/Dataset/HDF5/H5ID.h"
#include "BBP/Model/Microcircuit/Datasets/Morphology_Dataset.h"
#include "Parsers/Morphology_HDF5_File_Parser.h"


namespace bbp
{

static const unsigned int POINT_COLUMN_NUMBER = 4;
static const unsigned int SECTION_COLUMN_NUMBER = 3;

void Morphology_HDF5_File_Parser::read_morphology(
    const Filepath                & morphology,
    Morphology_Point_ID           & point_count,
    Section_ID                    & section_count,
    Morphology_Point_ID_Array     & section_start_points,
    Section_Type_Array            & section_types,
    Section_ID_Array              & section_parents,
    Vector_3D_Micron_Array        & point_positions,
    Micron_Array                  & point_diameters
) throw (IO_Error)
{
    // Disabling error reporting in non debug mode
#ifdef NDEBUG
    H5E_BEGIN_TRY
#endif

	herr_t status;
    hsize_t dims[2], max_dims[2];

    // Opening file
	H5ID file(H5Fopen(morphology.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT),
              H5Fclose);
    if (!file)
    {
        throw_exception(
            File_Access_Error("Reading morphology file '" + 
                              morphology.string() + "': error opening file"),
            FATAL_LEVEL, __FILE__, __LINE__);
    }

    // Opening datasets
    H5ID points_dataset(H5Dopen(file, "/points"), H5Dclose);
    H5ID structure_dataset(H5Dopen(file, "/structure"), H5Dclose);
    
    if (!points_dataset || !structure_dataset)
    {
        throw_exception(
            File_Parse_Error("Reading morphology file '" + 
                             morphology.string() + "': error opening datasets"),
            FATAL_LEVEL, __FILE__, __LINE__);
    }

    // Get points number
    H5ID points_dataspace(H5Dget_space(points_dataset), H5Sclose);
    if (H5Sget_simple_extent_ndims(points_dataspace) != 2 ||
        H5Sget_simple_extent_dims(points_dataspace, dims, max_dims) < 0 ||
        dims[1] != POINT_COLUMN_NUMBER)
    {
        throw_exception(
            File_Parse_Error("Reading morphology file '" + morphology.string() +
                             "': bad number of dimensions in"
                             " 'points' dataspace"), 
            FATAL_LEVEL, __FILE__, __LINE__);
    }
    
    Morphology_Point_ID point_count_tmp = dims[0];

    // Get section count
    H5ID sections_dataspace(H5Dget_space(structure_dataset), H5Sclose);
    if (H5Sget_simple_extent_ndims(sections_dataspace) != 2 ||
        H5Sget_simple_extent_dims(sections_dataspace, dims, max_dims) < 0 ||
        dims[1] != SECTION_COLUMN_NUMBER)
    {
        throw_exception(
            File_Parse_Error("Reading morphology file '" + morphology.string() +
                             "': bad number of dimensions in 'structure' "
                             "dataspace"), FATAL_LEVEL, __FILE__, __LINE__);
    }

    Section_ID section_count_tmp = dims[0];

    // Reading point data
    // Allocating arrays
    Vector_3D_Micron_Array point_positions_tmp
        (new Vector_3D<Micron>[point_count_tmp]);
    Micron_Array point_diameters_tmp(new Micron[point_count_tmp]);
    boost::scoped_array<double> raw_points(new double[point_count_tmp *
                                                      POINT_COLUMN_NUMBER]);
    // Reading
    status = H5Dread(points_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, 
                     H5S_ALL, H5P_DEFAULT, raw_points.get());
    if (status < 0) 
    {
        throw_exception(
            File_Parse_Error("Reading morphology file '" + morphology.string() +
                             "': Error reading '/points' dataspace"),
            FATAL_LEVEL, __FILE__, __LINE__);
    }
    // Extracting data from the tmp structure
    for (unsigned int i = 0; i < point_count_tmp; ++i)
    {
        Vector_3D<Micron> &v = point_positions_tmp[i];
        double * point = &raw_points[i * 4];
        v.x() = (Micron) point[0];
        v.y() = (Micron) point[1];
        v.z() = (Micron) point[2];
        point_diameters_tmp[i] = point[3];
        //std::cout << v.x() << ","<< v.y() << ","<< v.z() << "," << point[3] << std::endl;
        //std::cout << v.x() << ","<< v.y() << ","<< v.z() << std::endl;
    }

    // Reading structure data
    // Allocating arrays
    Morphology_Point_ID_Array section_starts_tmp
        (new Morphology_Point_ID[section_count_tmp]);
    Section_Type_Array section_types_tmp
        (new Section_Type[section_count_tmp]);
    Section_ID_Array section_parents_tmp
        (new Section_ID[section_count_tmp]);
    boost::scoped_array<int> raw_structure(new int[point_count_tmp *
                                                   SECTION_COLUMN_NUMBER]);
    status = H5Dread(structure_dataset, H5T_NATIVE_INT, H5S_ALL, 
                     H5S_ALL, H5P_DEFAULT, raw_structure.get());
    if (status < 0) 
    {
        throw_exception(
            File_Parse_Error("Reading morphology file '" + morphology.string() +
                             "': Error reading '/structure' dataspace"),
            FATAL_LEVEL, __FILE__, __LINE__);
    }
    for (unsigned int i = 0; i < section_count_tmp; ++i)
    {
        int * section = &raw_structure[i * 3];
        section_starts_tmp[i] = (Morphology_Point_ID) section[0] ;
        //section_types_tmp[i] = (Section_Type) section[1] ; 
        
        // Assign section type (bitfield) 
        switch ( (HDF5_Section_Type) section[1])
        {
            case HDF5_SOMA_TYPE:
                section_types_tmp[i] = SOMA;
                break;
            case HDF5_AXON_TYPE:
                section_types_tmp[i] = AXON;
                break;
            case HDF5_DENDRITE_TYPE:
                section_types_tmp[i] = DENDRITE;
                break;
            case HDF5_APICAL_DENDRITE_TYPE:
                section_types_tmp[i] = APICAL_DENDRITE;
                break;
            default:
                section_types_tmp[i] = UNDEFINED_SECTION_TYPE;
        }

        // Asserting that only section 0 (soma) has no parent 
        // (section parent -1)
        bbp_assert((i > 0 && section[2] != -1) ||
               (i == 0 && section[2] == -1));
        section_parents_tmp[i] = (Section_ID) section[2];
        //std::cout << i << ","<< section[1] << ","<< section[2] << std::endl;
    }
    // Assigning undefined section id as soma parent.
    section_parents_tmp[0] = UNDEFINED_SECTION_ID;

    // Copying local variables to final result.
    point_count = point_count_tmp;
    section_count = section_count_tmp;
    section_start_points = section_starts_tmp;
    section_types = section_types_tmp;
    section_parents = section_parents_tmp;
    point_positions = point_positions_tmp;
    point_diameters = point_diameters_tmp;

    // Renabling error reporting in non debug mode
#ifdef NDEBUG
    // \todo When an exception if thrown error reporting is not reenabled
    // We need a big try catch block.
    H5E_END_TRY;
#endif
}

}
