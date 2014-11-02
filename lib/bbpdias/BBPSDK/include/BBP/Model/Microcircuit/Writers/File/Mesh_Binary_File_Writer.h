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

#ifndef BBP_MESH_BINARY_FILE_WRITER_H
#define BBP_MESH_BINARY_FILE_WRITER_H

#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Microcircuit/Mesh.h"

namespace bbp {

/*!
*/
class Mesh_Binary_File_Writer
{

public:

    //! write the mesh data structure into a binary file.
    void write_mesh(
        const Filename        & file,
        Mesh                  & mesh
    );

};
}
#endif // BBP_MESH_BINARY_FILE_WRITER_H
