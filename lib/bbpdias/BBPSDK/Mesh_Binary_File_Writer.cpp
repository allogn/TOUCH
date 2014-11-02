/*

            Ecole Polytechnique Federale de Lausanne
            Brain Mind Institute,
            Blue Brain Project
            (c) 2007. All rights reserved.

            Authors: Sebastien Lasserre

*/

#include <fstream>
#include <boost/filesystem.hpp>

#include "BBP/Common/Exception/Exception.h"
#include "BBP/Model/Microcircuit/Mesh.h"
#include "BBP/Model/Microcircuit/Writers/File/Mesh_Binary_File_Writer.h"

namespace bbp {

void Mesh_Binary_File_Writer::write_mesh(
    const Filename        & file,
    Mesh                  & mesh
)
{
    std::ofstream datastream((char*)file.string().c_str(), std::ios::binary);

    if (!datastream)
    {
        throw_exception(
            IO_Error("Mesh_Binary_File_Writer: cannot create "
                     "binary file: " + file.string()),
            WARNING_LEVEL, __FILE__, __LINE__);
    }
    
    // write header infos for the current mesh
    Vertex_Index vertex_count = mesh.vertex_count();
    Triangle_Index triangle_count = mesh.triangle_count();
    Triangle_Index triangle_strip_length = mesh.triangle_strip_length();
    datastream.write((char*)&vertex_count, sizeof(Vertex_Index));
    datastream.write((char*)&triangle_count, sizeof(Triangle_Index));
    datastream.write((char*)&triangle_strip_length, sizeof(Triangle_Index));
    
    // write the vertices data
    datastream.write((char*)mesh.vertices().pointer(),
                     vertex_count * 3 * sizeof(Micron));
    datastream.write((char*)mesh.vertex_sections().pointer(),
                     vertex_count * sizeof(Section_ID));
    datastream.write((char*)mesh.vertex_relative_distances().pointer(), 
                     vertex_count * sizeof(float));
    
    // write the triangles data
    datastream.write((char*)mesh.triangles().pointer(), 
                     triangle_count * 3 * sizeof(Vertex_Index));
    datastream.write((char*)mesh.triangle_strip().pointer(), 
                     triangle_strip_length * sizeof(Vertex_Index));
}

}
