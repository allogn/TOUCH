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

#ifndef BBP_UNORDERED_INDEX_SET_H
#define BBP_UNORDERED_INDEX_SET_H

#include <cstddef>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/identity.hpp>

// ----------------------------------------------------------------------------

#ifndef BBP_INDEX_SET_H
template <typename Index, bool ordered>
class Index_Set;
#endif

#ifndef BBP_INTERVAL_SET_H
template <typename Index, typename diff_t>
class Interval_Set;
#endif

// ----------------------------------------------------------------------------

template <typename Index>
class Index_Set<Index, false>
{
public:
    class const_iterator;

    typedef const Index & const_reference;

    inline void insert(Index i);

    inline void remove(Index i);

    inline void insert_interval(Index start, Index end);

    inline void remove_interval(Index start, Index end);

    inline void insert(const Index_Set & other);

    inline bool exists(Index x) const;

    inline const_iterator find(Index x) const;

    inline const_iterator begin() const;

    inline const_iterator end() const;

    inline size_t size() const;

    inline void swap(Index_Set & other);

#ifndef NDEBUG
    void print()
    {
        std::cout << "Intervals " << _intervals << std::endl;
        std::cout << "Spare indices ";
        for (typename Spare_Index_Set::iterator i = _spare_indices.begin();
             i != _spare_indices.end();
             ++i)
        {
            std::cout << *i << ' ';
        }
        std::cout << std::endl;
    }
#endif

protected:
    static const size_t MAXIMUM_SPARE_ELEMENT_RUN_LENGTH = 3;
    static const size_t MINIMUM_INTERVAL_LENGTH = 3;

    friend class const_iterator;

    typedef Interval_Set<Index, std::ptrdiff_t> My_Interval_Set;

    typedef boost::multi_index::identity<Index> Index_Identity;
    typedef boost::multi_index_container
        <Index, // Type stored
         boost::multi_index::indexed_by<
             boost::multi_index::hashed_unique<Index_Identity> > >
    Spare_Index_Set;

    void collapse_sparse_elements_into_interval(Index & start, Index & end);

    void optimize_intervals(Index start, Index end);

    void optimize_spare_table(Index first_inserted_element,
                              Index last_inserted_element);

    My_Interval_Set     _intervals;
    Spare_Index_Set  _spare_indices;
};

//-----------------------------------------------------------------------------

