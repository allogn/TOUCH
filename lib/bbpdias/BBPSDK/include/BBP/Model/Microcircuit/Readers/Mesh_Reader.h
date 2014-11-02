/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2008. All rights reserved.

                Authors:	Sebastien Lasserre,
							Juan Hernando Vieites
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MESH_READER_H
#define BBP_MESH_READER_H

#include <set>
#include <boost/shared_ptr.hpp>


#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Datasets/Accessors/Mesh_Dataset_Accessor.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"
#include "BBP/Model/Microcircuit/Containers/Meshes.h"
#include "BBP/Model/Microcircuit/Mesh.h"

namespace bbp
{

// ----------------------------------------------------------------------------

class Morphologies;
class Mesh_Reader;
typedef boost::shared_ptr<Mesh_Reader> Mesh_Reader_Ptr;

// ----------------------------------------------------------------------------

//! Reader for mesh (surface geometry) data.
class Mesh_Reader 
	: public Mesh_Dataset_Accessor
{
public:
   
   virtual ~Mesh_Reader() {}
   
    /*!
       Creates a specific reader for the given data source
     */
    static Mesh_Reader_Ptr create_reader(const URI & mesh_source);

    //! Returns the data source this reader is bound to.
    virtual const Directory & source() const = 0;
    
    // Opens the data source
    virtual void open() = 0 /* throw something ? */;

    // Opens the data source
    virtual void close() = 0 /* throw something ? */;

    /*!
       Load meshes for each morphology of the morphology container.
       By default all the data are loaded but it is possible to avoid the loading
       of vertices, triangles or mapping information.
     */
    virtual void load(Meshes & meshes,
                      const std::set<Label> & names,
                      bool load_vertices = true,
                      bool load_triangles = true,
                      bool load_mapping = true,
                      bool load_triangle_strips = false
                     ) /* throw something ? */ = 0;
                             
    /*!
       Load meshes using a cell target.
       Loads unique meshes into the mesh container using the
       morpholgy names from the cell target given. The morphology names
       are retreived from the circuit source specified.
     */
    virtual void load(Meshes & meshes,
                      const Cell_Target & cells, 
                      const URI & circuit_source,
                      bool load_vertices = true,
                      bool load_triangles = true,
                      bool load_mapping = true,
                      bool load_triangles_strips = false
                      ) /* throw something ? */ = 0;

protected:
    // Convenience functions to use by derived classes if needed
    void compute_per_vertex_normals(Mesh &mesh);
};

// ----------------------------------------------------------------------------

}
#endif
