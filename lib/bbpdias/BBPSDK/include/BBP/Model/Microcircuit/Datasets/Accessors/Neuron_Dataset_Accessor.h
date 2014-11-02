/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2008. All rights reserved.

		Authors: Juan Hernando Vieites
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_NEURON_DATASET_ACCESSOR_H
#define BBP_NEURON_DATASET_ACCESSOR_H

#include "../../Neuron.h"

namespace bbp
{

//! Class that has access to Neuron private attributes.
class Neuron_Accessor
{
protected:
    static Cell_GID & cell_gid(Neuron & neuron)
    {
        return neuron._gid;
    }

    static Label & morphology_name(Neuron & neuron)
    {
        // We mustn't access the morphology object because it might not
        // have been even loaded. What we want to do is to store the 
        // morphology name for being able to link morphologies later on.
        return neuron._morphology_name;
    }

    static Column_Index & column_number(Neuron & neuron)
    {
        return neuron._column_gid;
    }

    static Minicolumn_GID & minicolumn_number(Neuron & neuron)
    {
        return neuron._minicolumn_gid;
    }

    static Cortical_Layer_Number & layer_number(Neuron & neuron)
    {
        return neuron._layer_number;
    }

    static Morphology_Type & morphology_type(Neuron & neuron)
    {
        return neuron._morphology_type;
    }

    static Electrophysiology_Type & electro_type(Neuron & neuron)
    {
        return neuron._electrophysiology_type;
    }

    static Vector_3D<Micron> & neuron_position(Neuron & neuron)
    {
        return neuron._position;
    }

    static Orientation & neuron_orientation(Neuron & neuron)
    {
        return neuron._orientation;
    }

    static Transform_3D<Micron> & global_transform(Neuron & neuron)
    {
        return neuron._global_transform;
    }

    static Synapses_Ptr & afferent_synapses(Neuron & neuron)
    {
        return neuron._afferent_synapses;
    }

    static Synapses_Ptr & efferent_synapses(Neuron & neuron)
    {
        return neuron._efferent_synapses;
    }

    static Cell_Index & index(Neuron & neuron)
    {
        return neuron._index;
    }

    static Cell_Index index(const Neuron & neuron)
    {
        return neuron._index;
    }
};

}
#endif
