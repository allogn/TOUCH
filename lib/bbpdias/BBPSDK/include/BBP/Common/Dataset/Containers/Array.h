/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2008. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_ARRAY_H
#define BBP_ARRAY_H

#include <boost/type_traits/add_const.hpp>
#include <boost/shared_array.hpp>
#include <cstring>
#include "BBP/Common/Exception/Exception.h"
#include "BBP/Common/Patterns/shared_array_helpers.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! STL container interface to a fixed size plain C array in memory.
/*!
    DISCLAIMER: The memory is not deallocated if the array has not been created
    by the container itself or ownership has been taken (this is not the 
    default!).
    Copying Array objects copies the internal array reference. When the 
    original array is owning the array, the ownership will be shared and 
    referenced counted.
    
    \todo A second template parameter could specify an interface object type
    to data stored internally in a raw array e.g. for structs. (TT)

    \ingroup Containers
*/
template <typename T>
class Array
{
    typedef typename boost::add_const<T>::type const_T;
public:
    //! Create an empty container for an array
    Array();
    //! Create a new fixed size array (and manage its memory lifetime).
    Array(size_t size);
    //! Map to an existing array (without taking memory lifetime ownership).
    Array(T * array_ptr, size_t array_size, bool take_ownership = false);
    //! Map to an existing array using a boost::shared_array
    Array(const boost::shared_array<T> & array, size_t array_size);

public:
    //! Get length of array.
    size_t size() const;

    //! Get pointer with read acccess to memory address of array offset in memory.
    const T * pointer() const;
    //! Get pointer with full acccess to memory address of array offset in memory.
    T * pointer();

    //! Get the internal pointer as a shared pointer
    const boost::shared_array<T> & shared_pointer();
    const boost::shared_array<const_T> &shared_pointer() const;

    //! Get full access to element at specified index.
    T & operator() (size_t index);
    //! Get full access to element at specified index.
    T & operator[] (size_t index);

    //! Forward iterator with linear access to visit all elements in container.
    class iterator;
    //! Get iterator to first element in the array.
    inline iterator begin();
    //! Get iterator point one element past the last element of the container.
    inline const iterator end();

    //! Get read access to element at specified index.
    /*! \todo Why two different syntaxes for the same thing? (JH) */
    const T & operator() (size_t index) const;
    //! Get read access to element at specified index.
    /*! \todo Why two different syntaxes for the same thing? (JH) */
    const T & operator[] (size_t index) const;

    //! Forward iterator with linear access to visit all elements in container.
    /*! \todo Random Access iterator */
    class const_iterator;
    //! Get iterator to first element in the array.
    inline const_iterator begin() const;
    //! Get iterator point one element past the last element of the container.
    inline const const_iterator end() const;

    //! Increase or decrease the number of elements stored in the array.
    /*!
        This will copy all the elements if necessary. Note this will resize 
        the array exactly. If used multiple times
        consider using reserve to size the array in advance.
        CAUTION: If the array is owned externally, the original array
        will stay the same size with the same contents afterwards independent
        of this array wrapper.
    */
    void resize(size_t new_size, const T & new_elements_default_value = T())
    {
        // if vector is upsized
        if (new_size > _size)
        {
            size_t old_size = _size;
            // if the new size is bigger than the old one
            if (new_size > _reserved)
            {
                reserve(new_size);
                _size = new_size;
            }
            // initialize new elements
            for (size_t i = old_size; i < new_size; ++i)
            {
                _array[i] = new_elements_default_value;
            }
        }
        // if vector is downsized
        else 
        {
            _size = new_size;
        }
    }

