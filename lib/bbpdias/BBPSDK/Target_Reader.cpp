/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2008. All rights reserved.

                Authors: Juan Hernando Vieites
*/
#include <boost/filesystem.hpp>

#include "BBP/Model/Microcircuit/Readers/Target_Reader.h"
#include "File/Target_File_Reader.h"

namespace bbp
{

// ----------------------------------------------------------------------------

static const char * TARGET_FILENAME = "start.target";

Target_Reader_Ptr 
Target_Reader::create_reader(const URI & target_source)
{
    Filepath path = uri_to_filename(target_source);
    if (path != "")
    {
        if (boost::filesystem::is_directory(path))
        {
            Filepath target_filepath = path / TARGET_FILENAME;
            if (boost::filesystem::is_regular(target_filepath))
            {
                return Target_Reader_Ptr
                    (new Target_File_Reader(target_filepath));
            }
        }
        else if (boost::filesystem::extension(path) == ".target")
        {
            return Target_Reader_Ptr
                (new Target_File_Reader(path));
        }
    }

    
    // No suitable reader found
    log_message("Target_Reader: No suitable reader for circuit_source: " +
                target_source, SEVERE_LEVEL);
    return Target_Reader_Ptr();
}

// ----------------------------------------------------------------------------

}
