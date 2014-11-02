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

#ifndef BBP_COMPARTMENT_VOLUME_MAPPING_H
#define BBP_COMPARTMENT_VOLUME_MAPPING_H

#include <map>
#include <set>
#include "BBP/Common/Math/Geometry/Volume.h"
#include "BBP/Model/Microcircuit/Types.h"

namespace bbp {

// ----------------------------------------------------------------------------

struct  Voxel_Compartment_Occupancy;
typedef std::map<Cell_GID, std::set<Voxel_Compartment_Occupancy> >	
        Compartments_in_Voxel;

// ----------------------------------------------------------------------------

//! Mapping between voxels (cube shaped volume elements) and compartments.
/*!
    \todo Review Compartments_in_Voxel use of std::map for efficiency. (TT)
*/
class Compartment_Volume_Mapping 
	: public Volume<Compartments_in_Voxel, Micron, Millisecond>
{
public:
	bool is_loaded;

    Compartment_Volume_Mapping() 
        : is_loaded(false) {}
};

// ----------------------------------------------------------------------------

//! Specifies the percentage of the voxel volume occupied by this compartment.
struct Voxel_Compartment_Occupancy
{
	//! Internal array index idenfifying the compartment.
	Report_Frame_Index  compartment;
	//! Percentage of voxel volume occupied by this compartment.
	Percentage			percentage;
};

// ----------------------------------------------------------------------------

}
#endif