    //! Allocated size of internal array.
    /*!
        Up to this boundary no reallocations are necessary
        when adding elements. 
        
        CAUTION: If the size is smaller than
        the currently reserved size, reserve will downsize
        if the new reserved size is smaller, but will not
        delete valid elements in the existing array. This means
        if there are 3 valid elements in an array reserved for
        6 elements and the new reserved size is 2, that instead
        of reallocating the array with the requested 2 elements, 
        it will be reallocated with 3 elements.
    */
    void reserve(size_t new_size)
    {
        // if upsize requested
        if (new_size > _reserved)
        {
            // allocate new array                
            T * new_array = new T [new_size];
            // copy elements
            std::memcpy(new_array, _array.get(), sizeof(T) * _size);
            _array.reset(new_array);
            _reserved = new_size;
        }
        // if downsize requested and there are more reserved elements 
        // than contained elements size it to the number of contained
        // elements or more if requested
        else if (_size < _reserved)
        {
            if (new_size < _size)
                new_size = _size;

            // allocate new array                
            T * new_array = new T [new_size];
            // copy elements
            std::memcpy(new_array, _array.get(), sizeof(T) * new_size);
            _array.reset(new_array);
            _reserved = new_size;
        }
    }

private:
    //! A smart pointer managed raw C array.
    boost::shared_array<T>     _array;
    //! Number of elements in the array.
    size_t  _size;
    //! Number of allocated internal array size.
    size_t  _reserved;
};

#ifndef SWIG // ---------------------------------------------------------------

template <typename T>
class Array<T>::iterator 
    : public std::iterator<std::forward_iterator_tag, T>
{
    friend class Array<T>;
    friend class Array<T>::const_iterator;
public:
    iterator()
      : _array(0), _current(0)
    {}

    iterator(T * memory_offset, size_t current_index)
      : _array(memory_offset), _current(current_index)
    {}

    iterator(const Array<T> & object, size_t current_index)
      : _array(object._array), _current(current_index)
    {}

    ~iterator(void)
    {
    }

    //! Compare two iterators for equality.
    template <class Other_Iterator>
    bool operator == (const Other_Iterator & rhs) const
    {
        return (_current == rhs._current) &&
               (_array == rhs._array);
    }

    //! Compare two iterators for inequality.
    template <class Other_Iterator>
    bool operator != (const Other_Iterator & rhs) const
    {
        return (_current != rhs._current) ||
               (_array != rhs._array);
    }

    //! Advance to next element in the container.
    iterator & operator ++()
    {
        ++_current;
        return * this;
    }

    //! Advance to next element in the container, but return previous.
    iterator operator ++(int)
    {
        iterator temp(*this);
        ++_current;
        return temp;
    }

    //! Get (dereference) element at current iterator position.
    T & operator * ()
    {
        return * (_array + _current);
    }

    //! Get pointer to element at current iterator position.
    T * operator -> ()
    {
        return & (this->operator *());
    }

private:
  T *      _array;
  size_t   _current;
  
};

// ----------------------------------------------------------------------------

template <typename T>
class Array<T>::const_iterator
    : public std::iterator<std::forward_iterator_tag, T>
{
    friend class Array<T>;
    friend class Array<T>::iterator;
public:
    const_iterator()
      : _array(0), _current(0)
    {}

    const_iterator(const T * memory_offset, size_t current_index)
      : _array(memory_offset), _current(current_index)
    {}

    const_iterator(const Array<T> & object, size_t current_index)
      : _array(object._array), _current(current_index)
    {}

    //! Compare two iterators for equality.
    template <class Other_Iterator>
        const_iterator(const Other_Iterator & rhs)
      : _array(rhs._array), _current(rhs._current)
    {
    }

    ~const_iterator(void)
    {
    }

    //! Compare two iterators for equality.
    template <class Other_Iterator>
    bool operator == (const Other_Iterator & rhs) const
    {
        return (_current == rhs._current) &&
               (_array == rhs._array);
    }

    //! Compare two iterators for inequality.
    template <class Other_Iterator>
    bool operator != (const Other_Iterator & rhs) const
    {
        return !(*this == rhs);
    }

    //! Advance to next element in the container.
    const_iterator & operator ++()
    {
        ++_current;
        return * this;
    }

    //! Advance to next element in the container, but return previous.
    const_iterator operator ++(int)
    {
        const_iterator temp(*this);
        ++_current;
        return temp;
    }

    //! Get (dereference) element at current iterator position.
    const T & operator * ()
    {
        return * (_array + _current);
    }

    //! Get pointer to element at current iterator position.
    const T * operator -> ()
    {
        return & (this->operator *());
    }

private:
  const T * _array;
  size_t    _current;
  
};

