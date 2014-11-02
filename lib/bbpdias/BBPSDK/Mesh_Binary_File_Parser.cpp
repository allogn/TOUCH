/*

            Ecole Polytechnique Federale de Lausanne
            Brain Mind Institute,
            Blue Brain Project
            (c) 2007. All rights reserved.

            Authors: Sebastien Lasserre

*/

#include <fstream>
#include <boost/filesystem.hpp>

#include "Mesh_Binary_File_Parser.h"

using namespace bbp ;

//-----------------------------------------------------------------------------
void Mesh_Binary_File_Parser::read_mesh(
    const Filepath             & file, 
    bool                         vertex_flag, 
    bool                         triangle_flag,
    bool                         triangle_strip_flag,
    bool                         mapping_flag,  
    Vertex_Index               & vertex_count,
    Triangle_Index             & triangle_count,
    Triangle_Index             & triangle_strip_length,
    Vector_3D_Micron_Array     & vertices,
    Section_ID_Array	       & vertex_sections,
    Float_Array                & vertex_relative_distances,
    Vertex_Index_Array         & triangles,
    Vertex_Index_Array         & triangle_strips
)

    throw (IO_Error)

{      
    vertex_count = 0;
    triangle_count = 0;
    
    // local variable to store the element count
    Vertex_Index             vertex_count_tmp = 0 ;
    Triangle_Index           triangle_count_tmp = 0 ;
    Triangle_Index           triangle_strip_length_tmp = 0 ;
    
    std::ifstream dataStream((char*)file.string().c_str(), std::ios::binary);

    if (!dataStream)
    {
        throw_exception(
            File_Access_Error("Mesh_Binary_File_Parser: cannot "
                              "open binary file: " + file.string()),
            FATAL_LEVEL, __FILE__, __LINE__);
    }

    dataStream.read((char*)&vertex_count_tmp, sizeof(Vertex_Index));
    dataStream.read((char*)&triangle_count_tmp, sizeof(Triangle_Index));
    dataStream.read((char*)&triangle_strip_length_tmp, sizeof(Triangle_Index));
 
    // set the seeks
    unsigned long vertexseek = 
        sizeof(Vertex_Index) + sizeof(Triangle_Index) * 2;
    unsigned long mappingseek = vertexseek + 
        vertex_count_tmp * 3 * sizeof(Micron);
    unsigned long triangleseek = mappingseek + 
        vertex_count_tmp * (sizeof(Section_ID) + sizeof(float));
    unsigned long trianglestripseek = triangleseek + 
        triangle_count_tmp * 3 * sizeof(Vertex_Index);
        
   //----------------------------------
    // load the vertex data 
    //----------------------------------
    if (vertex_flag) {
        // allocating memory
        Vector_3D_Micron_Array vertices_tmp(new Vector_3D<Micron>[vertex_count_tmp]) ;
        // store the data
        dataStream.seekg(vertexseek) ;
        for ( Vertex_Index i = 0 ; i < vertex_count_tmp ; ++i )
        {
            Micron position[3] ; 
            dataStream.read((char*) &position, 3 * sizeof(Micron));
            vertices_tmp[i].x() = position[0];
            vertices_tmp[i].y() = position[1];
            vertices_tmp[i].z() = position[2];
        }
        // copying local variables to final result
        vertex_count = vertex_count_tmp;
        vertices = vertices_tmp ;
    }
    
    //----------------------------------
    // load the triangle data
    //---------------------------------- 
    if (triangle_flag) {
        // allocating memory
        Vertex_Index_Array triangles_tmp(new Vertex_Index[triangle_count_tmp * 3]) ;
        // store the data
        dataStream.seekg(triangleseek) ;
        dataStream.read((char*)triangles_tmp.get(), 
                        triangle_count_tmp * 3 * sizeof(Vertex_Index));
        // copying local variables to final result
        triangle_count = triangle_count_tmp;
        triangles = triangles_tmp ;
    }

    if (triangle_strip_flag) {
        // allocating memory
        Vertex_Index_Array triangles_tmp
            (new Vertex_Index[triangle_strip_length_tmp]) ;
        // store the data
        dataStream.seekg(trianglestripseek) ;
        dataStream.read((char*)triangles_tmp.get(), 
                        triangle_strip_length_tmp * sizeof(Vertex_Index));
        // copying local variables to final result
        triangle_strip_length = triangle_strip_length_tmp;
        triangle_strips = triangles_tmp ;
    }

    //----------------------------------
    // load the mapping data
    //---------------------------------- 
    if (mapping_flag) {
        // allocating memory
        Section_ID_Array vertex_sections_tmp(new Section_ID[vertex_count]) ;
        Float_Array vertex_relative_distances_tmp(new float[vertex_count]) ;
        // store the data
        dataStream.seekg(mappingseek) ;
        dataStream.read((char*)vertex_sections_tmp.get(), 
                        vertex_count_tmp * sizeof(Section_ID)) ;
        dataStream.read((char*)vertex_relative_distances_tmp.get(), 
                        vertex_count_tmp * sizeof(float)) ;
        // copying local variables to final result
        vertex_sections = vertex_sections_tmp;
        vertex_relative_distances = vertex_relative_distances_tmp;
    }

    if (dataStream.fail())
    {
        throw_exception(
            File_Parse_Error("Mesh_Binary_File_Parser: parse error in file :"
                             + file.string()), 
            FATAL_LEVEL, __FILE__, __LINE__);
    }

    dataStream.close() ;
}
