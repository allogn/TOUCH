/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible author:     Juan Bautista Hernando Vieites
        Contributing author:    Thomas Traenkler

*/


#ifndef BBP_PROCESS_CONTROL_THREAD_H
#define BBP_PROCESS_CONTROL_THREAD_H

//#ifdef BBP_THREADS_SUPPORTED

//#define DEBUG_THREAD

#include <iostream>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>

#include "Control/Process_Control_Interface.h"


namespace bbp {


// ----------------------------------------------------------------------------

/*!
    Stream thread runs the processing loop in a separate thread. Should not
    be inherited from (!) to avoid deconstructor issues.

    All functions are thread safe.

    This class may be inherited mainly to provide specializations of 
    wait_for_suspend implementation.
*/
template <typename T>
class Process_Control_Thread
    : public virtual Process_Control_Interface
{

public:
    inline Process_Control_Thread();
    inline Process_Control_Thread(const Process_Control_Thread & rhs);
    inline ~Process_Control_Thread();

    //! Tell process to start.
    /*! This function will block until all calls blocked inside 
        wait_for_stop finish. Whether the thread is already processing
        or not is not guaranteed at the moment this function returns. */
    inline virtual void start();

    //! Tell the process to pause
    /*! This call is not blocking. For being sure that the thread is stopped
        wait_for_suspend has to be called.
        Between pause and wait_for_suspend the process method can be cancelled
        externally to avoid dead-locks. */
    inline virtual void pause();

    //! Tell the process to stop. 
    /*! This call is not blocking. For being sure that the thread is stoped
        wait_for_suspend has to be called.
        Between stop and wait_for_suspend the process method can be cancelled
        externally to avoid dead-locks. */
    inline virtual void stop();

    //! Waits for this thread to be suspended (paused or stopped).
    /*! If already suspended or thread not created it returns inmediately. */
    inline void wait_for_suspend();

    //! Waits for this thread to be suspended (paused or stopped).
    /*! If already suspended or thread not created it returns inmediately.
        @param lock An object modelling the Scoped_Lock boost concept
               When provided this call releases this lock at the moment it
               blocks for waiting. */
    template <typename Scoped_Lock>
    inline void wait_for_suspend(Scoped_Lock & lock);

    //! Waits for this thread to be stopped.
    /*! If already stopped or thread not created it returns inmediately. */
    inline void wait_for_stop();

    //! Waits for this thread to be stopped.
    /*! If already stopped or thread not created it returns inmediately.
        @param lock An object modelling the Scoped_Lock boost concept
               When provided this call releases this lock at the moment it
               blocks for waiting. */
    template <typename Scoped_Lock>
    inline void wait_for_stop(Scoped_Lock & lock);

    //! attach a stream process to be run by this thread
    /* @param object The object which will be called
       @param process The processing method from the object to be called each
       iteration of the thread loop.
       @param cancel_process A method to be called for cancelling process
       when and only when the thread is to be destroyed. */
    inline void attach(T* object, 
                       void(T::*process)(), void(T::*cancel_process)());

    //! check if stream process is attached to this thread
    inline bool is_attached() const;

    /*! Returns true if and only if the calling thread is the same as the 
        internal one */
    bool current() const;

    //! assignment between Process_Control_Thread objects is not allowed
    Process_Control_Thread & operator = (const Process_Control_Thread & rhs) { return * this; }

private:
    //! thread worker loop that runs the stream process
    inline void loop();

    template <typename Scoped_Lock>
    inline void wait_for_suspend_impl(Scoped_Lock & lock, bool until_stopped);

private:
    void                      (T::* _process)();
    void                      (T::* _cancel_process)();

    T                             * _object;

    std::auto_ptr<boost::thread>    _thread;

    // synchronization objects
    boost::mutex                    _command_mutex;
    boost::condition                _processing_loop_block;
    boost::condition                _wait_for_suspend_block;
    boost::condition                _wait_for_start_block;

    //! flag indicating thread is told to exit
    bool                            _exit;
    bool                            _suspended;

    //! counter for waiting processes. 
    /*! When this counter is not zero it inhibits the restarting of the 
        thread until cleared. */
    unsigned int                    _pending_waits;



};


// ----------------------------------------------------------------------------

template <typename T>
inline Process_Control_Thread<T>::Process_Control_Thread() :
    _process(0),
    _cancel_process(0),
    _exit(false),
    _suspended(false),
    _pending_waits(0)
{
}

// ----------------------------------------------------------------------------

template <typename T>
inline Process_Control_Thread<T>::Process_Control_Thread
    (const Process_Control_Thread<T> & rhs __attribute__((unused))) :
    _process(0),
    _cancel_process(0),
    _exit(false),
    _suspended(false),
    _pending_waits(0)
{
}

// ----------------------------------------------------------------------------

template <typename T>
inline Process_Control_Thread<T>::~Process_Control_Thread()
{
    {
#if !defined NDEBUG && defined DEBUG_THREAD
        std::cerr << "Cancelling thread " << this << std::endl;
#endif
        boost::mutex::scoped_lock lock(_command_mutex);
        
        // Trigger exit of thread processing loop
        _exit = true;
        // Unblock processing loop so it can exit
        _processing_loop_block.notify_all();
        
        // If we have a cancel_process, we call it to interrupt a 
        // possible block inside process funtion.
        if (_cancel_process)
        {
            bbp_assert(_object != 0);
            (_object->*_cancel_process)();
        }
    }

    // if thread is running, wait for it to exit
    if (_thread.get() != 0)
    {
#if !defined NDEBUG && defined DEBUG_THREAD
        std::cerr << "Joining thread " << this << std::endl;
#endif
        _thread->join();
    }
#if !defined NDEBUG && defined DEBUG_THREAD
        std::cerr << "Thread " << this << " exited" << std::endl;
#endif
}

// ----------------------------------------------------------------------------

template <typename T>
inline void Process_Control_Thread<T>::start()
{
    boost::mutex::scoped_lock lock(_command_mutex);

    if (_process == 0)
    {
        throw std::logic_error
            ("Error: No stream process connected to this thread.");
    }

    // create thread if necessary
    if (_thread.get() == 0)
    {
        _thread.reset(new boost::thread(
            boost::bind(& Process_Control_Thread::loop, this)));
#ifndef NDEBUG
        std::clog << "New thread created" << std::endl;
#endif
    }

    // if stream not started, start it, otherwise do nothing
    if (stream_state != STREAM_STARTED)
    {
        while (_pending_waits != 0)
        {
            _wait_for_start_block.wait(lock);
        }
        stream_state = STREAM_STARTED;
        _processing_loop_block.notify_all();
    }
}

// ----------------------------------------------------------------------------

template <typename T>
inline void Process_Control_Thread<T>::pause()
{
    boost::mutex::scoped_lock lock(_command_mutex);
    
    if (stream_state == STREAM_STARTED)
    { 
        stream_state = STREAM_PAUSED;
    }
}

// ----------------------------------------------------------------------------

template <typename T>
inline void Process_Control_Thread<T>::stop()
{
    boost::mutex::scoped_lock lock(_command_mutex);
    if (stream_state != STREAM_STOPPED)
    {
        stream_state = STREAM_STOPPED;
    }
}

// ----------------------------------------------------------------------------

template <typename T>
inline void Process_Control_Thread<T>::wait_for_suspend()
{
    boost::mutex::scoped_lock lock(_command_mutex);
    wait_for_suspend_impl(lock, false);
}

// ----------------------------------------------------------------------------

template <typename T>
template <typename Scoped_Lock>
inline void Process_Control_Thread<T>::wait_for_suspend(Scoped_Lock & lock)
{
    wait_for_suspend_impl(lock, false);
}

// ----------------------------------------------------------------------------

template <typename T>
inline void Process_Control_Thread<T>::wait_for_stop()
{
    boost::mutex::scoped_lock lock(_command_mutex);
    wait_for_suspend_impl(lock, true);
}

// ----------------------------------------------------------------------------

template <typename T>
template <typename Scoped_Lock>
inline void Process_Control_Thread<T>::wait_for_stop(Scoped_Lock & lock)
{
    wait_for_suspend_impl(lock, true);
}

// ----------------------------------------------------------------------------

template <typename T>
inline void Process_Control_Thread<T>::attach
(T* object, void(T::*process)(), void(T::*cancel_process)())
{
    this->_object = object;
    this->_process = process;
    this->_cancel_process = cancel_process;
}

// ----------------------------------------------------------------------------

template <typename T>
inline bool Process_Control_Thread<T>::is_attached() const
{
    if (_process == 0)
        return false;
    else
        return true;
}

// ----------------------------------------------------------------------------

template <typename T>
inline bool Process_Control_Thread<T>::current() const
{
    return _thread.get() != 0 && boost::thread() == *_thread;
}

// ----------------------------------------------------------------------------

template <typename T>
template <typename Scoped_Lock>
inline void Process_Control_Thread<T>::wait_for_suspend_impl
(Scoped_Lock & lock, bool until_stopped)
{
    // start cannot be called until this method exists, this prevents missing
    // the suspend event is stop and start are called too fast.

    ++_pending_waits;
#if !defined NDEBUG && defined DEBUG_THREAD
    std::cerr << "wait_for_suspend_impl " << this 
              << (until_stopped ? " only until stopped" : "") << std::endl;
#endif
    while (_thread.get() && !_suspended && 
           (!until_stopped || stream_state != STREAM_STOPPED))
    {
#if !defined NDEBUG && defined DEBUG_THREAD
        std::cerr << "blocking in wait_for_suspend_impl " << this 
                  << std::endl;
#endif
        _wait_for_suspend_block.wait(lock);
    }
#if !defined NDEBUG && defined DEBUG_THREAD
        std::cerr << "wait_for_suspend_impl " << this << " finished"
                  << std::endl;
#endif

    if (--_pending_waits == 0)
    {
        // Unblocking start if needed
        _wait_for_start_block.notify_all();
    }
}

// ----------------------------------------------------------------------------

template <typename T>
inline void Process_Control_Thread<T>::loop()
{
    for ( ; ; )
    {
        {
            boost::mutex::scoped_lock lock(_command_mutex);

            /* Blocking if thread is not started unless it is exiting */
            while (stream_state != STREAM_STARTED && _exit == false)
            {
#if !defined NDEBUG && defined DEBUG_THREAD
                std::cerr << "Suspending thread " << this << std::endl;
#endif
                _suspended = true;
                _wait_for_suspend_block.notify_all();
                // waiting for resume of processing
                _processing_loop_block.wait(lock);
            }
            _suspended = false;

            if (_exit)
            {
                return;
            }
        }

        // processing step
        (_object->*_process)();
    }
}

// ----------------------------------------------------------------------------

}
//#endif // threads
#endif // include guard
