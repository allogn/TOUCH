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

#ifndef BBP_STRUCTURE_DATASET_H
#define BBP_STRUCTURE_DATASET_H

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Datasets/Synapse_Dataset.h"
#include "BBP/Model/Microcircuit/Mappings/Segment_Voxel_Mapping.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Neuron;
class Structure_Dataset;
//! smart pointer to shared microcircuit data
typedef boost::shared_ptr<Structure_Dataset> 
		Structure_Dataset_Ptr;

// ----------------------------------------------------------------------------

//! Internal dataset storing the loaded static microcircuit data.
/*!
	This dataset is referenced from the microcircuit interface objects and 
	used as central internal memory of loaded static microcircuit information
	that can be shared between several instances of objects referencing
	the same data without replicating it.
	
	DISCLAIMER: This is an implementation detail that may be subject to 
	change, so do not access this dataset directly if you don't want to 
	rewrite your code when the dataset implementation changes.
*/
class Structure_Dataset 

{
public:
	//! Dataset containing all loaded synapse information for a microcircuit.
	Synapse_Dataset				synapse;
	//! Mapping between neuron segments and volume elements (voxels).
	Segment_Voxel_Mapping		segment_volume_mapping;
	//! Mapping from morphology type id to name.
	std::vector<std::string> Morphology_Types_Array;
	//! Mapping from electrophysiology type id to name.
	std::vector<std::string> Electrophysiology_Types_Array;
	//! Mapping from excitatory and inhibitory type id to name.
	std::vector<std::string> Excitatory_Array;
	//! Mapping from pyramidal and interneuron type id to name.
	std::vector<std::string> Pyramidal_Array;
};

// ----------------------------------------------------------------------------

}

#endif
