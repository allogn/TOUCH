/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors: Thomas Traenkler
                             Juan Hernando

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENT_REPORT_STREAM_READER_H
#define BBP_COMPARTMENT_REPORT_STREAM_READER_H

#include "BBP/Common/Types.h"
#include "BBP/Common/Filter/Filter_Reader.h"
#include "BBP/Common/Filter/Control/Playback_Control_Interface.h"

#include "BBP/Model/Experiment/Report_Specification.h"
#include "BBP/Model/Experiment/Compartment_Report_Frame.h"
#include "BBP/Model/Experiment/Readers/Compartment_Report_Reader.h"

namespace bbp
{

// ----------------------------------------------------------------------------

//! Streamed compartment report reader class. 
/*!
  This class makes use of Compartment_Report_Reader to get the data from
  the proper source.
  \todo Potential bug: I removed the output_ptr() from here since it seemed
  same to what is already implemented in Filter class and the partial overriding
  causes warnings in Intel C++ compiler. Make sure this is correct - check with 
  Juan why he put it here and what is the fate of the Director class. (TT)
*/
template <typename T,
          typename Allocator = std::allocator<T> >
class Compartment_Report_Stream_Reader :
    public virtual Filter_Reader<Compartment_Report_Frame<T, Allocator> >,
    public virtual Playback_Control_Interface<Millisecond>
{
//    friend class Director ; // to call output_ptr(Stream_Data_Interface * output) 
//    
    typedef Compartment_Report_Frame<T, Allocator>
        Compartment_Report_Frame_Type;
    typedef Filter_Reader<Compartment_Report_Frame_Type> 
        Filter_Reader_Type;
    typedef Compartment_Report_Reader<T, Allocator>
        Compartment_Report_Reader_Type;

public:
    //! Creates a streamed reader from the given report specification.
    inline Compartment_Report_Stream_Reader(const Report_Specification &specs,
                                            Buffer_Size size = 20);
    //! Destructor is implemented in derived classes.
    inline virtual ~Compartment_Report_Stream_Reader();

    //! Start the reader and buffer filling.
    inline virtual void start();

    //! Stop the reader.
    inline virtual void stop();

    //! Pause the reader until resumed with start.
    inline virtual void pause();

    //! Close report and free memory buffers.
    inline void close();

    //! Sets the cell target for reading.
    inline void cell_target(const Cell_Target & cell_target);

    //! \sa Filter_Reader::process
    inline virtual void process();

    const Report_Specification &specs();

    inline Compartment_Report_Stream_Reader<T, Allocator> & operator +
        (Filter_Data_Interface & data)
    {
        return dynamic_cast<Compartment_Report_Stream_Reader<T, Allocator> &>
            (Filter_Reader<Compartment_Report_Frame<T, Allocator> >::operator+
                (data));
    }
#ifndef SWIG
    using Filter_Reader<Compartment_Report_Frame<T, Allocator> >::operator+;
#endif

public :
    //// Assign the filter output pointer.
    ///*
    //    \sa Filter \sa Pipeline
    //    \todo May be protected?
    // */
    //inline virtual void output_ptr(Filter_Data_Interface * output);


protected:
    //! Fills the simulation data memory buffer from the binary file. 
    /*!
      This function must be called only from the internal Filter_Reader loop.
    */
    virtual void read_input();

    //! Helper function used inside read_input.
    void update_reader();
    
protected:
    //! \sa Playback_Control_Interface::mode_impl
    inline void mode_impl(Playback_Mode mode);

    //! \sa Playback_Control_Interface::time_impl
    inline void time_impl(Millisecond time);

    //! \sa Playback_Control_Interface::frame_impl
    inline void frame_impl(Frame_Number frame);
    
    //! \sa Playback_Control_Interface::speed_impl
    inline void speed_impl(Millisecond frame);

    //! \sa Playback_Control_Interface::frames_per_second_impl
    inline void frames_per_second_impl(Frame_Number frame);

    //! \sa Playback_Control_Interface::frames_per_second_impl
    inline void time_window_impl(Millisecond start, Millisecond end);
    
    Compartment_Report_Reader_Type _reader;

    /*! Flag raised by time_impl and frame_impl to indicate that the reader
        must jump to _next_frame_number. */
    bool _jump_to_next;
    /*! Flag raised by speed_impl and frames_per_second_impl to indicate that 
        the reader must update the frameskip rate. */
    bool _update_speed;
    /*! Flag raised by time_window_impl to indicate that the reader must update
        the time window. */
    bool _update_window;

    /*! Flag raised by cell_target to indicate that the reader must update
        its cell target. */
    bool _update_cell_target;

    Cell_Target _cell_target;
};


// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
Compartment_Report_Stream_Reader <T, Allocator>::
Compartment_Report_Stream_Reader(const Report_Specification & specs, 
                                 Buffer_Size size) :
    
