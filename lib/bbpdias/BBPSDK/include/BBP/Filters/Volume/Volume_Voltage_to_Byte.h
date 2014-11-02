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

#ifndef BBP_VOLUME_VOLTAGE_TO_BYTE_H
#define BBP_VOLUME_VOLTAGE_TO_BYTE_H

#include "BBP/Filters/Volume/Volume_Filter.h"


namespace bbp {

// ----------------------------------------------------------------------------

/*!
    \brief Filter converting a volume of voltages to a volume of byte values.
    \ingroup Filter
*/
class Volume_Voltage_to_Byte 
    : public Volume_Filter<Millivolt, Byte>
{
public:

	/*!
        \brief Constructs the filter and sets the dynamic range to clamp 
        between -65 to -20 mV.
    */
	inline Volume_Voltage_to_Byte();
	/*! 
        \brief Constructs the filter and sets the dynamic range during 
        initalization to specified values.
    */
	inline Volume_Voltage_to_Byte(	Millivolt minimum, 
									Millivolt maximum, 
									bool clamp = true,
									Byte out_of_bounds_value = 0);

	/*!
        \brief Set the dynamic range of the voltages mapped into the byte.

        \param clamp If set to true values outside the dynamic range are
        clamped to the nearest limit (i.e. 0 or 255).
        \param out_of_bounds_value If clamp is set to false, this parameter
        sets the value for data outside the dynamic range.
    */
	inline void dynamic_range(	Millivolt minimum, Millivolt maximum, 
								bool clamp = true, 
								Byte out_of_bounds_value = 0);

	//! Converts the active input voltage volume to a byte volume.
	inline void process();

private:

	Millivolt	minimum, 
				maximum, 
				dynamic_range_size,
				out_of_bounds_value;
	bool		clamp;

	//! Convert the voltage to byte for one volume element.
	inline Byte process_voxel(Millivolt & input);
};


// ----------------------------------------------------------------------------

void Volume_Voltage_to_Byte::process()
{
    if (initialized == false)
    {
        if (this->output().resolution() != this->input().resolution())
        {
            // Adjust dimensions of output volume to match input volume.
            this->output().resize(
                    this->input().resolution().x(), 
                    this->input().resolution().y(), 
                    this->input().resolution().z());
        }
        initialized = true;
    }

	Tensor_Resolution volume_resolution;
	volume_resolution = output().resolution();
	
	Volume<Millivolt, Micron, Millisecond> & in = input();
	Volume<Byte, Micron, Millisecond> & out = output();

	for (Count x = 0; x < volume_resolution.x(); ++x)
	for (Count y = 0; y < volume_resolution.y(); ++y)
	for (Count z = 0; z < volume_resolution.z(); ++z)
	{
		out(x,y,z) = process_voxel(in(x,y,z));
	}
}

// ----------------------------------------------------------------------------

Byte Volume_Voltage_to_Byte::process_voxel(Millivolt & input)
{
	if (input <= minimum) // below the dynamic range
	{
		if (clamp == true)
		{
			return (Byte) 0;
		}
		else
		{
			return (Byte) out_of_bounds_value;
		}
	}
	else if (input >= maximum) // above the dynamic range
	{
		if (clamp == true)
		{
			return (Byte) 255;
		}
		else
		{
			return (Byte) out_of_bounds_value;
		}
	}
	else // if we are within the dynamic range
	{
		return (Byte) ((input - minimum) / dynamic_range_size * 255.0f);
	}
}

// ----------------------------------------------------------------------------

Volume_Voltage_to_Byte::Volume_Voltage_to_Byte(void)
: minimum(-65.0), 
  maximum(-20.0), 
  out_of_bounds_value(0),
  clamp(true)
{
	dynamic_range_size = maximum - minimum;
#ifndef NDEBUG
	std::cout << "Dynamic range: " << dynamic_range_size << " mV" << std::endl;
#endif
}

// ----------------------------------------------------------------------------

Volume_Voltage_to_Byte::Volume_Voltage_to_Byte
(Millivolt minimum, Millivolt maximum, bool clamp, Byte out_of_bounds_value) 
: minimum(minimum), 
  maximum(maximum), 
  clamp(clamp)
{
	dynamic_range_size = maximum - minimum;
	out_of_bounds_value = out_of_bounds_value;
}

// ----------------------------------------------------------------------------

void Volume_Voltage_to_Byte::dynamic_range(
	Millivolt minimum, 
    Millivolt maximum, 
    bool clamp, 
    Byte out_of_bounds_value)
{
	this->clamp = clamp;
	this->minimum = minimum;
	this->maximum = maximum;
	dynamic_range_size = maximum - minimum;
	this->out_of_bounds_value = out_of_bounds_value;
}

// ----------------------------------------------------------------------------

}
#endif