template <typename T> inline std::ostream & operator <<
(std::ostream &out, const Array<T> & array)
{
    out << "( ";
    for (typename Array<T>::const_iterator i = array.begin();
         i != array.end(); ++i)
    {
        out << *i << ' ';
    }
    out << ')' << std::endl;
    return out;
}

// ----------------------------------------------------------------------------

template <typename T>
Array<T>::Array()
    : _size(0)
{
}

// ----------------------------------------------------------------------------

template <typename T>
Array<T>::Array(size_t size)
    : _array(new T[size]), _size(size)
{
}

// ----------------------------------------------------------------------------

template <typename T> inline
Array<T>::Array(T * array_ptr, size_t array_size, bool take_ownership) 
    : _size(array_size)
{
    if (take_ownership)
        _array.reset(array_ptr);
    else
        _array.reset(array_ptr, shared_array_non_deleter);
}

// ----------------------------------------------------------------------------

template <typename T> inline
Array<T>::Array(const boost::shared_array<T> & array, size_t array_size)
    : _array(array), 
      _size(array_size)
{}

// ----------------------------------------------------------------------------

template <typename T> inline
T & Array<T>::operator() (size_t index)
{
#ifdef SAFETY_MODE
        if (index >= _size)
            throw runtime_error("Out of bounds");
#endif
    bbp_assert(index < _size);
    return _array[index];
}

// ----------------------------------------------------------------------------

template <typename T> inline
T & Array<T>::operator[] (size_t index)
{
#ifdef SAFETY_MODE
        if (index >= _size)
            throw runtime_error("Out of bounds");
#endif
    bbp_assert(index < _size);
    return _array[index];
}

// ----------------------------------------------------------------------------

template <typename T> inline
const T & Array<T>::operator() (size_t index) const
{
#ifdef SAFETY_MODE
        if (index >= _size)
            throw runtime_error("Out of bounds");
#endif
    bbp_assert(index < _size);
    return _array[index];
}

// ----------------------------------------------------------------------------

template <typename T> inline
const T & Array<T>::operator[] (size_t index) const
{
#ifdef SAFETY_MODE
        if (index >= _size)
            throw runtime_error("Out of bounds");
#endif
    bbp_assert(index < _size);
    return _array[index];
}

// ----------------------------------------------------------------------------

template<typename T> inline
size_t Array<T>::size() const
{
    return _size;
}

// ----------------------------------------------------------------------------

template <typename T> inline
const T * Array<T>::pointer() const
{
    return _array.get();
}

// ----------------------------------------------------------------------------

template <typename T> inline
T * Array<T>::pointer()
{
    return _array.get();
}

// ----------------------------------------------------------------------------

template <typename T> inline
const boost::shared_array<typename boost::add_const<T>::type > &
Array<T>::shared_pointer() const
{
    /* Automatic cast from shared_array<T> to shared_array<const T> is not
       supported */
    return *static_cast<const boost::shared_array<const_T> *>
           (static_cast<const void*>(&_array));
}

// ----------------------------------------------------------------------------

template <typename T> inline
const boost::shared_array<T> &Array<T>::shared_pointer()
{
    return _array;
}

// ----------------------------------------------------------------------------

template <typename T>
typename Array<T>::iterator Array<T>::begin()
{
    return iterator(_array.get(), 0);
}

// ----------------------------------------------------------------------------

template <typename T>
typename Array<T>::iterator const Array<T>::end()
{
    return iterator(_array.get(), size());
}

// ----------------------------------------------------------------------------

template <typename T>
typename Array<T>::const_iterator Array<T>::begin() const
{
    return const_iterator(_array.get(), 0);
}

// ----------------------------------------------------------------------------

template <typename T>
typename Array<T>::const_iterator const Array<T>::end() const
{
    return const_iterator(_array.get(), size());
}

#endif // SWIG ----------------------------------------------------------------

}
#endif
