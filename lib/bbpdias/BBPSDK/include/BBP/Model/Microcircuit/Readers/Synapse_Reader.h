/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2008. All rights reserved.

		Responsible authors:    Thomas Traenkler
        Contributing authors:   Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SYNAPSE_READER_H
#define BBP_SYNAPSE_READER_H

#include <set>
#include <boost/shared_ptr.hpp>

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"

namespace bbp
{

// ----------------------------------------------------------------------------

class Synapses;
class Synapse;
class Synapse_Reader;
class Microcircuit;
typedef boost::shared_ptr<Synapse_Reader> Synapse_Reader_Ptr;

// ----------------------------------------------------------------------------

//! Loading filter to apply to a cell target when loading their synapses
/*!
  \sa Synapse_Reader, Synapse_Attributes
*/
enum Synapse_Loading_Filter {
    //! Load only shared synapses.
    /*! Only those synapses for which both the presynaptic and postsynaptic 
      neurons have already been loaded will be loaded and linked to the
      neurons. */
    ONLY_SHARED_SYNAPSES, 
    
    //! Load only afferent synapses.
    /*!
        All afferent synapses of the cell target used for loadeding will be 
        loaded and linked to their respective postsynaptic neurons. */
    ONLY_AFFERENT_SYNAPSES
};

/*!
   This class is the abstract interface for synapse readers which read
   synapse information and fill Synapses container instances.
*/
class Synapse_Reader
{
public:
   
    virtual ~Synapse_Reader() {}
   
    /*!
       Creates a specific reader for the given data source
     */
    static Synapse_Reader_Ptr create_reader();

    //! Returns the data source this reader is bound to.
    virtual URI source() const = 0;
    
    // Opens the data source
    /** The caller must not deallocate the microcircuit object while this
        object is in use. */
    virtual void open(const URI & source, Microcircuit & microcircuit) = 0 
        /* throw something ? */;

    // Opens the data source
    virtual void close() = 0 /* throw something ? */;

    //! Load selected attributes of shared synapses between specified cells.
    /*!
        Loads afferent and efferent synapse attributes for the neurons
        in the specified cell target 
        @param cells The cell target whose synapses must be loaded.
        @param attributes The attributes that must be loaded for those 
               synapses.
        @param filtering The type of filter to apply to the synapse set to
               load. Currently only shared and only afferent are supported.
     */
    virtual void load(const Cell_Target & cells, 
                      Synapse_Attributes_Flags attributes,
                      enum Synapse_Loading_Filter filtering) 
                      /* throw something ? */ = 0;
};

// ----------------------------------------------------------------------------

}
#endif
