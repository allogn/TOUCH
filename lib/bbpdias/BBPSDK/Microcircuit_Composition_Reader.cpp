/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

		Authors: Juan Hernando Vieites
*/
#include <boost/filesystem.hpp>

#include "BBP/Model/Microcircuit/Readers/Microcircuit_Composition_Reader.h"
#include "Microcircuit_MVD_File_Reader.h"

namespace bbp
{

static const char * MVD_CIRCUIT_FILE_NAME = "circuit.mvd2";

Microcircuit_Composition_Reader_Ptr 
Microcircuit_Composition_Reader::create_reader(const URI & circuit_source)
{
    boost::filesystem::path path = uri_to_filename(circuit_source);

    // Checking if path is a directory
    if (path != "")
    {
        if  (boost::filesystem::is_directory(path))
        {
            Filepath circuit_filename = path / MVD_CIRCUIT_FILE_NAME;
            if (boost::filesystem::is_regular(circuit_filename))
            {
                return Microcircuit_Composition_Reader_Ptr
                    (new Microcircuit_MVD_File_Reader(circuit_filename));
            }
        }
        else if (boost::filesystem::is_regular(circuit_source) &&
                 boost::filesystem::extension(circuit_source) == ".mvd2")
        {
            return Microcircuit_Composition_Reader_Ptr
                (new Microcircuit_MVD_File_Reader(path));
        }
    }

    // No suitable reader found
    log_message("Microcircuit_Composition_Reader: No suitable reader for"
                " circuit_source: " + circuit_source, SEVERE_LEVEL);
    return Microcircuit_Composition_Reader_Ptr();
}

}
