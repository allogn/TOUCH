/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute
        Blue Brain Project & Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Responsible author:     Thomas Traenkler
        Contributing authors:   Juan Bautista Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_FILTER_READER_H
#define BBP_FILTER_READER_H

#include "BBP/Common/Dataset/Containers/Buffer.h"
#include "BBP/Common/System/Time/Sleep.h"

#include "Filter.h"
#include "Pipeline.h"
#include "Filter_Data.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Reads elements of a stream from a memory buffer.
/*!
  All public functions provided are thread-safe.
  \ingroup Pipeline
*/
template <typename Output>
class Filter_Reader
    : public virtual Filter < Buffer <Output> , Output >
{
    friend class Pipeline;
    friend class Filter_Data_Interface;

public:

    Filter_Reader();

    /*! Derived classes must guarantee that the internal thread is not
        running after their destructor finished because, among other reasons,
        read is abstract (and calling it after the derived destructor has 
        executed has undefined behaviour). The easiest way is to stop the
        internal thread (stop and then wait_for_suspend) in their
        destructor. */
    virtual ~Filter_Reader();

    //! Reads the next object from buffer.
    /** \cond SHOW_IN_DEVELOPER_REFERENCE */
    /* \todo: This should go protected and triggered by start(). (TT)
       Answer (JH): Not really, start starts the thread that calls read()
       If a Filter object is going to have a thread then the design has
       to be reviewed. A Filter_Reader could be an object with empty input
       and a buffer output. That will remove read() and replace it with 
       process(), which could be protected then. Currently an external thread
       is calling process so it cannot be protected. */
    /** \endcond SHOW_IN_DEVELOPER_REFERENCE */
    inline virtual void process() /*throw (End_Of_Stream)*/;

    //! Sets up internal thread and starts it if filter configured
    /*!
      \bug Currently this function may block filling the first buffer
      position though non blocking behaviour is more desirable.
    */
    inline virtual void start();

    //! Pauses internal thread
    /*! Thread is paused, no blocked call is interrupted and buffer is
        left untouched. 

        Non blocking call. */
    inline virtual void pause();

    //! Stops internal thread
    /*! Thread is stopped, blocked calls to process are interrupted 
        (so a pipeline thread can also be stopped) and buffer is cleared.

        Non blocking call.

        \cond SHOW_IN_DEVELOPER_REFERENCE
        Note for developers: It is unsafe to call this function from read()
        it will lead to a deadlock or a race condition accessing the buffer.
        \endcond SHOW_IN_DEVELOPER_REFERENCE */
    inline virtual void stop();

//#ifdef BBP_THREADS_SUPPORTED
    //! Aborts the read function.
    /*! Default implementation is interrupting the writing to the buffer.
        Only writes that use write_interruption can be really interrupted.
        This is the default cancel_process method attached to the internal
        thread to interrupt its loop but it can be safely overrided. 

        Non blocking call. */
    virtual void cancel_read_input();

    //! Sets the operation mode for when the end of stream is reached.
    /*  In STOP_PIPELINE mode a End_Of_Stream exception is thrown, which, 
        eventually, stops the whole pipeline.
        In WAIT_FOR_NEW_DATA mode, process blocks on the buffer read until
        some external agent resets the reader to a state where it can
        output data. 

        Non blocking call. */
    inline void set_end_of_stream_mode(End_Of_Stream_Mode mode);
//#endif

protected:

//#ifdef BBP_THREADS_SUPPORTED
    //! Read a new object from a source and writes it into the internal buffer
    /*! Implementators are suggested to use write_interruption while writing
        into the buffer.
        this is a dummy function usually overridden by derived classes */
    //! \todo do we need a default implementation?.
    virtual void read_input()
    {
	    // without derived classes overriding the default behavior
	    // the buffer is assumed to be full at the beginning and 
	    // thus the end of stream is reached once it is empty
	    // so we set the flag here
	    _end_of_stream = true;

	    // do nothing (can be overridden in derived classes)
	    sleep(1000.0);
    }
//#endif

    //! Returns true if the stream has reached the end and the buffer is empty.
    /*  This function is called from process, when eos_mode is STOP_PIPELINE
        and this function returns true process will throw End_Of_Stream, 
        otherwise it will get blocked reading from the buffer. */
    inline virtual bool stream_finished() { 
        return _end_of_stream && this->input().is_empty();
    }

protected:
//#ifdef BBP_THREADS_SUPPORTED
    // This mutex has been made recursive to minimize the risk of
    // trivial deadlocks inside derived classes.
    boost::recursive_mutex  _command_mutex;

    boost::condition        _in_command_sequence_condition;
//#endif
    bool                    _in_stop_sequence;

//#ifdef BBP_THREADS_SUPPORTED
    // The variables related to the buffer have to be destroyed before the
    // thread since they may be used indirectly during thread destruction 
    // when cancel_read_input is called
    std::auto_ptr<typename Buffer<Output>::Interruption> _read_interruption;
    std::auto_ptr<typename Buffer<Output>::Interruption> _write_interruption;
    Process_Control_Thread<Filter_Reader>                _thread;
//#endif

    End_Of_Stream_Mode                                   _eos_mode;
    /** \brief whether reading position needs to be advanced before reading 
        next element */
    bool _advance_pending;

    //! To be used by derived classes to indicate end of stream reached.
    // This variable mustn't be written by Filter_Reader, but only read.
    bool _end_of_stream;
};


