/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2006-2007. All rights reserved.

                Author: Sebastien Lasserre
*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MESH_FILE_READER_H
#define BBP_MESH_FILE_READER_H

#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Microcircuit/Readers/Mesh_Reader.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Morphology;
typedef boost::shared_ptr<Morphology> Morphology_Ptr;
class Mesh;
typedef boost::shared_ptr<Mesh> Mesh_Ptr;

// ----------------------------------------------------------------------------

/*!
    File loader for mesh. 
    Reads binary files by default or the ASCI files if the binary are
    missing. It fills the morphology objects with the mesh object with data.
*/
class Mesh_File_Reader : public Mesh_Reader
{

public:
    Mesh_File_Reader(const Directory & mesh_path = ".");

    //! Returns the data source this reader is bound to.
    virtual const Directory & source() const
    {
        return _mesh_path;
    }

    // Test the mesh path for existence
    virtual void open();

    virtual void close() {}

    // @see Mesh_Reader::load
    virtual void load(Meshes & meshes,
                      const std::set<Label> & names,
                      bool load_vertices = true,
                      bool load_triangles = true,
                      bool load_mapping = true,
                      bool load_triangle_strips = false);
    
    // @see Mesh_Reader::load                         
    virtual void load(Meshes & meshes,
                      const Cell_Target & cells, 
                      const URI & circuit_source,
                      bool load_vertices = true,
                      bool load_triangles = true,
                      bool load_mapping = true,
                      bool load_triangle_strips = false);

    //! load specified mesh from file using the name of the morphology
    Mesh_Ptr read(const Label & name,
                  bool load_vertices,
                  bool load_triangles,
                  bool load_triangle_strips,
                  bool load_mapping
                  ) /* throw IO_Error */;

protected:
    void insert_new_or_updated
        (const std::string & name, Meshes & final_meshes, Meshes & meshes,
         bool load_vertices, bool load_triangles, bool load_triangle_strips,
         bool load_mapping);

    Directory _mesh_path;
};


// ----------------------------------------------------------------------------

inline Mesh_File_Reader::Mesh_File_Reader
    (const Directory & mesh_path) :
    _mesh_path(mesh_path)
{
}

// ----------------------------------------------------------------------------

}
#endif
