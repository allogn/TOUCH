/*

    Ecole Polytechnique Federale de Lausanne
    Brain Mind Institute,
    Blue Brain Project
    (c) 2006-2007. All rights reserved.

    Responsible author:	Juan Hernando Vieites

*/

/** \cond SHOW_IN_DEVELOPER_REFERENCE */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENT_REPORT_READER_IMPL_H
#define BBP_COMPARTMENT_REPORT_READER_IMPL_H

#include <boost/shared_ptr.hpp>

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Experiment/Report_Specification.h"
#include "BBP/Model/Experiment/Mappings/Compartment_Report_Mapping.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"

namespace bbp {

//! Abstract interface class for all report readers
class Compartment_Report_Reader_Impl
{
public:
    typedef boost::shared_ptr<Compartment_Report_Reader_Impl> pointer;

    //! Creates a reader for the given report specs and opens the data source
    static pointer create_reader(const Report_Specification & specs)
        /* throw (IO_Error_Exc, Bad_Data_Exc) */;

protected:
    inline Compartment_Report_Reader_Impl();

public:
    // Derived classes must close the data source
    virtual ~Compartment_Report_Reader_Impl() {}

public:
    inline size_t frame_size() const;

    //! Returns the mapping for the current cell_target selection.
    /* The actual pointer will change if the cell target selected is changed */
    inline const Compartment_Report_Mapping_Ptr & mapping() const;

    virtual void update_mapping_and_framesize(const Cell_Target &target) 
        /* throw (IO_Error_Exc) */ = 0;

    //! Loads the requested frames into a buffer.
    /**
       Frame numbers are in the original absolute reference frame of the
       report used to create the reader.
       The buffer must be allocated by the caller to hold (frame_size() * 
       last - first) T elements.
       This operation is not necessarily supported in every final 
       implementation.
     
     Loads a frame interval in the given buffer. Frame numbers are in the
     reference frame of the original report specs.
     @param buffer The user must preallocate the enough space in this buffer.
     The size of the expected buffer is (last - first + 1) * compartment count
     of current mapping * sizeof(float).
     @param first First frame to load. Bad_Data_Exc is thrown if this frame
     number is out of file bounds
     @param last Last frame to load. Bad_Data_Exc is thrown if this frame
     number is out of file bounds. If this frame number if lower that first
     nothing is done. 
    */
    template <typename T>
    void load_frames(T * buffer, Frame_Number first, Frame_Number last)
        /* throw (IO_Error_Exc, Unsupported_Operation_Exc) */;

    //! Loads next frame and advances cursors.
    /**
       This function returns true if and only if the data source has reached
       its end, independently from the time window used in 
       Compartment_Report_Reader.
     
     The frame currently pointed by the frame cursor is read and the cursor
     is incremented by the frame skip.
     The user must preallocate the enough space in the buffer. The size
     of the expected buffer compartments count of current 
     mapping * sizeof(float)
     @return Returns true when a frame has been successfully read and false
     when EOF was reached by previous call to this function.
     
       \sa Report::load_next_frame(T * buffer)
    */
    template <typename T>
    bool load_next_frame(T * buffer) /* throw (IO_Error_Exc) */;

    //! Sets the loading cursor at the given frame if possible
    /**
       Frame number are in the original absolute reference frame of the 
       report used to create the reader (start time = frame 0).

     @param framestamp The frame number to seek to in the original report
     specs reference frame.
     If the frame is UNDEFINED_FRAME_NUMBER the reader is reset to the
     first frame in the file.
     If the given frame number is out of the file bounds, Bad_Data_Exc is
     thrown and the current frame cursor is left unmodified.
     Next call to load_next_frame will read the given frame_number.
     
       An exception can be throw if the frame number, despite being inside
       the time window and report specs is out of the time window for the
       data source (e.g a binary file with an interval of the original report)
     */
    virtual void jump_to_frame(Frame_Number framestamp 
                               = UNDEFINED_FRAME_NUMBER)
        /* throw (Bad_Data_Exc, IO_Error_Exc,
                  Unsupported_Operation_Exc) */ = 0; 

    //! \sa Report_Reader::frame_skip(double)
    virtual void frame_skip(double frame_skip)
        /* throw (Unsupported_Operation_Exc) */ = 0;

    //! Returns the current frame skip.
    /**
       If setting frame skip is unsupported it will return 1
    */
    virtual double frame_skip() const = 0;

    //! Resets the data sources.
    /**
       After this call the data source can be considered to be at its initial
       state as if this object had just been created.
     */
    virtual void reset()
        /* throw (IO_Error_Exc, Unsupported_Operation_Exc) */ = 0;

    //! Returns the frame number of the last frame loaded by load_next_frame
    /*
       UNDEFINED_FRAME_NUMBER will be returned when no frame has been loaded 
       yet or if jump_to_frame has been called after last call to 
       load_next_frame;
       The frame number will be in the original absolute reference frame of the
       report used to create the reader.
    */
    virtual Frame_Number current_framestamp() const = 0;

protected:
    void load_frames_delegate(const std::string & buffer_element_typename,
                              void * buffer,
                              Frame_Number first, Frame_Number last)
        /* throw (IO_Error_Exc, Unsupported_Operation_Exc) */;

    bool load_next_frame_delegate(const std::string & buffer_element_typename,
                                  void * buffer)
        /* throw (IO_Error_Exc, Unsupported_Operation_Exc) */;

protected:
    size_t                         _frame_size;
    Compartment_Report_Mapping_Ptr _mapping;
    Cell_Target                    _current_cell_target;
};

//-----------------------------------------------------------------------------

Compartment_Report_Reader_Impl::Compartment_Report_Reader_Impl() :
    _frame_size(0)
{
}

//-----------------------------------------------------------------------------

size_t Compartment_Report_Reader_Impl::frame_size() const
{
    return _frame_size;
}

//-----------------------------------------------------------------------------

const Compartment_Report_Mapping_Ptr & 
Compartment_Report_Reader_Impl::mapping() const
{
    return _mapping;
}

//-----------------------------------------------------------------------------

template <typename T>
void Compartment_Report_Reader_Impl::load_frames
(T * buffer, Frame_Number first, Frame_Number last)
    /* throw (IO_Error_Exc, Unsupported_Operation_Exc) */
{
    load_frames_delegate(typeid(*buffer).name(), buffer, first, last);
}

//-----------------------------------------------------------------------------

template <typename T>
bool Compartment_Report_Reader_Impl::load_next_frame
(T * buffer) /* throw (IO_Error_Exc) */
{
    return load_next_frame_delegate(typeid(*buffer).name(), buffer);
}

//-----------------------------------------------------------------------------

}
#endif

/** \endcond SHOW_IN_DEVELOPER_REFERENCE */
