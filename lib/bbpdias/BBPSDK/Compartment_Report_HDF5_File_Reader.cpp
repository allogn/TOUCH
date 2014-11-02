/*

    Ecole Polytechnique Federale de Lausanne
    Brain Mind Institute,
    Blue Brain Project
    (c) 2006-2007. All rights reserved.

    Responsible author:	  Juan Hernando Vieites
    Contributing authors: Sebastien Lasserre,
                          Alvaro Rodriguez,
                          Thomas Traenkler
*/

// Using deprecated functions from HDF5 1.6.0
#define H5Dopen_vers 1
#include <hdf5.h>

#include <boost/filesystem.hpp>
#include <boost/progress.hpp>

#include "BBP/Common/Exception/Exception.h"
#include "BBP/Common/Math/Round.h"
#include "BBP/Common/Dataset/HDF5/H5ID.h"
#include "File/Compartment_Report_HDF5_File_Reader.h"
#include "report_specialization_selection.h"

namespace bbp
{

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
private:
    bool _display;
} s_display_progress;

//-----------------------------------------------------------------------------

// Reads an attribute from a dataset. Returns true if the attribute exists and
// could be read, returns false in any other case.
template <typename T>
bool read_attribute(const char * name, const H5ID & dataset, T & value);

//-----------------------------------------------------------------------------

Compartment_Report_HDF5_File_Reader::Compartment_Report_HDF5_File_Reader
(const Report_Specification & specs) :
    _path(uri_to_filename(specs.data_source())),
    _report_name(specs.label()),
    _sequence_start_frame(0),
    _frame_counter(0),
    _frame_skip(1.0),
    _current_framestamp(UNDEFINED_FRAME_NUMBER)
{
    namespace fs = boost::filesystem;

    static Report_Specialization_Register<
               Compartment_Report_HDF5_File_Reader,
               float
           > register_float_buffer;

    // Finding a suitable cell name from which has a h5 inside the search path.
    bool h5_file_found = false;
    if (!fs::is_directory(_path))
    {
        throw_exception(
            IO_Error("Compartment_Report_HDF5_File_Reader: data source "
                     "is not a directory: " + specs.data_source()),
            FATAL_LEVEL, __FILE__, __LINE__);
    }
    fs::directory_iterator entry(_path), end_entry;
    Cell_GID cell_GID = UNDEFINED_CELL_GID;
    while (cell_GID == UNDEFINED_CELL_GID && entry != end_entry)
    {
        fs::path filename = entry->path();
        std::string cell_name = fs::basename(filename);
        char * endptr;
        if (fs::is_regular(entry->status()) &&
            fs::extension(filename) == ".h5" &&
            // Checking if name matches a[0-9]+ pattern and storing the GID
            cell_name.size() > 1 && cell_name[0] == 'a' &&
            (cell_GID = strtol(&cell_name[1], &endptr, 10)) > 0 &&
            *endptr == '\0')
            h5_file_found = true;
        ++entry;
    }
    if (cell_GID == UNDEFINED_CELL_GID)
    {
        throw_exception(
            Bad_Data("Compartment_Report_HDF5_File_Reader: source path "
                     "doesn't contain any valid .h5 file" + 
                     specs.data_source()), FATAL_LEVEL, __FILE__, __LINE__);
    }
    
    // Not catching any exception here
    H5ID file, dataset;
    open_data_set(cell_GID, "data", file, dataset);
    float start_time, end_time, delta_time;
    if (// Trying to read attributes
        !read_attribute("tstart", dataset, start_time) ||
        !read_attribute("tstop", dataset, end_time) ||
        !read_attribute("Dt", dataset, delta_time) ||
        // And checking them
        start_time != specs.start_time() ||
        end_time != specs.end_time() ||
        delta_time != specs.timestep())
    {
/*!
    \todo Exception commented because the forward_skip is not taken into
    account in the HDF5. SL - 24.07.08
        throw_exception(
            Bad_Data("Compartment_Report_HDF5_File_Reader: inconsistent"
                     " report metadata found for '" + specs.label() +
                     "' in path " + specs.data_source()), 
            FATAL_LEVEL, __FILE__, __LINE__);
*/
    }
}

