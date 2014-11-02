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

#ifndef BBP_INTERVAL_SET_H
#define BBP_INTERVAL_SET_H

#include "BBP/Common/Exception/Exception.h"

#include <iostream>
#include <cassert>
#include <set>
#include <boost/iterator/iterator_facade.hpp>

namespace bbp
{
namespace detail
{

//! Template container for storing intervals of indices.
/*! The template parameter can be any integral discrete type that behaves
    like natural numbers with regard to comparison and addition. */
template <typename Index, typename diff_t = std::ptrdiff_t>
class Interval_Set
{
public:
    Interval_Set() : _size(0) {}

    //! Interval iterator.
    /*! It points to objects of type std::pair<Index, Index> represeting an
        interval [start, end]. */
    class const_iterator;
    
    //! Insert the closed interval [start, end] merging it if needed.
    inline void insert(Index start, Index end);

    //! Remove all intervals inside the closed interval [start, end].
    inline void remove(Index start, Index end);

    //! Remove the interval pointed by the given iterator.
    inline void remove(const_iterator interval);

    //! True if and only if x is inside some stored interval.
    inline bool is_present(Index x) const;

    //! Returns the distance of x to any interval inside the set.
    /*! returns std::numeric_limits<diff_t>::max if the set is empty and 0
        if x is inside a interval. */
    inline diff_t distance(Index x) const;

    //! Returns an iterator to an interval containing x or end() otherwise.
    inline const_iterator find(Index x) const;

    //! Returns the first interval in the collection.
    inline const_iterator begin() const;
  
    //! Returns the iterator representing the end of the storage.  
    inline const_iterator end() const;

    //! Returns the smallest interval whose end point is >= x.
    inline const_iterator lower_bound(Index x) const;

    //! Returns the smallest interval whose start point is > x.
    inline const_iterator upper_bound(Index x) const;

    //! Returns the number of intervals
    inline size_t intervals() const;

    //! Returns the smallest start point.
    /*! Undefined value returned if the container is empty. */
    inline Index lower_value() const;

    //! Returns the greatest end point.
    /*! Undefined value returned if the container is empty. */
    inline Index upper_value() const;

    //! Returns the number of indices contained by stored intervals. */
    inline size_t size() const;

    //! Swap this container with another one.
    inline void swap(Interval_Set & other);

protected:
    /** \cond SHOW_IN_DEVELOPER_REFERENCE */
    void print() const {
        for (typename Edge_Set::const_iterator i = _intervals.begin();
             i != _intervals.end();
             ++i)
            std::cout << '(' << i->first << ' ' << i->second << ')';
        std::cout << std::endl;
    }

    struct Edge_Compare
    {
        bool operator()(const std::pair<Index, bool> & p1,
                        const std::pair<Index, bool> & p2) const
        {
            if (p1.first < p2.first)
                return true;
            else if (p1.first == p2.first)
                return p1.second && !p2.second;
            else
                return false;
        }
    };

    /*! The first element of the pair is the edge value, the second element is
        true for the start of an interval and false for the end. */
    typedef std::multiset<std::pair<Index, bool>,  Edge_Compare> Edge_Set;
    Edge_Set _intervals;
    size_t   _size;

