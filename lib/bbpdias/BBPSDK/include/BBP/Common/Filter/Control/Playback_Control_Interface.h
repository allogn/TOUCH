/*

        Ecole Polytechnique Federale de Lausanne
        Blue Brain Project & Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Responsible author: Thomas Traenkler
        Contributing authors: Juan Hernando

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_PIPELINE_STREAM_PLAYBACK_CONTROL_H
#define BBP_PIPELINE_STREAM_PLAYBACK_CONTROL_H

#ifndef NDEBUG
#include <iostream>
#endif
#include <iostream>

#include <limits>
//#ifdef BBP_THREADS_SUPPORTED
#include <boost/thread/recursive_mutex.hpp>
//#endif

#include "BBP/Common/Types.h"
#include "BBP/Common/Math/Round.h"
#include "BBP/Common/Exception/Exception.h"
#include "Process_Control_Interface.h"


namespace bbp {

// ----------------------------------------------------------------------------

//! Modes of stream playback (triggered, frame or time based).
/*! 
    Three playback modes are supported: In time based mode
    synchronization is done by dropping frames, in frame based mode 
    synchronization is done by waiting, and in manually triggered mode 
*/
enum Playback_Mode
{
    //! trigger processing of each frame manually (go step by step)
    PROCESS_SINGLE_STEP_TRIGGER
//#ifdef BBP_THREADS_SUPPORTED
    ,
    //! frame based speed (accurate presentation has priority)
    PROCESS_FRAME_BASED,
    //! time based speed (correct timing has priority)
    /*!
        \todo This should be the same as realtime based and thus has become
        obsolete with the maybe more explicit name _REALTIME_. (TT)
    */
    PROCESS_TIME_BASED,
    //! time based speed (correct realtime timing, frames dropped if necessary)
    PROCESS_REALTIME_BASED
//#endif
};

// ----------------------------------------------------------------------------

