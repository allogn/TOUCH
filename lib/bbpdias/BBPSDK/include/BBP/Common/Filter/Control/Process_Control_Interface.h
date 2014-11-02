/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_PROCESS_CONTROL_INTERFACE_H
#define BBP_PROCESS_CONTROL_INTERFACE_H

namespace bbp {

// ----------------------------------------------------------------------------

//! state of the stream 
enum Stream_State
{
    STREAM_STARTED,
    STREAM_PAUSED,
    STREAM_STOPPED
};

// ----------------------------------------------------------------------------

/*!
    Stream control interface - start, pause, stop, and query state
    
    NOTE: Consider adding slot-signal mechanism. This will improving
    decoupling in the pipeline control logic.

    \ingroup Process_Interface
*/
class Process_Control_Interface
{

public:
    inline Process_Control_Interface();
    inline virtual ~Process_Control_Interface();

    //! start process
    inline virtual void start();
    //! pause process
    inline virtual void pause();
    //! stop process
    inline virtual void stop();
    //! query process state
    inline virtual Stream_State state() const;
    //// check if end of stream has been reached
    // inline virtual bool is_finished() { 
    // if ( stream_state = STREAM_STOPPED ) return true; }
protected:
    //! the state of the thread
    Stream_State        stream_state;
};


// ----------------------------------------------------------------------------

Process_Control_Interface::Process_Control_Interface() 
    : stream_state(STREAM_STOPPED)
{}

// ----------------------------------------------------------------------------

Process_Control_Interface::~Process_Control_Interface()
{}

// ----------------------------------------------------------------------------

Stream_State Process_Control_Interface::state() const 
{
    return stream_state; 
}

// ----------------------------------------------------------------------------

void Process_Control_Interface::start()
{
    stream_state = STREAM_STARTED;
}

// ----------------------------------------------------------------------------

void Process_Control_Interface::pause()
{
    stream_state = STREAM_PAUSED;
}

// ----------------------------------------------------------------------------

void Process_Control_Interface::stop()
{
    stream_state = STREAM_STOPPED; 
}

// ----------------------------------------------------------------------------

}
#endif
