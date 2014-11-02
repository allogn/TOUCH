/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		Thomas Traenkler
		(c) 2006-2007. All rights reserved.

		Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_VOLUME_VOLTAGE_TO_COLOUR_H
#define BBP_VOLUME_VOLTAGE_TO_COLOUR_H

#include "BBP/Common/Graphics/Color_RGBA.h"
#include "BBP/Common/Graphics/Color_Mapping.h"
#include "BBP/Filters/Volume/Volume_Filter.h"

namespace bbp {

// ----------------------------------------------------------------------------

/*!
    \ingroup Filter
*/
class Volume_Voltage_to_Colour :
	public Volume_Filter<Millivolt, Color_RGBA>
{

public:

	inline Volume_Voltage_to_Colour();
	inline ~Volume_Voltage_to_Colour();

	inline void process();

private:
	Color_Mapping<Millivolt> colour_mapping;
};


// ----------------------------------------------------------------------------

Volume_Voltage_to_Colour::Volume_Voltage_to_Colour(void)
: colour_mapping(-65.0, 0.0)
{
}

// ----------------------------------------------------------------------------

Volume_Voltage_to_Colour::~Volume_Voltage_to_Colour(void)
{
}

// ----------------------------------------------------------------------------

void Volume_Voltage_to_Colour::process()
{
	Tensor_Resolution volume_resolution;
	volume_resolution = this->output().resolution();
	
	Volume<Millivolt, Micron, Millisecond> & in = this->input();
	Volume<Color_RGBA, Micron, Millisecond> & out = this->output();

	for (Count x = 0; x < volume_resolution.x(); ++x)
	for (Count y = 0; y < volume_resolution.y(); ++y)
	for (Count z = 0; z < volume_resolution.z(); ++z)
	{
		out(x,y,z) = colour_mapping(in(x,y,z));
	}
}

// ----------------------------------------------------------------------------

}
#endif
