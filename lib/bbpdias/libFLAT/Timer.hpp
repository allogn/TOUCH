#ifndef TIMER_HPP
#define TIMER_HPP

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <ctime>
#include <cstdlib>
#include <sys/time.h>
typedef size_t Count;
#endif
#include <iostream>

namespace FLAT
{

	typedef float Millisecond;

	//! Stopwatch to Measure time.
	class Timer
	{
	public:
		Timer();

		void start();

		void start(const std::string& label);

		friend std::ostream & operator << (std::ostream & lhs, Timer& rhs);

		Millisecond stop();

		void reset();

	private:
		bool    _is_running;
		bool    _at_zero;
		std::string   _label;
	#ifdef WIN32
		DWORD   _start;
		DWORD   _elapsed_milliseconds;
	#else
		timeval _start;
		timeval _end;
		size_t  _elapsed_milliseconds;
	#endif
	};

}
#endif
