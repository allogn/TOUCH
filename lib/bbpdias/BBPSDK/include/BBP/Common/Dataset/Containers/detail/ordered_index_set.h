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

#ifndef BBP_ORDERED_INDEX_SET_H
#define BBP_ORDERED_INDEX_SET_H

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

template <typename Index>
class Index_Set<Index, true>
{
public:
    class const_iterator;
    typedef const_iterator iterator;

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
        for (ordered_spare_iterator i = _spare_indices.template 
                                        get<1>().begin();
             i != _spare_indices.template get<1>().end();
             ++i)
        {
            std::cout << *i << ' ';
        }
        std::cout << std::endl;
    }
#endif

protected:
    static const size_t MAXIMUM_SPARE_ELEMENT_RUN_LENGTH = 5;
    static const size_t MINIMUM_INTERVAL_LENGTH = 4;

    friend class const_iterator;

    typedef Interval_Set<Index, std::ptrdiff_t> My_Interval_Set;

    typedef boost::multi_index::identity<Index> Index_Identity;
    typedef boost::multi_index_container
    <Index, // Type stored
     boost::multi_index::indexed_by<
         boost::multi_index::hashed_unique<Index_Identity >,
         boost::multi_index::ordered_unique<Index_Identity> > >
    Spare_Index_Set;

    typedef typename Spare_Index_Set::template 
            nth_index_const_iterator<1>::type ordered_spare_iterator;

    void collapse_sparse_elements_into_interval(Index & start, Index & end);

    void optimize_intervals(Index start, Index end);

    void optimize_spare_table(ordered_spare_iterator first_inserted_element,
                              ordered_spare_iterator last_inserted_element);

    My_Interval_Set     _intervals;
    Spare_Index_Set  _spare_indices;
};

//-----------------------------------------------------------------------------

