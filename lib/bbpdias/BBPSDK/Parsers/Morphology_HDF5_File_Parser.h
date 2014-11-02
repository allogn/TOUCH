/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors: Sebastien Lasserre

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MORPHOLOGY_HDF5_FILE_PARSER_H
#define BBP_MORPHOLOGY_HDF5_FILE_PARSER_H

#include <boost/shared_ptr.hpp>

#include "BBP/Common/System/File/File.h"
#include "BBP/Common/Exception/Exception.h"
#include "BBP/Common/Types.h"

namespace bbp 
{

class Morphology;

/**    
*/
    
class Morphology_HDF5_File_Parser
{
public :

    //! extract morphologies data and store these in flat arrays 
    void read_morphology(
        const Filepath             & morphology,
        Morphology_Point_ID        & point_count,
        Section_ID                 & section_count,
        Morphology_Point_ID_Array  & section_start_points,
        Section_Type_Array         & section_types,
        Section_ID_Array           & section_parents,
        Vector_3D_Micron_Array     & point_positions,
        Micron_Array               & point_diameters
    ) 
#ifdef WIN32
    throw (IO_Error);
#else
    throw (IO_Error);
#endif
protected :
};

}
#endif
