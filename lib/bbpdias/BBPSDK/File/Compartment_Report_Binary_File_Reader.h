/*

    Ecole Polytechnique Federale de Lausanne
    Brain Mind Institute,
    Blue Brain Project
    (c) 2006-2007. All rights reserved.

    Responsible authors:	Juan Hernando Vieites
                            Thomas Traenkler

*/
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENT_REPORT_BINARY_FILE_READER_H
#define BBP_COMPARTMENT_REPORT_BINARY_FILE_READER_H

#include "BBP/Model/Experiment/Readers/detail/Compartment_Report_Reader_Impl.h"
#include "BBP/Model/Microcircuit/Types.h"
#include "Parsers/Compartment_Report_Binary_File_Parser.h"

namespace bbp
{

class H5ID;

/*!
    \todo Cross platform implementation of report data reading missing. (TT)
*/
class Compartment_Report_Binary_File_Reader :
    public Compartment_Report_Reader_Impl
{
public:
    //! Create a binary report reader using the source from the specs.
    /**
       The data source can be either a directory or a file. 
       In case the source is a directory a file starting with the report name
       as prefix will be searched, its extension determines which version of
       the reader will be used. If the data source is a file then that file
       is opened regardless of its name and the extension is used to
       determine the type.
       An exception is thrown if the extension is not recognized, there is
       a problem serching or opening the file or the specs are detected to
       not match the report open.
     */
    Compartment_Report_Binary_File_Reader(const Report_Specification & specs)
        /* throw (Bad_Data, IO_Error) */;

    // Moved to implementation to avoid problems with auto_ptr
    ~Compartment_Report_Binary_File_Reader();

    virtual void update_mapping_and_framesize(const Cell_Target &target)
        /* throw (IO_Error) */;

    //! Loads [first, last] frames on the given buffer.
    /*
        \todo Shouldn't this be non public? (TT)
      \sa Compartment_Report_Reader_Impl::load_frames
              (T *, Frame_Number, Frame_Number)
    */
    template <typename T>
    void load_frames_impl(T * buffer, Frame_Number first, Frame_Number last)
        /* throw (IO_Error, Unsupported) */;

    //! Loads next frame onto a buffer of frame_size() T elements
    /*
        \todo Shouldn't this be non public? (TT)
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
    virtual double frame_skip() const;

    //! \sa Compartment_Report_Reader_Impl::current_framestamp()
    virtual Frame_Number current_framestamp() const;

    //! \sa Compartment_Report_Reader_Impl::reset()
    virtual void reset()
        /* throw (IO_Error, Unsupported) */;

private:
    //! Selects a cell target for reading and computes its mapping.
    /*!
     This method prepares the reader for reading data for the given cell
     target. Cell appearing inside the cell target for which there is no
     simulation data should be ignored but included in the mapping as empty
     cells.
     @return The mapping computed for the cell target. This mapping will have
     the information needed to allocate the buffers for loading methods and 
     also the offsets and compartment counts for the given cell target within
     a frame.
     */
    Compartment_Report_Mapping_Ptr update_mapping
        (const Cell_Target & new_cell_target);

    Frame_Number _sequence_start_frame;
    size_t       _frame_counter;
    double       _frame_skip;
    Frame_Number _current_frame;
	Compartment_Report_Binary_File_Parser _parser;
    //std::auto_ptr<Compartment_Report_Binary_File_Reader_Impl> _impl;
};

} // namespace bbp end
#endif
