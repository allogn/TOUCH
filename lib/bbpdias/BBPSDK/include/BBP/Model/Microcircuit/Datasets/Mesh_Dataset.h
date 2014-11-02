/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors: Sebastien Lasserre
                 Juan Hernando Vieites

Current issues:
- (17-12-07) Static mesh counting from previous BDB version has been removed

*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_DATASET_MESH_H
#define BBP_DATASET_MESH_H

#include "BBP/Model/Microcircuit/Types.h"
#include <cstdio>

namespace bbp
{
//!  This class contains the mesh data for one morphology.
/*!
 It provides the data for the vertices and the triangles.

 For the vertices, we have :
    - position :             3 * 4  bytes
    - section ID :           4 bytes
    - relative distance :    4 bytes

    Total                    20 bytes per vertex
 
 For the triangles, we have :
    - indices :              3 * 4 bytes

    Total                    12 bytes per triangle
*/
class Mesh_Dataset
{        
public:
    friend class Mesh_Dataset_Accessor ;
    
    /* 
       Public data access members
    */
public:
    Mesh_Dataset() {}
    
    Mesh_Dataset(const Mesh_Dataset & rhs)
    {
        _vertex_count = rhs._vertex_count;
        _triangle_count = rhs._triangle_count;
        _triangle_strip_length = rhs._triangle_strip_length;
        
        // arrays
        Vector_3D<Micron> * vertices_temp = 
        new Vector_3D<Micron>[_vertex_count];
        std::memcpy(vertices_temp, rhs._vertices.get(), 
                    sizeof(Vector_3D<Micron>) * _vertex_count);
        _vertices = Vector_3D_Micron_Array(vertices_temp);
        
        Vector_3D<Micron> * normals_temp = 
        new Vector_3D<Micron>[_vertex_count];
        std::memcpy(normals_temp, rhs._normals.get(), 
                    sizeof(Vector_3D<Micron>) * _vertex_count);
        _normals = Vector_3D_Micron_Array(normals_temp);
        
        Section_ID * vertex_sections_temp =
        new Section_ID[_vertex_count];
        std::memcpy(vertex_sections_temp, rhs._vertex_sections.get(), 
                    sizeof(Section_ID) * _vertex_count);
        _vertex_sections = Section_ID_Array(vertex_sections_temp);
        
        float * vertex_relative_distances_temp =
        new float[_vertex_count];
        std::memcpy(vertex_relative_distances_temp, 
                    rhs._vertex_relative_distances.get(), 
                    sizeof(float) * _vertex_count);
        _vertex_relative_distances =
        Float_Array(vertex_relative_distances_temp);
        
        Vertex_Index * triangles_temp =
        new Vertex_Index[_triangle_count];
        std::memcpy(triangles_temp,
                    rhs._triangles.get(), 
                    sizeof(Vertex_Index) * _triangle_count);
        _triangles = Vertex_Index_Array(triangles_temp);
        
        Vertex_Index * triangle_strip_temp =
        new Vertex_Index[_triangle_strip_length];
        std::memcpy(triangle_strip_temp,
                    rhs._triangle_strip.get(),
                    sizeof(Vertex_Index) * _triangle_strip_length);        
        _triangle_strip = Vertex_Index_Array(triangle_strip_temp);
    }

    virtual ~Mesh_Dataset() {}

    /*!
       Return number of vertices in the mesh.
    */
    Vertex_Index vertex_count() const
    {
        return _vertex_count ;
    }

    /*!
       Return number of triangles in the mesh.
    */
    Triangle_Index triangle_count() const
    {
        return _triangle_count ;
    }
    
    /*!
       Return positions of the vertices in the mesh
       ( !! size = 3*_numVertices for x,y,z).
    */
    const Micron *raw_vertices() const
    {
        return reinterpret_cast<Micron*>(_vertices.get());
    }
    
    /*!
        Return flat array of positions for each vertex.
     */
    const Vector_3D<Micron> *vertices() const
    {
        return _vertices.get();
    }
    
    /*!
       Return normals of the vertices in the mesh
       ( !! size = 3*_numVertices for nx,ny,nz).
    */
    const Micron *raw_normals() const
    {
        return reinterpret_cast<Micron*>(_normals.get());
    }
    
    /*!
        Return flat array of positions for each vertex normal.
     */
    const Vector_3D<Micron> *normals() const
    {
        return _normals.get();
    }
    
    /*!
       Return section Index of the mesh.
    */
    const Section_ID *vertex_sections() const
    {
        return _vertex_sections.get();
    }
    
    /*!
       Return relative distance in the section for each vertices 
       of the mesh (between 0 and 1)).
    */
    const float *vertex_relative_distances() const
    {
        return _vertex_relative_distances.get();
    }
    
    /*!
       Return triangles in the mesh 
       ( !! size = 3 * _numVertices for v1,v2,v3  ).
    */
    const Vertex_Index *triangles() const
    {
        return _triangles.get();
    }
    
    
    //------------------------------------------------------------------------------------------------------------------------------------
    // MEMBER ATTRIBUTES
    //------------------------------------------------------------------------------------------------------------------------------------
protected :     
    //! number of vertices in the mesh
    Vertex_Index _vertex_count;

    //! number of triangles in the mesh
    Triangle_Index _triangle_count;

    //! length of the triangle strip of the mesh
    Triangle_Index _triangle_strip_length;
    
    //! positions of the vertices in the mesh ( !! size = 3*_numVertices for x,y,z)
    Vector_3D_Micron_Array _vertices;

    //! normals for all the vertices
    Vector_3D_Micron_Array _normals; 
        
    //! section Index of the mesh
    Section_ID_Array _vertex_sections;
    
    //! relative distance in the section for each vertices of the mesh (between 0 and 1))
    Float_Array _vertex_relative_distances;
    
    //! triangles in the mesh ( !! size = 3*_numVertices for v1,v2,v3  )
    Vertex_Index_Array _triangles;

    //! triangle strip of this mesh
    Vertex_Index_Array _triangle_strip;
};

}
#endif // BBP_DATASET_MESH_H
