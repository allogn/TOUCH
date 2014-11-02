#include <boost/filesystem.hpp>


#include "BBP/Model/Microcircuit/Containers/Meshes.h"
#include "BBP/Model/Microcircuit/Writers/File/Mesh_Binary_File_Writer.h"
#include "Parsers/Mesh_ASCII_File_Parser.h"
#include "Parsers/Mesh_Binary_File_Parser.h"
#include "Parsers/MVD_File_Parser.h"

#include "File/Mesh_File_Reader.h"

namespace bbp {


// ----------------------------------------------------------------------------

void Mesh_File_Reader::open()
{
    // Checking the mesh path
    if (!boost::filesystem::is_directory(_mesh_path))
    {
        throw_exception(IO_Error("Bad mesh path: '" + _mesh_path.string() + 
                                 "'"), FATAL_LEVEL, __FILE__, __LINE__);
    }    
}

// ----------------------------------------------------------------------------

void Mesh_File_Reader::load(
    Meshes & final_meshes,
    const std::set<Label> & names,
    bool load_vertices,
    bool load_triangles,
    bool load_mapping,
    bool load_triangle_strips)
{
    Meshes meshes;
    for (std::set<Label>::const_iterator name = names.begin();
         name != names.end();
         ++name) {
        insert_new_or_updated(*name, final_meshes, meshes,
                              load_vertices, load_triangles, 
                              load_triangle_strips, load_mapping);
    }

    // Final insertion from local container to result
    for (Meshes::iterator i = meshes.begin();
         i != meshes.end();
         ++i)
    {
        // This insertion replaces old existing meshes with its updated 
        // version also
        final_meshes.insert(i.label(), i.ptr());
    }
}

// ----------------------------------------------------------------------------

void Mesh_File_Reader::load(
    Meshes & final_meshes,
    const Cell_Target & cells, 
    const URI & circuit_source,
    bool load_vertices,
    bool load_triangles,
    bool load_mapping,
    bool load_triangle_strips)
{
    Meshes meshes;

    boost::filesystem::path mvd_file = uri_to_filename(circuit_source);
    if (mvd_file == "" || boost::filesystem::extension(mvd_file) != ".mvd2")
    {
        throw_exception(Bad_Data_Source("Loading meshes: circuit_source '" + 
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
            msg << "Loading meshes: bad cell target, neuron gid " 
                << *cell_gid << " not in mvd file '" << mvd_file << "'";
            throw_exception(Bad_Data(msg.str()), 
                            FATAL_LEVEL, __FILE__, __LINE__);
        }

        const Label & label = mvd_parser.morphology_names()[index];
        if (meshes.find(label) == meshes.end())
        {
            insert_new_or_updated(label, final_meshes, meshes, 
                                  load_vertices, load_triangles,
                                  load_triangle_strips, load_mapping);
        }
    }

    // Final insertion from local container to result
    for (Meshes::iterator i = meshes.begin();
         i != meshes.end();
         ++i)
    {
        // This insertion replaces old existing meshes with its updated 
        // version also
        final_meshes.insert(i.label(), i.ptr());
    }
}


// ----------------------------------------------------------------------------

Mesh_Ptr Mesh_File_Reader::read(
    const Label & name,
    bool load_vertices,
    bool load_triangles,
    bool load_triangle_strips,
    bool load_mapping
)  /* throw IO_Error */
{
    Mesh_Ptr mesh_ptr(new Mesh());
    Mesh &mesh = *mesh_ptr.get();
    
    Filename binary_filename(_mesh_path / (name + ".bin"));    
    if(!boost::filesystem::exists(binary_filename)) 
    {
        Filename vertex_filename, triangle_filename, 
            triangle_strip_filename, mapping_filename;
        if (load_vertices)
            vertex_filename = _mesh_path / (name + ".vert");
        if (load_triangles)
            triangle_filename = _mesh_path / (name + ".tri");
        if (load_triangle_strips)
            triangle_strip_filename = _mesh_path / (name + ".strip");
        if (load_mapping)
            mapping_filename = _mesh_path / (name + ".map");
        Mesh_ASCII_File_Parser parser;
        parser.read_mesh(
            vertex_filename,
            triangle_filename,
            triangle_strip_filename,
            mapping_filename,
            vertex_count(mesh),
            triangle_count(mesh),
            triangle_strip_length(mesh),
            vertices(mesh),
            vertex_sections(mesh),
            vertex_relative_distances(mesh),
            triangles(mesh),
            triangle_strip(mesh));
    } else {
        Mesh_Binary_File_Parser parser;
        parser.read_mesh(
            binary_filename,
            load_vertices,
            load_triangles,
            load_triangle_strips,
            load_mapping,
            vertex_count(mesh),
            triangle_count(mesh),
            triangle_strip_length(mesh),
            vertices(mesh),
            vertex_sections(mesh),
            vertex_relative_distances(mesh),
            triangles(mesh),
            triangle_strip(mesh));
    }
    
    return mesh_ptr;
}

void Mesh_File_Reader::insert_new_or_updated
(const std::string & name, Meshes & final_meshes, Meshes & meshes,
 bool load_vertices, bool load_triangles, bool load_triangle_strips,
 bool load_mapping)
{
    Mesh_Ptr mesh;
    
    // Serching if the mesh already exists
    Meshes::iterator old_mesh = final_meshes.find(name);
    if (old_mesh != final_meshes.end())
    {
        bool needs_vertices = 
            load_vertices && old_mesh->vertices().pointer() == 0;
        bool needs_triangles = 
            load_triangles && old_mesh->triangles().pointer() == 0;
        bool needs_triangle_strips = 
            load_triangle_strips && old_mesh->triangle_strip().pointer() == 0;
        bool needs_mapping = 
            load_mapping && old_mesh->vertex_sections().pointer() == 0;
        
        if (needs_vertices || needs_triangles || 
            needs_triangle_strips || needs_mapping)
        {
            // Loading missing datasets into mesh variable
            mesh = read(name, needs_vertices, needs_triangles, 
                        needs_triangle_strips, needs_mapping);

            // Add data not loaded from original mesh (if it has it)
            if (!needs_vertices)
            {
                vertices(*mesh) = vertices(*old_mesh);
                vertex_count(*mesh) = vertex_count(*old_mesh);
            }
            if (!needs_triangles)
            {
                triangles(*mesh) = triangles(*old_mesh);
                triangle_count(*mesh) = triangle_count(*old_mesh);
            }
            if (!needs_triangle_strips)
            {
                triangles(*mesh) = triangles(*old_mesh);
                triangle_count(*mesh) = triangle_count(*old_mesh);
            }
            if (!needs_mapping)
            {
                vertex_sections(*mesh) = vertex_sections(*old_mesh);
                vertex_relative_distances(*mesh) = 
                    vertex_relative_distances(*old_mesh);
            }
            normals(*mesh) = normals(*old_mesh);
        }
    }
    else
    {
        mesh = read(name, load_vertices, load_triangles, 
                    load_triangle_strips, load_mapping);
    }

    // Checking if something new has been read
    if (mesh.get() == 0)
        return;

    // computing additionnal data from the raw data if needed
    if (mesh->vertices().pointer() != 0 && 
        (mesh->triangles().pointer() != 0 || 
         mesh->triangle_strip().pointer() != 0) && 
        mesh->normals().pointer() == 0)
    {
        compute_per_vertex_normals(*mesh);
    }

    // Writing binary file if the whole mesh has been loaded and the binary
    // file doesn't exist yet.
    Filename binary_filename(_mesh_path / (name + ".bin"));    
    if(!boost::filesystem::exists(binary_filename)) 
    {
        if (mesh->vertices().pointer() != 0 && 
            mesh->triangles().pointer() != 0 && 
            mesh->triangle_strip().pointer() != 0 &&
            mesh->vertex_sections().pointer() != 0)
        {
            try
            {
                Mesh_Binary_File_Writer writer;
                writer.write_mesh(binary_filename, *mesh);
            } 
            catch (IO_Error)
            {
                std::cerr << "Error when writing the Binary file !\n";
            }
        }
    }

    if (old_mesh == final_meshes.end())
    {
        final_meshes.insert(name, mesh);
    }
    else
    {
        vertices(*old_mesh) = vertices(*mesh);
        vertex_count(*old_mesh) = vertex_count(*mesh);
        triangles(*old_mesh) = triangles(*mesh);
        triangle_count(*old_mesh) = triangle_count(*mesh);
        vertex_sections(*old_mesh) = vertex_sections(*mesh);
        vertex_relative_distances(*old_mesh) = vertex_relative_distances(*mesh);
        normals(*old_mesh) = normals(*mesh);
    }
}


}
