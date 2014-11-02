/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2008. All rights reserved.

		Responsible authors:    Thomas Traenkler
        Contributing authors:   Juan Hernando Vieites

*/

#include <boost/filesystem.hpp>

#include "File/Synapse_HDF5_File_Reader.h"
#include "BBP/Model/Microcircuit/Readers/Synapse_Reader.h"
#include "BBP/Model/Microcircuit/Synapse.h"


namespace bbp
{


// ----------------------------------------------------------------------------

Synapse_Reader_Ptr 
Synapse_Reader::create_reader()
{
    /*!
        \todo The check for the appropriate reader should be done during the 
        open function as this might change between read sessions e.g. switching
        from network to file or change network address. (TT)
        Why would anyone need to switch the data source on the fly?
        For simplicity, we shouldn't allow this and consider that experiments
        are incompatible in that case. A required condition for Experiments
        to be compatible must be that URIs for each data source are the 
        same (JH).
        This is not about different data sources for the same experiment,
        but about maintaining the Experiment instance between reads of
        different experiments. Another solution could of course be to
        create a new Experiment object, but then the object needs to be
        created as a pointer if the rest of the code is depending on that
        object (consider the context of BlueHub). (TT)
    */
    return Synapse_Reader_Ptr(new Synapse_HDF5_File_Reader());
}

// ----------------------------------------------------------------------------

}