template <typename Output>
std::ostream & operator << (std::ostream & lhs, Filter_Reader<Output> & rhs);


// ----------------------------------------------------------------------------

template <typename Output>
Filter_Reader<Output>::Filter_Reader() 
  : _in_stop_sequence(false),
    _eos_mode(STOP_PIPELINE),
    _advance_pending(false),
    _end_of_stream(false)
{
}

// ----------------------------------------------------------------------------

template <typename Output>
Filter_Reader<Output>::~Filter_Reader() 
{
//#ifdef BBP_THREADS_SUPPORTED
    bbp_assert(_thread.state() == STREAM_STOPPED);
//#endif
}

// ----------------------------------------------------------------------------

template <typename Output>
std::ostream & operator << (std::ostream & lhs, Filter_Reader<Output> & rhs)
{
    lhs << rhs.name() << std::endl;

    // output
    if (rhs.output.is_connected() == true)
    {
        lhs << "Output: " << * rhs.output;
    }
    else
    {
        lhs << "Output: -";
    }
    return lhs;
}

// ----------------------------------------------------------------------------

template <typename Output>
void Filter_Reader<Output>::process() /*throw (End_Of_Stream)*/
{
    //! \todo study use cases and improve thread safety of this function.
    
    {
//#ifdef BBP_THREADS_SUPPORTED
        boost::recursive_mutex::scoped_lock command_lock(_command_mutex);
//#endif

        // Removing from buffer last object used if needed
        if (_advance_pending)
        {
            // This operation won't do anything if the buffer 
            // was cleared by stop.
            this->input().advance_read_position();
        }

        // Checking if the end of the stream was reached inside read
        if (stream_finished() == true && _eos_mode == STOP_PIPELINE)
        {
            log_message("Filter_Reader::process() finished.", DEBUG_LEVEL);
            throw_exception(End_Of_Stream("End of stream inside Filter_Reader"),
                            DEBUG_LEVEL, __FILE__, __LINE__);
        }
    }

    try
    {
//#ifdef BBP_THREADS_SUPPORTED
        // This read is blocking and may be cancelled by stop
        this->output() = this->input().read(_read_interruption.get());
//#else
//      this->output() = this->input().read(0);
//#endif
        _advance_pending = true;
    }
    catch (typename Buffer<Output>::Buffer_Interruption &)
    {
        // Read was interrupted don't have to advance next call.
        _advance_pending = false;

        if (stream_finished()) {
            // Throw the end of stream exception expected by caller.
            log_message("Filter_Reader::process() finished.", DEBUG_LEVEL);
            throw_exception(End_Of_Stream("End of stream inside Filter_Reader"), 
                            DEBUG_LEVEL, __FILE__, __LINE__);
        } else {
            // Unexpected interruption
            throw_exception(Stream_Interruption("Filter_Reader interrupted"), 
                            DEBUG_LEVEL, __FILE__, __LINE__);
        }
    }
    catch (...)
    {
        // Unkown exception caught, don't have to advance next call.
        _advance_pending = false;
        throw;
    }

}

// ----------------------------------------------------------------------------

template <typename Output>
void Filter_Reader<Output>::start()
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock command_lock(_command_mutex);

    log_message("Filter_Reader::start() start called.", DEBUG_LEVEL);
    while (_in_stop_sequence)
    {
        log_message("Filter_Reader::start() waiting for stop to finish.",
                    DEBUG_LEVEL);

        _in_command_sequence_condition.wait(command_lock);
    }