//! Stream playback control interface class controls a time based stream.
/*!
    Abstract playback control class. This class specifies an interface for
    control of time based playback. Controls include start, pause, stop, speed,
    and framerate.
    \ingroup Process_Interface
*/
template <typename Time_Unit>
class Playback_Control_Interface
    : public virtual Process_Control_Interface
{
public:

    Playback_Control_Interface();

    virtual ~Playback_Control_Interface() {};

    //! Set playback mode
    /*!
      Priority is either the accurate timing of frames or 
      to process all frames. For accurate realtime timing, frames may be
      skipped, while for processing all frames the playback may wait for the
      processing to complete.
      The default mode is PROCESS_SINGLE_STEP_TRIGGER if no thread support
      has been enabled and PROCESS_FRAME_BASED otherwise
    */
    void mode(Playback_Mode mode)
        /* throw (Unsupported) */;

    //! Jump to frame closest to specified time.
    /*!
      The timestamp is interpreted in the absolute reference frame. The
      underlying data stream will jump as soon as possible, until then
      time() and frame() will return the stamps for the latest processed
      frame.

      Will throw if random access to the underlying stream data
      is not permitted. \bug
    */
    inline void time(Time_Unit time)
        /* throw (Unsupported) */;

    //! Return current time of the playback.
    inline Time_Unit time() const;
    
    
    //! Jump to specified frame.
    /*!
      Frame numbers can be relative to the original start and end times
      or to the current time window.

      Will throw if random access to the underlying stream data
      is not permitted unless frame is UNDEFINED_FRAME_NUMBER.

      @param frame A frame number or UNDEFINED_FRAME_NUMBER, the latter
             is equivalent to calling reset().
      @param relative_to_original If false frame is interpreted to be
             relative to the current interest window, otherwise it will
             be in the original window. In any case the window start time
             maps to frame 0.
    */
    inline void frame(Frame_Number frame, bool relative_to_original = true)
        /* throw (Unsupported) */;

    //! Return current frame of the playback.
    /*!
      Frame numbers are relative to the time window used in initialization
      and not to the current interest window.
      @param relative_to_original If false frame is interpreted to be
             relative to the current interest window, otherwise it will
             be in the original window. In any case the window start time
             maps to frame 0.
      @return UNDEFINED_FRAME_NUMBER is no frame has been processed yet.
    */
    inline Frame_Number frame(bool relative_to_original = true) const;

    //! Returns the default stream delta time between frames.
    inline Time_Unit timestep() const;

    //! Playback speed 
    /*!
      The interpretation of playback speed depends on the playback mode and
      the seek capabilities of the underlying data stream.
      
      In FRAME BASED and MANUALLY TRIGGERED modes a factor of 2.0 would mean 
      to skip every second frame, while a factor of 0.1 would only update the
      frame every tenth step of the processing pipeline. Any non-integer value
      greater than one will be rounded to the closest integer - i.e. 2.6
      will result in only every third frame to be presented.
      The value of frame_per_second is ignored.

      In time based modes the combination of speed and frames_per_second are
      used to now the frame refresh rate.

      For non real-time mode, a factor of 2.0 with 25 frames per second will
      will produce framestamps at a distance of 0.4 ms * 2.0 = 0.8 ms. While
      0.01 with 20 fps will have a timestep of 0.5 ms * 0.01 = 0.005 ms.      

      In realtime mode a factor of 2.0 would playback the stream twice as 
      fast as realtime, i.e. 10 seconds of simulation would be played back in
      5 seconds. A factor of 0.1 would playback the stream in slow motion,
      a second would be played back in 10 seconds, while a factor of -1.0 
      would playback the stream backwards at normal speed.
      TODO: realtime needs reviewing to specify when and how frames are 
      skipped.

      Speed can be changed for all streams that support random access from
      their sources. If random access is not supported this operation may
      throw Unsupported.
    */
    inline void speed(Time_Unit factor)
        /* throw (Unsupported) */;
    
    //! Returns the current playback speed
    /*!
      Default value is 1 for all modes.
    */
    inline Time_Unit speed() const;
    
    //! Sets the frames per second at which stream frames should be presented. 
    /*!
       This is only relevant for time based modes (realtime or not).
       This setting is important to reduce the sheer amount of data that is 
       going to be streamed and thus speeds up the processing significantly.

       For example 25 frames per second with a speed of 1.0 in a stream
       with samples every 0.1 milliseconds will result in updating the 
       current stream frame one time out of 400 recorded frames for this 
       second.
    */
    inline void frames_per_second(Frame_Number framerate)
        /* throw (Unsupported) */;

    /*!
        Reports the current framerate at which stream frames
        are set to be presented (in relationship to playback speed).

        Default value is 25.
    */  
    inline Frame_Number frames_per_second() const;

    //! Changes the interest window
    /*!
      Times given are clampled to the absolute times used during internal 
      initialization (i.e Report times in a stream report reader class).
      The values actually stored will be rounded using timestep as scale.

      When the time window is set the stream is reset to the first frame
      in the time window calling frame().

      This operation may not be supported by all streams.
    */
    inline void time_window(Time_Unit start, Time_Unit end)
        /* throw (Unsupported) */;

    //! Commands the associated process to loop over the interest time window.
    void repeat(bool flag)
        /* throw (Unsupported) */;

    //! Returns whether the end of the stream has been reached or not.
    /*!
      Returns true if and only if playback repeat is not set and the last
      call to read went over the end or the start of the interest time 
      window, depending on playback if forward or backward.
      Note that the current time and frame stamps are internally clamped 
      in any case so this is the only safe way of checking if the end
      has been reached.
    */
    bool finished();

    //! Resets the playback to the beginning if possible
    virtual void reset()
        /* throw (Unsupported) */;

    //! Resets and stops the stream.
    inline virtual void stop();

protected:
    /** \cond SHOW_IN_DEVELOPER_REFERENCE */

    //! Called from mode. To be overrided by derived classes.
    /*!
      Implementation function called after mode() writes any attribute and
      after internal mutex lock.
      May throw if the operation or the mode are not supported.
    */
    virtual void mode_impl(Playback_Mode mode)
        /* throw (Unsupported) */ {}

    //! Called from time. To be overrided by derived classes.
    /*!
      Implementation function called before time() writes any attribute and
      after internal mutex lock.
      May throw if operation if not supported.
     */
    virtual void time_impl(Time_Unit time)
        /* throw (Unsupported) */ {}
    
    //! Called from frame. To be overrided by derived classes.
    /*!
      Implementation function called before time() writes any attribute and
      after internal mutex lock.
      @param frame A frame number given relative to the current interest 
             window or UNDEFINED_FRAME_NUMBER to jump to beginning of the
             stream if possible.
      May throw if operation if not supported.
     */
    virtual void frame_impl(Frame_Number frame)
        /* throw (Unsupported) */ {}

    //! Called from speed. To be overrided by derived classes.
    /*!
      Implementation function called before time() writes any attribute and
      after internal mutex lock.
      May throw if operation if not supported.
    */
    virtual void speed_impl(Time_Unit speed)
        /* throw (Unsupported) */ {}

    //! Called from frames_per_second. To be overrided by derived classes.
    /*!
      Implementation function called before time() writes any attribute and
      after internal mutex lock.
      May throw if operation if not supported.
    */
    virtual void frames_per_second_impl(Frame_Number fps)
        /* throw (Unsupported) */ {}

    //! Called from time_window. To be overrided by derived classes.
    /*!
      Implementation function called before time() writes any attribute and
      after internal mutex lock and clamping/rounding.
      May throw if operation if not supported.
    */
    virtual void time_window_impl(Millisecond start, Millisecond end)
        /* throw (Unsupported) */ {}

    //! Updates the time and frame number depending on the current mode.
    /*!
      This is a convenience function for derived classes. It updates
      the internal frame and time stamps according to the playback mode.
      It might not be suitable when actual timestamps depends on frame dropping
      or external data sources like network streaming.

      The new time and frame number are clamped to the current time window.
      Before updating, if the playback is not set to loop and the stream
      is at the end the state is changed to paused and the time and frame
      are remain unchanged.
    */
    virtual inline void update();

    //! Initializes the reference times for playback.
    /*!
      @param frame_timestep Time step used to translate timestamps to frame
      numbers
      @param start_time An absolute start point. This time will correspond to
      absolute frame 0.
      @param end_time An absolute end point. The [start_time, end_time)
      interval is open, which means that the translation of end_time into
      a framestamp will be one frame past the real last frame.
      This invariant will hold:
      (end_time - start_time) / timestep = frame_count
    */
    void set_absolute_times(Time_Unit frame_timestep, 
                            Time_Unit start_time, Time_Unit end_time);

protected:
//#ifdef BBP_THREADS_SUPPORTED
    // Made recursive to minimize the risk of trivial deadlocks in derived
    // classes.
    mutable boost::recursive_mutex  _playback_command_mutex;
//#endif

    Time_Unit               _absolute_start_time,
                            _absolute_end_time,
                            _start_time,
                            _end_time,
                            _frame_timestep;

    // All Frame numbers are relative to the original time window
    Frame_Number            _start_frame_number,
                            _end_frame_number;

    // Stamps for the last frame processed.
    Time_Unit               _current_time;
    Frame_Number            _current_frame_number;

    // Stamps for the next frame to be processed.
    Time_Unit               _next_time;
    Frame_Number            _next_frame_number;

    Frame_Number            _total_number_of_frames;

    Frame_Number            _playback_framerate;
    double                  _playback_speed;
    Playback_Mode           _playback_mode;
    bool                    _playback_repeat;
    bool                    _playback_finished;

    /** \endcond SHOW_IN_DEVELOPER_REFERENCE */
};

