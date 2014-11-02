/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible author:		Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_DATASET_MICROCIRCUIT_DYNAMICS_H
#define BBP_DATASET_MICROCIRCUIT_DYNAMICS_H

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Experiment/Compartment_Report_Frame.h"
#include "BBP/Model/Experiment/Mappings/Compartment_Volume_Mapping.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Dynamics_Dataset;
//! smart pointer to shared dynamic microcircuit data
typedef boost::shared_ptr<Dynamics_Dataset> 
		Dynamics_Dataset_Ptr;

// ----------------------------------------------------------------------------

//! Internal dataset storing the loaded microcircuit dynamic data.
/*!
	This dataset is referenced from the microcircuit interface objects and 
	used as central internal memory of loaded dynamic microcircuit information
	that can be shared between several instances of objects referencing
	the same data without replicating it.
	
	DISCLAIMER: This is an implementation detail that may be subject to 
	change, so do not access this dataset directly if you don't want to 
	rewrite your code when the dataset implementation changes.
    \todo Insert Scaling factors for synaptic conductances. These should be
    derived from Connection_Scalings for the active 
    Experiment_Specification and then store the value for each GID. (TT)
*/
class Dynamics_Dataset
{
public:
	//! compartment membrane voltage frame (row = neuron, column = compartment)
	Compartment_Report_Frame<Millivolt>             compartment_voltages;
	//! compartment variable frame (row = neuron, column = compartment)
	Compartment_Report_Frame<Simulation_Value>      compartment_variables;
	//! mapping between neuron compartments and volume elements (voxels)
	Compartment_Volume_Mapping						compartment_volume_mapping;
};

// ----------------------------------------------------------------------------

}

#endif