    _jump_to_next(false),
    _update_speed(false),
    _update_window(false),
    _update_cell_target(false)
{
    _reader.open(specs);
    this->input().resize(size);

    set_absolute_times(_reader.specs().timestep(),
                       _reader.start_time(), _reader.end_time());
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
// Don't split this line, some SWIG version complains about a parse error
Compartment_Report_Stream_Reader<T, Allocator>::~Compartment_Report_Stream_Reader()
{
    stop();
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::start()
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(this->_command_mutex);
//#endif
    // Final overriding
    if (_cell_target.size() == 0)
    {
        throw_exception(
          runtime_error("Compartment_Report_Stream_Reader: No cell target has"
                        " been specified"), FATAL_LEVEL);
    }
    Filter_Reader_Type::start();
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::stop()
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(this->_command_mutex);
//#endif
    // Final overriding.
    Filter_Reader_Type::stop();
    Playback_Control_Interface<Millisecond>::stop();
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::pause()
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(this->_command_mutex);
//#endif
    // Final overriding.
    Filter_Reader_Type::pause();
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::close()
{
    _reader.close();
    this->input().clear();
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::cell_target
(const Cell_Target & target)
{
    _cell_target = target;
    _update_cell_target = true;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::mode_impl
(Playback_Mode mode)
{
    /*! \todo Why is a parameter passed when not used? (TT) Because this 
        function is called from the base class for the derived classes to
        perform additional operations, so the derived class may or may not
        use the parameter at its own will. The base class updates the attribute
        once this call is finished so it passes the value in case the derived
        class needs it (which doesn't happen here).
        The key point is that this class contains a thread, so, for
        thread safety instead of updating the status of the reader here, I 
        just flag what has to be done and the update will be done consistently
        in a single mutual exclusion block just before the next read proceeds. 
        The problem is that unsupported operations will be detected by
        the inner thread and not here. */
    //! \todo throw exception if unsupported
    _update_speed = true;
    this->_end_of_stream = false;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::time_impl
(Millisecond time)
{
    /*! \todo Why is a parameter passed when not used? (TT) The same as 
        before. */
    //! \todo throw exception if unsupported
    _jump_to_next = true;
    this->_end_of_stream = false;    
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::frame_impl
(Frame_Number frame)
{
    /*! \todo Why is a parameter passed when not used? (TT) The same as 
        before. */
    //! \todo throw exception if unsupported
    _jump_to_next = true;
    this->_end_of_stream = false;
}

// ----------------------------------------------------------------------------
    
template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::speed_impl
(Millisecond speed_factor)
{
    /*! \todo Why is a parameter passed when not used? (TT) The same as 
        before. */
    //! \todo throw exception if unsupported
    _update_speed = true;
    this->_end_of_stream = false;    
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::frames_per_second_impl
(Frame_Number frames_per_second)
{
    /*! \todo Why is a parameter passed when not used? (TT) The same as 
        before. */
    //! \todo Why is a parameter passed when not used? (TT)
    _update_speed = true;
    this->_end_of_stream = false;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::time_window_impl
(Millisecond start, Millisecond end)
{
    /*! \bug Why is a parameter passed when not used? (TT) The same as 
        before. */
    _update_window = true;
    this->_end_of_stream = false;
}

// ---------------------------------------------------------------------------- 

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::process()
{
    try
    {
        Filter_Reader_Type::process();

//#ifdef BBP_THREADS_SUPPORTED
        boost::recursive_mutex::scoped_lock command_lock(this->_command_mutex);
//#endif

        // Updating current time using the output from the reader.
        _current_time = this->output().time();
        _current_frame_number = (Frame_Number)
            round((_current_time - _absolute_start_time) /
                  _frame_timestep);
    } 
    catch (...)
    {
        throw;
    }
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
const Report_Specification & 
Compartment_Report_Stream_Reader <T, Allocator>::specs()
{
    return _reader.specs();
}


// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::read_input()
{
    // Getting the reference to the buffer line to write.
    Compartment_Report_Frame<T> frame;

    update_reader();

    bool end_of_stream;
    try
    {
        end_of_stream = !_reader.load_next_frame(frame);        
    }
    catch (std::exception & e)
    {
        std::cerr << "Compartment_Report_Stream_Reader:read_input() "
                     "exception caught. " << e.what() << std::endl;
        // Exception generated inside read_frame.
        this->_end_of_stream = true;
        // Suspending this thread. When process() empties the buffer and
        // no looping has been requested
        this->pause();
        
        return;
    }

    // It doesn't matter if the user tries to jump now, we can always
    // assume that this function finished before.    
    if (!end_of_stream)
    {
        try
        {
//#ifdef BBP_THREADS_SUPPORTED
            this->input().write(frame, this->_write_interruption.get());
//#else
//            this->input().write(frame);
//#endif
        }
        catch (typename Buffer<Compartment_Report_Frame<T> >::
               Buffer_Interruption &)
        {
            /*! \bug A frame can be missed here, the reader should go one
                step backwards to avoid it. We can also store it internally
                until next read_input. If the user calls any function that
                makes the reader jump to another frame, we can get rid of
                the frame. */
#ifndef NDEBUG
            std::cout << "Compartment_Report_Stream_Reader interrupted" 
                      << std::endl;
#endif
        }
    }
    else 
    {
        if (_playback_repeat)
        {
            // Resseting reader to the end or the beggining 
            // depending on the sign of _playback_speed
            try 
            {
                if (_playback_speed > 0)
                    _reader.reset();
                else
                    _reader.reset(_end_frame_number - _start_frame_number);
                end_of_stream = false;
            }
            catch (...)
            {
                // Unsupported operation
                end_of_stream = true;
            }
        }

        if (end_of_stream)
        {
            this->_end_of_stream = true;
            // Suspending this thread.
            // process() will call stop once the buffer is  empty
            this->pause();
            // but first we need to interrupt any possibly blocked read.
//#ifdef BBP_THREADS_SUPPORTED
            this->_read_interruption->signal();
//#endif
            // If some tries to restart the reader without changing
            // the _playback_repeat flag or jumping to another frame
            // it will pause again as _reader.load_next_frame is
            // expected to return false again.
        }
    }
}

// ----------------------------------------------------------------------------

//template <typename T, typename Allocator>
//void Compartment_Report_Stream_Reader <T, Allocator>::
//output_ptr(Filter_Data_Interface * output)
//{
//    Filter_Data<Compartment_Report_Frame<T, Allocator> > * temp = 
//        dynamic_cast<Filter_Data<Compartment_Report_Frame<T, Allocator> > *> 
//        (output);
//    if (temp != 0)
//    {
//        this->output = * temp;
//        this->output.connected = true;
//        /*!
//            \todo We don't have a context object for the moment, check what
//            happens with this.
//        */
//            //this->output->context_pointer = temp->
//    }
//    else
//    {
//        throw Stream_Data_Incompatible();
//    }
//}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Stream_Reader <T, Allocator>::update_reader()
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(this->_playback_command_mutex);
//#endif
    if (_jump_to_next)
    {
        if (_next_frame_number == UNDEFINED_FRAME_NUMBER)
            _reader.reset();
        else
            _reader.reset(_next_frame_number - _start_frame_number);
        this->input().clear();
        _jump_to_next = false;
    }

    if (_update_speed)
    {
        switch (_playback_mode)
        {
        case PROCESS_SINGLE_STEP_TRIGGER:
//#ifdef BBP_THREADS_SUPPORTED
        case PROCESS_FRAME_BASED:
//#endif
            _reader.frame_skip(_playback_speed);
            break;
//#ifdef BBP_THREADS_SUPPORTED
        case PROCESS_TIME_BASED:
        case PROCESS_REALTIME_BASED:
            // Time based advance.
            // (1000 / delta) = frames in a second
            // (fps / f) = frames to process in a second
            // (1000 / delta) / (fps / f) = frame rate skip
            _reader.frame_skip
                (_playback_speed * 1000.0 / (_playback_framerate * 
                                             _reader.specs().timestep()));
//#endif
        }
        _update_speed = false;
    }

    if (_update_window)
    {
        // Notice that the time window has already been rounded by the base 
        // class, so the reader discretization won't make any modification 
        // to the time values.
        _reader.time_window(_start_time, _end_time);
        _update_window = false;
    }

    bbp_assert(_cell_target.size() != 0);
    if (_update_cell_target)
    {
        _reader.cell_target(_cell_target);
        _update_cell_target = false;
    }
}

}
#endif
