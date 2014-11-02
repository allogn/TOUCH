/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2007. All rights reserved.

        Responsible authors:	Nikolai Chapochnikov, Thomas Traenkler

*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_VOLUME_MAXIMUM_INTENSITY_PROJECTION_H
#define BBP_VOLUME_MAXIMUM_INTENSITY_PROJECTION_H

#include "BBP/Common/Filter/Filter.h"
#include "BBP/Model/Microcircuit/Mappings/Segment_Voxel_Mapping.h"
#include "BBP/Common/Math/Matrix.h"

namespace bbp {

// ----------------------------------------------------------------------------

enum Camera_View
{
	SIDE_VIEW,
	TOP_VIEW
};

// ----------------------------------------------------------------------------

//! projects the maximum voxel value from top, side and front plane into a matrix
/*!
    \ingroup Filter
*/
template <typename Volume_Data_Type,
          typename Space_Unit, 
          typename Time_Unit>
class Volume_Maximum_Intensity_Projection
	: public Filter 
				< Volume <Volume_Data_Type, Space_Unit, Time_Unit>, 
				  Matrix <Volume_Data_Type> >
{

public :
	Camera_View camera_view;

	Volume_Maximum_Intensity_Projection(Volume_Data_Type default_value = -65.0);
	~Volume_Maximum_Intensity_Projection();

	//! computes and normalized the tissue density inside the voxels of a volume
	inline void process();

private:
	
	Volume_Data_Type default_value;

};


// ----------------------------------------------------------------------------

template <typename Volume_Data_Type,
          typename Space_Unit, 
          typename Time_Unit>
Volume_Maximum_Intensity_Projection<Volume_Data_Type, Space_Unit, Time_Unit>
::Volume_Maximum_Intensity_Projection(Volume_Data_Type default_value)
: camera_view(SIDE_VIEW),
  default_value(default_value) 
{
}

// ----------------------------------------------------------------------------

template <typename Volume_Data_Type,
          typename Space_Unit, 
          typename Time_Unit>
Volume_Maximum_Intensity_Projection<Volume_Data_Type, Space_Unit, Time_Unit>
::~Volume_Maximum_Intensity_Projection()
{
}

// ----------------------------------------------------------------------------

template <typename Volume_Data_Type,
          typename Space_Unit, 
          typename Time_Unit>
void
Volume_Maximum_Intensity_Projection<Volume_Data_Type, Space_Unit, Time_Unit>
::process()
{
    Volume <Volume_Data_Type, Space_Unit, Time_Unit> & in  = this->input();
    Matrix <Volume_Data_Type>                    & out = this->output();

    if (this->initialized == false)
    {
	    if (camera_view == SIDE_VIEW)
	    {
		    out.resize(in.resolution().x(), 
                       in.resolution().y());
	    }
	    else // top view
	    {
		    out.resize(in.resolution().x(), 
                       in.resolution().z());
	    }
        this->initialized = true;
    }

	// local variables
    Count		resolution_x = in.resolution().x();  
    Count		resolution_y = in.resolution().y();  
    Count		resolution_z = in.resolution().z();

	Volume <Volume_Data_Type, Space_Unit, Time_Unit>	& volume = in;
	Matrix <Volume_Data_Type>						& matrix = out;

	// clear matrix
	matrix.fill(default_value);

	if (camera_view == SIDE_VIEW)
	{
		// find maximum intensity for side view
		for (Count x = 0; x < resolution_x; x++)
		for (Count y = 0; y < resolution_y; y++)
		for (Count z = 0; z < resolution_z; z++)	
		{

			if (matrix(x,y) < volume(x,y,z))
			{
				matrix (x,y) = volume (x,y,z);
			}
		}
	}
	else
	{
		// find maximum intensity for top view
		for (Count x = 0; x < resolution_x; x++)
		for (Count y = 0; y < resolution_y; y++)
		for (Count z = 0; z < resolution_z; z++)	
		{

			if (matrix(x,z) < volume(x,y,z))
			{
				matrix (x,z) = volume (x,y,z);
			}
		}
	}
}

// ----------------------------------------------------------------------------

} 

#endif
