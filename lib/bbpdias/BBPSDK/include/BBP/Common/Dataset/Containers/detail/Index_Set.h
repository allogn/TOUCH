/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project 
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Juan Hernando

*/
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_INDEX_SET_H
#define BBP_INDEX_SET_H

#include <iostream>

#include "Interval_Set.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/identity.hpp>

namespace bbp
{
namespace detail
{

//! Template container to store sets of indices.
/*! The Index parameter can be any class or typename which has the semantics of
    natural numbers for addition and comparison operations.
    Ordered containers will be iterated in order from lowest to greatest index
    and will provide bidrectional iterators. Unordered containers will provide
    only forward iterators and the iteration sequence is unspecified. */
template <typename Index, bool ordered = true>
class Index_Set
{
public:
    class const_iterator;
    typedef const_iterator iterator;

    //! Insert the given index.
    inline void insert(Index i);

    //! Remove the given index.
    inline void remove(Index i);

    //! Insert a range of indices.
    inline void insert_interval(Index start, Index end);

    //! Remove all indices inside the closed interval [start, end].
    inline void remove_interval(Index start, Index end);

    //! Add another index set to this
    inline void insert(const Index_Set & other);

    //! Returns true if and only if x is contained inside.
    inline bool exists(Index x) const;

    //! Returns an iterator pointing to x if that index is stored, and end() otherwise
    inline const_iterator find(Index x) const;

    //! Returns an iterator to the first index inside the collection.
    inline const_iterator begin() const;

    //! Returns an iterator to the end of the collection.
    inline const_iterator end() const;

    //! Returns how many indices are stored inside the container.
    inline size_t size() const;

    //! Swap this container with another one.
    inline void swap(Index_Set & other) const;
};

//-----------------------------------------------------------------------------

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
#include "ordered_index_set.h"
#include "unordered_index_set.h"
/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

#ifdef _DOXYGEN_
/** \cond SHOW_IN_USER_REFERENCE */
//! Const iterator
/*! Bidirectional for the ordered Index_Set version and forward only for
    the unordered one. */
template <typename Index, bool ordered = true>
class Index_Set<Index>::const_iterator
{
public:
    //! Compare two iterators for equality.
    bool operator ==(const const_iterator &) const;
    //! Advance to next element in the container, prefix notation.
    const_iterator & operator ++();
    //! Advance to next element in the container, postfix notation.
    const_iterator operator ++(int);
    //! Step back to previous element in the container, prefix notation.
    /*! Only available if the container is ordered. */
    const_iterator & operator --();
    //! Step back to previous element in the container, postfix notation.
    /*! Only available if the container is ordered. */
    const_iterator operator --(int);
    //! Dereference the current interval pointed by the iterator.
    Index operator * () const;
    //! Dereference the current interval pointed by the iterator.
    Index operator -> () const;
};
/** \endcond SHOW_IN_USER_REFERENCE */
#endif

template <typename Index, bool ordered>
inline std::ostream & operator << (std::ostream & out, 
                                   const Index_Set<Index, ordered> & set);


//-----------------------------------------------------------------------------

template <typename Index, bool ordered>
std::ostream & operator << (std::ostream & out, 
                            const Index_Set<Index, ordered> & set)
{
    out << "[";
    typename Index_Set<Index, ordered>::const_iterator i = set.begin();
    while (i != set.end())
    {
        out << *i++;
        if (i != set.end())
            out << ", ";
    }
    out << "]";

    return out;
}

}
}
#endif
