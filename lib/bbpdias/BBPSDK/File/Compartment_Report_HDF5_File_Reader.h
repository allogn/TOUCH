/*

    Ecole Polytechnique Federale de Lausanne
    Brain Mind Institute,
    Blue Brain Project
    (c) 2006-2007. All rights reserved.

    Responsible author:	Juan Hernando Vieites

*/
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENT_REPORT_HDF5_FILE_READER_H
#define BBP_COMPARTMENT_REPORT_HDF5_FILE_READER_H

#include "BBP/Common/Math/Round.h"
#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Experiment/Readers/detail/Compartment_Report_Reader_Impl.h"

namespace bbp
{

class H5ID;

class Compartment_Report_HDF5_File_Reader :
    public Compartment_Report_Reader_Impl
{
public:
    //! Create a hdf5 report reader using the source from the specs.
    /**
       The data source is expected to be a directory containing .h5 files.
       The metadata from the first file found in the directory is checked 
       against the specs given.
       An exception is thrown is no .h5 file is found, the data source is not
       a directory or the metada is inconsistent with the specs.
     */
    Compartment_Report_HDF5_File_Reader(const Report_Specification & specs)
        /* throw (Bad_Data, IO_Error) */;

    virtual void update_mapping_and_framesize(const Cell_Target &target)
        /* throw (IO_Error) */;

    //! Loads [first, last] frames on the given buffer.
    /*
      \sa Compartment_Report_Reader_Impl::load_frames
              (T *, Frame_Number, Frame_Number)
    */
    template <typename T>
    void load_frames_impl(T * buffer, Frame_Number first, Frame_Number last)
        /* throw (IO_Error, Unsupported) */;

    //! Loads next frame onto a buffer of frame_size() T elements
    /*
      \sa Compartment_Report_Reader_Impl::load_next_frame(T * buffer)
    */
    template <typename T>
    bool load_next_frame_impl(T * buffer) /* throw (IO_Error) */;

    //! \sa Compartment_Report_Reader_Impl::jump_to_frame(Frame_Number)
    virtual void jump_to_frame(Frame_Number framestamp)
        /* throw (IO_Error, Unsupported) */; 

    //! \sa Compartment_Report_Reader_Impl::frame_skip(double)
    virtual void frame_skip(double frame_skip)
        /* throw (IO_Error, Unsupported) */; 

    //! \sa Compartment_Report_Reader_Impl::frame_skip()
    virtual double frame_skip() const
    {
        return _frame_skip;
    }

    //! \sa Compartment_Report_Reader_Impl::reset()
    virtual void reset()
        /* throw (IO_Error, Unsupported) */;

    //! \sa Compartment_Report_Reader_Impl::current_framestamp */
    Frame_Number current_framestamp() const
    {
        return _current_framestamp;
    }

protected:
    void open_data_set(Cell_GID cell, const char * dataset_name,
                       H5ID & file, H5ID & dataset) const;

protected:
    Filepath     _path;
    Label        _report_name;

    // _sequence_start_frame and _frame_counter are used to minimize rounding
    // problems in long runs.
    double       _sequence_start_frame;
    size_t       _frame_counter;
    double       _frame_skip;

    Frame_Number _current_framestamp;
};

}
#endif

