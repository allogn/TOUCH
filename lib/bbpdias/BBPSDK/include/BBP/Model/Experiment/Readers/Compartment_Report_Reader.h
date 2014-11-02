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

#ifndef BBP_COMPARTMENT_REPORT_H
#define BBP_COMPARTMENT_REPORT_H

#include <boost/shared_array.hpp>

#include "BBP/Common/Patterns/shared_array_helpers.h"
#include "BBP/Model/Experiment/Compartment_Report_Frame.h"
#include "Report_Reader.h"

namespace bbp {

class Compartment_Report_Reader_Impl;

//! This is a extended report class for generic compartment type reports
/*! 
    This class adds cell target selection and full report access members.
    All operation are supported as long as the data source is a path to hdf5 
    report files or a binary report.

    The Allocator parameter is used to provide specific allocation routines
    for arrays of T used when Compartment_Report_Frame objects are read.
    For simplicity (in specification not in implementation :) ) we will keep
    the STL allocator implementation. By default std::allocator will be
    used (and should be ok in most cases.
    \ingroup Report
*/
template <typename T, typename Allocator = std::allocator<T> >
class Compartment_Report_Reader 
	: public Report_Reader
{
public:
    //! Creates this report to access specific data source
    /*! Current data source support is limited to paths pointing to 
        directories which store HDF5 report files and paths to binary files. */
    Compartment_Report_Reader(const Report_Specification &specs);

    Compartment_Report_Reader();

    //! Opens a report.
    /*!
     This function inherits from Report_Reader, see Report_Reader
     for a detailed documentation of this function.
     \sa Report_Reader::open()
     */
    virtual void open()
        /* throw (Bad_Data, IO_Error)*/;

    //! Open the specified report. 
    /*!     
        This function inherits from Report_Reader, see Report_Reader
        for a detailed documentation of this function.
        \sa Report_Reader::open()
    */
    void open(const Report_Specification &specs)
        /* throw (Bad_Data, IO_Error)*/;

    //! \sa Report_Reader::close()
    /*
      Closing the reader frees all resources and invalidates mapping and
      cell target selection.
    */
    virtual void close();

    //! Access report data previously loaded by load_full_report()
    /*! The returned buffer is arranged following the description from
        Compartment_Report_Mapping specs.
        Target selection (\sa cell_target) invalidates the buffer 
        for further access. Consider using frame level access functions if
        thread safety is an issue. */
    inline const T * buffer(Frame_Number framestamp) const;

    //! Access report data previously loaded by load_full_report()
    /*! The returned buffer is arranged following the description from
        Compartment_Report_Mapping specs.
        Target selection (\sa cell_target) invalidates the buffer 
        for further access. Consider using frame level access functions if
        thread safety is an issue. */
    inline const T * buffer(Millisecond timestep) const;

    //! Returns the mapping for the current cell_target selection.
    /*! References to the returned objects must be considered temporal as
        mapping is invalidated by cell target selection (\sa cell_target)
        Use the smart pointer version for thread safety. */
    inline const Compartment_Report_Mapping & mapping() const;

    //! Pointer version of the function above
    /*! The returned mapping can be used safely even after cell target 
        selection. */
    inline const Const_Compartment_Report_Mapping_Ptr & mapping_ptr() const;

    //! Returns the current target of interest
    inline const Cell_Target & cell_target() const;

    //! Set the interest cell target
    /*! Data buffer is invalidated and a new compartment mapping, relative
        to the given cell target, is computed. */
    void cell_target(const Cell_Target & cell_target)
        /* throw (IO_Error) */;

    //! Load the full report into memory
    /*!
        This function inherits from Report_Reader, see Report_Reader
        for a detailed documentation of this function.
        \sa Report_Reader::load_full_report()
     */
    virtual void load_full_report()
        /* throw (IO_Error, Unsupported) */;

    //! Check if frame at specified time is already loaded.
    /*!
        This function inherits from Report_Reader, see Report_Reader
        for a detailed documentation of this function.
        \sa Report_Reader::preloaded().
    */
    virtual bool preloaded(Millisecond timestamp) const;

    /*! Unload report data.
     Any target selection remains valid.
     This function inherits from Report_Reader, see Report_Reader
     for a detailed documentation of this function.
     \sa Report_Reader::unload()
    */
    virtual void unload();

#ifndef SWIG // Work around for SWIG bug related to how inclusion of abstract
             // functions in scope prevents the wrapping of this class
             // constructors
    // Solving partial overriding problems
    using Report_Reader::load_frame;
#endif // SWIG

    //! Load specified frame in the report.
    /*!
     This function inherits from Report_Reader, see Report_Reader
     for a detailed documentation of this function.
     \sa Report_Reader::load_frame(Frame_Number)
     */
    virtual void load_frame(Frame_Number frame_number, Report_Frame & frame)
        /* throw (IO_Error, Unsupported) */;

    //! Loads the next time sample in the report.
    /*! 
        This function inherits from Report_Reader, see Report_Reader
        for a detailed documentation of this function.
        \sa Report_Reader::load_next_frame(Report_Frame & frame) 
     */
    virtual bool load_next_frame(Report_Frame & frame)
        /* throw (IO_Error) */;

    //! Set the current position in the report to the specified frame.
    /*! 
        This function inherits from Report_Reader, see Report_Reader
        for a detailed documentation of this function.
        \todo Should be renamed to position(). (TT)
        \sa Report_Reader::reset(Frame_Number) 
     */
    virtual void reset(Frame_Number framestamp = UNDEFINED_FRAME_NUMBER)
        /* throw (IO_Error, Unsupported) */;

    //! Set the current position in the report to the specified time.
    /*! 
        This function inherits from Report_Reader, see Report_Reader
        for a detailed documentation of this function.
        \todo Should be renamed to position(). (TT)        
        \sa Report_Reader::reset(Millisecond) 
     */
    virtual void reset(Millisecond timestamp)
        /* throw (IO_Error, Unsupported) */;

    //! Set the number of frames to be skipped when advancing to next frame.
    /*!
        This function inherits from Report_Reader, see Report_Reader
        for a detailed documentation of this function.
        \sa Report_Reader::frame_skip(Frame_Number) 
     */
    virtual void frame_skip(double skip_rate)
        /* throw (Unsupported) */;

protected:
    typename Compartment_Report_Frame<T, Allocator>::Context *
    create_context() const;

    virtual void check_reader_compatibility() /* throw (Bad_Data) */ const;

protected:
    typedef boost::shared_ptr<Compartment_Report_Reader_Impl>
        Compartment_Report_Reader_Impl_Ptr;

    Compartment_Report_Reader_Impl_Ptr _reader;
    Compartment_Report_Mapping_Ptr     _mapping;
    Cell_Target                        _current_cell_target;

    boost::shared_array<T>             _buffer;

    Allocator _allocator;
};

template<typename T, typename Allocator>
inline Compartment_Report_Frame<T, Allocator> &
downcast_to_compartment_report_frame(Report_Frame &base_frame);

}