//-----------------------------------------------------------------------------

void Compartment_Report_HDF5_File_Reader::update_mapping_and_framesize
(const Cell_Target & target)
{
    Cell_Index cell_index = 0;
    Report_Frame_Index next_compartment_index = 0;

    Compartment_Report_Mapping_Ptr mapping(new Compartment_Report_Mapping);
    std::vector<std::vector<Report_Frame_Index> > offset_mapping;
    offset_mapping.resize(target.size());

    for (Cell_Target::iterator cell_ID = target.begin();
         cell_ID != target.end();
         ++cell_ID, ++cell_index)
    {
        H5ID file, dataset;
        try
        {
            open_data_set(*cell_ID, "mapping", file, dataset);
        }
        catch (File_Open_Error) 
        {
            continue;
        }

        // Opening the dataspace
        H5ID space(H5Dget_space(dataset), H5Sclose);
        int rank = H5Sget_simple_extent_ndims(space);
        if (rank != 2)
        {
            std::string cell_name = 
                "a" + boost::lexical_cast<std::string>(*cell_ID);
            std::string dataset_name = 
                "/" + cell_name + "/" + (_report_name + "/") + "mapping";
            throw_exception(
                File_Parse_Error("Compartment_Report_HDF5_File_Reader: "
                                 "Error, not 2 dimensional array on " + 
                                 dataset_name), 
                FATAL_LEVEL, __FILE__, __LINE__);
        }

        hsize_t dims[2];        
        H5Sget_simple_extent_dims(space, dims, NULL);

        boost::shared_array<float> data(new float[dims[1]]);
        H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
                data.get());

        // Getting the last section id;
        size_t largest_section_id = 0;
        for(size_t i = 0; i < dims[1]; ++i) 
        {
            if (data[i] > largest_section_id)
                largest_section_id = (size_t) data[i];
        }

        std::vector<Report_Frame_Index> & offsets = offset_mapping[cell_index];
        offsets.resize(largest_section_id + 1, UNDEFINED_REPORT_FRAME_INDEX);

        size_t last_section = UNDEFINED_SECTION_ID;
        for(size_t i = 0; i < dims[1]; ++i, ++next_compartment_index)
        {
            size_t section = (size_t) data[i];
            if (last_section != section)
            {
                last_section = section;
                // Storing the start index of a new section with at 
                // least 1 compartment
                offsets[section] = next_compartment_index;
            }
        }
    }

    // next_compartment_index contains the total number of compartments
    _frame_size = next_compartment_index;

    // Updating internal mapping pointer.
    mapping->swap_mapping(offset_mapping, next_compartment_index);
    _mapping = mapping;

    _current_cell_target = target;
}

//-----------------------------------------------------------------------------

void Compartment_Report_HDF5_File_Reader::jump_to_frame(Frame_Number position)
    /* throw (IO_Error, Unsupported) */
{
    bool attribute_found = false;
    Frame_Number last_frame = UNDEFINED_FRAME_NUMBER; // To silent warning.
    Cell_Target::iterator cell_ID = _current_cell_target.begin();
    while (!attribute_found && cell_ID != _current_cell_target.end())
    {
        H5ID file, dataset;
        open_data_set(*cell_ID, "data", file, dataset);

        float   start_time  = 0.0f, 
                end_time    = 0.0f, 
                delta_time  = 0.0f;

        if (!read_attribute("tstart", dataset, start_time) ||
            !read_attribute("tstop", dataset, end_time) ||
            !read_attribute("Dt", dataset, delta_time))
        {
            std::string name = "a" + boost::lexical_cast<std::string>(*cell_ID);
            std::string filename = (_path / (name + ".h5")).string();
            throw_exception(
                File_Parse_Error("Compartment_Report_HDF5_File_Reader: "
                                 "Error reading report time attributes "
                                 "from file:" + filename),
                FATAL_LEVEL, __FILE__, __LINE__);
        }

#ifdef WIN32
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif
        last_frame = (Frame_Number) round((end_time - start_time) / 
                                          delta_time) - 1;
#ifdef WIN32
#pragma warning ( pop )
#endif
        attribute_found = true;

        ++cell_ID;
    }


    //! \bug What do we do if cell_target is emtpy?
    if (_current_cell_target.size() != 0 && 
        (!attribute_found || position > last_frame))
    {
        throw_exception(
            File_Parse_Error("Compartment_Report_HDF5_File_Reader: "
                             "frame number out of bounds"),
            SEVERE_LEVEL, __FILE__, __LINE__);
    }

    _sequence_start_frame = position;
    _frame_counter = 0;
    _current_framestamp = UNDEFINED_FRAME_NUMBER;
}

