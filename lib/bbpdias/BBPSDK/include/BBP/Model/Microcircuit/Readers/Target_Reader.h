/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Juan Hernando Vieites
*/
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_TARGET_READER_H
#define BBP_TARGET_READER_H

#include <boost/shared_ptr.hpp>
#include "BBP/Model/Microcircuit/Types.h"

namespace bbp
{

// ----------------------------------------------------------------------------

class Targets;
class Target_Reader;
typedef boost::shared_ptr<Target_Reader> Target_Reader_Ptr;

// ----------------------------------------------------------------------------

//! Generic reader for microcircuit targets.
/*!
	\sa Target, Targets, Cell_Target
*/
class Target_Reader
{
   
public:
   //! Virtual destructor that can be reimplemented in derived classes.
   virtual ~Target_Reader() {}
   
    /*!
      Creates a target reader given a target_source
      Currently supported sources are the base path of the circuit (which
      should contain a ncs/start.target file) a path containing a start.target
      file or a directlry a .target filename.
      If no suitable data source is provided a null pointer is returned.
    */
    static Target_Reader_Ptr create_reader(const URI & target_source);

public:
    /*! 
      Loads targets from the Blue Target files
      If any exception occurs the target parameter is left in unknown state.
    */    
	virtual void load(Targets & targets) = 0;
    
    /*! 
      Loads user target from the Blue Target files, using the system targets as an additional look up source
      If any exception occurs the target parameter is left in unknown state.
    */    
	virtual void load(Targets & targets, const Targets &systemTargets ) = 0;
};

// ----------------------------------------------------------------------------

}
#endif
