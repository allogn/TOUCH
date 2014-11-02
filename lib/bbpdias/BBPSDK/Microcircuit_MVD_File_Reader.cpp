/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

		Authors: Juan Hernando Vieites
*/

#include <boost/filesystem.hpp>

#include "BBP/Model/Microcircuit/Neuron.h"
#include "Microcircuit_MVD_File_Reader.h"
#include "Parsers/MVD_File_Parser.h"

namespace bbp
{

// ----------------------------------------------------------------------------

void Microcircuit_MVD_File_Reader::open()
{
    if (!boost::filesystem::exists(_circuit_filename))
    {
        throw_exception(File_Open_Error("Circuit .mvd file not found: " + 
                                        _circuit_filename.string()),
                        FATAL_LEVEL, __FILE__, __LINE__);
    }
    if (!boost::filesystem::is_regular(_circuit_filename))
    {
        throw_exception(File_Open_Error("Invalid circuit mvd file: " + 
                                        _circuit_filename.string()),
                        FATAL_LEVEL, __FILE__, __LINE__);
    }
}

// ----------------------------------------------------------------------------

void Microcircuit_MVD_File_Reader::close()
{
}

// ----------------------------------------------------------------------------

void Microcircuit_MVD_File_Reader::load(Neurons & neurons, 
                                        const Cell_Target & target, 
					                    const Structure_Dataset_Ptr & structure)
{
    // Empty targets are skipped, otherwise the parser gets the information
    // of the whole column in the mvd file.
    if (target.size() == 0)
        return;

    MVD_File_Parser parser(_circuit_filename);
    parser.parse_file(target);

    structure->Morphology_Types_Array = parser.morphology_types_array();
    structure->Electrophysiology_Types_Array = parser.electrophysiology_types_array();
    structure->Excitatory_Array = parser.excitatory_types_array();
    structure->Pyramidal_Array = parser.pyramidal_types_array();

    for (size_t i = 0; i < parser.cells_loaded(); ++i) {
        Neuron_Ptr neuron_ptr(new Neuron(*microcircuit(neurons)));
        Neuron &neuron = *neuron_ptr.get();
        cell_gid(neuron) = parser.cell_gids()[i];
        morphology_name(neuron) = parser.morphology_names()[i];
        column_number(neuron) = parser.column_gids()[i];
        minicolumn_number(neuron) = parser.minicolumn_gids()[i];
        layer_number(neuron) = parser.layers()[i] + 1;
		parser.morphology_types()[i]._dataset = structure;
        morphology_type(neuron) = Morphology_Type(parser.morphology_types()[i]);
	    parser.electrophysiology_types()[i]._dataset = structure;
        electro_type(neuron) = Electrophysiology_Type(
            parser.electrophysiology_types()[i]);
        neuron_position(neuron) = parser.positions()[i];
        neuron_orientation(neuron) = Orientation(parser.y_axis_rotations()[i], Vector_3D<float>(0, 1, 0));
        global_transform(neuron) = Translation_3D<Micron>(neuron.position()) * Rotation_3D<Micron>(neuron_orientation(neuron));

        /*! \todo Check what happens if already in circuit. (TT) Answer: 
            Neuron is replaced by the new one and its morphology is linked
            to it. Synapses, instead, are not linked to the neuron and 
            previous information is lost. Nevertheless, Microcircuit_Reader
            filters the neurons already loaded, so this is not an issue. (JH) */
        neurons.insert(neuron_ptr);
    }
}

// ----------------------------------------------------------------------------

}

