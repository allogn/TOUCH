/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible author:      Thomas Traenkler
        Contributing authors:    Sebastien Lasserre

        TODO: 

        - Get informations about the connection_scaling (SL - 22/01/2008)
                           
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_BLUE_CONFIG_FILE_READER_H
#define BBP_BLUE_CONFIG_FILE_READER_H

#include <fstream>
#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Experiment/Experiment.h"

namespace bbp {

class Experiment;

// ----------------------------------------------------------------------------

//! File loader for simulator configuration data. 
/*!
    Adapter to Blue Config reader of Blue Dataset_Microcircuit that fills 
    the interfaces with data.
*/
class Blue_Config_File_Reader 
    : public Experiment_Accessor
{
public:
        
    //! Specifies path to configuration file.
    Blue_Config_File_Reader(Filepath blue_config_filepath);
        
    //! Specifies path to configuration file and replaces the URI prefix.
    /*!
        \param  blue_config_filepath Path to the Blue Config configuration \
                file.
        \param  directory_prefix Original directory prefix that is \
                to be replaced in all paths.
        \param  replacement_directory_prefix Replacement directory prefix.
    */
    Blue_Config_File_Reader(Filepath        blue_config_filepath,
                            const Directory &     original_prefix,
                            const Directory &     replacement_prefix);

public:

    //! reads blue config file infos into experiment
    void read(Experiment & experiment);
    
public:

    //! attributes for the Experiment object
    std::string    date, 
                   time, 
                   svn_url, 
                   svn_revision,
                   forward_skip,
                   duration,
                   timestep,
                   description,
                   circuit_target,
                   directory_prefix,
                   morphologies_directory,
                   metype_directory,
                   microcircuit_directory,
                   nrn_directory,
                   mesh_directory,
                   current_directory,
                   output_directory,
                   target_filename,
                   blue_config_filepath;
        
private:

    //! Blue Config file.
    std::ifstream   file;
    //! Source of the BlueConfig.
    Filepath        _blue_config_filename;
    //! drive letter that will replace /bglscratch
    Directory       _original_prefix,
                    _replacement_prefix;
};

// ----------------------------------------------------------------------------

}
#endif