template <typename Index>
class Index_Set<Index, false>::const_iterator : 
    public boost::iterator_facade<
        typename Index_Set<Index, false>::const_iterator,
        Index,
        std::forward_iterator_tag,
        Index>
{
    friend class Index_Set;
    friend class boost::iterator_core_access;


    typedef typename Index_Set<Index, false>::My_Interval_Set::const_iterator 
                     interval_iterator;
    typedef typename Index_Set<Index, false>::Spare_Index_Set::const_iterator
                     spare_iterator;

public:
    //! Default constructor to an end() iterator.
    const_iterator() {}

private:
    const_iterator(const Index_Set & set, 
                   const interval_iterator & interval,
                   const spare_iterator & spare_index) :
        _interval_iterator_end(set._intervals.end()),
        _interval(interval),
        _spare_index_end(set._spare_indices.end()),
        _spare_index(spare_index)
    {
#ifndef NDEBUG
        for (typename Spare_Index_Set::const_iterator i =
                 set._spare_indices.begin();
             i != set._spare_indices.end();
             ++i)
            bbp_assert(set._intervals.distance(*i) > 1);
#endif
        if (_interval != _interval_iterator_end)
            _current = _interval->first;
        else if (_spare_index != _spare_index_end)
            _current = *_spare_index;
    }

    const_iterator(const Index_Set & set, 
                   const interval_iterator & interval,
                   const spare_iterator & spare_index,
                   Index current) :
        _current(current),
        _interval_iterator_end(set._intervals.end()),
        _interval(interval),
        _spare_index_end(set._spare_indices.end()),
        _spare_index(spare_index)
    {
#ifndef NDEBUG
        for (typename Spare_Index_Set::const_iterator i =
                 set._spare_indices.begin();
             i != set._spare_indices.end();
             ++i)
            bbp_assert(set._intervals.distance(*i) > 1);
#endif
    }

private:
    void increment()
    {
        if (_interval != _interval_iterator_end)
        {
            if (_current >= _interval->first &&
                _current < _interval->second)
                // Next element is inside the current interval.
                ++_current;
            else
            {
                ++_interval;
                if (_interval != _interval_iterator_end)
                    _current = _interval->first;
                else
                    _current = *_spare_index;
            }
        }
        else
        {
            _current = *++_spare_index;
        }
    }

    bool equal(const_iterator const & other) const
    {
        return (
            (
            _interval          == _interval_iterator_end &&
            other._interval    == _interval_iterator_end &&
            _spare_index       == _spare_index_end       &&
            other._spare_index == _spare_index_end
            )
            ||
            (
            _interval          == other._interval        &&
            _spare_index       == other._spare_index     &&
            _current           == other._current 
            )
               );
    }

    Index dereference() const
    {
        return _current;
    }

    Index _current;
    interval_iterator _interval_iterator_end;
    interval_iterator _interval;
    spare_iterator _spare_index_end;
    spare_iterator _spare_index;
};

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, false>::insert(Index i)
{
    switch (_intervals.distance(i)) 
    {
    case 0: 
        break; // Nothing to do.
    case 1:
        {
            bbp_assert(_spare_indices.find(i) == _spare_indices.end());
            Index start = i, end = i;
            collapse_sparse_elements_into_interval(start, end);
            _intervals.insert(start, end);
            break; // Inserting as an interval.
        }
    default:
        {
            std::pair<typename Spare_Index_Set::iterator, bool> insertion =
                _spare_indices.insert(i);
            if (insertion.second)
                optimize_spare_table(i, i);
        }
    }
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, false>::remove(Index i)
{
    _intervals.remove(i, i);
    _spare_indices.erase(i);
    optimize_intervals(i, i);
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, false>::insert_interval(Index start, Index end)
{
    size_t length = (size_t)(end - start);
    if (length == 0)
    {
        insert(start);
        return;
    }
        
    if (length < MINIMUM_INTERVAL_LENGTH &&
        _intervals.distance(start) > 1 && _intervals.distance(end) > 1)
    {
        // This interval is too small and it's not next to other one.
        // Inserting it in the spare index table.
        for (Index i = start; i < end + 1; ++i)
            _spare_indices.insert(i);

        optimize_spare_table(start, end);
    }
    else
    {
        collapse_sparse_elements_into_interval(start, end);
        // Inserting the interval.
        _intervals.insert(start, end);
    }
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, false>::remove_interval(Index start, Index end)
{
    _intervals.remove(start, end);
    for (Index i = start; i <= end; ++i)
        _spare_indices.erase(i);
    optimize_intervals(start, end);
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, false>::insert(const Index_Set & other)
{
    for (typename My_Interval_Set::const_iterator i = other._intervals.begin();
         i != other._intervals.end();
         ++i)
        insert_interval(i->first, i->second);
    for (typename Spare_Index_Set::const_iterator i = 
             other._spare_indices.begin();
         i != other._spare_indices.end();
         ++i)
        insert(*i);
}

//-----------------------------------------------------------------------------

template <typename Index>
bool Index_Set<Index, false>::exists(Index x) const
{
    if (_spare_indices.find(x) != _spare_indices.end())
        return true;
    else
        return _intervals.is_present(x);
}

//-----------------------------------------------------------------------------

template <typename Index>
typename Index_Set<Index, false>::const_iterator 
Index_Set<Index, false>::find(Index x) const
{
    typename Spare_Index_Set::const_iterator i = _spare_indices.find(x);
    if (i != _spare_indices.end())
        return const_iterator(*this, _intervals.end(), i, x);
    else
    {
        typename My_Interval_Set::const_iterator i = _intervals.find(x);
        if (i != _intervals.end())
            return const_iterator(*this, i, _spare_indices.begin(), x);
        else
            return end();
    }
}

//-----------------------------------------------------------------------------

template <typename Index>
typename Index_Set<Index, false>::const_iterator 
Index_Set<Index, false>::begin() const
{
    return const_iterator(*this, _intervals.begin(), 
                          _spare_indices.begin());
}

//-----------------------------------------------------------------------------

template <typename Index>
typename Index_Set<Index, false>::const_iterator 
Index_Set<Index, false>::end() const
{
    return const_iterator(*this, _intervals.end(), 
                          _spare_indices.end());
}

//-----------------------------------------------------------------------------

template <typename Index>
size_t Index_Set<Index, false>::size() const
{
    return _intervals.size() + _spare_indices.size();
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, false>::swap(Index_Set & other)
{
    _intervals.swap(other._intervals);
    _spare_indices.swap(other._spare_indices);
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, false>::optimize_spare_table(Index first, Index last)
{
    // Searching the largest contiguous range exteneded from [first, last]
    while (_spare_indices.find(first - 1) != _spare_indices.end())
        --first;
    while (_spare_indices.find(last + 1) != _spare_indices.end())
        ++last;

    if ((size_t)(last - first) > MAXIMUM_SPARE_ELEMENT_RUN_LENGTH)
    {
        // The interval is large enough to deserve conversion to an interval.
        for (Index i = first; i <= last; ++i)
            _spare_indices.erase(i);
        _intervals.insert(first, last);
    }
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, false>::optimize_intervals(Index start, Index end)
{
    typename My_Interval_Set::const_iterator previous = 
        _intervals.find(start - 1);
    typename My_Interval_Set::const_iterator next = 
        _intervals.find(end + 1);

    if (previous != _intervals.end() &&
        size_t(previous->second - previous->first) < MINIMUM_INTERVAL_LENGTH)
    {
        for (Index index = previous->first; index <= previous->second; ++index)
            _spare_indices.insert(index);
        _intervals.remove(previous);
    } 

    if (next != _intervals.end() &&
        size_t(next->second - next->first) < MINIMUM_INTERVAL_LENGTH)
    {
        for (Index index = next->first; index <= next->second; ++index)
            _spare_indices.insert(index);
        _intervals.remove(next);
    } 
}


//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, false>::collapse_sparse_elements_into_interval
(Index & start, Index & end)
{
    // Removing the range [start, end] from the spare indices
    for (Index i = start; i < end && _spare_indices.size() > 0; ++i)
        _spare_indices.erase(i);

    // Checking if there are contiguous indices to [start, end] and 
    // enlarging the interval while removing those indices.
    typename Spare_Index_Set::iterator index;
    // Searching next to end using the undorderde index and 
    // converting iterator to an ordered one.
    while ((index = _spare_indices.find(end + 1)) != _spare_indices.end())
    {
        // Enlarging tail of the interval
        ++end;
        _spare_indices.erase(index);
    }

    // Searching previous to start using the unordered index and
    // converting iterator to and ordered one.
    while ((index = _spare_indices.find(start - 1)) != _spare_indices.end())
    {
        // Enlarging tail of the interval
        --start;
        _spare_indices.erase(index);
    }

#ifdef BBP_UNORDERED_INDEX_SET_SHRINKS
    if (_spare_indices.load_factor() < _spare_indices.max_load_factor() * 0.4)
    {
        Spare_Index_Set shrunk;
        shrunk.rehash(_spare_indices.bucket_count() * 0.4);
        for (typename Spare_Index_Set::iterator i = _spare_indices.begin();
             i != _spare_indices.end();
             ++i)
        {
            shrunk.insert(*i);
        }
        _spare_indices.swap(shrunk);
    }
#endif

}

//-----------------------------------------------------------------------------

#endif
