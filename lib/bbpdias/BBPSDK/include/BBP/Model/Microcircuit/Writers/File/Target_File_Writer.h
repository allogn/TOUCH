/*
 *  Target_File_Writer.h
 *  bc_extBBP-SDK
 *
 *  Created by James King on 4/14/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_TARGET_FILE_WRITER
#define BBP_TARGET_FILE_WRITER

#include <string>

#include "BBP/Model/Microcircuit/Targets/Targets.h"

namespace bbp {

//! Facilitate the writing of user targets to a file
class Target_File_Writer
{
public:
    //! Write a target file to disk.
    /*! This function is intended to really only write
        the user target file since the standard start.target should not 
        need to be modified outside of BlueBuilder
        \param targets Target container object whose targets
        \param out_file output file path
    */
    void write( const Targets &targets, const std::string &out_file );
    
 private:
        
    void write_cell_target( const Target &target, FILE *fout );
    
    void write_section_target( const Target &target, FILE *fout );
    
    void write_compartment_target( const Target &target, FILE *fout );
    
    void write_synapse_target( const Target &target, FILE *fout );
};

}
#endif
