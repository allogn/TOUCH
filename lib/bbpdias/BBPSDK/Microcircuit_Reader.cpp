/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2008. All rights reserved.

                Authors: Juan Hernando Vieites
*/

#include "BBP/Model/Experiment/Experiment.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"
#include "BBP/Model/Microcircuit/Readers/Microcircuit_Reader.h"
#include "BBP/Model/Microcircuit/Readers/Target_Reader.h"
#include "BBP/Model/Microcircuit/Containers/Morphologies.h"

namespace bbp
{

// ----------------------------------------------------------------------------

Microcircuit_Reader_Ptr 
Microcircuit_Reader::create_reader(const Experiment & experiment)
{
    Microcircuit_Reader_Ptr reader(new Microcircuit_Reader());
    reader->base_construction(experiment.circuit_source(),
                              experiment.synapse_source(),
                              experiment.synapse_source(),
                              experiment.morphologies_source(),
                              experiment.meshes_source());
    
    return reader;
}

// ----------------------------------------------------------------------------

Microcircuit_Reader_Ptr
Microcircuit_Reader::create_reader(const URI & circuit_source,
                                   const URI & synapse_source,
                                   const URI & target_source,
                                   const URI & morphologies_source,
                                   const URI & meshes_source)
{
    Microcircuit_Reader_Ptr reader(new Microcircuit_Reader());
    reader->base_construction(circuit_source, synapse_source,
                              target_source, morphologies_source,
                              meshes_source);

    return reader;
}

// ----------------------------------------------------------------------------

void Microcircuit_Reader::base_construction
(const URI & circuit_source,
 const URI & synapse_source,
 const URI & target_source,
 const URI & morphology_source,
 const URI & mesh_source)
{
    _target_source = target_source;

    if (circuit_source != "")
    {
        // Creating the circuit reader
        _circuit_reader = 
            Microcircuit_Composition_Reader::create_reader(circuit_source);
        if (_circuit_reader.get() == 0)
        {
            throw_exception(
                Bad_Data_Source("Couldn't find a suitable reader for "
                                "circuit source: " + circuit_source),
                FATAL_LEVEL, __FILE__, __LINE__);
        }
    }

    if (morphology_source != "")
    {
        // Creating the morphology reader
        _morphology_reader = 
            Morphology_Reader::create_reader(morphology_source);
        if (_morphology_reader.get() == 0)
        {
            throw_exception(
                Bad_Data_Source("Couldn't find a suitable reader for"
                                " morphology source:" + morphology_source),
                FATAL_LEVEL, __FILE__, __LINE__);
        }
    }


    if (synapse_source != "")
    {
        // Creating the synapse reader
        _synapse_source = synapse_source;
        _synapse_reader = Synapse_Reader::create_reader();
    }
    
    if (mesh_source != "")
    {
        // Creating the mesh reader
        _mesh_reader = Mesh_Reader::create_reader(mesh_source);
        if (_mesh_reader.get() == 0)
        {
            throw_exception(
                Bad_Data_Source("Couldn't find a suitable reader for "
                                "mesh source: " + mesh_source),
                FATAL_LEVEL, __FILE__, __LINE__);
        }
    }
}

// ----------------------------------------------------------------------------

void Microcircuit_Reader::open(Microcircuit & microcircuit)
{
    _microcircuit = &microcircuit;

    if (_circuit_reader.get())
        _circuit_reader->open();

    if (_morphology_reader.get())
        _morphology_reader->open();

    if (_mesh_reader.get())
        _mesh_reader->open();

    if (_synapse_reader.get())
        _synapse_reader->open(_synapse_source, *_microcircuit);

    // Reading circuit targets;
    if (_target_source != "")
    {
        log_message("Loading targets from '" + _target_source +
                    "'", INFORMATIVE_LEVEL);
        Target_Reader_Ptr reader = 
            Target_Reader::create_reader(_target_source);
        if (reader.get() == 0)
        {
            throw_exception(
                Bad_Data_Source("Couldn't find a suitable reader for "
                                "circuit targets source: " + _target_source),
                FATAL_LEVEL, __FILE__, __LINE__);
        }

        reader->load(_microcircuit->_targets);
    }
    else
    {
        log_message("Undefined microcircuit target source, no targets loaded.",
                    WARNING_LEVEL);
    }
}

// ----------------------------------------------------------------------------

void Microcircuit_Reader::close()
{
    if (_circuit_reader.get())
        _circuit_reader->close();

    if (_morphology_reader.get())
        _morphology_reader->close();

    if (_mesh_reader.get())
        _mesh_reader->close();

    if (_synapse_reader.get())
        _synapse_reader->close();
}

// ----------------------------------------------------------------------------

void Microcircuit_Reader::load(const Cell_Target & target, int options)
{
    //! \todo Enforce loading once until synapses support partial loading. (TT)
    Neurons & neurons           = _microcircuit->neurons();
    Morphologies & morphologies = _microcircuit->morphologies();
    Meshes & meshes             = _microcircuit->meshes();

    if (options & NEURONS)
    {
        if (_circuit_reader.get() == 0)
        {
            throw_exception(
                Undefined_Data_Source("Circuit data source is undefined"),
                FATAL_LEVEL, __FILE__, __LINE__);
        }

        log_message("Loading neurons...", INFORMATIVE_LEVEL);

        // Loading neurons
        
        _circuit_reader->load(_microcircuit->neurons(), 
                              target - _microcircuit->neurons(),
                              _microcircuit->dataset().structure);
        // If we are not going to load morphologies we must check it
        // already loaded morphologies can be attached to the loaded neurons
        if ((options & MORPHOLOGIES) == 0 && (morphologies.size() != 0))
        {
            link_neurons_with_morphologies(neurons, morphologies, target);
        }

        log_message("Loading neurons done", INFORMATIVE_LEVEL);
    }

    if (options & MORPHOLOGIES)
    {
        if (_morphology_reader.get() == 0)
        {
            throw_exception(
                Undefined_Data_Source("Morphology data source is undefined"),
                FATAL_LEVEL, __FILE__, __LINE__);
        }

        log_message("Loading morphologies...", INFORMATIVE_LEVEL);

        // Loading morphologies
        _morphology_reader->load(morphologies, target,
                                 _circuit_reader->source());


        // Attaching morphologies to neurons
        link_neurons_with_morphologies(neurons, morphologies, target);

        // If we are not going to load meshes we must check it
        // already loaded meshes can be attached to the loaded morphologies
        if ((options & MESHES) != 0 && (meshes.size() != 0))
        {
            link_morphologies_with_meshes(morphologies, meshes);
        }

        log_message("Loading morphologies done", INFORMATIVE_LEVEL);
    }

    if ((options & MESH_VERTICES) == MESH_VERTICES || 
        (options & MESH_TRIANGLES) == MESH_TRIANGLES || 
        (options & MESH_TRIANGLE_STRIPS) == MESH_TRIANGLE_STRIPS || 
        (options & MESH_MAPPING) == MESH_MAPPING)
    {
        if (_mesh_reader.get() == 0)
        {
            throw_exception(
                Undefined_Data_Source("Mesh data source is undefined"),
                FATAL_LEVEL, __FILE__, __LINE__);
        }

        log_message("Loading meshes...", INFORMATIVE_LEVEL);

        // Loading meshes
        _mesh_reader->load(
            _microcircuit->meshes(), target, 
            _circuit_reader->source(),
            (options & MESH_VERTICES) == MESH_VERTICES,
            (options & MESH_TRIANGLES) == MESH_TRIANGLES,
            (options & MESH_MAPPING) == MESH_MAPPING,
            (options & MESH_TRIANGLE_STRIPS) == MESH_TRIANGLE_STRIPS);
        
        // Attaching loaded meshes to their morphologies
        link_morphologies_with_meshes(morphologies, meshes);

        log_message("Loading meshes done", INFORMATIVE_LEVEL);
    }

    if (options & SYNAPSES)
    {
        if (_synapse_reader.get() == 0)
        {
            throw_exception(
                Undefined_Data_Source("Synapse data source is undefined"),
                FATAL_LEVEL, __FILE__, __LINE__);
        }

        Synapse_Attributes_Flags temp = SYNAPSE_NO_ATTRIBUTES;

        log_message("Loading synapses...", INFORMATIVE_LEVEL);

        // Loading synapses
        if (options & SYNAPSES_DYNAMICS_CONSTANTS)
        {
            temp |= SYNAPSE_DYNAMICS_CONSTANTS;
        }

        if (options & SYNAPSES_TYPE)
        {
            temp |= SYNAPSE_TYPE;
        }

        if (options & SYNAPSES_POSITION)
        {
            temp |= SYNAPSE_POSITION;
        }

        if (options & SYNAPSES_CONNECTIVITY)
        {
            temp |= SYNAPSE_CONNECTION;
        }

        _synapse_reader->load(target, temp, 
                              options & AFFERENT_SYNAPSES ? 
                              ONLY_AFFERENT_SYNAPSES : ONLY_SHARED_SYNAPSES);
        
        log_message("Loading synapses done", INFORMATIVE_LEVEL);
    }
}

// ----------------------------------------------------------------------------

void Microcircuit_Reader::link_neurons_with_morphologies
(Neurons & neurons,  Morphologies & morphologies, const Cell_Target & target)
{
    for (Cell_Target::iterator id = target.begin();
         id != target.end();
         ++id)
    {
        Neurons::iterator neuron = neurons.find(*id);
        // Checking if neuron has been loaded but it hasn't its morphology
        // linked
        if (neuron != neurons.end() &&
            neuron->_morphology.get() == 0)
        {
            // Searching the morphology
            Morphologies::iterator morphology = 
                morphologies.find(neuron->_morphology_name);
            if (morphology != morphologies.end())
            {
                // linking
                const Neuron_Ptr n(boost::const_pointer_cast<
                    Neuron, const Neuron>(neuron.ptr()));
                const Morphology_Ptr m(boost::const_pointer_cast<
                    Morphology, const Morphology>(morphology.ptr()));
                n->_morphology = m;
            }
        }
    }
}

// ----------------------------------------------------------------------------

void Microcircuit_Reader::link_morphologies_with_meshes
(Morphologies & morphologies, Meshes & meshes)
{
    for (Morphologies::iterator morphology = morphologies.begin(); 
                morphology != morphologies.end(); ++morphology)
    {
        // Checking if morphology hasn't its mesh linked
        if (morphology->_mesh.get() == 0)
        {
            // Searching the morphology
            Meshes::iterator mesh = 
                meshes.find(morphology->label());
            if (mesh != meshes.end())
            {
                // linking
                const Morphology_Ptr m(boost::const_pointer_cast<
                    Morphology, const Morphology>(morphology.ptr()));
                const Mesh_Ptr m2(boost::const_pointer_cast<
                    Mesh, const Mesh>(mesh.ptr()));
                m->_mesh = m2;
            }
        }
    }
}

// ----------------------------------------------------------------------------

}
