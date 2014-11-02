/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2008. All rights reserved.

        Responsible authors:    Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_PARALLEL_H
#define BBP_PARALLEL_H

#include <string>

#include <boost/version.hpp>
#include <vector>

// if boost version < 1.35 then do hardware_concurrency() manually
#if BOOST_VERSION < 103500
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <process.h>
#else // WIN32 (not)
#ifdef __linux__ // linux
#include <sys/sysinfo.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) // linux
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(__sun) // linux
#include <unistd.h>
#endif // linux
#endif // win32
#endif // boost version

//#ifdef BBP_THREADS_SUPPORTED
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>
//#endif

#include "BBP/Common/Types.h"

namespace bbp {

/*!
    \todo This file should contain multi-threaded or MPI(message passing 
    interface) parallel programming concepts and abstract from the parallel 
    concept as much as possible. Consider OpenMP on supported compilers to 
    achieve e.g. parallel for - but not supported on older platforms, make
    sure it works on all before taking decision. (TT)

    \todo thread_local is the C++ 0x keyword for thread local static storage, 
    which should be used if available, for the moment compiler specific
    keywords are used. (TT)
    http://gcc.gnu.org/onlinedocs/gcc-3.3.1/gcc/Thread-Local.html#Thread-Local
    http://www.intel.com/software/products/compilers/clin/docs/ug_cpp/lin1057.htm
    http://msdn.microsoft.com/en-us/library/ms859913.aspx
*/
#ifdef WIN32
#define thread_static __declspec( thread ) static
#else // if Intel C++ compiler or GNU C++ compiler 
#define thread_static __thread
#endif

// ----------------------------------------------------------------------------

//#ifdef BBP_THREADS_SUPPORTED
//! Get the number of threads the hardware can compute in parallel.
/*!
    \return The number of hardware threads available on the current system 
    (e.g. number of CPUs or cores or hyperthreading units), or 0 if this 
    information is not available. 
*/
inline unsigned hardware_concurrency()
{
#if BOOST_VERSION >= 103500
    return boost::thread::hardware_concurrency();
#else
#ifdef WIN32
    SYSTEM_INFO info={0};
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
#else // win32

#if defined(PTW32_VERSION) || defined(__hpux)
    return pthread_num_processors_np();
#elif defined(__linux__)
    return get_nprocs();
#elif defined(__APPLE__) || defined(__FreeBSD__)
    int count;
    size_t size=sizeof(count);
    return sysctlbyname("hw.ncpu",&count,&size,NULL,0)?0:count;
#elif defined(__sun)
    int const count=sysconf(_SC_NPROCESSORS_ONLN);
    return (count>0)?count:0;
#else
    return 0;
#endif
#endif // win32
#endif // boost
}
//#endif

// ----------------------------------------------------------------------------

//! Primitive for parallel multi-threaded iteration over the input container.
/*!
    WARNING: This is experimental code!
    
    The container must be thread read-only to avoid thread safety
    related issues. The code assumes the iterators remain valid unless the
    size of the container or the number of threads change.
*/
template <typename Container, typename Functor>
class Parallel_For_Each
{
public:
    Parallel_For_Each()
        : _number_of_threads(hardware_concurrency()),
          _previous_number_of_threads(0),
          _previous_container_size(0),
          _previous_container(0)
    {
    }

    Parallel_For_Each(Functor f, unsigned number_of_threads)
        : _f(f),
          _number_of_threads(number_of_threads),
          _previous_number_of_threads(0),
          _previous_container_size(0),
          _previous_container(0)
    {
    }

public:
    void operator() (const Container & container)
    {
        // Parallel threads that will be used to work on the container.
        boost::thread_group     _worker_threads;
        size_t container_size    = container.size();
        /*
            Compute container range for each thread if size
            or number of threads changed and thus iterators might
            have been invalidated.
        */
        if ( (_previous_container_size    != container_size   ) || 
             (_previous_number_of_threads != _number_of_threads) )
        {
            size_t thread_range_size = container_size / _number_of_threads;
            size_t residue           = container_size % _number_of_threads;
            _previous_container_size = container_size;
            _previous_number_of_threads = _number_of_threads;

            _range_sizes.clear();
            _range_sizes.reserve(_number_of_threads);
            for (size_t k = 0; k < _number_of_threads; ++k)
            {
                _range_sizes.push_back(thread_range_size);
            }
            // Distribute the residue equally (load balancing).
            size_t i = 0;
            while (residue != 0)
            {
                _range_sizes[i] += 1;
                ++i;
                if (i >= _number_of_threads)
                    i = 0;
                --residue;
            }
        }
        
        /*
            If size of container and number of threads has not changed,
            we need to check if the container changed and update only
            the iterators, not the range sizes.
        */
        if (_previous_container != & container)
        {
            _previous_container = & container;
            /*
                Calculate range iterators that point to the container ranges
                where the worker threads will process the input (non 
                overlapping).
            */
            _iterator_ranges.clear();
            _iterator_ranges.reserve(_number_of_threads);
            
            typename Container::const_iterator iter = container.begin();
            for (unsigned i = 0; i < _number_of_threads; ++i)
            {
                Interval<typename Container::const_iterator> iterator_range;
                iterator_range.begin = iter;

                /*! 
                    \todo This could be accelerated for containers that support
                    adding +n in one call. (TT)

                */
                for (size_t j = 0; j < _range_sizes[i]; ++j)
                {
                    ++iter;
                }
                iterator_range.end = iter;
                _iterator_ranges.push_back(iterator_range);
            }
        }

        /*
            Dispatch the subproblem range iterators to worker threads and 
            wait for all worker threads to complete before this control 
            thread returns control to the calling code.
        */
        for (unsigned i = 0; i < _number_of_threads; ++i)
        {
            _worker_threads.create_thread(boost::bind(
                & Parallel_For_Each<Container, Functor>::
                    worker_thread_iteration, this, _iterator_ranges[i]));
        } 
        _worker_threads.join_all();   
    }

private:
    void worker_thread_iteration
        (const Interval<typename Container::const_iterator> & interval)
    {
        for(typename Container::const_iterator i = interval.begin;
            i != interval.end; ++i)
        {
            _f(*i);
        }
    }

    Functor _f;

    // Thread input container range [begin,end)
    std::vector< Interval<typename Container::const_iterator> >   
        _iterator_ranges;
    std::vector< size_t >  _range_sizes;
    // Number of parallel threads that will be used to work on the 
    // container.
    unsigned    _number_of_threads;
    unsigned    _previous_number_of_threads;
    size_t      _previous_container_size;
    const Container  * _previous_container;
};

// ----------------------------------------------------------------------------

}

#endif
