/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        Thomas Traenkler
        (c) 2006-2008. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_TIMER_H
#define BBP_TIMER_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#else
#include <ctime>
#include <cstdlib>
#include <sys/time.h>
#endif
#include "BBP/Common/Exception/Exception.h"
#include "BBP/Common/Types.h"

namespace bbp {

//! Stopwatch to measure time.
class Timer
{
public:
    Timer() 
        : _is_running(false),
          _at_zero(true)
    {
    }

public:

    //! Resets and starts timer.
    void start()
    {
        _is_running = true;
#ifdef WIN32
        _start = GetTickCount();
#else
        gettimeofday(& _start, NULL);
#endif
        _at_zero = false;
    }

    //! Resets, labels and starts timer.
    void start(const std::string & label)
    {
        _label = label;
        start();
    }

#ifdef WIN32
    //! Continue timer.
    void resume()
    {
        if (_is_running == false)
        {
            _is_running = true;
            _start = GetTickCount();
            // start
            if (_at_zero == true)
            {
                _at_zero = false;
            }
            // continue
            else
            {
                _start -= _elapsed_milliseconds;
            }
        }
        else
        {
            throw_exception(std::runtime_error("Timer already running"), 
                SEVERE_LEVEL, __FILE__, __LINE__);
        }
    }
#endif

    //! Stop timer.
    /*!
        @return Elapsed time in milliseconds since start of timer.
    */
    Millisecond stop()
    {
#ifdef WIN32
        _elapsed_milliseconds = GetTickCount() - _start;
#else
        gettimeofday(& _end, NULL);
        _elapsed_milliseconds = (_end.tv_sec * 1000 + _end.tv_usec / 1000)
            - (_start.tv_sec * 1000 + _start.tv_usec / 1000);
#endif
        _is_running = false;
        return (Millisecond) _elapsed_milliseconds;
    }

    //! Reset timer to zero and stops it if necessary.
    void reset()
    {
        _at_zero = true;
        _is_running = false;
        _label = "";
    }

    //! Print the elapsed time since
    void print()
    {
        if (_is_running == true)
        {
#ifdef WIN32
            _elapsed_milliseconds = GetTickCount() - _start;
#else
            gettimeofday(& _end, NULL);
            _elapsed_milliseconds = (_end.tv_sec * 1000 + _end.tv_usec / 1000)
                - (_start.tv_sec * 1000 + _start.tv_usec / 1000);
#endif
        }
        if (_label != "")
        {
            std::cout << _label << ": " 
                << _elapsed_milliseconds 
                << " ms" << std::endl;
        }
        else
        {
            std::cout << "Elapsed time: " 
                << _elapsed_milliseconds
                << " ms" << std::endl;
        }
    }

private:
    bool    _is_running;
    bool    _at_zero;
    Label   _label;
#ifdef WIN32
    //! System defined millisecond ticks at start of timer.
    DWORD   _start;
    //! Millisecond ticks elapsed since start of timer.
    DWORD   _elapsed_milliseconds;
#else
    timeval _start, _end;
    Count   _elapsed_milliseconds;
#endif
};

}
#endif  // namespace bbp
