/*

            Ecole Polytechnique Federale de Lausanne
            Brain Mind Institute,
            Blue Brain Project
            (c) 2008. All rights reserved.

            Authors: Sebastien Lasserre
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_DATASET_MESH_ACCESSOR_H
#define BBP_DATASET_MESH_ACCESSOR_H

//!\ todo Fix this properly dataset vs. mesh object. (TT)
#include "../../Mesh.h"

namespace bbp
{

/**
   Class that has access to Mesh_Dataset protected attributes.
 */
class Mesh_Dataset_Accessor
{
protected:
    static Vertex_Index & vertex_count(Mesh & mesh)
    {
        return mesh._vertex_count;
    }
    
    static Triangle_Index & triangle_count(Mesh & mesh)
    {
        return mesh._triangle_count;
    }
    
    static Triangle_Index & triangle_strip_length(Mesh & mesh)
    {
        return mesh._triangle_strip_length;
    }

    static Vector_3D_Micron_Array & vertices(Mesh & mesh)
    {
        return mesh._vertices;
    }
    
    static Vector_3D_Micron_Array & normals(Mesh & mesh)
    {
        return mesh._normals;
    }
    
    static Section_ID_Array & vertex_sections(Mesh & mesh)
    {
        return mesh._vertex_sections;
    }
    
    static Float_Array & vertex_relative_distances(Mesh & mesh)
    {
        return mesh._vertex_relative_distances;
    }
    
    static Vertex_Index_Array & triangles(Mesh & mesh)
    {
        return mesh._triangles;
    }  

    static Vertex_Index_Array & triangle_strip(Mesh & mesh)
    {
        return mesh._triangle_strip;
    }  

    // ------------------------------------------------------------------------

    static const Vertex_Index & vertex_count(const Mesh & mesh)
    {
        return mesh._vertex_count;
    }
    
    static const Triangle_Index & triangle_count(const Mesh & mesh)
    {
        return mesh._triangle_count;
    }
    
    static const Triangle_Index & triangle_strip_length(const Mesh & mesh)
    {
        return mesh._triangle_strip_length;
    }

    static const Vector_3D_Micron_Array & vertices(const Mesh & mesh)
    {
        return mesh._vertices;
    }
    
    static const Vector_3D_Micron_Array & normals(const Mesh & mesh)
    {
        return mesh._normals;
    }
    
    static const Section_ID_Array & vertex_sections(const Mesh & mesh)
    {
        return mesh._vertex_sections;
    }
    
    static const Float_Array & vertex_relative_distances(const Mesh & mesh)
    {
        return mesh._vertex_relative_distances;
    }
    
    static const Vertex_Index_Array & triangles(const Mesh & mesh)
    {
        return mesh._triangles;
    }

    static const Vertex_Index_Array & triangle_strip(const Mesh & mesh)
    {
        return mesh._triangle_strip;
    }
};

}
#endif
