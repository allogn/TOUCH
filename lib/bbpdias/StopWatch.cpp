#ifndef STOPWATCH
#define STOPWATCH

#include <sys/time.h>
#include <iostream>

class StopWatch
{
private:
	struct timeval begin;
	struct timeval duration;
    bool running;

public:
    StopWatch()
    {
        running = false;
    }
    void start();
    void stop();
    void reset();
    friend std::ostream & operator << (std::ostream & lhs, StopWatch & rhs);
};

std::ostream& operator << (std::ostream & lhs, StopWatch & rhs)
	{
	if (rhs.running==true)
	{
		//rhs.stop();
		lhs << rhs.duration.tv_sec << "," << rhs.duration.tv_usec;
		//rhs.start();
	}
	else
		lhs << rhs.duration.tv_sec << "," << rhs.duration.tv_usec;
	return lhs;
	}

void StopWatch::start()
	{
	gettimeofday(&begin, NULL);
    running = true;
	}

void StopWatch::stop()
	{
    running = false;
    timeval now;
    gettimeofday(&now, NULL);
    if (now.tv_usec < begin.tv_usec)
		{
    	now.tv_sec--;
    	now.tv_usec += 1000000;
		}
    duration.tv_sec  += (now.tv_sec  - begin.tv_sec);
    duration.tv_usec += (now.tv_usec - begin.tv_usec); // TODO adjust overflow of uSec to Sec
	}

void StopWatch::reset()
	{
	running = false;
	duration.tv_sec = 0;
	duration.tv_usec=0;
	begin.tv_sec=0;
	begin.tv_usec=0;
	}


#endif
