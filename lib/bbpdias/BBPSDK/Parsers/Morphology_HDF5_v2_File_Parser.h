/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors: Sebastien Lasserre

*/


#ifndef BBP_MORPHOLOGY_HDF5_V2_FILE_PARSER_H
#define BBP_MORPHOLOGY_HDF5_V2_FILE_PARSER_H

#include <boost/shared_ptr.hpp>

#include "BBP/Common/System/File/File.h"
#include "BBP/Common/Exception/Exception.h"
#include "BBP/Common/Types.h"

#include "../File/Morphology_HDF5_File_Reader.h"
namespace bbp 
{

class Morphology;

/**    
*/
    
class Morphology_HDF5_v2_File_Parser
{
public :

    //! extract morphologies data and store these in flat arrays 
    void read_morphology(
        const Filepath                 & morphology,
        Morphology_Point_ID            & point_count,
        Section_ID                     & section_count,
        Morphology_Point_ID_Array      & section_start_points,
        Section_Type_Array             & section_types,
        Section_ID_Array               & section_parents,
        Vector_3D_Micron_Array         & point_positions,
        Micron_Array                   & point_diameters,
        Morphology_Reader::H5File_Mode   mode
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
