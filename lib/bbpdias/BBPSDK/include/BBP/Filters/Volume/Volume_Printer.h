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

#ifndef BBP_VOLUME_PRINTER_H
#define BBP_VOLUME_PRINTER_H

#include "BBP/Common/Filter/Filter.h"
#include "BBP/Common/Math/Geometry/Volume.h"

namespace bbp {
	
//! prints the contents of the current compartment report frame to the console.
/*!
    \ingroup Filter
*/
template <typename Voxel_Data_Type, 
          typename Space_Unit,
          typename Time_Unit>
class Volume_Printer :
	public Filter <Volume <Voxel_Data_Type, Space_Unit, Time_Unit>, 
	               Volume <Voxel_Data_Type, Space_Unit, Time_Unit> >
{
public:
	Volume_Printer()
	{
		label = "Volume";
		this->output = this->input;
	}

	Volume_Printer(Label label)
		: label(label)
	{}

	~Volume_Printer()
	{}

	inline virtual void process()
	{
		std::cout << label << " " << * this->input;
		//std::cout << input->context().compartment_mapping->number_of_compartments(5152, 99);
	}

private:
	Label label;
};

}
#endif