void Compartment_Report_HDF5_File_Reader::frame_skip(double frame_skip)
    /* throw (IO_Error, Unsupported) */
{
    _frame_skip = frame_skip;
}

//-----------------------------------------------------------------------------

void Compartment_Report_HDF5_File_Reader::reset()
    /* throw (IO_Error, Unsupported) */
{
    _sequence_start_frame = 0;
    _frame_counter = 0;
}

//-----------------------------------------------------------------------------

template <>
void Compartment_Report_HDF5_File_Reader::load_frames_impl
(float * buffer, Frame_Number first, Frame_Number last)
        /* throw (IO_Error, Unsupported) */
{
    // The offset for the first comparment of the cell being processed
	Report_Frame_Index first_compartment_offset = 0;

    std::auto_ptr<boost::progress_display> progress;
    if (first != last && s_display_progress)
        progress.reset(new boost::progress_display
                       (_current_cell_target.size()));

    for (Cell_Target::iterator cell_ID = _current_cell_target.begin();
         cell_ID != _current_cell_target.end();
         ++cell_ID)
    {
        // Opening file and dataset for current cell
        H5ID file, dataset;
        try
        {
            open_data_set(*cell_ID, "data", file, dataset);
        }
        catch (File_Open_Error) 
        {
            // Skipping cell
            continue;
        }

        // Getting the dataspace sizes
        H5ID space(H5Dget_space(dataset), H5Sclose);
        int rank = H5Sget_simple_extent_ndims(space);
        if (rank != 2)
        {
            std::string cell_name = 
                "a" + boost::lexical_cast<std::string>(*cell_ID);
            std::string dataset_name = 
                "/" + cell_name + "/" + (_report_name + "/") + "data";
            throw_exception(
                File_Parse_Error("Compartment_Report_HDF5_File_Reader: "
                                 "Error, not 2 dimensional array on " + 
                                 dataset_name), 
                FATAL_LEVEL, __FILE__, __LINE__);
        }
        const int FRAMES = 0;
        const int COMPARTMENTS = 1;
        hsize_t source_sizes[2];
        H5Sget_simple_extent_dims(space, source_sizes, NULL);

        // Selecting the hyperslab (in this case a rectangle) of the dataspace
        // where the data to be read comes from.
        space.reset(H5Dget_space(dataset), H5Sclose);
        hsize_t read_offsets[2];
        read_offsets[FRAMES] = first;
        read_offsets[COMPARTMENTS] = 0;
        hsize_t read_counts[2] ;
        read_counts[FRAMES] = last - first + 1;
        read_counts[COMPARTMENTS] = source_sizes[COMPARTMENTS];
        H5Sselect_hyperslab(space, H5S_SELECT_SET, 
                            read_offsets, NULL, read_counts, NULL);

        // Creating the definition of the dataspace where the data read
        // will be stored to.
        hsize_t target_sizes[2];
        target_sizes[FRAMES] = last - first + 1;
        target_sizes[COMPARTMENTS] = frame_size();
		H5ID target_space(H5Screate_simple(2, target_sizes, NULL),
                          H5Sclose);

        hsize_t target_offsets[2];
        target_offsets[FRAMES] = 0;
        target_offsets[COMPARTMENTS] = first_compartment_offset;
        hsize_t target_counts[2];
        target_counts[FRAMES] = last - first + 1;
        target_counts[COMPARTMENTS] = source_sizes[COMPARTMENTS];
		H5Sselect_hyperslab(target_space, H5S_SELECT_SET, 
                            target_offsets, NULL, target_counts, NULL);

        
		H5Dread(dataset, H5T_NATIVE_FLOAT, target_space, space, 
                H5P_DEFAULT, buffer);

		first_compartment_offset += source_sizes[COMPARTMENTS];

        if (progress.get())
        {
            ++(*progress);
        }
    }
}