    /** \endcond SHOW_IN_DEVELOPER_REFERENCE */
};

//------------------------------------------------------------------------------

template<typename T, typename diff_t>
inline std::ostream & operator << (std::ostream & out, 
                                   const Interval_Set<T, diff_t> & set);

//------------------------------------------------------------------------------

#ifdef _DOXYGEN_
/** \cond SHOW_IN_USER_REFERENCE */
//! Bidirectional const iterator
template <typename Index, typename diff_t>
class Interval_Set<Index, diff_t>::const_iterator
{
public:
    //! Compare two iterators for equality.
    bool operator ==(const const_iterator &) const;
    //! Advance to next element in the container, prefix notation.
    const_iterator & operator ++();
    //! Advance to next element in the container, postfix notation.
    const_iterator operator ++(int);
    //! Step back to previous element in the container, prefix notation.
    const_iterator & operator --();
    //! Step back to previous element in the container, postfix notation.
    const_iterator operator --(int);
    //! Dereference the current interval pointed by the iterator.
    std::pair<Index, Index> operator * () const;
    //! Dereference the current interval pointed by the iterator.
    std::pair<Index, Index> operator -> () const;
};
/** \endcond SHOW_IN_USER_REFERENCE */
#endif

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
//! Bidirectional iterator
template <typename Index, typename diff_t>
class Interval_Set<Index, diff_t>::const_iterator :
    public boost::iterator_facade<
        typename Interval_Set<Index, diff_t>::const_iterator,
        std::pair<Index, Index>, 
        std::bidirectional_iterator_tag,
        std::pair<Index, Index> 
    >
{
    friend class Interval_Set;
    typedef std::multiset<std::pair<Index, bool>,  Edge_Compare> Edge_Set;
    /** \cond */
    friend class boost::iterator_core_access;
    /** \endcond */

public:
    //! Default constructor iterator.
    const_iterator() {}

private:
    explicit const_iterator
    (const typename Edge_Set::const_iterator & start_edge) :
        _start_edge(start_edge)
    {}

private:
    void increment()
    {
        ++_start_edge;
        ++_start_edge;
    }

    void decrement()
    {
        --_start_edge;
        --_start_edge;
    }

    bool equal(const_iterator const & other) const
    {
        return _start_edge == other._start_edge;
    }

    std::pair<Index, Index> dereference() const
    {
        typename Edge_Set::const_iterator end_edge = _start_edge;
        end_edge++;
        return std::pair<Index, Index>(_start_edge->first, end_edge->first);
    }

    typename Edge_Set::const_iterator _start_edge;
};
/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

//------------------------------------------------------------------------------

template <typename Index, typename diff_t>
void Interval_Set<Index, diff_t>::insert(Index start_value, Index end_value)
{
    bbp_assert(start_value <= end_value);
    std::pair<Index, bool> start(start_value, true);
    std::pair<Index, bool> end(end_value, false);
    
    if (_intervals.size() == 0)
    {
        _intervals.insert(start);
        _intervals.insert(end);
        _size = end_value - start_value + 1;
        return;
    }

    // Finding the first edge whose value is less or equal than start_value.
    typename Edge_Set::iterator previous_to_start = 
        _intervals.lower_bound(start);
    if (previous_to_start != _intervals.end())
    {
        if (previous_to_start == _intervals.begin())
            previous_to_start = _intervals.end();
        else
            previous_to_start--;
    }
    else
        previous_to_start = (++_intervals.rbegin()).base();

    // Adding start edge as needed.
    typename Edge_Set::iterator position;
    bool falls_inside;
    size_t overlapping_portion = 0;
    Index  overlapping_start = Index(); // initialized to silent a warning.

    if (previous_to_start == _intervals.end())
    {
        // Previous element doesn't exist and there is neither any of 
        // equal value.
        // We have to insert start.
        position = _intervals.insert(start);
        falls_inside = false;
    } 
    else if (!previous_to_start->second)
    {
        // Previous element is the end of one interval.
        if (previous_to_start->first + 1 == start_value)
        {
            // The end of previous interval end is one unit less than the start
            // of this interval. Removing the edge to fuse both.
            position = previous_to_start;
            position--;
            _intervals.erase(previous_to_start);
        }
        else
        {
            // We have to insert start.
            position = _intervals.insert(previous_to_start, start);
        }
        falls_inside = false;
    }
    else
    {
        // Start has fallen inside another interval we don't have to insert it.
        position = previous_to_start;
        falls_inside = true;
        overlapping_start = start_value;
    }

    // Now we have to check where the end goes.
    ++position;
    while (position != _intervals.end() && position->first <= end_value)
    {
        // Calculating the length of a possible interval overlapping the 
        // interval being inserted.
        if (falls_inside) 
        {
            // Previous position was the start of an overlapping interval.
            bbp_assert(!position->second);
            overlapping_portion += position->first - overlapping_start + 1;
        }
        else
        {
            overlapping_start = position->first;
        }

        falls_inside = position->second;
        
        // Note that the post-increment is evaluated before the function call
        // So position is actually pointing to the next one before the previous
        // element is erased.
        _intervals.erase(position++);
    }

    if (position != _intervals.end() && 
        position->second && position->first == end_value + 1)
    {
        // The end of the interval connects with the start of the next one.
        // We remove the start of the following one and don't insert this
        // edge.
        _intervals.erase(position);
    } 
    else if (!falls_inside)
    {
        // End edge is not inside a previously existing interval so we
        // have to add it.
        _intervals.insert(position, end);
    }
    else
    {
        overlapping_portion += end_value - overlapping_start + 1;
    }

    _size += size_t(end_value - start_value + 1) - overlapping_portion;
    bbp_assert(_intervals.size() % 2 == 0);
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
void Interval_Set<Index, diff_t>::remove(Index start_value, Index end_value)
{
    bbp_assert(start_value <= end_value);

    if (_intervals.size() == 0)
        return;

    // Finding the first edge whose value is less or equal than start_value.
    typename Edge_Set::iterator next_to_start = 
        _intervals.lower_bound(std::make_pair(start_value, true));
    typename Edge_Set::iterator previous_to_start = next_to_start;
    if (next_to_start == _intervals.begin())
        previous_to_start = _intervals.end();
    else if (next_to_start == _intervals.end())
        // Nothing to remove here
        return;
    else
        --previous_to_start;

    typename Edge_Set::iterator position;
    bool inside;
    Index overlapping_start;
    size_t overlapping_portion = 0;

    if (previous_to_start != _intervals.end())
    {
        // start_value is greater or equal than some interval edge.
        if (previous_to_start->second)
        {
            // Inserting the new end of the interval starting at 
            // previous_to_start.
            position = 
                _intervals.insert(std::make_pair(start_value - 1, false));
            inside = true;
            overlapping_start = start_value;
        }
        else
        {
            position = previous_to_start;
            inside = false;
        }
        ++position;
    }
    else
    {
        // start_value is less than the start of any interval.
        inside = false;
        position = _intervals.begin();
        overlapping_start = position->first;
    }
    // Position has the next edge after the last interval before the removal
    // interval.

    while (position != _intervals.end() && position->first <= end_value)
    {
        if (inside)
            overlapping_portion += position->first - overlapping_start + 1;
        else
            overlapping_start = position->first;

        inside = position->second;
        
        // Note that the post-increment is evaluated before the function call
        // So position is actually pointing to the next one before the previous
        // element is erased.
        _intervals.erase(position++);
    }

    if (inside)
    {
        bbp_assert(position != _intervals.end());
        // End edge is not inside a previously existing interval so we
        // have to add it.
        _intervals.insert(std::make_pair(end_value + 1, true));
        overlapping_portion += end_value - overlapping_start + 1;
    }

    _size -= overlapping_portion;
    bbp_assert(_intervals.size() % 2 == 0);    
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
void Interval_Set<Index, diff_t>::remove(const_iterator interval)
{
    if (interval != end())
    {
        typename Edge_Set::const_iterator stop_edge = interval._start_edge;
        ++stop_edge;
        _size -= stop_edge->first - interval._start_edge->first + 1;
        // The following code will be standard conformat in a near future and
        // it already is for GNU STL and any others implementing 
        // multiset::iterator and multiset::const_iterator as the same type.
		// In the meantime we provide a less efficient version for MSVC
#ifdef _MSC_VER 
		_intervals.erase(*interval._start_edge);
        _intervals.erase(*stop_edge);
#else
        _intervals.erase(interval._start_edge);
        _intervals.erase(stop_edge);
#endif
    }            
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
bool Interval_Set<Index, diff_t>::is_present(Index x) const
{
    if (_intervals.size() == 0)
        return false;
    else
    {
        typename Edge_Set::iterator next = 
            _intervals.lower_bound(std::make_pair(x, false));
        if (next == _intervals.end() ||
            next == _intervals.begin())
            // x cannot be inside any interval.
            return false;
        return !next->second || next->first == x;
    }
}

//------------------------------------------------------------------------------

#ifdef WIN32
#undef min
#endif

template<typename Index, typename diff_t>
diff_t Interval_Set<Index, diff_t>::distance(Index x) const
{
    if (_intervals.size() == 0)
        return std::numeric_limits<diff_t>::max();
    else
    {
        typename Edge_Set::const_iterator next = 
            _intervals.lower_bound(std::make_pair(x, true));
        // The reverse iterator is offset by one element to the
        // left by default.
        typename Edge_Set::const_reverse_iterator previous(next);

        if (previous == _intervals.rend())
            // x is before the first interval.
            return next->first - x;
        else if (next == _intervals.end())
            // x is past the last interval.
            return x - previous->first;
        else
        {
            if (previous->second)
                // x is inside an iterval.
                return 0;
            else
                // x is between two intervals
                return std::min(previous->first > x ? 
                                previous->first - x : x - previous->first,
                                next->first > x ?
                                next->first - x : x - next->first);
        }
    }
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
typename Interval_Set<Index, diff_t>::const_iterator 
Interval_Set<Index, diff_t>::find(Index x) const
{
    if (_intervals.size() == 0)
        return end();
    else
    {
        typename Edge_Set::const_iterator next = 
            _intervals.lower_bound(std::make_pair(x, false));
        // Note that if x equals the start edge of any interval then
        // next will be the end edge due to the use of (x, false) in the
        // search.
        if (next == _intervals.end() ||
            next == _intervals.begin())
            // x cannot be inside any interval.
            return end();

        typename Edge_Set::const_iterator previous = next;
        --previous;

        if (previous->second)
            return const_iterator(previous);
        else
            return end();
    }
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
typename Interval_Set<Index, diff_t>::const_iterator 
Interval_Set<Index, diff_t>::begin() const
{
    if (_intervals.size() == 0)
        return end();
    else
        return const_iterator(_intervals.begin());
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
typename Interval_Set<Index, diff_t>::const_iterator 
Interval_Set<Index, diff_t>::end() const
{
    return const_iterator(_intervals.end());
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
typename Interval_Set<Index, diff_t>::const_iterator 
Interval_Set<Index, diff_t>::lower_bound(Index x) const
{
    typename Edge_Set::const_iterator bound = 
        _intervals.lower_bound(std::make_pair(x, false));
    if (bound == _intervals.end())
        return end();
    if (bound->second)
        return const_iterator(bound);
    else
        return const_iterator(--bound);
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
typename Interval_Set<Index, diff_t>::const_iterator 
Interval_Set<Index, diff_t>::upper_bound(Index x) const
{
    typename Edge_Set::iterator bound = 
        _intervals.upper_bound(std::make_pair(x, false));
    if (bound == _intervals.end())
        return end();
    if (bound->second)
        return const_iterator(bound);
    else
        return const_iterator(++bound);
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
size_t Interval_Set<Index, diff_t>::intervals() const
{
    return _intervals.size() / 2;
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
Index Interval_Set<Index, diff_t>::lower_value() const
{
    return _intervals.begin()->first;
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
Index Interval_Set<Index, diff_t>::upper_value() const
{
    return _intervals.rbegin()->second;
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
size_t Interval_Set<Index, diff_t>::size() const
{
    return _size;
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
void Interval_Set<Index, diff_t>::swap(Interval_Set & other)
{
    _intervals.swap(other._intervals);
}

//------------------------------------------------------------------------------

template<typename Index, typename diff_t>
std::ostream & operator << (std::ostream & out, 
                            const Interval_Set<Index, diff_t> & set)
{
    typename Interval_Set<Index, diff_t>::iterator i = set.begin();
    while (i != set.end())
    {
        out << '[' << i->first << ", " << i->second << "]";
        if (++i != set.end())
            out << ", ";
    }

    return out;
}

//------------------------------------------------------------------------------

}
}
#endif
