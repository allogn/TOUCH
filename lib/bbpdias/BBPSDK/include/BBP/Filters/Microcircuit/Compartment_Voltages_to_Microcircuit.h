/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

		Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENT_VOLTAGES_TO_MICROCIRCUIT_H
#define BBP_COMPARTMENT_VOLTAGES_TO_MICROCIRCUIT_H

#include "BBP/Model/Microcircuit/Microcircuit.h"
#include "BBP/Model/Microcircuit/Datasets/Accessors/Neuron_Dataset_Accessor.h"
#include "BBP/Model/Experiment/Compartment_Report_Frame.h"
#include "BBP/Common/Filter/Filter.h"

namespace bbp {
	
//! Links the compartment voltages with the microcircuit object.
/*!
    \ingroup Filter
*/
class Compartment_Voltages_to_Microcircuit :
	public Filter<Compartment_Report_Frame<Millivolt>, Microcircuit>,
    private Neuron_Accessor
{
private:
    bool initialized;
public:
	Compartment_Voltages_to_Microcircuit() 
        : initialized(false)
	{
	}

	~Compartment_Voltages_to_Microcircuit(void)
	{
	}

	//! start process
	inline virtual void start()
	{
        initialized = false;
		stream_state = STREAM_STARTED;
	}

	inline virtual void process()
	{
        if (initialized == false)
        {
            Cell_Index current_index = 0;

		    /*!
			    \todo this is just a quick fix to get up and running, mapping should
			    only be rechecked if sth changes (i.e. during a stopped pipeline)

			    \todo make sure we wait for context to be valid!

			    \todo check if the cell target set ordering always corresponds to 
			    the binary file neuron index ordering (that means, are neuron indices
			    always sorted in the binary file frame or can they be out of order?)
		    */

		    for (Cell_Target::iterator i = input().context().cell_target().begin();
			    i != input().context().cell_target().end(); ++i)
		    {
                index(output().neuron(*i)) = current_index;
			    ++current_index;
		    }
            initialized = true;
        }


        //! \bug this causes problems with Juan if done during configuration
		//assuming the reader provides already an object during start process
		output().update(input());

		// link the compartment voltages with the microcircuit object 
        //! \todo Why is this done twice? (TT)
        output().update(input());
	}
};

}
#endif
