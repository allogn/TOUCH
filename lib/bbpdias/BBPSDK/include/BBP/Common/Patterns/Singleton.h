/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project 2008

        Responsible authors:    Thomas Traenkler
        
        References:                 
        1) Gamma et al., Design Patterns, 1995
        2) http://blogs.msdn.com/oldnewthing/archive/2004/03/08/85901.aspx

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SINGLETON_H
#define BBP_SINGLETON_H

namespace bbp {

// ----------------------------------------------------------------------------

//! The singleton design pattern enforces exactly one instance of a class.
/*!
    CAUTION: This code is only threadsafe if handled with care!
    
    The singleton class function template assures a maximum of ONE instance of 
    a class at a given time that provides a global access point to this object.

    The problem that this class solved in contrast to an externally defined
    global variable is that it is initalized even before the main() function 
    is called, and thus constructors using the singleton will have a proper
    instance of that object which is not the case for a global extern variable.

    Thread safety: It is only threadsafe if the first call to this function is 
    only in a single thread or handled by a singleton manager. For all 
    subsequent calls, threads can be pre-empted without harm. This problem 
    cannot be resolved by a mutex since the mutex itself will cause a race 
    between threads for the mutex. A singleton manager should wrap the 
    initalization of this object if called when multiple threads are already 
    executing.

    For a treatment of this problem and why it can not be resolved 
    conceptually, see:

    http://blogs.msdn.com/oldnewthing/archive/2004/03/08/85901.aspx
    
    A more general treatment of design patterns here:
    Gamma et al., Design Patterns, 1995
    
    \ingroup Design_Pattern
*/
template <typename T>
T & singleton()
{
	/*!
		This is initialized during first access to singleton()
		with persistent lifetime until program termination.
	*/
    static T _instance;
	return   _instance;
}

// ----------------------------------------------------------------------------

}
#endif