//-----------------------------------------------------------------------------
 
template <>
bool Compartment_Report_HDF5_File_Reader::load_next_frame_impl
(float * buffer) /* throw (IO_Error) */
{
    // \bug No overflow check is performed
    Frame_Number next_frame = (Frame_Number)_sequence_start_frame + 
        (Frame_Number) round(_frame_counter * _frame_skip);

    // Checking if we have reached the end
    bool attribute_found = false;
    Frame_Number last_frame = UNDEFINED_FRAME_NUMBER; // To silent warning.
    Cell_Target::iterator cell_ID = _current_cell_target.begin();
    do {
        H5ID file, dataset;
        open_data_set(*cell_ID, "data", file, dataset);

        float   start_time  = 0.0f, 
                end_time    = 0.0f, 
                delta_time  = 0.0f;
        if (!read_attribute("tstart", dataset, start_time) ||
            !read_attribute("tstop", dataset, end_time) ||
            !read_attribute("Dt", dataset, delta_time))
        {
            std::string name = "a" + boost::lexical_cast<std::string>(*cell_ID);
            std::string filename = (_path / (name + ".h5")).string();
            throw_exception(
                File_Parse_Error("Compartment_Report_HDF5_File_Reader: "
                                 "Error reading report time attributes "
                                 "from file:" + filename), 
                FATAL_LEVEL, __FILE__, __LINE__);
        }
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4701 )
        last_frame = (Frame_Number) round((end_time - start_time) / 
                                          delta_time) - 1;
#pragma warning ( pop )
#endif
        attribute_found = true;

        ++cell_ID;
    } while (!attribute_found && cell_ID != _current_cell_target.end());


    if (!attribute_found || last_frame < next_frame)
    {
        // End of report reached
        return false;
    }
    else
    {
        load_frames_impl(buffer, next_frame, next_frame);
        _current_framestamp = next_frame;
        ++_frame_counter;

        return true;
    }
}

//-----------------------------------------------------------------------------

void Compartment_Report_HDF5_File_Reader::open_data_set
(Cell_GID cell_ID, const char * dataset_name, H5ID & file, H5ID & dataset) const
{
    // Opening file for current cell
    std::string cell_name = "a" + boost::lexical_cast<std::string>(cell_ID);
    Filename filename = _path / (cell_name + ".h5");

    file.reset(H5Fopen(filename.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT),
               H5Fclose);
    if (!file)
    {
        throw_exception(
            File_Open_Error("Compartment_Report_HDF5_File_Reader: error "
                            "opening file:" + filename.string()),
            FATAL_LEVEL, __FILE__, __LINE__);
    }

    // Opening the dataset
    std::string dataset_full_name = 
        "/" + cell_name + "/" + (_report_name + "/") + dataset_name;
    H5E_BEGIN_TRY
        dataset.reset(H5Dopen(file, dataset_full_name.c_str()), H5Dclose);
    H5E_END_TRY;
    if (!dataset)
    {
        throw_exception(
            File_Parse_Error("Compartment_Report_HDF5_File_Reader: "
                             "Dataset " + dataset_full_name + " not found "
                             "in file: " + filename.string()),
            FATAL_LEVEL, __FILE__, __LINE__);
    }
}

//-----------------------------------------------------------------------------

template <typename T>
bool read_attribute(const char * name, const H5ID & dataset, T & value)
{
    H5ID attribute(H5Aopen_name(dataset, name), H5Aclose);
    if (!attribute)
        return false;
    herr_t status = H5Aread(attribute, H5T_NATIVE_FLOAT, &value);
    return status >= 0;
}

//-----------------------------------------------------------------------------

}
