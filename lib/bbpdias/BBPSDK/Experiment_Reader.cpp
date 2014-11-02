/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2008. All rights reserved.

                Authors: Thomas Trankler, Sebastien Lasserre
*/

#include <boost/filesystem.hpp>

#include "BBP/Model/Experiment/Readers/File/Blue_Config_File_Reader.h"
#include "BBP/Model/Microcircuit/Readers/Target_Reader.h"
#include "BBP/Model/Experiment/Readers/Experiment_Reader.h"


namespace bbp
{

// ----------------------------------------------------------------------------

void Experiment_Reader::read(Experiment & experiment)
{
    log_message("Loading BlueConfig: " + _source, INFORMATIVE_LEVEL);

    // read blue config and init the paths of the Experiment
    if (_replace == false) 
    {
        // Read BlueConfig file.
        Blue_Config_File_Reader reader(_source);
        reader.read(experiment);
    }
    else 
    {
        // Read BlueConfig file and replace source URI prefix.
        Blue_Config_File_Reader reader(_source, 
            _original_prefix, _replacement_prefix);
        reader.read(experiment);
    }

}

// ----------------------------------------------------------------------------

void Experiment_Reader::load_user_targets(Experiment &experiment)
{    
    // read user targets if any
    if (user_target_source(experiment) != "")
    {
        log_message("Loading user targets from: " + 
                    user_target_source(experiment), 
                    INFORMATIVE_LEVEL);

        Target_Reader_Ptr  user_target_reader = 
            Target_Reader::create_reader(user_target_source(experiment));
        if (user_target_reader.get() == 0)
        {
            throw_exception(
                Bad_Data_Source("Couldn't find a suitable reader for "
                                "user targets source: " + 
                                user_target_source(experiment)),
                FATAL_LEVEL, __FILE__, __LINE__);
        }

        user_target_reader->load(user_targets(experiment), experiment.microcircuit().targets() );
    }
    else
    {
        log_message("No user targets to load");
    }
}

// ----------------------------------------------------------------------------

}
