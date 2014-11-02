/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

		Author: Thomas Traenkler
                Juan Hernando Vieites
*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MORPHOLOGY_HDF5_FILE_READER_H
#define BBP_MORPHOLOGY_HDF5_FILE_READER_H

#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Microcircuit/Readers/Morphology_Reader.h"

namespace bbp {

class Morphology;
typedef boost::shared_ptr<Morphology> Morphology_Ptr;

// ----------------------------------------------------------------------------

/*!
	File loader for morphologies . Reads HDF5 morphology files and 
	fills the morphology objects with data.
*/
class Morphology_HDF5_File_Reader : public Morphology_Reader
{

public:
	Morphology_HDF5_File_Reader(const Filepath & path = ".");

    //! Returns the data source this reader is bound to.
    virtual URI source() const;

    // Test the morphology path for existence
    virtual void open();

    virtual void close() {}

    // @see Morphology_Reader::load
    virtual void load(Morphologies & morphologies,
                      const std::set<Label> & labels,
                      H5File_Mode mode = HDF5_V1);
    /*
      @see Morphology_Reader::load
      In this case circuit_source must be the path where the circuit.mvd2 file
      can be found.
    */
    virtual void load(Morphologies & morphologies,
                      const Cell_Target & cells, 
                      const URI & circuit_source);

	//! load specified morphology from the hdf5 file
	Morphology_Ptr read(const Label & label, H5File_Mode mode = HDF5_V1) /* throw IO_Error */;

protected:
    Filepath _morphology_path;
};

// ----------------------------------------------------------------------------

inline Morphology_HDF5_File_Reader::Morphology_HDF5_File_Reader
    (const Filepath & morphology_path) :
    _morphology_path(morphology_path)
{
}

// ----------------------------------------------------------------------------

//! Returns the data source this reader is bound to.
inline URI Morphology_HDF5_File_Reader::source() const
{
    return _morphology_path.string();
}

// ----------------------------------------------------------------------------

}
#endif
