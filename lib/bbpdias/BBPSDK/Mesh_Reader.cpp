/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2008. All rights reserved.

                Authors: Sebastien Lasserre, Juan Hernando Vieites
*/

#include <boost/filesystem.hpp>

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Readers/Mesh_Reader.h"
#include "File/Mesh_File_Reader.h"

namespace bbp
{

static const char * MESH_SUBPATH = "high/TXT";

// ----------------------------------------------------------------------------

Mesh_Reader_Ptr 
Mesh_Reader::create_reader(const URI & mesh_source)
{
    Directory path = uri_to_filename(mesh_source);
    if (path != "" && boost::filesystem::is_directory(path)) {
        return Mesh_Reader_Ptr
            (new Mesh_File_Reader(path / MESH_SUBPATH));
    }

    /* No suitable reader found */
    log_message("Mesh_Reader: No suitable reader for"
                " mesh source: " + mesh_source, SEVERE_LEVEL);
    return Mesh_Reader_Ptr();
}

void Mesh_Reader::compute_per_vertex_normals(Mesh &mesh)
{
    const Vector_3D<Micron> * vertices = mesh.vertices().pointer();
    // Allocating and initializing array
    const Vertex_Index vertex_count = mesh.vertex_count();

    Vector_3D_Micron_Array & normals = Mesh_Dataset_Accessor::normals(mesh); 
    normals.reset(new Vector_3D<Micron>[vertex_count]);
    for (Vertex_Index i = 0; i < vertex_count ; ++i)
        normals[i].reset();
    
    if (mesh.triangles().pointer() != 0) {
        /* Computing normals from a triangle soup */
        const Vertex_Index * triangles = mesh.triangles().pointer();
        const Triangle_Index triangle_count = mesh.triangle_count();
        for (size_t i = 0; i < triangle_count * 3; i += 3) {
            bbp_assert(triangles[i] < vertex_count &&
                   triangles[i + 1] < vertex_count &&
                   triangles[i + 2] < vertex_count);
            const Vector_3D<Micron> u = vertices[triangles[i]];
            const Vector_3D<Micron> v = vertices[triangles[i + 1]];
            const Vector_3D<Micron> w = vertices[triangles[i + 2]];
            Vector_3D<Micron> face_normal = (v - u) ^ (w - u);
            normals[triangles[i]] += face_normal;
            normals[triangles[i + 1]] += face_normal;
            normals[triangles[i + 2]] += face_normal;
        }
    } else {
        /* Computing normals from a triangle strip */
        const Vertex_Index * strip = mesh.triangle_strip().pointer();
        const Triangle_Index strip_length = mesh.triangle_strip_length();
        bool positive = true;
        for (size_t i = 0; i < strip_length - 2; ++i, positive = !positive) {
            if (strip[i] == strip[i + 1] || strip[i] == strip[i + 2] ||
                strip[i + 1] == strip[i + 2]) {
                /* Skipping degenerated triangle */
                continue;
            }
            bbp_assert(strip[i] < vertex_count &&
                   strip[i + 1] < vertex_count &&
                   strip[i + 2] < vertex_count);
            const Vector_3D<Micron> u = vertices[strip[i]];
            const Vector_3D<Micron> v = positive ? vertices[strip[i + 1]] :
                                                   vertices[strip[i + 2]];
            const Vector_3D<Micron> w = positive ? vertices[strip[i + 2]] :
                                                   vertices[strip[i + 1]];
            Vector_3D<Micron> face_normal = (v - u) ^ (w - u);
            normals[strip[i]] += face_normal;
            normals[strip[i + 1]] += face_normal;
            normals[strip[i + 2]] += face_normal;
        }
    }

    for (size_t i = 0; i < vertex_count; ++i) {
        Micron length = normals[i].length();
        if (length == 0)
            log_message("Normal calculation resulted in 0 length normal", 
                        INFORMATIVE_LEVEL);
        else
            normals[i].normalize();
    }
}

// ----------------------------------------------------------------------------

}
