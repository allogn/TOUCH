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

#ifndef BBP_MESH_ASCII_FILE_PARSER_H
#define BBP_MESH_ASCII_FILE_PARSER_H

#include "BBP/Common/System/File/File.h"
#include "BBP/Common/Exception/Exception.h"
#include "BBP/Model/Microcircuit/Types.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Parser for an ASCII encoded plain text mesh file.
class Mesh_ASCII_File_Parser
{
    
public :
    
    /**
        If the vertexFile, triangleFile or mappingFile are NULL (""), the 
		parser will not load the data for these particular sections.
    */
    void read_mesh(
        const Filepath             & vertex_file,
        const Filepath             & triangle_file,
        const Filepath             & triangle_strip_file,  
        const Filepath             & mapping_file,
        Vertex_Index               & vertex_count,
        Triangle_Index             & triangle_count,
        Triangle_Index             & triangle_strip_length,
        Vector_3D_Micron_Array     & vertices,
        Section_ID_Array           & vertex_sections,
        Float_Array                & vertex_relative_distances,
        Vertex_Index_Array         & triangles,
        Vertex_Index_Array         & triangle_strip
    )

        throw (IO_Error);

};

// ----------------------------------------------------------------------------

}

#endif /*BBP_MESH_ASCI_FILE_PARSER_H*/
