/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible author:     Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SYNAPSE_HDF5_FILE_READER_H
#define BBP_SYNAPSE_HDF5_FILE_READER_H

#define H5Dopen_vers 1
#include <hdf5.h>
#undef H5Dopen_vers

#include "BBP/Common/Dataset/HDF5/H5ID.h"

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"
#include "BBP/Model/Microcircuit/Datasets/Synapse_Dataset.h"
#include "BBP/Model/Microcircuit/Datasets/Accessors/Neuron_Dataset_Accessor.h"
#include "BBP/Model/Microcircuit/Readers/Synapse_Reader.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Loads synapse data from a HDF5 file into memory.
/*! 
	Reader for HDF5 file with all the synapse information for a circuit.
    The file for a 10.000 neuron circuit is approx. 1.3 GB (!). 
	\todo Check if HDF5 data could be read without throwing HDF5 error.
	\todo Check that GID is within range (1..n) (TT)
    \bug When the HDF5 file does not contain a synapse dataset for a neuron
    e.g. in the case when there are no synapses in this dataset, in Matlab
    an HDF5 error is shown and also the closing of the H5 handle fails.
    Fix this. (TT)
*/
class Synapse_HDF5_File_Reader
    : public Synapse_Reader,
      private Neuron_Accessor,
      private Synapse_Accessor
{

public:
    //! Constructor connecting a microcircuit to the reader and select
    //! which synapse attributes are to be loaded.
    Synapse_HDF5_File_Reader ();
    //! Destructor.
    ~Synapse_HDF5_File_Reader();


   //! Returns the data source this reader is bound to.
    virtual URI source() const;

    //! Opens the HDF5 file with the synapse information.
    /*!
        \param path of the HDF5 file with the synapse dataset
        \param flags loading flags which specify which synapse properties
                     are to be loaded
    */
    void open(const URI & source, Microcircuit & microcircuit);
    //! Closes the HDF5 file with the synapse information.
    void close();
    //! Reads afferent and efferent synapses.
    /*!
        DISCLAIMER: Note this currently clears/invalidates the whole synapse
        dataset, so after this operation alls Synapse and Synapses objects are
        invalidated.!

        \todo Refine implementation when revisiting the synapse 
        data structure of GabrielStudio (TM). (TT)

        \todo Specifying a Connection_Target could be used to specify more 
        complex cases than shared synapses, e.g. incoming or outgoing 
        synapses from a specific target. e.g. only synapses from layer IV 
        to III. (TT)

        Notes about the current implemenation (subject to change):

        Loading currently clears the existing Synapse_Dataset 
        and loads it with new information for the specified Cell_Target.
        If a Cell_GID in the Cell_Target is not in the microcircuit 
        currently, a new neuron will be added to the Microcircuit instance. 

        \bug Creating a Neuron is the behaviour that we really want for this
        function?. Loading synapses for neurons that haven't been loaded and
        trying to access them later could be consider a clear case of misuse
        from the user (JH).
        
        All synapses between the Neuron objects that are part of the
        specified Cell_Target will be stored. The type of filtering applied
        decides after which synapses are linked to the neurons in the
        microcircuit and which not.
    */
    void load(const Cell_Target & cells, Synapse_Attributes_Flags attributes,
              Synapse_Loading_Filter filtering);

private:
    //! \todo use H5ID for better exception safety.
    H5ID                        _file_handle;
    H5ID                        _dataset_hdf5, 
                                _dataspace; 
    hsize_t                     _dims_out[2];
    herr_t                      _status;
    Cell_GID                    _current_neuron_gid;
    std::vector<float>          _data_table;
    Synapse_Index               _current_dataset_size;
    std::string                 _neuron_dataset_name;
    Microcircuit *              _microcircuit;
    Synapse_Attributes_Flags    _attributes;
    Filepath                    _synapse_path;

    //! Reads an afferent synapse dataset for a specific neuron into memory.
    /*!
        This function is called implicitly in read function and does
        not reload the dataset if already cached.
    */
    void read_HDF5_dataset(Cell_GID cell_gid);
    //! Read number of synapses for this neuron.
    /*!
        This function is used in the calculation to resize arrays.
    */
    Synapse_Index read_size_of_HDF5_dataset(Cell_GID cell_gid);
    //! Loads dataset with afferent and efferent synapses.
    void load_synapse_dataset(
        Synapse_Dataset & dataset, const Cell_Target & cells);
    //! Links the synapse dataset content to the microcircuit interface.
    void link_to_synapse_dataset(
        Synapse_Dataset & dataset, Microcircuit & microcircuit,
        const Cell_Target & cells, Synapse_Loading_Filter filtering);
};

// ----------------------------------------------------------------------------

}
#endif /*BBP_SYNAPSE_HDF5_FILE_READER_H*/
