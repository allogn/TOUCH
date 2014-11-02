/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible authors:    Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_STIMULI_SPECIFICATION_H
#define BBP_STIMULI_SPECIFICATION_H

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include "BBP/Model/Experiment/Stimulus_Specification.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! A set of stimulus protocol specifications of an experiment.
/*! 
    The class is a container for simulation stimulus specifications.
    \sa Stimulus_Specification Report Compartment_Report Experiment
    \ingroup Experiment_Containers
*/
class Stimuli_Specification
{
public:
    /*!
        \name Access (read-only)
        For access and navigation inside the container.
    */
    //@{
    //! Forward iterator for container to visit all elements (read-only).
    class const_iterator;
    //! Get first element of the container for const linear access.
    inline const_iterator begin() const;
    //! Get const iterator that indicates the end of the container.
    inline const_iterator end() const;
    /*! 
      Get stimulus via identifier label.
      Iterator to end returned if stimulus not found
    */
    inline const_iterator find(const Label & name) const;
    /*!
        \name Access (full)
        For access and navigation inside the container.
    */
    //@{

    //! Forward iterator for container to visit all elements (full access).
    class iterator;
    //! Get first element of the container for const linear access.
    inline iterator begin();
    //! Get const iterator that indicates the end of the container.
    inline iterator end();
    /*!
      Get stimulus via identifier label.
      Iterator to end returned if stimulus not found
    */
    inline iterator find(const Label & name);
    //! Get number of stimuli in the container.
    inline size_t size() const;
    //@}

    /*!
        \name Management
        Insert to and remove elements from the container.
    */
    //@{
    inline void insert(const Stimulus_Specification & stimulus);
    //! Erase stimuli with specified identifier label.
    inline void erase(const Label & name);
    //! Remove all stimuli.
    inline void clear();
    //@}

    //! Print the stimuli inside this container
    inline void print() const;

protected:
    typedef std::map<Label, Stimulus_Specification> Stimuli_Specification_Map;
    Stimuli_Specification_Map        _elements;
};

//! Outputs basic textual stimulus information to the standard output stream.
inline std::ostream & operator << (std::ostream & lhs, 
                                   const Stimuli_Specification & rhs);
}
#include "BBP/Model/Experiment/Stimulus_Specification.h"

namespace bbp
{

// ----------------------------------------------------------------------------

class Stimuli_Specification::iterator
    : public std::iterator<std::forward_iterator_tag, 
        Stimulus_Specification>
{
    friend class Stimuli_Specification;
    friend class Stimuli_Specification::const_iterator;

public:
    iterator()
    {}

private:
    explicit iterator(Stimuli_Specification::
        Stimuli_Specification_Map::iterator iterator)
        : _current(iterator)
    {}

public:
    //! Compare two iterators for equality.
    template <class Other_Iterator>
    bool operator == (const Other_Iterator & other) const
    {
        return (_current == other._current);
    }

    //! Compare two iterators for inequality.
    template <class Other_Iterator>
    bool operator != (const Other_Iterator & other) const
    {
        return (_current != other._current);
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
    Stimulus_Specification & operator * () const
    {
        return _current->second;
    }

    //! Get pointer to element at current iterator position.
    Stimulus_Specification * operator -> () const
    {
        return & (this->operator *());
    }

private:
    Stimuli_Specification::Stimuli_Specification_Map::iterator _current;

};

// ----------------------------------------------------------------------------

class Stimuli_Specification::const_iterator
    : public std::iterator<std::forward_iterator_tag, 
        const Stimulus_Specification>
{
    friend class Stimuli_Specification;
    friend class Stimuli_Specification::iterator;

public:
    const_iterator()
    {}

    const_iterator(Stimuli_Specification::iterator other) :
        _current(other._current)
    {}

private:
    explicit const_iterator(Stimuli_Specification::
        Stimuli_Specification_Map::const_iterator iterator)
        : _current(iterator)
    {}

public:
    //! Compare two iterators for equality.
    template <class Other_Iterator>
    bool operator == (const Other_Iterator & other) const
    {
        return (_current == other._current);
    }

    //! Compare two iterators for inequality.
    template <class Other_Iterator>
    bool operator != (const Other_Iterator & other) const
    {
        return (_current != other._current);
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
    const Stimulus_Specification & operator * () const
    {
        return _current->second;
    }

    //! Get pointer to element at current iterator position.
    const Stimulus_Specification * operator -> () const
    {
        return & (this->operator *());
    }

private:
    Stimuli_Specification::Stimuli_Specification_Map::const_iterator _current;

};

// ----------------------------------------------------------------------------

Stimuli_Specification::iterator Stimuli_Specification::
    find(const Label & name)
{
    return iterator(_elements.find(name));
}

// ----------------------------------------------------------------------------

Stimuli_Specification::iterator Stimuli_Specification::begin(void)
{
    return iterator (_elements.begin());
}

// ----------------------------------------------------------------------------

Stimuli_Specification::iterator Stimuli_Specification::end(void)
{
    return iterator (_elements.end());
}

// ----------------------------------------------------------------------------

Stimuli_Specification::const_iterator Stimuli_Specification::
    find(const Label & name) const
{
    return const_iterator(_elements.find(name));
}

// ----------------------------------------------------------------------------

Stimuli_Specification::const_iterator Stimuli_Specification::begin(void) const
{
    return const_iterator (_elements.begin());
}

// ----------------------------------------------------------------------------

Stimuli_Specification::const_iterator Stimuli_Specification::end(void) const
{
    return const_iterator (_elements.end());
}

// ----------------------------------------------------------------------------

void Stimuli_Specification::erase(const Label & name)
{
    _elements.erase(name);
}

// ----------------------------------------------------------------------------

void Stimuli_Specification::clear()
{
    Stimuli_Specification_Map _;
    _elements.swap(_);
}

// ----------------------------------------------------------------------------

size_t Stimuli_Specification::size() const
{
    return _elements.size();
}

// ----------------------------------------------------------------------------

void Stimuli_Specification::print() const
{
    std::cout << *this;
}

// ----------------------------------------------------------------------------

void Stimuli_Specification::insert(const Stimulus_Specification & stimulus)
{
    _elements.insert(std::make_pair(stimulus.label(), stimulus));
}

// ----------------------------------------------------------------------------

std::ostream & operator << 
    (std::ostream & lhs, const Stimuli_Specification & rhs)
{
    for (Stimuli_Specification::const_iterator i = rhs.begin();
         i != rhs.end();
         ++i)
    {
        lhs << *i << std::endl;
    }
    return lhs;
}

// ----------------------------------------------------------------------------

}
#endif
