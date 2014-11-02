/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible author:             Thomas Traenkler
        Contributing authors:           Juan Hernando Vieites

*/

#include "BBP/Model/Microcircuit/Neuron.h"
#include "BBP/Model/Microcircuit/Containers/Neurons.h"

namespace bbp
{

// ----------------------------------------------------------------------------

Neuron::Neuron(Microcircuit & microcircuit)
: _index(UNDEFINED_CELL_INDEX),
  _microcircuit(microcircuit._shared_this),
  _gid(UNDEFINED_CELL_GID),
  _column_gid(UNDEFINED_COLUMN_GID),
  _minicolumn_gid(UNDEFINED_MINICOLUMN_GID),
  _layer_number(UNDEFINED_CORTICAL_LAYER_NUMBER),
  _electrophysiology_type(UNDEFINED_ELECTROPHYSIOLOGY_TYPE,
                          microcircuit.dataset().structure),
  _position(UNDEFINED_VECTOR_3D_MICRON),
  _orientation(UNDEFINED_ORIENTATION),
  _afferent_synapses(new Synapses(microcircuit._shared_this.lock())),
  _efferent_synapses(new Synapses(microcircuit._shared_this.lock()))
{
}

// ----------------------------------------------------------------------------

Neuron::Neuron(Microcircuit & microcircuit, Cell_GID gid)
: _index(UNDEFINED_CELL_INDEX),
  _microcircuit(microcircuit._shared_this),
  _gid(gid), 
  _column_gid(UNDEFINED_COLUMN_GID),
  _minicolumn_gid(UNDEFINED_MINICOLUMN_GID),
  _layer_number(UNDEFINED_CORTICAL_LAYER_NUMBER),
  _electrophysiology_type(UNDEFINED_ELECTROPHYSIOLOGY_TYPE,
                          microcircuit.dataset().structure),
  _position(UNDEFINED_VECTOR_3D_MICRON),
  _orientation(UNDEFINED_ORIENTATION),
  _afferent_synapses(new Synapses(microcircuit._shared_this.lock())),
  _efferent_synapses(new Synapses(microcircuit._shared_this.lock()))
{
}

// ----------------------------------------------------------------------------

Neuron::Neuron(Microcircuit & microcircuit, 
               Cell_GID gid,
               Cell_Index index)
: _index(index),
  _microcircuit(microcircuit._shared_this),
  _gid(gid), 
  _column_gid(UNDEFINED_COLUMN_GID),
  _minicolumn_gid(UNDEFINED_MINICOLUMN_GID),
  _layer_number(UNDEFINED_CORTICAL_LAYER_NUMBER),
  _electrophysiology_type(UNDEFINED_ELECTROPHYSIOLOGY_TYPE,
                          microcircuit.dataset().structure),
  _position(UNDEFINED_VECTOR_3D_MICRON),
  _orientation(UNDEFINED_ORIENTATION),
  _afferent_synapses(new Synapses(microcircuit._shared_this.lock())),
  _efferent_synapses(new Synapses(microcircuit._shared_this.lock()))
{
}

// ---------------------------------------------------------------------------

//Neuron::Neuron(Microcircuit_Ptr microcircuit, const std::string gid_string)
//: index(UNDEFINED_CELL_INDEX),
//  number_of_synapses(UNDEFINED_SYNAPSE_INDEX)
//{
//    // if start with 'a' and a digit (e.g. a5...)
//    if ((gid_string[0] == 'a') &&
//        (gid_string[1] == '0' ||
//         gid_string[1] == '1' ||
//         gid_string[1] == '2' ||
//         gid_string[1] == '3' ||
//         gid_string[1] == '4' ||
//         gid_string[1] == '5' ||
//         gid_string[1] == '6' ||
//         gid_string[1] == '7' ||
//         gid_string[1] == '8' ||
//         gid_string[1] == '9'))
//    {
//        Label raw_gid_string;
//        raw_gid_string = (gid_string).substr(1);
//        gid = boost::lexical_cast<Cell_GID>(raw_gid_string);
//    }
//    else
//        throw std::logic_error("Neuron could not be created. string not "
//			"a valid GID");
//}

// ----------------------------------------------------------------------------

const Neurons Neuron::presynaptic_neurons() const
{
    return Neurons(_afferent_synapses->presynaptic_neurons());
}

// ----------------------------------------------------------------------------

const Neurons Neuron::postsynaptic_neurons() const
{
    return Neurons(_efferent_synapses->postsynaptic_neurons());
}

// ----------------------------------------------------------------------------


Neurons Neuron::presynaptic_neurons()
{
    return Neurons(_afferent_synapses->presynaptic_neurons());
}

// ----------------------------------------------------------------------------

Neurons Neuron::postsynaptic_neurons()
{
    return Neurons(_efferent_synapses->postsynaptic_neurons());
}

// ----------------------------------------------------------------------------

Micron Neuron::path_to_soma(
    Section_ID section_id, Section_Normalized_Distance distance)
{
#ifdef BBP_SAFETY_MODE
	if (_morphology == 0)
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
    bbp_assert(_morphology != 0);
    return _morphology->path_to_soma(section_id, distance);
}

// ----------------------------------------------------------------------------
Neuron::Neuron(){}
}
