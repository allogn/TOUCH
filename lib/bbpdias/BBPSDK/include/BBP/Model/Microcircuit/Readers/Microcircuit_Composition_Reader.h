/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

		Authors: Juan Hernando Vieites
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MICROCIRCUIT_COMPOSITION_READER_H
#define BBP_MICROCIRCUIT_COMPOSITION_READER_H

#include <boost/shared_ptr.hpp>

#include "BBP/Common/Types.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"
#include "BBP/Model/Microcircuit/Datasets/Accessors/Neuron_Dataset_Accessor.h"

namespace bbp
{

// ----------------------------------------------------------------------------

class Neurons;
class Cell_Target;
class Microcircuit_Composition_Reader;
typedef boost::shared_ptr<Microcircuit_Composition_Reader>
    Microcircuit_Composition_Reader_Ptr;

// ----------------------------------------------------------------------------

//! Abstract interface for reading microcircuit composition.
/*!
   This class is the abstract interface for circuit readers which read 
   neuron basic information and fill Neurons container class.
*/
 class Microcircuit_Composition_Reader 
     : public Neuron_Accessor
{

public:
   
   virtual ~Microcircuit_Composition_Reader() {};
   
    static Microcircuit_Composition_Reader_Ptr 
        create_reader(const URI & circuit_source);

protected:
    //! Protected constructor to prevent direct construction.
	Microcircuit_Composition_Reader()
	{}

public:
    //! Returns the data source this reader is bound to.
    /*!
        The source returned here is not guaranteed to be the same as the one
        used in construction. In any case it will be a narrowing on the 
        original. For example, for a mvd reader, if a path is provided, the 
        existence of a circuit.mvd2 file will be tested and the source will 
        be narrowed to path/circuit.mvd2.
     */
    virtual URI source() const = 0;

    //! Opens the data source
    virtual void open() = 0 /* throw something ? */;

    //! Opens the data source
    virtual void close() = 0 /* throw something ? */;

    //! Loads the circuit composition 
    /*!
        E.g. cells types, morphology names, positions, ...
    */
    virtual void load(Neurons & neurons, 
                      const Cell_Target & target, 
                      const Structure_Dataset_Ptr & structure) = 0;
};

// ----------------------------------------------------------------------------

}
#endif
