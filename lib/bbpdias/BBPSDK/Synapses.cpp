/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:	Thomas Traenkler
		Contributing authors:	Juan Hernando Vieites

*/

#include "BBP/Model/Microcircuit/Containers/Synapses.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"

namespace bbp {

// ----------------------------------------------------------------------------

Synapses::Synapses(const Label & label)
 : _label(label),
   _elements(new Synapse_Index_Set())
{
}

// ----------------------------------------------------------------------------

Synapses::Synapses(Microcircuit_Ptr microcircuit, const Label & label)
 : _label(label),
   _elements(new Synapse_Index_Set()),
   _microcircuit(microcircuit)
{
}

// ----------------------------------------------------------------------------

Neurons Synapses::presynaptic_neurons()
{
    Neurons result(_microcircuit.lock());
    for (Synapses::iterator i = begin(); i != end(); ++i)
    {
        Neuron_Ptr neuron = i->presynaptic_neuron_ptr();
        if (neuron.get() == 0) {
            throw_exception
                (Neuron_Not_Found("a" + boost::lexical_cast<std::string>
                                  (i->presynaptic_neuron_gid())),
                 SEVERE_LEVEL);
        }
        result.insert(neuron);
    }
    return result;
}

// ----------------------------------------------------------------------------

Neurons Synapses::postsynaptic_neurons()
{
    
    Neurons result(_microcircuit.lock());
    for (Synapses::iterator i = begin(); i != end(); ++i)
    {
        Neuron_Ptr neuron = i->postsynaptic_neuron_ptr();
        if (neuron.get() == 0) {
            //! \todo log a message or throw an exception?
            throw_exception
                (Neuron_Not_Found("a" + boost::lexical_cast<std::string>
                                  (i->postsynaptic_neuron_gid())), 
                 SEVERE_LEVEL);
        }
        result.insert(neuron);
    }
    return result;
}

// ----------------------------------------------------------------------------

const Neurons Synapses::presynaptic_neurons() const
{
    Neurons result(_microcircuit.lock());
    for (Synapses::const_iterator i = begin(); i != end(); ++i)
    {
        Neuron_Ptr neuron = boost::const_pointer_cast<Neuron, const Neuron>
            (i->presynaptic_neuron_ptr());
        if (neuron.get() == 0) 
        {
            throw_exception
                (Neuron_Not_Found("a" + boost::lexical_cast<std::string>
                                  (i->presynaptic_neuron_gid())),
                 SEVERE_LEVEL);
        }
        result.insert(neuron);
    }
    return result;
}

// ----------------------------------------------------------------------------

const Neurons Synapses::postsynaptic_neurons() const
{
    Neurons result(_microcircuit.lock());
    for (Synapses::const_iterator i = begin(); i != end(); ++i)
    {
        Neuron_Ptr neuron = boost::const_pointer_cast<Neuron, const Neuron>
            (i->postsynaptic_neuron_ptr());
        if (neuron.get() == 0)
        {
            throw_exception
                (Neuron_Not_Found("a" + boost::lexical_cast<std::string>
                                  (i->postsynaptic_neuron_gid())),
                 SEVERE_LEVEL);
        }
        result.insert(neuron);    
    }
    return result;
}

// ----------------------------------------------------------------------------

}