#include "BBP/Model/Experiment/Compartment_Report_Frame.h"
#include "detail/Compartment_Report_Reader_Impl.h"

namespace bbp
{

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
Compartment_Report_Reader<T, Allocator>::Compartment_Report_Reader()
{
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
Compartment_Report_Reader<T, Allocator>::Compartment_Report_Reader
(const Report_Specification &specs) :
    Report_Reader(specs)
{
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void Compartment_Report_Reader<T, Allocator>::open()
{
    this->check_reader_compatibility();
    _reader = 
        Compartment_Report_Reader_Impl::create_reader(this->_specification);
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void Compartment_Report_Reader<T, Allocator>::open
(const Report_Specification &specs)
{
    Report_Reader::open(specs);
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void Compartment_Report_Reader<T, Allocator>::close()
{
    _reader.reset();
    unload();
    _mapping.reset();
    _current_cell_target = Cell_Target();
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
const T * Compartment_Report_Reader<T, Allocator>::buffer
(Frame_Number framestamp) const
{
    return &_buffer[framestamp * _reader->frame_size()];
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
const T * Compartment_Report_Reader<T, Allocator>::buffer
(Millisecond timestamp) const
{
    Frame_Number framestamp = relative_frame_for_timestamp(timestamp);
    return &_buffer[framestamp * _reader->frame_size()];
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
const Compartment_Report_Mapping & 
Compartment_Report_Reader<T, Allocator>::mapping() const
{
    return *_mapping.get();
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
const Const_Compartment_Report_Mapping_Ptr & 
Compartment_Report_Reader<T, Allocator>::mapping_ptr() const
{
    return _mapping;
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
const Cell_Target & 
Compartment_Report_Reader<T, Allocator>::cell_target() const
{
    return _current_cell_target;
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void Compartment_Report_Reader<T, Allocator>::cell_target
(const Cell_Target & cell_target)
{
    bbp_assert(_reader.get() != 0);
    _current_cell_target = cell_target;
    _reader->update_mapping_and_framesize(cell_target);
    _mapping = _reader->mapping();
    unload();
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void Compartment_Report_Reader<T, Allocator>::load_full_report()
{
    bbp_assert(_reader.get() != 0);
    size_t frame_size = _reader->frame_size();
    Frame_Number frame_count =  window_frame_count();
    if (frame_size > 0)
    {
        size_t buffer_size = frame_size * frame_count;
        _buffer.reset(new T[buffer_size]);
    }
    else
        return;
    Frame_Number first_frame = absolute_frame_for_timestamp(_start_time);

    _reader->load_frames(_buffer.get(), 
                         first_frame, first_frame + frame_count - 1);
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
bool Compartment_Report_Reader<T, Allocator>::preloaded
(Millisecond timestamp) const
{
    return (_buffer.get() != 0 &&
            timestamp >= _start_time && 
            timestamp <= _end_time - _specification.timestep());
}


//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void Compartment_Report_Reader<T, Allocator>::unload()
{
    _buffer.reset();
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void Compartment_Report_Reader<T, Allocator>::load_frame
(Frame_Number frame_number, Report_Frame & base_frame)
{
    if (frame_number >= window_frame_count())
    {
        // Out of current interest window bounds.
        throw_exception
            (Bad_Data("Report_Reader::load_frame: frame number "
                      " out of bounds"), SEVERE_LEVEL);
    }

    bbp_assert(_reader.get() != 0);
    if (_buffer.get()) 
    {
        Compartment_Report_Frame<T, Allocator> & final_frame = 
            downcast_to_compartment_report_frame<T, Allocator>(base_frame);
        
        // Creating the context object for the Compartment_Report_Frame
        final_frame._context.reset(create_context());
        // Setting buffer slice for this frame and timestamp.
        T * slice = &_buffer[frame_number * _reader->frame_size()];
        final_frame.frame(boost::shared_array<T>(
           slice, shared_array_slice_deallocator<T>(_buffer)));
        final_frame._timestamp = (frame_number * _specification.timestep() +
                                  _specification.start_time());

    }
    else
    {
        Frame_Number first_frame = absolute_frame_for_timestamp(_start_time);
        
        // If the frame number is out of the bound of the data source an 
        // exception is thrown by jump_to_frame.
        _reader->jump_to_frame(frame_number + first_frame);
        load_next_frame(base_frame);
    }
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
bool Compartment_Report_Reader<T, Allocator>::load_next_frame
(Report_Frame & base_frame)
{
    bbp_assert(_reader.get() != 0);
    Compartment_Report_Frame<T, Allocator> & final_frame = 
        downcast_to_compartment_report_frame<T, Allocator>(base_frame);

    if (_reader->frame_size() == 0)
    {
        // Nothing to load
        log_message("Trying to read a frame for an empty target", 
                    WARNING_LEVEL);
        return false;
    }

    /*! 
	 \todo Should we pass the time window to the reader so this check is 
	 not needed? (Instead we will have code redundancy).
	*/

    Frame_Number last_frame = _reader->current_framestamp();
    if (last_frame != UNDEFINED_FRAME_NUMBER &&
        (last_frame + _reader->frame_skip() <
         absolute_frame_for_timestamp(_start_time) ||
         last_frame + _reader->frame_skip() >
         // absolute_frame_for_timestamp performs clamping so _end_time will
         // be mapped to the real last frame and not one after the last.
         absolute_frame_for_timestamp(_end_time)))
    {
        // Out of current window.
        return false;
    }

    Compartment_Report_Frame<T, Allocator> frame;

    // Allocating buffer
    T * buffer = 0;
    frame.allocate((size_t) _reader->frame_size());
    buffer = frame.frame();

    // Loading the data
    if (!_reader->load_next_frame(buffer))
    {
        return false;
    }

    final_frame = frame;
    final_frame._timestamp =
        _reader->current_framestamp() * _specification.timestep() +
        _specification.start_time();

    // Creating the context object for the Compartment_Report_Frame
    final_frame._context.reset(create_context());

    return true;
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void Compartment_Report_Reader<T, Allocator>::reset(Frame_Number frame_number)
{
    bbp_assert(_reader.get() != 0);
    _reader->reset();

    // Moving to first frame or the user given if possible.
    Frame_Number first_frame = absolute_frame_for_timestamp(_start_time);
    if (frame_number != UNDEFINED_FRAME_NUMBER)
    {
        _reader->jump_to_frame(frame_number + first_frame);
    }
    else
    {
        try
        {
            // We try to jump the begininning of the interest window
            _reader->jump_to_frame(first_frame);
        }
        catch (Unsupported &)
        {
            // Operation wasn't permitted on this data source
            // We don't throw as user doesn't expect any exception in 
            // this case
            //! \todo What do we do with next frame?
        }
    }
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void Compartment_Report_Reader<T, Allocator>::reset(Millisecond timestamp)
{
    Report_Reader::reset(timestamp);
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void Compartment_Report_Reader<T, Allocator>::frame_skip(double skip)
{
    bbp_assert(_reader.get() != 0);
    _reader->frame_skip(skip);
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
void 
Compartment_Report_Reader<T, Allocator>::check_reader_compatibility() const
{
    if (_specification.type() != SOMA_REPORT &&
        _specification.type() != COMPARTMENT_REPORT &&
        _specification.type() != SUMMATION_REPORT
       )
    {
        throw Bad_Data("Invalid report specification for "
                           "Compartment_Report_Reader");
    }
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
Compartment_Report_Frame<T, Allocator> &
downcast_to_compartment_report_frame(Report_Frame & base_frame)
{
    Compartment_Report_Frame<T, Allocator> * frame = 
        dynamic_cast<Compartment_Report_Frame<T, Allocator>*>(& base_frame);
    if (frame == 0)
    {
        throw Unsupported
            (("Compartment_Report_Reader::load_frame: Bad frame type :" +
              std::string(typeid(base_frame).name())).c_str());
    }
    return * frame;
}

//-----------------------------------------------------------------------------

template<typename T, typename Allocator>
typename Compartment_Report_Frame<T, Allocator>::Context *
Compartment_Report_Reader<T, Allocator>::create_context() const
{
    typename Compartment_Report_Frame<T, Allocator>::Context * context =
        new typename Compartment_Report_Frame<T, Allocator>::Context();
    context->_compartment_mapping = _mapping;
    context->_cell_target = _current_cell_target;
    context->_report = _specification;
    context->_report.start_time(_start_time);
    context->_report.end_time(_end_time);

    return context;
}

//-----------------------------------------------------------------------------

}
#endif
