#include "Timer.hpp"

namespace FLAT
{

	Timer::Timer()
        : _is_running(false),
          _at_zero(true)
    {
    }

    void Timer::start()
    {
        _is_running = true;
#ifdef WIN32
        _start = GetTickCount();
#else
        gettimeofday(& _start, NULL);
#endif
        _at_zero = false;
    }

    void Timer::start(const std::string& label)
    {
        _label = label;
        start();
    }

    std::ostream & operator << (std::ostream & lhs, Timer& rhs)
    {
        if (rhs._is_running == true)
        {
#ifdef WIN32
        	rhs._elapsed_milliseconds = GetTickCount() - rhs._start;
#else
            gettimeofday(& rhs._end, NULL);
            rhs._elapsed_milliseconds = (rhs._end.tv_sec * 1000 + rhs._end.tv_usec / 1000)
                - (rhs._start.tv_sec * 1000 + rhs._start.tv_usec / 1000);
#endif
        }
        if (rhs._label != "")
        {
            lhs<< rhs._label << ": "
                << (rhs._elapsed_milliseconds+0.0)/(1000+0.0)
                << " sec" ;
        }
        else
        {
            lhs << (rhs._elapsed_milliseconds+0.0)/(1000+0.0)
            << " sec";
        }
        return lhs;
    }

    Millisecond Timer::stop()
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

    void Timer::reset()
    {
        _at_zero = true;
        _is_running = false;
        _label = "";
    }

}
