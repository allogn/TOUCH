/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute, 
        Blue Brain Project & Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Responsible author:     Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_FILTER_WRITER_H
#define BBP_FILTER_WRITER_H

#include "BBP/Common/Dataset/Containers/Buffer.h"
#include "BBP/Common/Filter/Filter.h"
#include "BBP/Common/Filter/Pipeline.h"
#include "BBP/Common/Filter/Filter_Data.h"
#include "BBP/Common/System/Time/Sleep.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Writes elements of a stream to a memory buffer.
/*!
    \ingroup Pipeline
*/
template <typename Input>
class Filter_Writer
    : public virtual Filter < Input, Buffer <Input> >
{
    friend class Pipeline;
    friend class Filter_Data_Interface;

public:

    Filter_Writer();
    //! deconstructor
    virtual ~Filter_Writer();
    
    //! writes the next object to the buffer
    /*!
        \todo This should go protected and triggered by start(). (TT)
    */
    inline virtual void process();

    //! tell process to start
    inline void start();
    
    //! tell the process to pause
    inline void pause();
    
    //! tell the process to stop (does not clear buffer)
    inline void stop();

//#ifdef BBP_THREADS_SUPPORTED
    //! Aborts the write function.
    // Default implementation is interrupting the writing to the buffer.
    // Only writes that use write_interruption can be really interrupted.
    // This is the default cancel_process method attached to the internal
    // thread to interrupt its loop but it can be safely overrided.
    virtual void cancel_write();
//#endif

    /*! Sets the operation mode for when the end of stream is reached.
        In STOP_PIPELINE mode a End_Of_Stream exception is thrown, which, 
        eventually, stops the whole pipeline.
        In WAIT_FOR_NEW_DATA mode, process blocks on the buffer read until
        some external agent resets the reader to a state where it can
        output data.
    */
    //inline void set_end_of_stream_mode(End_Of_Stream_Mode mode);

protected:

//#ifdef BBP_THREADS_SUPPORTED
    // The variables related to the buffer have to be destroyed before the
    // thread since they may be used indirectly during thread destruction 
    // when cancel_read is called
    std::auto_ptr<typename Buffer<Input>::Interruption> _read_interruption;
    std::auto_ptr<typename Buffer<Input>::Interruption> _write_interruption;
    Process_Control_Thread <Filter_Writer>              _thread;
//#endif
    //End_Of_Stream_Mode    eos_mode;

    //! Read a new object from a source and writes it into the internal buffer
    // Implementators are suggested to use write_interruption while writing
    // into the buffer.
    virtual void write();

    ///*! Function called to check if the internal thread has reached the
    //     end of its data stream.
    //     This function is called from process, when it returns true
    //     End_Of_Stream is thrown.
    //   */
    //   virtual bool stream_finished() = 0;
};


// ----------------------------------------------------------------------------

template <typename Input>
Filter_Writer<Input>::Filter_Writer()
//: eos_mode(STOP_PIPELINE)
{
//#ifdef BBP_THREADS_SUPPORTED
    _write_interruption.reset(this->output().create_write_interruption());
    _read_interruption.reset(this->output().create_read_interruption());
//#endif
}

// ----------------------------------------------------------------------------

template <typename Input>
Filter_Writer<Input>::~Filter_Writer()
{
//#ifdef BBP_THREADS_SUPPORTED
    bbp_assert(_thread.state() == STREAM_STOPPED);
//#endif
}

// ----------------------------------------------------------------------------

template <typename Input>
std::ostream & operator << (std::ostream & lhs, Filter_Writer<Input> & rhs)
{
    lhs << rhs.name() << std::endl;

    // input
    if (rhs.input.is_connected() == true)
    {
        lhs << "Input: " << * rhs.input;
    }
    else
    {
        lhs << "Input: -";
    }

    return lhs;
}

// ----------------------------------------------------------------------------

template <typename Input>
void Filter_Writer<Input>::process()/* throw (End_Of_Stream)*/
{
    //// Checking if the end of the stream was reached inside read
    //if (stream_finished() == true && eos_mode == STOP_PIPELINE)
    //{
    //    throw End_Of_Stream();
    //}

    try
    {
//#ifdef BBP_THREADS_SUPPORTED
        this->output().write(this->input(), _write_interruption.get());
//#else 
//      this->output().write(this->input());
//#endif
    }
    catch (...)
    {
        std::cerr << "Exception caught in Filter_Writer::process"
                  << std::endl;
    }
}

// ----------------------------------------------------------------------------

template <typename Input>
inline void Filter_Writer<Input>::start()
{
    if (this->stream_state != STREAM_STARTED) 
    {
//#ifdef BBP_THREADS_SUPPORTED
        // attach the stream processing to the thread if not yet done
        if (_thread.is_attached() == false)
        {
            _thread.attach(this, & Filter_Writer::write, 
                           & Filter_Writer::cancel_write);
        }
        
        if (Filter_Writer<Input>::is_configured() == true)
        {
            _thread.start();
        }
//#endif
        //! \todo Shouldn't this not  be set if is_configured == false? (TT)
        this->stream_state = STREAM_STARTED;
    }
}

// ----------------------------------------------------------------------------

template <typename Input>
inline void Filter_Writer<Input>::pause()
{
    if (this->stream_state == STREAM_STARTED) 
    {
//#ifdef BBP_THREADS_SUPPORTED
        _thread.pause();
//#endif
        this->stream_state = STREAM_PAUSED;
    }
}

// ----------------------------------------------------------------------------

template <typename Input>
inline void Filter_Writer<Input>::stop()
{
    if (this->stream_state != STREAM_STOPPED)
    {
//#ifdef BBP_THREADS_SUPPORTED
        // Stop internal thread
        _thread.stop();
        // Cancel reads and writes to buffer.
        _read_interruption->signal();
        _write_interruption->signal();
        // Sync with thread
        _thread.wait_for_suspend();
        //// Safely clearing the buffer.
        //this->output->clear();
//#endif
        this->stream_state = STREAM_STOPPED;
    }
}

// ----------------------------------------------------------------------------

//#ifdef BBP_THREADS_SUPPORTED
template <typename Input>
void Filter_Writer<Input>::cancel_write()
{ 
    _read_interruption->signal();
}
//#endif

// ----------------------------------------------------------------------------

template <typename Input>
void Filter_Writer<Input>::write()
{
//#ifdef BBP_THREADS_SUPPORTED
    sleep(1000.0);
//#endif
}

// ----------------------------------------------------------------------------

}
#endif
