/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SLEEP_H
#define BBP_SLEEP_H

#include <cmath>

#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>

namespace bbp {

//! Sleep program flow for a specified number of milliseconds.
inline void sleep(double milliseconds)
{
    boost::xtime time;
    boost::xtime_get(& time, boost::TIME_UTC);
    unsigned int seconds = (unsigned int) std::floor(milliseconds / 1000.0);
    time.sec += seconds;
    time.nsec += boost::int_fast32_t((milliseconds - seconds) * 1000000);
    boost::thread::sleep(time); // Sleep for 1 second
}



}
#endif
