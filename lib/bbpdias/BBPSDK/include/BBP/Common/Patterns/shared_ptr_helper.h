/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project 2008

        Responsible authors:    Juan Hernando Vieites
                                Thomas Traenkler
        
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SHARED_PTR_HELPER_H
#define BBP_SHARED_PTR_HELPER_H

namespace bbp {

// ----------------------------------------------------------------------------

//! Deleter doing nothing for passing to boost::shared_ptr<T> ctor.
/*!
    Passing pointers that should not be deallocated to a smart pointer
    can be done by providing an empty deleter, so the object is not dellocated
    when the last smart pointer pointing to that location goes out of scope.
    Note the empty deleter is copied to other smart pointers during
    assignment.
*/
inline void shared_ptr_non_deleter(void *) {}

// ----------------------------------------------------------------------------

}
#endif
