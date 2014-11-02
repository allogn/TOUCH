/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible author:     Thomas Traenkler
        Contributing authors:   Carolyn Langen
                                Jim King
                                Jie Bao 
                                Nikolai Chapochnikov

*/

//#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>

#include "File/Synapse_HDF5_File_Reader.h"
#include "BBP/Model/Microcircuit/Containers/Synapses.h"
#include "BBP/Common/System/File/File.h"

namespace bbp {

//-----------------------------------------------------------------------------

const static class Display_Progress
{
public:
    Display_Progress()
    {
        std::string _;
#ifndef WIN32
        _display = bbp::getenv("BBP_SHOW_PROGRESS", _);
#else
        _display = true;
#endif
    }
    operator bool() const
    {
        return _display;
    }

    bool _display;
} s_display_progress;

// ----------------------------------------------------------------------------

 Synapse_HDF5_File_Reader::Synapse_HDF5_File_Reader()
  : _current_neuron_gid(UNDEFINED_CELL_GID),
    _current_dataset_size(0),
    _attributes(SYNAPSE_ALL_ATTRIBUTES)
{
}

// ----------------------------------------------------------------------------

Synapse_HDF5_File_Reader::~Synapse_HDF5_File_Reader()
{
    close();
}

// ----------------------------------------------------------------------------

inline URI Synapse_HDF5_File_Reader::source() const
{
    return _synapse_path.string();
}

// ----------------------------------------------------------------------------

void Synapse_HDF5_File_Reader::open(const URI &         source,
                                    Microcircuit &      microcircuit)
{
    Filepath path = uri_to_filename(source);
    _synapse_path = path;
    _microcircuit = & microcircuit;

    if (boost::filesystem::is_directory(_synapse_path))
    {
        path /= "nrn.h5";
    }

    // Disabling error reporting in non debug mode
#ifdef NDEBUG
    H5E_BEGIN_TRY
#endif

    // Open synapse file with HDF5 reader.
    _file_handle.reset
        (H5Fopen(path.string().c_str(),H5F_ACC_RDONLY,H5P_DEFAULT),
         H5Fclose);

    // Renabling error reporting in non debug mode
#ifdef NDEBUG
    H5E_END_TRY
#endif

    if (!_file_handle)
    {
        std::string error = "Could not open synapse HDF5 file: '"
            + path.string() + "'";
        throw_exception(File_Access_Error(error), 
                        FATAL_LEVEL, __FILE__, __LINE__);
    }
}

// ----------------------------------------------------------------------------

void Synapse_HDF5_File_Reader::close()
{
    // Check if synapse HDF5 file is open.
    if (_file_handle)
    {
        // If a synapse dataset is open, close it.
        if (_current_neuron_gid != UNDEFINED_CELL_GID)
        {
            _dataset_hdf5.reset();
            _dataset_hdf5.reset();
            _data_table.clear();
            _current_neuron_gid = UNDEFINED_CELL_GID;
        }

        // If synapse HDF5 file is open, close it.
        _file_handle.reset();
    }
    
    _synapse_path = "";
}

// ----------------------------------------------------------------------------

void Synapse_HDF5_File_Reader::read_HDF5_dataset(Cell_GID cell_gid)
{
    // If requested dataset is not cached, load the dataset.
    if (_current_neuron_gid != cell_gid)
    {
        // Clearing previous cell table if needed
        _data_table.clear();

        // Open the requested dataset.
        std::stringstream temp;
        temp << "a" << cell_gid;
        _neuron_dataset_name = temp.str();
        _dataset_hdf5.reset(H5Dopen(_file_handle, _neuron_dataset_name.c_str()),
                            H5Dclose);
        if (!_dataset_hdf5)
        {
            _current_neuron_gid = UNDEFINED_CELL_GID;
            throw_exception(File_Parse_Error(
                 "a" + boost::lexical_cast<std::string>(cell_gid) + 
                 " synapse dataset does not exist in HDF5 file"),
                 WARNING_LEVEL, __FILE__, __LINE__);
        }

        // Retrieve dataset dimensions.
        _dataspace.reset(H5Dget_space(_dataset_hdf5), H5Sclose);
        if (H5Sget_simple_extent_ndims(_dataspace) != 2)
        {
            _current_neuron_gid = UNDEFINED_CELL_GID;
            throw_exception(
                File_Parse_Error("Synapse dataset is not two dimensional."),
                FATAL_LEVEL, __FILE__, __LINE__);
        }
        if (H5Sget_simple_extent_dims(_dataspace, _dims_out, NULL) < 0)
        {
            _current_neuron_gid = UNDEFINED_CELL_GID;
            throw_exception(
                File_Parse_Error("Synapse dataset dimensions could"
                                 " not be retrieved."), 
                FATAL_LEVEL, __FILE__, __LINE__);

        }

        // Read selected dataset into memory.
        _data_table.resize(_dims_out[0] * _dims_out[1]);
        _status = H5Dread(_dataset_hdf5 ,H5T_NATIVE_FLOAT, H5S_ALL,
                          H5S_ALL,H5P_DEFAULT,& _data_table[0]);
        _current_dataset_size = _dims_out[0]; 
        _current_neuron_gid = cell_gid;    
    }
}

// ----------------------------------------------------------------------------

Synapse_Index Synapse_HDF5_File_Reader::read_size_of_HDF5_dataset
    (Cell_GID cell_gid)
{
    // If requested dataset is not cached, load the dataset.
    if (_current_neuron_gid != cell_gid)
    {
        // Open the requested dataset.
        std::stringstream temp;
        temp << "a" << cell_gid;
        _neuron_dataset_name = temp.str();
        _dataset_hdf5.reset(H5Dopen(_file_handle, _neuron_dataset_name.c_str()),
                            H5Dclose);
        if (!_dataset_hdf5)
        {
            _current_neuron_gid = UNDEFINED_CELL_GID;
            throw_exception(File_Parse_Error(
                 "a" + boost::lexical_cast<std::string>(cell_gid) + 
                 " synapse dataset does not exist in HDF5 file"),
                WARNING_LEVEL, __FILE__, __LINE__);
        }

        // Clear data for any previous cell (despite we haven't loaded any data.
        _data_table.clear();    
    }

    // Get dataset dimensions.
    _dataspace.reset(H5Dget_space(_dataset_hdf5), H5Sclose);
    if (H5Sget_simple_extent_ndims(_dataspace) != 2)
    {
        _current_neuron_gid = UNDEFINED_CELL_GID;
        throw_exception(File_Parse_Error("Synapse dataset is not two dimens"
                                         "ional."), 
                        FATAL_LEVEL, __FILE__, __LINE__);
    }
    if (H5Sget_simple_extent_dims(_dataspace, _dims_out, NULL) < 0)
    {
        _current_neuron_gid = UNDEFINED_CELL_GID;
        throw_exception(
            File_Parse_Error("Synapse dataset dimensions could not be "
                             "retrieved."), FATAL_LEVEL, __FILE__, __LINE__);
    }

    _current_neuron_gid = cell_gid;

    return _dims_out[0];
}

// ----------------------------------------------------------------------------

void Synapse_HDF5_File_Reader::load(const Cell_Target & cells,
                                    Synapse_Attributes_Flags attributes,
                                    Synapse_Loading_Filter filtering)
{
    // Shorthands for neurons and dataset.
    //Neurons &           neurons = _microcircuit->neurons();
    Synapse_Dataset &   dataset = _microcircuit->dataset().structure->synapse;
    
    // Store which synapse properties are to be loaded.
    _attributes = attributes;

    // Create Neuron objects in Microcircuit for Cell_GIDs
    // that are not yet in the Microcircuit::neurons container.
    // \todo Isn't it dangerous doing this by default. Wouldn't it be better
    //       that Microcircuit_Reader filters out the GIDs for which no neuron
    //       has been loaded yet.
    //for (Cell_Target::iterator i = cells.begin(); i != cells.end(); ++i)
    //{
    //    if (neurons.find(*i) == neurons.end())
    //    {
    //        neurons.insert(Neuron_Ptr(new Neuron(_microcircuit, *i)));
    //    }
    //}

    load_synapse_dataset(dataset, cells);
    link_to_synapse_dataset(dataset, * _microcircuit, cells, filtering);
}

// ----------------------------------------------------------------------------

/*!
    \todo Shared vs. all synapses case (TT)
    \bug All efferent synapses are linked, while only shared should be
    in this implementation. (TT)
*/
void Synapse_HDF5_File_Reader::link_to_synapse_dataset(
    Synapse_Dataset & dataset, Microcircuit & microcircuit,
    const Cell_Target & cells, Synapse_Loading_Filter filtering)
{
    Neurons &   neurons (microcircuit.neurons());
    Synapse     synapse (microcircuit, UNDEFINED_SYNAPSE_INDEX);
    Cell_GID    postsynaptic_neuron (UNDEFINED_CELL_GID), 
                presynaptic_neuron (UNDEFINED_CELL_GID);

    // Clear Synapses containers of neurons involved.
    for (Neurons::iterator i = neurons.begin(); i != neurons.end(); ++i)
    {
        i->afferent_synapses().clear();
        i->efferent_synapses().clear();
    }

    // For all synapses in dataset, link them to existing Neuron objects
    // in Microcircuit::neurons() container.
    std::auto_ptr<boost::progress_display> progress;
    if (s_display_progress && dataset.size() > 0) {
        std::cout << "Linking synapses to neurons" << std::endl;
        progress.reset(new boost::progress_display(dataset.size()));
    }
    for (size_t i = 0; i < dataset.size(); ++i)
    {
        // Look up pre and postsynaptic Cell_GID for an index in the dataset.
        synapse_index(synapse) = i; 
        postsynaptic_neuron = dataset.postsynaptic_neuron_of_synapses()[i];
        presynaptic_neuron = dataset.presynaptic_neuron_of_synapses()[i];


        switch (filtering)
        {
        case ONLY_SHARED_SYNAPSES: 
        {
            // only insert synapses that are part of the loaded cell target
            if (cells.find(presynaptic_neuron) != cells.end() &&
                cells.find(postsynaptic_neuron) != cells.end())
            {
                // Connect synapse to presynaptic neuron 
                // if the neuron is in the microcircuit.
                Neurons::iterator j = neurons.find(presynaptic_neuron);
                if (j != neurons.end())
                {
                    j->efferent_synapses().insert(synapse);
                }
                // Connect synapse to postsynaptic neuron 
                // if the neuron is in the microcircuit.
                Neurons::iterator k = neurons.find(postsynaptic_neuron);
                if (k != neurons.end())
                {
                    k->afferent_synapses().insert(synapse);
                }
            }
            break;
        }
        case ONLY_AFFERENT_SYNAPSES:
        {
            // Connect synapse to presynaptic neuron 
            // if the neuron is in the microcircuit.
            Neurons::iterator i = neurons.find(postsynaptic_neuron);
            if (i != neurons.end())
            {
                i->afferent_synapses().insert(synapse);
            }       
            break;
        }
        }

        if (progress.get())
            ++(*progress);
    }
}

// ----------------------------------------------------------------------------

/*!
	\todo shared vs. all synapses case (TT)
	\todo Synapses of a neuron should be stored contiguously in per neuron
	arrays. (TT)
*/
void Synapse_HDF5_File_Reader::load_synapse_dataset(
    Synapse_Dataset & dataset, const Cell_Target & cells)
{
    // Check if HDF5 file open, if not throw exception.
    if (!_file_handle)
    {
        std::string error("Could not read synapses for neuron. (HDF5 file not"
            " open. Synapse_HDF5_File_Reader::read(Microcircuit &))");
        throw_exception(File_Open_Error(error), FATAL_LEVEL,
                        __FILE__, __LINE__);
    }

    // Check how many synapses are requested to be loaded.
    Synapse_Index total = 0;
    for (Cell_Target::iterator i = cells.begin(); i != cells.end(); ++i)
    {
        try 
        {
            total += read_size_of_HDF5_dataset(* i);
        } 
        catch (File_Parse_Error & error) 
        {
            /*
                These errors are not fatal in some tests circuits that don't
                have synapses inside some neurons.
            */
            log_message(error.what(), WARNING_LEVEL, __FILE__, __LINE__);
        }
    }

    // Clear synapse dataset and set attributes to be read.
    dataset.clear();
    dataset.attributes(_attributes);

    // Resize dataset to number of synapses to be loaded.
    dataset.reserve(total);

    // Load the afferent synapses for each neuron.
    std::auto_ptr<boost::progress_display> progress;
    if (s_display_progress) {
        std::cout << "Loading synapses dataset" << std::endl;
        progress.reset(new boost::progress_display(cells.size()));
    }
    for (Cell_Target::iterator i = cells.begin(); i != cells.end(); ++i)
    {
        // Read dataset for this neuron in memory if not cached.
        try
        {
            read_HDF5_dataset(* i);
        } 
        catch (File_Parse_Error &) 
        {
            // This errors are not fatal in some tests circuits that don't
            // have synapses inside some neurons. Skipping neuron
            continue;
        }

        static std::vector<Synapse_Index> dataset_indices;
        dataset_indices.clear();

        // Store the requested properties for afferent synapses of this neuron.
        
        // Get dataset locations for the synapses to be added.
        for (Synapse_Index synapse_index = 0; 
             synapse_index < _current_dataset_size; ++synapse_index)
        {
            dataset_indices.push_back(dataset.insert());
        }

        // SYNAPSE STRUCTURE AND DYNAMICS _________________________________

        // Store synapse type.
        if (_attributes & SYNAPSE_TYPE)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                //! \todo Check if int is 32 bit here. (TT)
                dataset.type(dataset_indices[synapse_index]) 
                    = static_cast<Synapse_Type_ID>((boost::int32_t) 
                      _data_table[synapse_index * _dims_out[1] + 13]);
            }
        }

