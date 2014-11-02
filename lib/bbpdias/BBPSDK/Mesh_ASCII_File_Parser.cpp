/*

            Ecole Polytechnique Federale de Lausanne
            Brain Mind Institute,
            Blue Brain Project
            (c) 2006. All rights reserved.

            Authors: Sebastien Lasserre

*/
#include <fstream>
#include <sstream>

#include "Mesh_ASCII_File_Parser.h"

using namespace bbp;

//-----------------------------------------------------------------------------

void Mesh_ASCII_File_Parser::read_mesh(
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
)  throw (IO_Error)
{
    /** \bug Unsafe code. Should be replaced by C++ style code */
    char line[3000];
    char Token[3000];
        
    vertex_count = 0;
    triangle_count = 0;
    triangle_strip_length = 0;
    //----------------------------------
    // load the vertex data
    //----------------------------------
    if (vertex_file.string() != "")
    {
        Micron x,y,z;
        Vertex_Index vertexNum = 0;
        FILE* file = fopen(vertex_file.string().c_str(), "r");
        if (file == NULL)
        {
            throw_exception(
                File_Access_Error("Mesh_ASCII_File_Parser: cannot open"
                                  " ascii vert file: " + vertex_file.string()),
                FATAL_LEVEL, __FILE__, __LINE__);
        }
        while (fgets(line, 3000, file))
        {
            sscanf(line,"%s",Token);
            if ((Token[0] == '#') || ( Token[0] == '\n' ))
                continue;
            vertex_count = atoi(line);
            break;
        }
        // allocating memory
        Vector_3D_Micron_Array vertices_tmp
            (new Vector_3D<Micron>[vertex_count]);
        // store the data
        while (vertexNum < vertex_count)
        {
            if (fscanf(file,"%f %f %f\n",&x, &y, &z) != 3) {
                throw_exception(
                    File_Parse_Error("Mesh_ASCII_File_Parser: error parsing"
                                     " vert file: " + vertex_file.string()),
                    FATAL_LEVEL, __FILE__, __LINE__);
            }
            Vector_3D<Micron> &v = vertices_tmp[vertexNum];
            v.x() = x;
            v.y() = y;
            v.z() = z;
            ++vertexNum;
        }
        fclose(file);
        //copying local variables to final result
        vertices = vertices_tmp;
    }
    //----------------------------------
    // load the triangle data
    //----------------------------------
    if (triangle_file.string() != "")
    {
        Vertex_Index v1, v2, v3;
        float nx, ny, nz;  
        Triangle_Index triangleNum = 0;
        FILE* file = fopen(triangle_file.string().c_str(), "r");
        if (file == NULL)
        {
            throw_exception(
                IO_Error("Mesh_ASCII_File_Parser: cannot open "
                         "ascii tri file: " + triangle_file.string()),
                FATAL_LEVEL, __FILE__, __LINE__);
        }
        while (fgets(line, 3000, file))
        {
            sscanf(line,"%s",Token);
            if ((Token[0] == '#') || ( Token[0] == '\n' ))
                    continue;
            triangle_count = atoi(line);
            break;
        }
         // allocating memory
        Vertex_Index_Array triangles_tmp(new Vertex_Index[triangle_count * 3]);
        // store the data
        while (triangleNum < triangle_count)
        {
            if (fscanf(file,"%d %d %d %f %f %f\n",
                       &v1, &v2, &v3, &nx, &ny, &nz) != 6) {
                throw_exception(
                    IO_Error("Mesh_ASCII_File_Parser: error parsing "
                             "tri file: " + triangle_file.string()),
                    FATAL_LEVEL, __FILE__, __LINE__);
            }
                
            triangles_tmp[triangleNum * 3]   = v1;
            triangles_tmp[triangleNum * 3 + 1] = v2;
            triangles_tmp[triangleNum * 3 + 2] = v3;
            ++triangleNum;
        }
        fclose(file);
        // copying local variables to final result
        triangles = triangles_tmp;
    }
    //----------------------------------
    // load the triangle strip data
    //----------------------------------
    if (triangle_strip_file.string() != "")
    {
        std::ifstream file(triangle_strip_file.string().c_str());
        if (!file)
        {
            throw_exception(
                IO_Error("Mesh_ASCII_File_Parser: cannot open "
                         "ascii strip file: " + triangle_strip_file.string()),
                FATAL_LEVEL, __FILE__, __LINE__);
        }
        size_t count;
        file >> count;
        if (file.fail() || count == 0) {
            throw_exception(
                File_Parse_Error("Error reading strip file '" + 
                                 triangle_strip_file.string() +
                                 "': error reading size"),
                FATAL_LEVEL, __FILE__, __LINE__);
        }

        Vertex_Index_Array triangle_strip_tmp(new Vertex_Index[count]);
        // store the data
        for (size_t i = 0; i < count; ++i) 
        {
            Vertex_Index index;
            file >> index;
            triangle_strip_tmp[i] = index;
        }
        if (file.fail())
        {
            throw_exception(
                File_Parse_Error("Error reading strip file '" + 
                                 triangle_strip_file.string() +
                                 "': error reading triangle strip"),
                FATAL_LEVEL, __FILE__, __LINE__);            
        }
        // copying local variables to final result
        triangle_strip = triangle_strip_tmp;
        triangle_strip_length = count;
    }
    //----------------------------------
    // load the mapping data
    //----------------------------------
    if (mapping_file.string() != "")
    {
        Vertex_Index vertex_count_in_map_file;
        std::ifstream datastream((char*)mapping_file.string().c_str(), 
                                 std::ios::binary);
        if (!datastream)
        {
            throw_exception(IO_Error("Mesh_ASCII_File_Parser : cannot open "
                                     "map file: " + mapping_file.string()),
                            FATAL_LEVEL, __FILE__, __LINE__);
        }
        datastream.read((char*)&vertex_count_in_map_file, sizeof(unsigned int));
        if (vertex_count_in_map_file != vertex_count)
        {
            datastream.close();
            std::stringstream stmp;
            stmp << "Mesh_ASCII_File_Parser : not same number of vertices in"
                    " vert file (" << vertex_count <<") and in map file (" 
                 << vertex_count_in_map_file << ") - Either the vert and map"
                    " files don't correspond, either the vert file is not"
                    " loaded.";
            throw_exception(IO_Error(stmp.str()), 
                            FATAL_LEVEL, __FILE__, __LINE__);
        }
         // allocating memory
        Section_ID_Array vertex_sections_tmp(new Section_ID[vertex_count]);
        Float_Array vertex_relative_distances_tmp(new float[vertex_count]);
        // read the data
        for (size_t i = 0; i < vertex_count; ++i)
        {
            bbp::Section_ID section;
            float distance;
            datastream.read((char*) &section, sizeof(section));
            datastream.read((char*) &distance, sizeof(distance));
            vertex_sections_tmp[i] = section;
            vertex_relative_distances_tmp[i] = distance;
        }
        datastream.close();
        // copying local variables to final result
        vertex_sections = vertex_sections_tmp;
        vertex_relative_distances = vertex_relative_distances_tmp;
    }
}
