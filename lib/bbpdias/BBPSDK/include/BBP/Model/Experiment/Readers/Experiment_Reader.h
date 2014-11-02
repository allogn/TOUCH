/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2006-2007. All rights reserved.

                Authors: Thomas Traenkler, 
						 Sebastien Lasserre

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_EXPERIMENT_READER_H
#define BBP_EXPERIMENT_READER_H

#include <boost/shared_ptr.hpp>

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Experiment/Experiment.h"
#include "BBP/Common/String/String.h"

namespace bbp {

class Experiment;

/*!
        Reader for microcircuit experiment data. This class will read microcircuit
        experiment data currently from files with the appropriate file readers. In
        the future it will be enabled to optionally read data from a network stream.
		\todo Provide an interface to replace a target label in the 
		BlueConfigFile during the loading using BlueConfigReader. 
		(SL - 22/01/2008)
 */
class Experiment_Reader 
    : public Experiment_Accessor
{
public:

    //! Create new experiment reader using a URI source.
    /*!
        Actually this source could only be a BlueConfig file. It the future
        this source could be eventually read from a network protocol.
    */
    Experiment_Reader(const URI & source) 
    : _source(source), 
      _replace(false) 
    {}

    //! Create new experiment reader using a URI source and replace URI prefix.
    /*!
        Actually this source could only be a BlueConfig file. It the future
        this source could be eventually read from a network protocol.
    */
    Experiment_Reader(const URI & source,
        const std::string & original_prefix,
        const std::string & replacement_prefix)
    : _source(source),
      _replace(true),
      _original_prefix(original_prefix),
      _replacement_prefix(replacement_prefix)
    {}

    // Destroy experiment reader object.
    virtual ~Experiment_Reader() {}
    
public:
    //! Reads experiment data into memory structures. 
    virtual void read(Experiment & experiment);

    //! Loads the user targets for the specified Experiement
    void load_user_targets(Experiment &experiment);

private:

    //! the experiment object that will be filled with data by this reader
    URI             _source;
    //! If the URI source prefix is to be replaced or not.
    bool            _replace;
    //! The original source URI prefix that is to be replaced.
    std::string     _original_prefix;
    //! The replacement source URI prefix that is to be placed.
    std::string     _replacement_prefix;
};

}
#endif
