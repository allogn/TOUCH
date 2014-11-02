/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

		Authors: Thomas Traenkler, Nikolai Chapochnikov

*/

#include "BBP/Filters/Microcircuit/Compartment_Voltage_Voxel_Average.h"

namespace bbp {

// ----------------------------------------------------------------------------

Compartment_Voltage_Voxel_Average::~Compartment_Voltage_Voxel_Average()
{
}

// ----------------------------------------------------------------------------

void Compartment_Voltage_Voxel_Average::start()
{
    Filter <Microcircuit, Volume <Micron, Millivolt> >::start();
}

// ----------------------------------------------------------------------------

void Compartment_Voltage_Voxel_Average::open(Filename mapping_filename,
                                             bool portable_format)
{
    std::ifstream file(mapping_filename.string().c_str());
	
	if (file.is_open() == false)
	{
		std::cerr << "Volume compartment mapping file could not be opened (" 
			<< mapping_filename << ")" << std::endl;
		throw std::runtime_error( std::string("Volume compartment mapping file"
			" could not be opened (") 
			+ mapping_filename.string() + ")");
	}

    if (portable_format)
    {
        boost::archive::text_iarchive ia(file);
	    std::cout << "Volume Compartment Mapping: reading from portable text"
            "file (" << mapping_filename.string()  << ")" << std::endl;
        ia >> * this;
	    std::cout << "Volume Compartment Mapping: in memory" << std::endl;
    }
    else
    {
        boost::archive::binary_iarchive ia(file);
	    std::cout << "Volume Compartment Mapping: reading from native binary "
            "file (" << mapping_filename.string()  << ")" << std::endl;
        ia >> * this;
	    std::cout << "Volume Compartment Mapping: in memory" << std::endl;
    }

	mapping_loaded = true;
}

// ----------------------------------------------------------------------------

Label Compartment_Voltage_Voxel_Average::name() const
{
    return "Compartment Voltages Voxel Average";
}

// ----------------------------------------------------------------------------

}
