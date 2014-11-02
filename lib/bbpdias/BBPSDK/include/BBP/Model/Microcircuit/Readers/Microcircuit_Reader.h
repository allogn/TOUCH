/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Authors: Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MICROCIRCUIT_READER_H
#define BBP_MICROCIRCUIT_READER_H

#include <boost/shared_ptr.hpp>

#include "BBP/Model/Microcircuit/Types.h"
#include "Microcircuit_Composition_Reader.h"
#include "Morphology_Reader.h"
#include "Mesh_Reader.h"
#include "Synapse_Reader.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Experiment;
class Microcircuit;
class Morphologies;
class Meshes;
class Neurons;
class Synapses;
class Synapse_Dataset;
class Cell_Target;
class Microcircuit_Reader;
class Synapse_Reader;

typedef boost::shared_ptr<Morphology> Morphology_Ptr;
typedef boost::shared_ptr<Neuron> Neuron_Ptr;
typedef boost::shared_ptr<Microcircuit_Reader> Microcircuit_Reader_Ptr;
typedef boost::shared_ptr<Synapse_Reader> Synapse_Reader_Ptr;

// ----------------------------------------------------------------------------

//! Reader for Microcircuit data.
class Microcircuit_Reader
{

public:
   
   virtual ~Microcircuit_Reader() {}; 
   
    static Microcircuit_Reader_Ptr create_reader(const Experiment & experiment);

    static Microcircuit_Reader_Ptr create_reader(const URI & circuit_source,
                                                 const URI & synapse_source,
                                                 const URI & target_source,
                                                 const URI & morphology_source,
                                                 const URI & mesh_source);

protected:
    // Protected constructor to prevent direct construction.
    Microcircuit_Reader()
    {}

    void base_construction(const URI & circuit_source,
                           const URI & synapse_source,
                           const URI & target_source,
                           const URI & morphology_source,
                           const URI & mesh_source);

public:
    //! Opens the Microcircuit data sources and reads target information.
    /** Caller mustn't deallocate the microcircuit object passed will this
        object is in use. */
    virtual void open(Microcircuit & microcircuit)
        /* throw IO_Error() */;

    virtual void close();

    virtual void load(const Cell_Target & target, int options);

protected:
    // Microcircuit access functions for derived clases
    //inline static Cortical_Layer_Number     & layer_number(Microcircuit & microcircuit);
    inline static Morphologies     & morphologies(Microcircuit & microcircuit);
    inline static Meshes           & meshes(Microcircuit & microcircuit);
    inline static Neurons          & neurons(Microcircuit & microcircuit);
    //inline static Synapses         & synapses(Microcircuit & microcircuit);
    //inline static Synapse_Dataset  & raw_synapses(Microcircuit & microcircuit);

    void link_neurons_with_morphologies(Neurons & neurons, 
                                        Morphologies & morphologies,
                                        const Cell_Target & target);
    
    void link_morphologies_with_meshes(Morphologies & morphologies, 
                                       Meshes & meshes);

protected:
    Microcircuit_Composition_Reader_Ptr    _circuit_reader;
    Morphology_Reader_Ptr                  _morphology_reader;
    Mesh_Reader_Ptr                        _mesh_reader;
    Synapse_Reader_Ptr                     _synapse_reader;

    URI                                    _target_source;
    URI                                    _synapse_source;
    Microcircuit *                         _microcircuit;
};


// ----------------------------------------------------------------------------

}
#include "BBP/Model/Microcircuit/Microcircuit.h"

namespace bbp
{
// ----------------------------------------------------------------------------

//Cortical_Layer_Number & Microcircuit_Reader::layer_number(Microcircuit & microcircuit)
//{
//    return microcircuit._number_of_layers;
//}

// ----------------------------------------------------------------------------

Morphologies & Microcircuit_Reader::morphologies(Microcircuit & microcircuit)
{
    return * microcircuit._morphologies;
}

// ----------------------------------------------------------------------------

Neurons & Microcircuit_Reader::neurons(Microcircuit & microcircuit)
{
    return * microcircuit._neurons;
}

//// ----------------------------------------------------------------------------
//
//Synapses & Microcircuit_Reader::synapses(Microcircuit & microcircuit)
//{
//    return * microcircuit._synapses;
//}

// ----------------------------------------------------------------------------

//Synapse_Dataset & Microcircuit_Reader::raw_synapses(Microcircuit & microcircuit)
//{
//    return microcircuit._dataset_synapses;
//}

// ----------------------------------------------------------------------------

}
#endif
