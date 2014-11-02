/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2007. All rights reserved.

        Responsible authors:    Nikolai Chapochnikov
                                Thomas Traenkler

*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SEGMENT_VOXEL_DENSITY_H
#define BBP_SEGMENT_VOXEL_DENSITY_H

#include "BBP/Common/Filter/Filter.h"
#include "BBP/Common/Math/Geometry/Volume.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"
#include "BBP/Model/Microcircuit/Mappings/Segment_Voxel_Mapping.h"


namespace bbp {

// ----------------------------------------------------------------------------

/*!
    \brief Computes the segment volume density inside of a discrete resolution
    volume.
    \ingroup Filter
*/
class Segment_Voxel_Density
    : public Filter <Segment_Voxel_Mapping, Volume <Byte, Micron, Millisecond> >
{
public :
    Segment_Voxel_Density();
    ~Segment_Voxel_Density();

public :
    //! the cells that should be included in the density calculation
    Cell_Target cell_target;

public :
    //! Intialize and start the filter.
    inline void start();
    //! Computes and normalizes the tissue density inside voxels of a volume.
    inline void process();
    
};


// ----------------------------------------------------------------------------

inline Segment_Voxel_Density::Segment_Voxel_Density()
{
}

// ----------------------------------------------------------------------------

inline Segment_Voxel_Density::~Segment_Voxel_Density()
{
#ifdef BBP_DEBUG
    //! \todo remove this debug output.
    std::cout << "Segment_Voxel_Density::~Segment_Voxel_Density()" << std::endl;
#endif
}

// ----------------------------------------------------------------------------

inline void Segment_Voxel_Density::start()
{
    output().resize( input().resolution().x(), 
                    input().resolution().y(), 
                    input().resolution().z());
    stream_state = STREAM_STARTED;
}

// ----------------------------------------------------------------------------

inline void Segment_Voxel_Density::process()
{
    Byte                        normed_value;
    Micron3                     temp_density    = 0;
    Micron3                     maximum_density = 0;
    Count                       resolution_x    = this->output().resolution().x();
    Count                       resolution_y    = this->output().resolution().y();
    Count                       resolution_z    = this->output().resolution().z();
    Segment_Voxel_Mapping  &   segment_mapping = input();

    // find maximum density
    std::clog << "Calculating maximum density ..." << std::endl;
    for (Count x = 0; x < resolution_x; x++)
    for (Count y = 0; y < resolution_y; y++)
    for (Count z = 0; z < resolution_z; z++)
    {
        temp_density = 0;

        for (Segment_Indices_in_Voxel::iterator i 
            = segment_mapping(x,y,z).begin();
            i != segment_mapping(x,y,z).end(); ++i)
        {       
            temp_density += i->volume;
        }

        if (maximum_density < temp_density) 
            maximum_density = temp_density;
    }
    std::clog << "Maximum density inside this volume : " 
              << maximum_density << std::endl;
    
    // calculate and normalize density 
    std::clog << "Normalizing density values to fit "
                 "within byte range [0..250]" << std::endl;
    for (Count x = 0; x < resolution_x; x++)
    for (Count y = 0; y < resolution_y; y++)
    for (Count z = 0; z < resolution_z; z++)
    {
        temp_density = 0;
#ifndef NDEBUG
        Segments_in_Voxel segments_in_voxel =
            segment_mapping.segments_in_voxel(x,y,z);
        for (Segments_in_Voxel::const_iterator i = 
            segments_in_voxel.begin(); i != segments_in_voxel.end();
            ++i)
        {
            i->segment.print();
            std::cout << "volume " << i->volume << std::endl;
        }
#endif

        for (Segment_Indices_in_Voxel::iterator i 
            = segment_mapping(x,y,z).begin();
            i != segment_mapping(x,y,z).end(); ++i)
        {       
            temp_density += i->volume;
        }
#ifndef NDEBUG
        std::cout << "total volume in voxel " << temp_density << std::endl;
#endif

        normed_value = Byte ( round(temp_density * 250 / maximum_density) );
#ifndef NDEBUG
        std::cout << "normalized value in voxel " << (int) normed_value << std::endl;
#endif

        output().operator()(x,y,z) = normed_value;
#ifndef NDEBUG
        std::cout << "read back from voxel" << (int) output().operator()(x,y,z) 
            << std::endl;
#endif

    }
    std::clog << "Density values normalized." << std::endl; 
}

// ----------------------------------------------------------------------------

}

#endif

