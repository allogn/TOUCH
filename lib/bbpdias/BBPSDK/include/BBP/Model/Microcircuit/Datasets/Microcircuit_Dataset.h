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

#ifndef BBP_MICROCIRCUIT_DATASET_H
#define BBP_MICROCIRCUIT_DATASET_H

#include "Structure_Dataset.h"
#include "Dynamics_Dataset.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Internal dataset storing the loaded microcircuit dataset.
/*!
	This dataset is referenced from the microcircuit interface objects and 
	used as central internal memory of loaded microcircuit information
	that can be shared between several instances of objects referencing
	the same data without replicating it.
	
	DISCLAIMER: This is an implementation detail that may be subject to 
	change, so do not access this dataset directly if you don't want to 
	rewrite your code when the dataset implementation changes.
*/
class Microcircuit_Dataset
{
public:
    Microcircuit_Dataset()
        : structure (new Structure_Dataset()),
          dynamics  (new Dynamics_Dataset())
    {}
    Microcircuit_Dataset(const Structure_Dataset_Ptr  structure,
                         const Dynamics_Dataset_Ptr   dynamics)
        : structure (structure),
          dynamics  (dynamics)
    {}

    Structure_Dataset_Ptr structure;
    Dynamics_Dataset_Ptr  dynamics;
};

// ----------------------------------------------------------------------------

}

#endif