//#endif

    // If stream is not started stream_finished cannot change its value
    if (this->stream_state != STREAM_STARTED && !stream_finished())
    {
        /*! \bug Creating this objects every time start is called was causing
          deadlocks before the mutex between start, stop and pause existed.
          It should be checked what can happen now. */
        log_message("Filter_Reader::start() creating interruption objects.",
                    DEBUG_LEVEL);
//#ifdef BBP_THREADS_SUPPORTED
        _read_interruption.reset(this->input().create_read_interruption());
        _write_interruption.reset(this->input().create_write_interruption());

        // attach the stream processing to the thread if not yet done
        if (_thread.is_attached() == false)
        {
            _thread.attach(this, & Filter_Reader::read_input, 
                           & Filter_Reader::cancel_read_input);
        }
//#endif
        
        if (Filter_Reader<Output>::is_configured() == true)
        {
            log_message("Filter_Reader::start() commanding thread to start",
                        DEBUG_LEVEL);
            this->stream_state = STREAM_STARTED;
//#ifdef BBP_THREADS_SUPPORTED
            _thread.start();
//#endif
        }

        // Checking if the end of the stream was previously reached inside read
        // In that case, if the end of stream mode is STOP_PIPELINE we won't 
        // start the stream reader again.
        if (stream_finished() == true && _eos_mode == STOP_PIPELINE)
        {
            this->stream_state = STREAM_STOPPED;
        }
    }

    /*! \bug Buffer initialization should be solved somehow 
       differently. This read was giving problems to RTNeuron in 
       the original version, and right now is causing a deadlock between
       the buffer read and read_input due to the locks held by 
       Compartment_Report_Stream_Reader::start() (JH) */
    //if (this->stream_state != STREAM_STOPPED)
    //{
    //    try
    //    {
    //        // This statement is blocking.
    //        * this->output = this->input->read(_read_interruption.get());
    //    }
    //    catch (...)
    //    {
    //        // Read was interrupted don't have to advance next call.
    //        _advance_pending = false;
    //    }
    //}
}

// ----------------------------------------------------------------------------

template <typename Output>
void Filter_Reader<Output>::pause()
{
    // This function can overlap safely with stop. Note that that only happens
    // when stop if blocked inside wait_for_stop. In the end the stream state
    // will be stopped which is what we expect from both: pause(); stop(); and
    // stop(); pause(); if each call was atomic.
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock command_lock(_command_mutex);
//#endif

    log_message("Filter_Reader::pause() pause called.", DEBUG_LEVEL);
    if (this->stream_state == STREAM_STARTED) 
    {
        log_message("Filter_Reader::pause() command thread pause.", 
                    DEBUG_LEVEL);
//#ifdef BBP_THREADS_SUPPORTED
        _thread.pause();
//#endif
        this->stream_state = STREAM_PAUSED;
    }
}

// ----------------------------------------------------------------------------

template <typename Output>
void Filter_Reader<Output>::stop()
{
//#ifdef BBP_THREADS_SUPPORTED
    // Check for threding issues. To facilitate debugging in release mode
    // we will leave this condition always enabled.
    if (_thread.current())
    {
        std::cerr << "Filter_Reader::stop() mustn't be called within"
            " Filter_Reader::read(). Aborting" << std::endl;
        abort();
    }

    boost::recursive_mutex::scoped_lock command_lock(_command_mutex);

    while (_in_stop_sequence)
    {
        // We don't allow several stop sequences to overlap.
        log_message("Filter_Reader::stop() waiting for another stop to finish.",
                    DEBUG_LEVEL);
        _in_command_sequence_condition.wait(command_lock);
    }
    
    _in_stop_sequence = true;

    log_message("Filter_Reader::stop() called.", DEBUG_LEVEL);
    if (this->stream_state != STREAM_STOPPED)
    {
        // Stop internal thread
        _thread.stop();
        log_message("Filter_Reader::stop() command thread stop.", DEBUG_LEVEL);

        // Cancel reads and writes to buffer.
        // These operations won't block for an undefined amount of time.
        _write_interruption->signal();
        log_message("Filter_Reader::stop() write interrupted.", DEBUG_LEVEL);
        _read_interruption->signal();
        log_message("Filter_Reader::stop() read interrupted.", DEBUG_LEVEL);
        // Synching with thread
        // This call will produce a deadlock if called from read_input
        // Using command_lock here makes possible to call other functions from
        // this interface, those functions can check whether _in_stop_sequence
        // is true and wait for completion. This mechanism is used to minimize
        // deadlock risk.
        _thread.wait_for_suspend(command_lock);

        log_message("Filter_Reader::stop() thread suspended.", DEBUG_LEVEL);
        // Clearing the buffer.
        this->input().clear();
        _advance_pending = false;
        
        this->stream_state = STREAM_STOPPED;
    }

    _in_stop_sequence = false;
    _in_command_sequence_condition.notify_one();
//#else
//    this->stream_state = STREAM_STOPPED;
//#endif
    log_message("Filter_Reader::stop() complete.", DEBUG_LEVEL);
}

//#ifdef BBP_THREADS_SUPPORTED // -----------------------------------------------

template <typename Output>
void Filter_Reader<Output>::cancel_read_input()
{ 
    boost::recursive_mutex::scoped_lock command_lock(_command_mutex);
    _write_interruption->signal();
}

// ----------------------------------------------------------------------------

template <typename Output>
void Filter_Reader<Output>::set_end_of_stream_mode(End_Of_Stream_Mode mode)
{
    boost::recursive_mutex::scoped_lock command_lock(_command_mutex);
    _eos_mode = mode;
}

//#endif // ---------------------------------------------------------------------

}
#endif