template <typename Index>
class Index_Set<Index, true>::const_iterator : 
    public boost::iterator_facade<
        typename Index_Set<Index, true>::const_iterator,
        Index,
        std::bidirectional_iterator_tag, 
        Index>
{
    friend class Index_Set;
    friend class boost::iterator_core_access;


    typedef typename Index_Set<Index, true>::My_Interval_Set::const_iterator 
                     interval_iterator;
    typedef typename Index_Set<Index, true>::Spare_Index_Set::template
                     nth_index_const_iterator<1>::type spare_iterator;

public:
    //! Default constructor to an end() iterator.
    const_iterator() {}

private:
    const_iterator(const Index_Set & set,
                   const interval_iterator & interval,
                   const spare_iterator & spare_index,
                   Index current) :
        _current(current),
        _interval_iterator_end(set._intervals.end()),
        _interval_iterator_begin(set._intervals.begin()),
        _interval(interval),
        _spare_index_end(set._spare_indices.template get<1>().end()),
        _spare_index_begin(set._spare_indices.template get<1>().begin()),
        _spare_index(spare_index)
    {
#ifndef NDEBUG
        for (typename Spare_Index_Set::const_iterator i =
                 set._spare_indices.begin();
             i != set._spare_indices.end();
             ++i)
            bbp_assert(set._intervals.distance(*i) > 1);

        if (_interval != _interval_iterator_end) 
        {
            if (_spare_index == _spare_index_end ||
                _interval->first < *_spare_index)
            {
                bbp_assert(_current >= _interval->first);
                bbp_assert(_current <= _interval->second);
            }
            else
                bbp_assert(_current == *_spare_index);
        }            
        else
            bbp_assert(_spare_index != _spare_index_end && 
                   _current == *_spare_index);
#endif
    }

    const_iterator(const Index_Set & set, 
                   const interval_iterator & interval,
                   const spare_iterator & spare_index) :
        _interval_iterator_end(set._intervals.end()),
        _interval_iterator_begin(set._intervals.begin()),
        _interval(interval),
        _spare_index_end(set._spare_indices.template get<1>().end()),
        _spare_index_begin(set._spare_indices.template get<1>().begin()),
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
            if (_spare_index == _spare_index_end)
                _current = _interval->first;
            else
                _current = std::min(_interval->first, *_spare_index);
        else if (_spare_index != _spare_index_end)
            _current = *_spare_index;
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
                if (_current == _interval->second)
                {
                    // Reached the end of current interval.
                    // Going to next one. 
                    ++_interval;
                }
                else if (_spare_index != _spare_index_end)
                {
                    ++_spare_index;
                }

                // Checking if next element has to be from the next interval or
                // from spare index table.
                if (_spare_index != _spare_index_end)
                {
                    if (_interval != _interval_iterator_end &&
                        *_spare_index > _interval->first)
                    {
                        // Invariant checking
                        bbp_assert(*_spare_index > _interval->second);
                        // Next element must be from the next interval.
                        _current = _interval->first;
                    }
                    else
                    {
                        // Next element must be from the spare table.
                        _current = *_spare_index;
                    }
                }
                else
                {
                    /*! 
                        \todo Review this code - I removed a bug here 
                        with incrementing an end iterator. (TT)
                    */
                    if (_interval != _interval_iterator_end)
                    {
                        _current = _interval->first;
                    }
                    else
                    {
                        ++_current;
                    }
                }
            }
        }
        else
        {
            _current = *++_spare_index;
        }
    }

    void decrement()
    {
        if (_interval_iterator_end != _interval_iterator_begin)
        {
            if (_interval != _interval_iterator_end &&
                _current > _interval->first && _current <= _interval->second)
                // Previous element is inside the current interval.
                --_current;
            else
            {
                // Current element is pointed by _spare_index or past the end.
                // Previous element can be --_spare_index
                // or inside an interval.
                spare_iterator previous_spare = _spare_index;
                if (_spare_index != _spare_index_begin)
                    --previous_spare;

                if (_interval != _interval_iterator_begin)
                {
                    interval_iterator previous_interval = _interval;
                    --previous_interval;

                    if (_spare_index != _spare_index_begin &&
                        // Now we know that previous_spare is defined.
                        *previous_spare > previous_interval->second)
                    {
                        _current = *previous_spare;
                        _spare_index = previous_spare;
                    }
                    else
                    {
                        // Previous element (if defined) will be pointed by
                        // previous_interval. It doesn't matter if it's 
                        // invalid, in that case the user is writting code with 
                        // undefined behaviour.
                        _current = previous_interval->second;
                        _interval = previous_interval;
                    }
                }
                else
                {
                    // Previous element (if defined) will be pointed by
                    // previous_spare. As said above we don't care if it's
                    // invalid.
                    _current = *previous_spare;
                    _spare_index = previous_spare;
                }
            }
        }
        else
        {
            _current = *--_spare_index;
        }
    }   

    bool equal(const const_iterator & other) const
    {
        return ((_interval == _interval_iterator_end &&
                 other._interval == _interval_iterator_end &&
                 _spare_index == _spare_index_end &&
                 other._spare_index == _spare_index_end) ||
                (_interval == other._interval &&
                 _spare_index == other._spare_index &&
                 _current == other._current));
    }

    Index dereference() const
    {
        return _current;
    }

    Index _current;
    interval_iterator _interval_iterator_end;
    interval_iterator _interval_iterator_begin;
    interval_iterator _interval;
    spare_iterator _spare_index_end;
    spare_iterator _spare_index_begin;
    spare_iterator _spare_index;
};

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, true>::insert(Index i)
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
                optimize_spare_table
                    (_spare_indices.template project<1>(insertion.first),
                     _spare_indices.template project<1>(insertion.first));
        }
    }
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, true>::remove(Index i)
{
    _intervals.remove(i, i);
    _spare_indices.erase(i);
    optimize_intervals(i, i);
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, true>::insert_interval(Index start, Index end)
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
        typename Spare_Index_Set::iterator start_insert_position;
        typename Spare_Index_Set::iterator end_insert_position;
        start_insert_position = _spare_indices.insert(start).first;
        for (size_t i = 1; i < (size_t) (end - start); ++i)
            _spare_indices.insert(start + i);            
        end_insert_position = _spare_indices.insert(end).first;

        optimize_spare_table
            (_spare_indices.template project<1>(start_insert_position),
             _spare_indices.template project<1>(end_insert_position));
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
void Index_Set<Index, true>::remove_interval(Index start, Index end)
{
    _intervals.remove(start, end);
    _spare_indices.template get<1>().erase
        (_spare_indices.template get<1>().lower_bound(start),
         _spare_indices.template get<1>().upper_bound(end));
    optimize_intervals(start, end);
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, true>::insert(const Index_Set & other)
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
bool Index_Set<Index, true>::exists(Index x) const
{
    if (_spare_indices.find(x) != _spare_indices.end())
        return true;
    else
        return _intervals.is_present(x);
}

//-----------------------------------------------------------------------------

template <typename Index>
typename Index_Set<Index, true>::const_iterator 
Index_Set<Index, true>::find(Index x) const
{
    typedef typename Index_Set<Index>::Spare_Index_Set::template
                     nth_index_const_iterator<1>::type spare_iterator;
    spare_iterator i = _spare_indices.template get<1>().lower_bound(x);
    typename My_Interval_Set::const_iterator j = _intervals.lower_bound(x);
    if ((i != _spare_indices.template get<1>().end() && *i == x) ||
        (j != _intervals.end() && j->first <= x))
        return const_iterator(*this, j, i, x);
    else
        return end();
}

//-----------------------------------------------------------------------------

template <typename Index>
typename Index_Set<Index, true>::const_iterator 
Index_Set<Index, true>::begin() const
{
    return const_iterator(*this, _intervals.begin(), 
                          _spare_indices.template get<1>().begin());
}

//-----------------------------------------------------------------------------

template <typename Index>
typename Index_Set<Index, true>::const_iterator 
Index_Set<Index, true>::end() const
{
    return const_iterator(*this, _intervals.end(), 
                          _spare_indices.template get<1>().end());
}

//-----------------------------------------------------------------------------

template <typename Index>
size_t Index_Set<Index, true>::size() const
{
    return _intervals.size() + _spare_indices.size();
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, true>::swap(Index_Set & other)
{
    _intervals.swap(other._intervals);
    _spare_indices.swap(other._spare_indices);
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, true>::optimize_spare_table
(ordered_spare_iterator first, ordered_spare_iterator last)
{
    typedef typename Spare_Index_Set::template nth_index<1>::type 
        Ordered_Sparse_Index_Set;
    Ordered_Sparse_Index_Set & spare_indices_ordered = 
        _spare_indices.template get<1>();

    typedef typename Spare_Index_Set::template nth_index<1>::type::
        reverse_iterator reverse_ordered_spare_iterator;
    reverse_ordered_spare_iterator previous(first);

    // Searching the largest contiguous range exteneded from [first, last]
    Index start = *first;
    while (previous != spare_indices_ordered.rend() && *previous == start - 1)
    {
        --start;
        ++previous;
    }
    Index end = *last++;
    while (last != spare_indices_ordered.end() && *last == end + 1)
    {
        ++end;
        ++last;
    }

    if ((size_t)(end - start) > MAXIMUM_SPARE_ELEMENT_RUN_LENGTH)
    {
        // The interval is large enough to deserve conversion to an interval.
        spare_indices_ordered.erase(previous.base(), last);
        _intervals.insert(start, end);
    }
}

//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, true>::optimize_intervals(Index start, Index end)
{
    typename My_Interval_Set::const_iterator previous = 
        _intervals.find(start - 1);
    typename My_Interval_Set::const_iterator next = 
        _intervals.find(end + 1);

    if (previous != _intervals.end() &&
        (size_t)(previous->second - previous->first) < MINIMUM_INTERVAL_LENGTH)
    {
        for (Index index = previous->first; index <= previous->second; ++index)
            _spare_indices.insert(index);
        _intervals.remove(previous);
    } 

    if (next != _intervals.end() &&
        (size_t)(next->second - next->first) < MINIMUM_INTERVAL_LENGTH)
    {
        for (Index index = next->first; index <= next->second; ++index)
            _spare_indices.insert(index);
        _intervals.remove(next);
    } 
}


//-----------------------------------------------------------------------------

template <typename Index>
void Index_Set<Index, true>::collapse_sparse_elements_into_interval
(Index & start, Index & end)
{
    // Removing the range [start, end] from the spare indices
    for (Index i = start; i < end && _spare_indices.size() > 0; ++i)
        _spare_indices.erase(i);

    typedef typename Spare_Index_Set::template nth_index<1>::type 
        Ordered_Sparse_Index_Set;
    Ordered_Sparse_Index_Set & spare_indices_ordered = 
        _spare_indices.template get<1>();
    typedef typename Ordered_Sparse_Index_Set::reverse_iterator
        reverse_ordered_spare_iterator;

    // Checking if there are contiguous indices to [start, end] and 
    // enlarging the interval while removing those indices.

    // Searching next to end using the undorderde index and 
    // converting iterator to an ordered one.
    ordered_spare_iterator tail = 
        _spare_indices.template project<1>(_spare_indices.find(end + 1));

    while (tail != spare_indices_ordered.end() && *tail == end + 1)
    {
        // Enlarging tail of the interval
        ++end;
        spare_indices_ordered.erase(tail++);
    }

    // Searching previous to start using the unordered index and
    // converting iterator to and ordered one. 
    // Searching back all contiguous elements from head to remove then
    // after.
    ordered_spare_iterator head_first = 
        _spare_indices.template project<1>(_spare_indices.find(start - 1));
    if (head_first != spare_indices_ordered.end())
        ++head_first;
    reverse_ordered_spare_iterator head(head_first);
    
    while (head != spare_indices_ordered.rend() && *head == start - 1)
    {
        --start;
        ++head;
    }
    spare_indices_ordered.erase(head.base(), head_first);


    bbp_assert(_spare_indices.find(start - 1) == _spare_indices.end());
    bbp_assert(_spare_indices.find(end + 1) == _spare_indices.end());
}

//-----------------------------------------------------------------------------

#endif // INCLUDEGUARD
