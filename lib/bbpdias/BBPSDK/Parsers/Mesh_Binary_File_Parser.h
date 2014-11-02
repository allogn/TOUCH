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

#ifndef BBP_MESH_BINARY_FILE_PARSER_H
#define BBP_MESH_BINARY_FILE_PARSER_H

#include <boost/shared_ptr.hpp>

#include "BBP/Common/System/File/File.h"
#include "BBP/Common/Exception/Exception.h"
#include "BBP/Model/Microcircuit/Types.h"

namespace bbp 
{
/**
*/

class Mesh_Binary_File_Parser
{
    
public :
    
    //! read the mesh data
    void read_mesh(
        const Filepath          & file,
        bool                      vertex_flag,
        bool                      triangles_flag,
        bool                      triangle_strip_flag,
        bool                      mapping_flag,
        Vertex_Index            & vertex_count,
        Triangle_Index          & triangle_count,
        Triangle_Index          & triangle_strip_length,
        Vector_3D_Micron_Array  & vertices,
        Section_ID_Array		& vertex_sections,
        Float_Array             & vertex_relative_distances,
        Vertex_Index_Array      & triangles,
        Vertex_Index_Array      & triangle_strips
        ) 
        throw (IO_Error);

};

}

#endif /*BBP_MESH_BINARY_FILE_PARSER_H*/
