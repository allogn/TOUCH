/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Authors: Thomas Traenkler
                 Juan Hernando Vieites

*/

#include <typeinfo>

#include "BBP/Model/Microcircuit/Readers/Microcircuit_Reader.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"
#include "BBP/Model/Microcircuit/Distiller/Distiller.h"
#include "BBP/Model/Microcircuit/Containers/Synapses.h"
#include "BBP/Model/Microcircuit/Containers/Morphologies.h"

namespace bbp {
// ----------------------------------------------------------------------------

Microcircuit_Ptr Microcircuit::create()
{
    Microcircuit_Ptr ptr(new Microcircuit());
    ptr->_shared_this = ptr;
    // We don't need the _strong_shared_this since the reference that 
    // _shared_this needs will be external.
    ptr->_strong_shared_this.reset();
    return ptr;
}

// ----------------------------------------------------------------------------

Microcircuit::Microcircuit()
{
    // Note that the internal shared_ptr never deallocates the object.
    _strong_shared_this = Microcircuit_Ptr(this, shared_ptr_non_deleter);
    _shared_this = _strong_shared_this;
}

// ----------------------------------------------------------------------------

Distiller Microcircuit::distiller()
{
    return Distiller(*this);
}

const Morphologies & Microcircuit::morphologies() const
{
    return * _morphologies;
}

// ----------------------------------------------------------------------------

Morphologies & Microcircuit::morphologies()
{
    return * _morphologies;
}

// ----------------------------------------------------------------------------

const Meshes & Microcircuit::meshes() const
{
    return _meshes;
}

// ----------------------------------------------------------------------------

Meshes & Microcircuit::meshes()
{
    return _meshes;
}

// ----------------------------------------------------------------------------

const Neurons & Microcircuit::neurons() const
{
    return *_neurons;
}

// ----------------------------------------------------------------------------

Neurons & Microcircuit::neurons()
{
    return *_neurons;
}

// ----------------------------------------------------------------------------

Neuron & Microcircuit::neuron(Cell_GID gid)
{
    Neurons::iterator neuron = _neurons->find(gid);
    if (neuron != _neurons->end())
    {
        return * neuron;
    }
    else
    {
        Neuron_Not_Found exc("a" + boost::lexical_cast<std::string>(gid));
        log_message(exc.what(), WARNING_LEVEL);
        throw exc;
    }
}

// ----------------------------------------------------------------------------

const Neuron & Microcircuit::neuron(Cell_GID gid) const
{
    Neurons::iterator neuron = _neurons->find(gid);
    if (neuron != _neurons->end())
    {
        return * neuron;
    }
    else
    {
        Neuron_Not_Found exc("a" + boost::lexical_cast<std::string>(gid));
        log_message(exc.what(), WARNING_LEVEL);
        throw exc;
    }
}

// ----------------------------------------------------------------------------

Neuron_Ptr Microcircuit::neuron_ptr(Cell_GID gid)
{
    Neurons::iterator neuron = _neurons->find(gid);
    if (neuron == _neurons->end())
        return Neuron_Ptr();
    else
        return neuron.ptr();
}

// ----------------------------------------------------------------------------

Const_Neuron_Ptr Microcircuit::neuron_ptr(Cell_GID gid) const
{
    Neurons::iterator neuron = _neurons->find(gid);
    if (neuron == _neurons->end())
        return Neuron_Ptr();
    else
        return neuron.ptr();
}

// ----------------------------------------------------------------------------

const Targets & Microcircuit::targets() const
{
    return _targets;
}

// ----------------------------------------------------------------------------

void Microcircuit::print() const
{
    std::cout << *this;
}

// ----------------------------------------------------------------------------

void Microcircuit::open(const Experiment & experiment)
{
    _neurons.reset(new Neurons(_shared_this.lock(), "All"));
    _morphologies.reset(new Morphologies());

    try 
    {
        close();
        _reader = Microcircuit_Reader::create_reader(experiment);
        _reader->open(*this);
    } 
    catch (...) 
    {
        close();
        throw;
    }
}

// ----------------------------------------------------------------------------

void Microcircuit::open(const Microcircuit_Reader_Ptr & reader)
{
    _neurons.reset(new Neurons(_shared_this.lock(), "All"));
    _morphologies.reset(new Morphologies());

    try {
        close();
        _reader = reader;
        _reader->open(*this);
    } catch (...) {
        close();
        throw;
    }
}

// ----------------------------------------------------------------------------

void Microcircuit::close()
{
    if (_reader.get())
    {
        // Removing reader.
        _reader.reset();

        // Clearing containers and datasets.
        _neurons.reset();
        _morphologies.reset();
        _meshes.clear();
        _targets.clear();
        _dataset = Microcircuit_Dataset();
    }
}

// ----------------------------------------------------------------------------

void Microcircuit::load(const Cell_Target & target,
                        Microcircuit_Loading_Flag_Set options)
{
	if (!_reader.get())
	{
		std::string error ("Microcircuit_Reader NULL pointer.");
		throw_exception(runtime_error(error), FATAL_LEVEL);
	}

	try
	{
		_reader->load(target, options);
	}
	catch(std::exception & e)
	{
		std::cerr << "Exception in Microcircuit::load() - " 
			<< typeid(e).name() << " " << e.what() << std::endl;
		std::cerr << "Freeing allocated memory" << std::endl;
		this->unload();
		throw;
	}
	catch(...)
	{
        std::cerr << "Exception in Microcircuit::load()" << std::endl
		         << "Freeing allocated memory" << std::endl;
		this->unload();
		throw;
	}

	if (options & NEURONS)
	{
		// Invalidating the current binding.
		update(Compartment_Report_Frame<Millivolt>());
	}
}

// ----------------------------------------------------------------------------

void Microcircuit::load(Target & target,
                        Microcircuit_Loading_Flag_Set options)
{
    load(target.cell_target(), options);
}

// ----------------------------------------------------------------------------

void Microcircuit::unload()
{
    //! \todo 1) Check what happens with Synapse_Dataset etc. during clear().
    _dataset.structure->synapse.clear();
    _morphologies->clear();
    _neurons->clear();
    _meshes.clear();

    // Invalidating current binding.
    update(Compartment_Report_Frame<Millivolt>());
}

// ----------------------------------------------------------------------------

void Microcircuit::update(const Compartment_Report_Frame<Millivolt> & frame)
{
    _dataset.dynamics->compartment_voltages = frame;
    // Checking if target has changed in order to reset all the indices of
    // the neurons inside the target and set to undefined the old ones.
    bool target_changed = false;
    Cell_Target target;
    if (!frame.has_valid_context())
    {
        target_changed = true;
    }
    else
    {
        target = frame.context().cell_target();
        target_changed = _currently_bound_cell_target != target;
    }

    if (target_changed)
    {
        if (_currently_bound_cell_target.size() != 0)
        {
            // Setting all old neuron indices to undefined.
            for (Cell_Target::const_iterator gid = 
                     _currently_bound_cell_target.begin();
                 gid != _currently_bound_cell_target.end();
                 ++gid)
            {
                Neurons::iterator neuron = _neurons->find(*gid);
                if (neuron != _neurons->end())
                    neuron->_index = UNDEFINED_CELL_INDEX;
            }
        }

        Cell_Target::const_iterator gid = target.begin();
        for (Cell_Index index = 0; gid != target.end(); ++gid, ++index)
        {
            Neurons::iterator neuron = _neurons->find(*gid);
            if (neuron != _neurons->end())
                neuron->_index = index;
        }

        _currently_bound_cell_target = target;
    }
}

// ----------------------------------------------------------------------------

Cell_Target Microcircuit::cell_target(const Label & target_name) const
{
    return _targets.cell_target(target_name);
}

// ----------------------------------------------------------------------------

const Microcircuit_Dataset & Microcircuit::dataset() const
{
    return _dataset;
}

// ----------------------------------------------------------------------------

Microcircuit_Dataset & Microcircuit::dataset()
{
    return _dataset;
}

// ----------------------------------------------------------------------------

std::ostream & operator << (std::ostream & lhs, const Microcircuit & rhs)
{
    lhs << "Microcircuit" << std::endl
        << "___________________________________" << std::endl
        << "Neurons: " << rhs._neurons->size() << std::endl
        << "Synapses: ";
    
    //! \todo Check this calc. (TT)
    Count number_of_synapses = 0;
    for (Neurons::const_iterator i = rhs.neurons().begin(); 
         i != rhs.neurons().end(); ++i)        
    {
        number_of_synapses += i->afferent_synapses().size();
    }
    
    lhs << number_of_synapses << std::endl
        << "Morphologies: " << rhs._morphologies->size() << std::endl;

    return lhs;
}

// ----------------------------------------------------------------------------

// \todo Should be created on the fly to save memory. (TT)
//Synapses & Microcircuit::synapses()
//{
//    return * _synapses;
//}
//
//// ----------------------------------------------------------------------------
//
//const Synapses & Microcircuit::synapses() const
//{
//    return * _synapses;
//}

// ----------------------------------------------------------------------------

}
