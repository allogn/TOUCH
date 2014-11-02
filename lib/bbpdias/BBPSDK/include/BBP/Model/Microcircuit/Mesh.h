/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2008. All rights reserved.

        Authors: Juan Hernando Vieites
*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MESH_H
#define BBP_MESH_H

#include <boost/shared_ptr.hpp>
#include "BBP/Common/Dataset/Containers/Array.h"
#include "BBP/Model/Microcircuit/Datasets/Mesh_Dataset.h"

namespace bbp
{

//! Interface to a triangle based surface geometry of a Neuron.
class Mesh 
	: public Mesh_Dataset
{
public:
    Mesh()
    {
    }

    Mesh(const Mesh & rhs)
        : Mesh_Dataset((Mesh_Dataset) rhs)
    {
    }

    //! Returns the number of vertices in the mesh.
    Vertex_Index vertex_count() const
    {
        return _vertex_count;
    }

    //! Returns the number of triangles in the mesh.
    Triangle_Index triangle_count() const
    {
        return _triangle_count;
    }
    
    //! Returns the length of the triangle strip of the mesh
    Triangle_Index triangle_strip_length() const
    {
        return _triangle_strip_length;
    }

    //! Returns the array of positions for each vertex.
    const Array< Vector_3D<Micron> > vertices() const
    {
        return Array< Vector_3D<Micron> >(_vertices, _vertex_count);
    }
    
    //! Return the flat vector of positions for each vertex normal.
    const Array< Vector_3D<Micron> > normals() const
    {
        return Array< Vector_3D<Micron> >(_normals, _vertex_count);
    }

    //! Returns the section indices of the mesh vertices.
    const Array< Section_ID > vertex_sections() const
    {
        return Array< Section_ID >(_vertex_sections, _vertex_count);
    }
    
    /*! 
      Returns the relative distance in the section for each vertex
      of the mesh (between 0 and 1)).
    */
    const Array<float> vertex_relative_distances() const
    {
        return Array<float>(_vertex_relative_distances, _vertex_count);
    }
    
    /*!
      Returns the triangles of the mesh 
      ( !! size = 3 * _triangle_count for v1,v2,v3  ).
    */
    const Array< Vertex_Index > triangles() const
    {
        return Array< Vertex_Index >(_triangles, 3*_triangle_count);
    }

    //! Return a single triangle strip for the mesh.
    /*!
      The strip consists of several strips joined by degenerate triangles.
    */
    const Array< Vertex_Index > triangle_strip() const
    {
        return Array< Vertex_Index >(_triangle_strip, _triangle_strip_length);
    }
};

typedef boost::shared_ptr<Mesh> Mesh_Ptr;

}

#endif // BBP_MESH