// ----------------------------------------------------------------------------

/*! 
    \bug warning C4305: 'initializing' : truncation from 'double' to 
    'bbp::Millisecond'
*/
template <typename Time_Unit>
Playback_Control_Interface<Time_Unit>::
Playback_Control_Interface()
    : _start_time(0.0f),
      _end_time(0.0f),
      _frame_timestep(0.1f),
      _start_frame_number(0),
      _end_frame_number(0),
      _current_time(0),
      _current_frame_number(UNDEFINED_FRAME_NUMBER),
      _next_time(0.0f),
      _next_frame_number(0),
      _total_number_of_frames(0),
      _playback_framerate(25),
      _playback_speed(1.0f),
//#ifdef BBP_THREADS_SUPPORTED
      _playback_mode(PROCESS_FRAME_BASED),
//#else
//      _playback_mode(PROCESS_SINGLE_STEP_TRIGGER),
//#endif
      _playback_repeat(false),
      _playback_finished(false)
{
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::
mode(Playback_Mode mode)
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(_playback_command_mutex);
//#endif
    mode_impl(mode);
    _playback_mode = mode;
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::time(Time_Unit time)
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(_playback_command_mutex);
//#endif
    time_impl(time);
    
    // Set time and frame.
    _next_time = time;

    if (_next_time < _start_time)
        _next_time = _start_time;
    else if (_next_time > _end_time - _frame_timestep)
        _next_time = _end_time;

    _next_frame_number = _start_frame_number +
        (Frame_Number) round((_next_time - _start_time) / _frame_timestep);

    // check and correct for out of bounds
    if (_next_frame_number < _start_frame_number)
        _next_frame_number = _start_frame_number;
    else if (_next_frame_number > _end_frame_number)
        _next_frame_number = _end_frame_number;

}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
Time_Unit Playback_Control_Interface<Time_Unit>::time() const
{
    // No mutex needed here.
    return _current_time;
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::frame
(Frame_Number frame, bool relative_to_original)
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(_playback_command_mutex);
//#endif

    if (relative_to_original && frame != UNDEFINED_FRAME_NUMBER)
        frame_impl(frame - _start_frame_number);
    else
        frame_impl(frame);

    // Set time and frame.
    _next_frame_number = frame;
    
    if (_next_frame_number != UNDEFINED_FRAME_NUMBER)
    {
        // Check and correct for out of bounds.
        if (_next_frame_number < _start_frame_number)
            _next_frame_number = _start_frame_number;
        else if (_next_frame_number > _end_frame_number)
            _next_frame_number = _end_frame_number;
    
        _next_time = 
            _start_time + ((frame - _start_frame_number) * _frame_timestep);
        if (_next_time < _start_time)
            _next_time = _start_time;
        else if (_next_time > _end_time)
            _next_time = _end_time;
    }
    else
    {
        _next_time = UNDEFINED_MILLISECOND;
    }
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
Frame_Number Playback_Control_Interface<Time_Unit>::frame
(bool relative_to_original) const
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(_playback_command_mutex);
//#endif

    if (_current_frame_number == UNDEFINED_FRAME_NUMBER)
        return UNDEFINED_FRAME_NUMBER;
    else if (relative_to_original)
        return _current_frame_number;
    else
        return _current_frame_number - _start_frame_number;
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
Time_Unit Playback_Control_Interface<Time_Unit>::timestep() const
{
    // No mutex needed here.
    return _frame_timestep;
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::frames_per_second
(Frame_Number framerate)
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(_playback_command_mutex);
//#endif
    frames_per_second_impl(framerate);
    
    _playback_framerate = framerate;
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
Frame_Number Playback_Control_Interface<Time_Unit>::frames_per_second() const
{
    // No mutex needed here.
    return _playback_framerate;
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::repeat(bool flag)
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(_playback_command_mutex);
//#endif

    // Wy try to jump to the current frame number, if that's not possible
    // it means that looping is not possible.
    if (flag)
    {
        try
        {
            frame(_current_frame_number);
        }
        catch (...) 
        {
            //! \todo Solve the problem with exception definitions location
            //throw Unsupported();
            throw_exception(std::runtime_error("Unsupported"), 
                            FATAL_LEVEL, __FILE__, __LINE__);
        }
    }

    _playback_repeat = flag;
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
bool Playback_Control_Interface<Time_Unit>::finished()
{
    return _playback_finished;
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::reset()
{
    frame(UNDEFINED_FRAME_NUMBER);
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::speed(Time_Unit factor)
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(_playback_command_mutex);
//#endif

    /*!
        \todo Check with Juan if this is in line with skip_frame in 
        Report_Reader. (TT)
    */
//#ifdef BBP_THREADS_SUPPORTED
    if ((_playback_mode == PROCESS_FRAME_BASED || 
         _playback_mode == PROCESS_SINGLE_STEP_TRIGGER) &&
        factor > 1)
        factor = round(factor);
//#else
//    if (_playback_mode == PROCESS_SINGLE_STEP_TRIGGER && factor > 1)
//        factor = round(factor);
//#endif
    
    speed_impl(factor);
    _playback_speed = factor;
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
Time_Unit Playback_Control_Interface<Time_Unit>::speed() const
{
    // No mutex needed here.
    return _playback_speed;
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::time_window
(Time_Unit start, Time_Unit end)
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(_playback_command_mutex);
//#endif

    /*! 
        \note These checks were put to avoid some unidentifiable runtime error
        when running spatiotemporal_average commandline tool with time window
        0.0 to 5.0 ms for a report that has forward skip to 700 and has data
        in the interval 700 to 750 ms. (TT)
    */

    if (start < _absolute_start_time)
    {
        std::stringstream temp;
        temp << "Begin of interest time window is not inside the original "
            "time window interval: " << start
            << " < " << _absolute_start_time;

        throw_exception(std::out_of_range(temp.str().c_str()), WARNING_LEVEL,
            __FILE__, __LINE__);
    }
            
    if (end > _absolute_end_time)
    {
        std::stringstream temp;
        temp << "End of interest time window is not inside the original "
            "time window interval: " << end
            << " > " << _absolute_end_time;

        throw_exception(std::out_of_range(temp.str().c_str()), WARNING_LEVEL,
            __FILE__, __LINE__);
    }

    // Rounding values according to the timestep
    Frame_Number start_frame =
        (Frame_Number) round((start - _absolute_start_time) / _frame_timestep);
    Frame_Number end_frame =
        (Frame_Number) round((end - _absolute_start_time) / _frame_timestep);

    start = start_frame * _frame_timestep + _absolute_start_time;
    end = end_frame * _frame_timestep + _absolute_start_time;

    if (start < _absolute_start_time)
        start = _absolute_start_time;
    if (end > _absolute_end_time)
        end = _absolute_end_time;

    if (start > end)
        end = start;

    time_window_impl(start, end);

    _start_time = start;
    _end_time = end;

    _start_frame_number = start_frame;
    _end_frame_number = end_frame;

    frame(_start_frame_number);
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::update()
{
    // Checking if we've reached the end
    if (_next_frame_number == UNDEFINED_FRAME_NUMBER) {
        _next_frame_number = _start_frame_number;
    } else {
        if (_playback_repeat == false &&
            (_next_frame_number >= _end_frame_number && _playback_speed > 0 ||
             _next_frame_number <= _start_frame_number && _playback_speed < 0))
        {
            _playback_finished = true;
            pause();
            return;
        }
        else
        {
            _playback_finished = false;
        }
    }

    // Updating current stamps with next stamps.

    _current_frame_number = _next_frame_number;
    _current_time = _next_time;


    // Updating next time and frame stamps.
//#ifdef BBP_THREADS_SUPPORTED
    if (_playback_mode == PROCESS_TIME_BASED || 
        _playback_mode == PROCESS_REALTIME_BASED)
//#else
//    if (false)
//#endif
    {
        // current time incremented with the period
        _next_time += (Time_Unit) (1.0 / _playback_framerate 
                        * 1000 * _playback_speed);
        _next_frame_number = (Frame_Number) 
            floor(_start_frame_number + 
            ((_next_time - _start_time) / _frame_timestep));
    }
    else // playback_mode == PROCESS_FRAME_BASED or PROCESS_SINGLE_STEP_TRIGGER
    {
        _next_frame_number += (Frame_Number) floor(_playback_speed);
        _next_time = _start_time + 
            (_next_frame_number - _start_frame_number) * _frame_timestep;
    }

    // if playing forward
    if (_playback_speed > 0.0)
    {
        // check if end reached
        if (_next_time >= _end_time || 
            _next_frame_number > _end_frame_number)
        {
            // if repeat enabled repeat
            if (_playback_repeat == true)
            {
                _next_time = _start_time;
                _next_frame_number = _start_frame_number;
            }
            // otherwise end playback
            else
            {
                // latest time in playback
                _next_time = _end_time - _frame_timestep; 
                _next_frame_number = _end_frame_number;
            }
        }
    }
    // if playing backwards 
    else if (_playback_speed < 0.0)
    {
        // check if begin reached
        if (_next_time < _start_time || 
            _next_frame_number < _start_frame_number)
        {
            // if repeat enabled repeat
            if (_playback_repeat == true)
            {
                // latest time in playback
                _next_time = _end_time - _frame_timestep;
                _next_frame_number = _end_frame_number;
            }
            // otherwise end playback
            else
            {
                _next_time = _start_time;
                _next_frame_number = _start_frame_number;
            }
        }
    }
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::set_absolute_times
(Time_Unit frame_timestep, 
 Time_Unit start_time, Time_Unit end_time)
{
    this->_frame_timestep = frame_timestep;
    
    bbp_assert(start_time <= end_time);

    this->_start_frame_number = 0;
    this->_absolute_start_time = this->_start_time = start_time;

    this->_total_number_of_frames = 
        (Frame_Number) round((end_time - start_time) / frame_timestep);
    this->_end_frame_number = this->_total_number_of_frames - 1;
    this->_absolute_end_time = this->_end_time = end_time;
    time(start_time);
}

// ----------------------------------------------------------------------------

template <typename Time_Unit>
void Playback_Control_Interface<Time_Unit>::stop()
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::recursive_mutex::scoped_lock lock(_playback_command_mutex);
//#endif

    reset();
    this->stream_state = STREAM_STOPPED;
}

// ----------------------------------------------------------------------------

}
#endif
