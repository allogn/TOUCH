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

#ifndef BBP_COMPARTMENT_REPORT_PRINTER_H
#define BBP_COMPARTMENT_REPORT_PRINTER_H

#include "BBP/Common/Types.h"
#include "BBP/Common/Filter/Filter.h"
#include "BBP/Model/Experiment/Compartment_Report_Frame.h"

namespace bbp {
	
//! prints the contents of the current compartment report frame to the console.
/*!
    \ingroup Filter
*/
template <typename Compartment_Report_Data_Type>
class Compartment_Report_Printer :
	public Filter<Compartment_Report_Frame<Compartment_Report_Data_Type>, 
		          Compartment_Report_Frame<Compartment_Report_Data_Type> >
{
public:
	Compartment_Report_Printer()
	{}

	Compartment_Report_Printer(Label label)
		: label(label)
	{}


	~Compartment_Report_Printer()
	{}

	inline virtual void process()
	{
        std::cout << label << " " << this->input() << std::endl;
		//std::cout << input().context().compartment_mapping->number_of_compartments(5152, 99);
	}

private:
	Label label;
};


}
#endif
