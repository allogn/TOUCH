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

#ifndef BBP_MORPHOLOGY_READER_H
#define BBP_MORPHOLOGY_READER_H

#include <set>
#include <boost/shared_ptr.hpp>

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Datasets/Accessors/Morphology_Dataset_Accessor.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"

namespace bbp
{

// ----------------------------------------------------------------------------

class Morphologies;
class Morphology;
class Sections;
class Morphology_Reader;
typedef boost::shared_ptr<Morphology_Reader> Morphology_Reader_Ptr;

// ----------------------------------------------------------------------------

/*!
   This class is the abstract interface for morphology readers which read 
   morphology information and fill Morphologies container class.
*/
class Morphology_Reader : protected Morphology_Dataset_Accessor
{
public:
    enum H5File_Mode{
        HDF5_V1,
        HDF5_V2_RAW,
        HDF5_V2_UNRAVELED,
        HDF5_V2_REPAIRED
    };

    virtual ~Morphology_Reader() {};
   
    /*!
       Creates a specific reader for the given data source
     */
    static Morphology_Reader_Ptr create_reader(const URI & morphology_source);

    //! Returns the data source this reader is bound to.
    virtual URI source() const = 0;
    
    // Opens the data source
    virtual void open() = 0 /* throw something ? */;

    // Opens the data source
    virtual void close() = 0 /* throw something ? */;

    /*!
       Load morphologies using theirs names.
       Loads unique morphologies into the morphology container using the
       names contained in the specified set.
       Morphologies already existing in the target container won't be loaded
     */
    virtual void load(Morphologies & morphologies,
                      const std::set<Label> & names,
                      H5File_Mode mode = HDF5_V1) /* throw something ? */ = 0;
    /*!
       Load morphologies using a cell target.
       Loads unique morphologies into the morphology container using the
       morpholgy names from the cell target given. The morphology names
       are retreived from the circuit source specified.
       Morphologies already existing in the target container won't be loaded
     */
    virtual void load(Morphologies & morphologies,
                      const Cell_Target & cells, 
                      const URI & circuit_source) /* throw something ? */ = 0;

protected:
    // Convenience functions to use by derived classes if needed
    void compute_points_per_section_and_offsets(Morphology &morphology);
    void compute_extents_and_lengths(Morphology &morphology);
    void compute_children(Morphology &morphology);
    void compute_branch_orders(Morphology & morphology);
    void traverse_children(Morphology & morphology,
                           Sections & sections, 
                           Section_Branch_Order current_branch_order);
    
};

// ----------------------------------------------------------------------------

}
#endif