        // Store conductance.
        if (_attributes & SYNAPSE_CONDUCTANCE)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.conductance(dataset_indices[synapse_index])
                    = _data_table[synapse_index * _dims_out[1] +  8];
            }
        }

        // Store utilization probability.
        if (_attributes & SYNAPSE_UTILIZATION)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.utilization(dataset_indices[synapse_index])
                    = _data_table[synapse_index * _dims_out[1] +  9];
            }
        }

        // Store depression time constant.
        if (_attributes & SYNAPSE_SHORT_TERM_DEPRESSION)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.depression(dataset_indices[synapse_index])
                    = static_cast<Millisecond_Time_Constant>(
                    (boost::int32_t)_data_table
                    [synapse_index * _dims_out[1] + 10]); 
            }
        }

        // Store facilitation time constant.
        if (_attributes & SYNAPSE_SHORT_TERM_FACILITATION)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                //! \todo Check if int is 32 bit here. (TT)
                dataset.facilitation(dataset_indices[synapse_index])
                    = static_cast<Millisecond_Time_Constant>((boost::int32_t)(
                    _data_table[synapse_index * _dims_out[1] + 11]));
            }
        }

        // Store facilitation time constant.
        if (_attributes & SYNAPSE_ABSOLUTE_SYNAPTIC_EFFICACY)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                //! \todo Check if int is 32 bit here. (TT)
                dataset.efficacy(dataset_indices[synapse_index])
                    = static_cast<Millivolt>((boost::int32_t)
                    _data_table[synapse_index * _dims_out[1] + 17]);
            }
        }

        // Store position.
        if (_attributes & SYNAPSE_POSITION)
        {
			// nothing to be done here since this needs to be derived
        }

        // PRESYNAPTIC ____________________________________________________

        // Store presynaptic neuron.
        if (_attributes & SYNAPSE_PRESYNAPTIC_NEURON)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.presynaptic_neuron_of_synapse(
                    dataset_indices[synapse_index])
                    = static_cast<Cell_GID>((boost::int32_t)
                        _data_table[synapse_index * _dims_out[1] + 0]); 
            }
        }

        // presynaptic section
        if (_attributes & SYNAPSE_PRESYNAPTIC_SECTION)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.presynaptic_section_id(
                    dataset_indices[synapse_index])
                    = (boost::int32_t)_data_table
                        [synapse_index * _dims_out[1] + 5]; 
            }
        }

        // presynaptic segment
        if (_attributes & SYNAPSE_PRESYNAPTIC_SEGMENT)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.presynaptic_segment_id(
                    dataset_indices[synapse_index])
                    = (boost::int32_t)_data_table
                        [synapse_index * _dims_out[1] + 6]; 
            }
        }

        // distance within presynaptic segment
        if (_attributes & SYNAPSE_PRESYNAPTIC_SEGMENT_DISTANCE)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.presynaptic_segment_distance(
                    dataset_indices[synapse_index])
                    = _data_table[synapse_index * _dims_out[1] + 7]; 
            }
        }

        // POSTSYNAPTIC ___________________________________________________


        // postsynaptic neuron
        if (_attributes & SYNAPSE_POSTSYNAPTIC_NEURON)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.postsynaptic_neuron_of_synapse(
                    dataset_indices[synapse_index]) = *i;
            }
        }

        // postsynaptic section
        if (_attributes & SYNAPSE_POSTSYNAPTIC_SECTION)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.postsynaptic_section_id(
                    dataset_indices[synapse_index])
                    = (boost::int32_t)_data_table
                        [synapse_index * _dims_out[1] + 2]; 
            }
        }

        // postsynaptic segment
        if (_attributes & SYNAPSE_POSTSYNAPTIC_SEGMENT)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.postsynaptic_segment_id(
                    dataset_indices[synapse_index])
                    = (boost::int32_t)_data_table
                        [synapse_index * _dims_out[1] + 3]; 
            }
        }

        // distance within postsynaptic segment
        if (_attributes & SYNAPSE_POSTSYNAPTIC_SEGMENT_DISTANCE)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.postsynaptic_segment_distance(
                    dataset_indices[synapse_index])
                    = _data_table[synapse_index * _dims_out[1] + 4]; 
            }
        }

        // postsynaptic neuron
        if (_attributes & SYNAPSE_POSTSYNAPTIC_NEURON)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.postsynaptic_neuron_of_synapse(
                    dataset_indices[synapse_index]) = *i;
            }
        }

        // postsynaptic potential decay time constant
        if (_attributes & SYNAPSE_DECAY)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.decay(dataset_indices[synapse_index])
                    = _data_table[synapse_index * _dims_out[1] + 12]; 
            }
        }

        // postsynaptic delay from presynaptic soma to synapse including 
		// transmission over synaptic cleft.
        if (_attributes & SYNAPSE_DELAY)
        {
            for (Synapse_Index synapse_index = 0; 
                 synapse_index < _current_dataset_size; ++synapse_index)
            {
                dataset.delay(dataset_indices[synapse_index])
                    = _data_table[synapse_index * _dims_out[1] + 1];
            }
        }

        if (progress.get())
            ++(*progress);
    }
    std::cout << std::endl;
}

// ----------------------------------------------------------------------------

}
