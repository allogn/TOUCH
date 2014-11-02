/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Juan Hernando Vieites
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_NEURONS_H
#define BBP_NEURONS_H

#include <iostream>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Common/Math/Geometry/Circle.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Neuron;
typedef boost::shared_ptr<Neuron>           Neuron_Ptr;
typedef boost::shared_ptr<const Neuron>     Const_Neuron_Ptr;
typedef boost::weak_ptr<Microcircuit>       Microcircuit_Weak_Ptr;

// ----------------------------------------------------------------------------

//! A set of neurons that are part of the same microcircuit.
/*!
    Similar to a Cell_Target (set of cells), except that the neuron data can be
    accessed, not only the cell GIDs. A neuron target = set of neurons. May
    reference all neurons in the microcircuit or a subset. Provides boolean
    operations to e.g. intersect with another neurons container.
    \todo Inheritance from Cell_Target and Container<T,...>. (TT)
    \ingroup Microcircuit_Containers
*/
class Neurons
    // : public Cell_Target (or Cells) and Container<const Neuron, ...>
{
    friend class Microcircuit;
    friend class Mutable_Microcircuit;
    friend class Synapses;
    friend class Neurons_Accessor;
    friend std::ostream & operator<< (std::ostream & lhs, const Neurons & rhs);

public:
    //! Create a container not linked to any microcircuit.
    /*! The microcircuit link will be set at the moment the first
        neuron is inserted. After that, the container will be tied to that
        microcircuit (until it's placed as left hand side in an assigment). */
    inline Neurons(const Label & label = "");

    //! Create a container out of a cell_target from a microcircuit
    Neurons(const Cell_Target & cell_target, Microcircuit & microcircuit);
	//! Create a container out of a target from a microcircuit
	/*!
		\todo Is this really needed as there should be an implicit
		conversion from Target -> Cell_Target? (TT)
	*/
    Neurons(const Target      &      target, Microcircuit & microcircuit);

protected:
    //! Create a container associated with the specified microcircuit.
    /*! This container will be attached to that microcircuit during its 
        lifetime (unless it's placed as left hand side in an assigment). */
    inline Neurons(const Microcircuit_Ptr microcircuit, 
                   const Label & label = "");

public:
    //! This is an ugly hack for boost python... too little time now though
    //! \todo clean up this ugly hack for boost python
    void load_from_cell_target_and_microcircuit(const Cell_Target & cell_target, Microcircuit & microcircuit);

    //! Get label of this set of cells (whitespaces not allowed, e.g. "Layer4")
    inline const Label & label() const;
    //! Set label of this set of cells (whitespaces not allowed, e.g. "Layer4")
    inline void label(const Label & label);

    //! Get description for this set of cells.
    inline const Text & description() const;
    //! Set description for this set of cells.
    inline void description(const Text & text);

    //! implicit conversion to a cell target
    operator Cell_Target() const;

    //! Print global identifiers of neurons in this container
    inline void print() const;

    // ITERATOR ---------------------------------------------------------------

    /*!
        \name Access
        For access and navigation inside the neurons container.
    */
    //@{
    //! Forward iterator for the Neurons container to visit all elements
    class iterator;
 
    //! Search for specified neuron identifier.
    /*!
        end iterator returned if not found
    */
    inline iterator find(Cell_GID gid);
    //! Returns the first element of the container for linear access
    inline iterator begin();
    //! Returns the iterator that indicates the end of the container
    inline iterator end();

    //! Forward iterator for the Neurons container to visit all elements
    class const_iterator;
    //! Search for specified neuron identifier.
    /*!
        end iterator returned if not found
    */
    inline const_iterator find(Cell_GID gid) const;
    //! Returns the first element of the container for const linear access
    inline const_iterator begin() const;
    //! Returns the const iterator that indicates the end of the container
    inline const_iterator end() const;

    //! Check if a Neuron object is part of the container.
    inline bool contains(const Neuron & neuron) const;
    //! Get number of neurons referenced in the container.
    inline size_t size() const;
    //@}


    // INSERT/REMOVE CONTAINER ELEMENTS -----------------------------------------

    /*!
        \name Modification
        Insert or remove elements from the container.
    */
    //@{
    //! Insert a neuron pointed to by a smart pointer into the neuron set.
    inline void insert(const Neuron_Ptr & neuron);

    //! Insert a neuron pointed by an iterator.
    inline void insert(const iterator & neuron);

    ////! Insert a neuron copying it.
    //inline void insert(const Neuron & neuron);
    /*!
      Erase neuron with specified global identifier.
      If the neuron gid is not inside the target nothing is done.
    */
    inline void erase(Cell_GID neuron);
    //! Drop all neurons from container.
    /*! The container is dettached from its microcircuit, so this container
        can store neurons from any other microcircuit than the original. */
    inline void clear();
    //! Add another set of neurons into this container
    inline Neurons & operator   += (Neurons & rhs);
    //! Insert a neuron into the neuron set.
    Neurons & operator          += (const Neuron_Ptr & neuron);
    //@}

    // LOGICAL OPERATORS ------------------------------------------------------

    /*!
        \name Boolean Operations
        Intersection, union, or exclusion operations between containers
        or volumes.
    */
    //@{
    //! boolean intersection (AND) operator between two neuron sets
    inline const Neurons operator & (Neurons & rhs) const;
    // //! boolean intersection (AND) operator between a neuron set and a volume
    //inline Neurons operator & (const Box<Micron> & rhs);
    //! Neurons within a radius of a specified location
    /*!
        (effectively a cylinder y-aligned)
    */
    const Neurons operator & (const Circle<Micron> & circle) const;
    //! boolean intersection (AND) operator between a neuron set and a cell target
    inline const Neurons operator & (Cell_Target & rhs) const;
    //! Boolean union (OR) operator between two neuron sets
    inline const Neurons operator | (Neurons & rhs) const;
    //! Boolean difference (substract) operator between two neuron sets.
    inline const Neurons operator - (Neurons & rhs) const;
    //! Boolean exclusive or (XOR) operator between two neuron sets.
    inline const Neurons operator ^ (Neurons & rhs) const;
    //! Union of two set of neurons. Equivalent to OR operator.
    inline const Neurons operator + (const Neurons & rhs) const;
    //! Union of a set of neurons with a single neuron.
    const Neurons operator + (const Const_Neuron_Ptr & neuron) const;

    //! Neurons within a radius of a specified location
    /*!
        (effectively a cylinder y-aligned)
    */
    Neurons operator & (const Circle<Micron> & circle);
    // //! boolean intersection (AND) operator between a neuron set and a volume
    //inline Neurons operator & (const Box<Micron> & rhs);
    //! boolean intersection (AND) operator between a neuron set and a cell target
    inline Neurons operator & (Cell_Target & rhs);
    //! boolean intersection (AND) operator between two neuron sets
    inline Neurons operator & (Neurons & rhs);
    //! Boolean union (OR) operator between two neuron sets
    inline Neurons operator | (Neurons & rhs);
    //! Boolean difference (substract) operator between two neuron sets.
    inline Neurons operator - (Neurons & rhs);
    //! Boolean exclusive or (XOR) operator between two neuron sets.
    inline Neurons operator ^ (Neurons & rhs);
    //! Union of two set of neurons. Equivalent to OR operator.
    inline Neurons operator + (Neurons & rhs);
    //! Union of a set of neurons with a single neuron.
    Neurons operator + (const Neuron_Ptr & neuron);
    //@}

private:
    //! cells in this set specified by their global identifier
    /*!
        \todo With Boost 1.36 being released, this can be changed
        to an unordered_map aka. hash map for faster lookups. However
        we would force all users to update to 1.36 or include the 
        implementation in our library for backwards compatibility. (TT)
        http://www.boost.org/doc/libs/1_36_0/doc/html/unordered/reference.html#header.boost.unordered_map.hpp
    */
    typedef std::map<Cell_GID, Neuron_Ptr> Neuron_Map;

    //! neurons in the neurons container
    Neuron_Map            _elements;
    //! name of the neurons container
    Label                 _label;
    Text                  _description;
    //! Microcircuit link to ensure all neurons are from the same circuit.
    Microcircuit_Weak_Ptr _microcircuit;
};

//! Outputs the neuron GIDs to the output stream in ASCII character format
inline std::ostream & operator << (std::ostream & lhs, const Neurons & rhs);

}
// ----------------------------------------------------------------------------

#include "BBP/Model/Microcircuit/Neuron.h"

// ----------------------------------------------------------------------------

namespace bbp
{
#ifndef SWIG

// ----------------------------------------------------------------------------

#ifdef BBP_SAFETY_MODE
#  define CHECK_MICROCIRCUIT_POINTER(m)              \
    if ((m).lock().get() == 0)                       \
    {                                                \
        throw_exception(Microcircuit_Access_Error(), \
            FATAL_LEVEL, __FILE__, __LINE__);        \
    }
#  define CHECK_CONTAINER_MICROCIRCUIT_POINTER(c)                         \
    if ((c)._microcircuit.lock().get() == 0 && (c)._elements.size() != 0) \
    {                                                                     \
        throw_exception(Microcircuit_Access_Error(),                      \
            FATAL_LEVEL, __FILE__, __LINE__);                             \
    }
#else
#  define CHECK_MICROCIRCUIT_POINTER(m) bbp_assert((m).lock().get() != 0)
#  define CHECK_CONTAINER_MICROCIRCUIT_POINTER(c) \
    bbp_assert((c)._microcircuit.lock().get() != 0 || (c)._elements.size() == 0)
#endif

// ----------------------------------------------------------------------------

/** cond SHOW_IN_DEVELOPER_REFERENCE */
class Neurons_Accessor
{
public:
    static Microcircuit_Ptr microcircuit(Neurons & neurons)
    {
    	//std::cout << neurons._elements.size() << std::endl;
    	//std::cout << neurons._microcircuit.lock().get() << std::endl;
        CHECK_CONTAINER_MICROCIRCUIT_POINTER(neurons);
        Microcircuit_Ptr mp = neurons._microcircuit.lock();
        return mp;
    }
};
/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

// ----------------------------------------------------------------------------

class Neurons::iterator
    : public std::iterator<std::forward_iterator_tag, Neuron>
{
    friend class Neurons;
    friend class Neurons::const_iterator;

public:
    iterator()
    {}

private:
    explicit iterator(Neurons::Neuron_Map::iterator neuron_iterator)
        : _current(neuron_iterator)
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
    Neuron & operator * ()
    {
        return * _current->second.get();
    }

    //! Get pointer to element at current iterator position.
    Neuron * operator -> ()
    {
        return & (this->operator *());
    }

    //! Get smart pointer to element at current iterator position.
    /*! If the iterator points to end the returned value is undefined */
    const Neuron_Ptr ptr() const
    {
        return _current->second;
    }

private:
    Neurons::Neuron_Map::iterator _current;
};

// ----------------------------------------------------------------------------

class Neurons::const_iterator
    : public std::iterator<std::forward_iterator_tag, const Neuron>
{
    friend class Neurons;
    friend class Neurons::iterator;

public:
    const_iterator()
    {}

    const_iterator(Neurons::iterator iterator)
        : _current(iterator._current)
    {}

private:
    explicit const_iterator
    (Neurons::Neuron_Map::const_iterator neuron_iterator)
        : _current(neuron_iterator)
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
    const Neuron & operator * () const
    {
        return *_current->second.get();
    }

    //! Get pointer to element at current iterator position.
    const Neuron * operator -> () const
    {
        return & (this->operator *());
    }

    //! Get smart pointer to element at current iterator position.
    const Const_Neuron_Ptr ptr() const
    {
        return _current->second;
    }

private:

    Neurons::Neuron_Map::const_iterator _current;
};

// ----------------------------------------------------------------------------

Neurons::Neurons(const Label & label) :
    _label(label)
{
}

// ----------------------------------------------------------------------------

Neurons::Neurons(const Microcircuit_Ptr microcircuit, const Label & label) : 
    _label(label),
    _microcircuit(microcircuit)
{
}

// ----------------------------------------------------------------------------

Neurons::iterator Neurons::begin()
{
    return iterator(_elements.begin());
}

// ----------------------------------------------------------------------------

Neurons::iterator Neurons::end()
{
    return iterator(_elements.end());
}

// ----------------------------------------------------------------------------

Neurons::const_iterator Neurons::begin() const
{
    return const_iterator(_elements.begin());
}

// ----------------------------------------------------------------------------

Neurons::const_iterator Neurons::end() const
{
    return const_iterator(_elements.end());
}

// ----------------------------------------------------------------------------

const Neurons Neurons::operator & (Neurons & rhs) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Neurons result(_microcircuit.lock(), 
                   Label("(") + _label + " & " + rhs._label + Label(")"));
    std::set_intersection(_elements.begin(), _elements.end(),
        rhs._elements.begin(), rhs._elements.end(),
        std::inserter(result._elements, result._elements.begin()));
    return result;
}

// ----------------------------------------------------------------------------

const Neurons Neurons::operator & (Cell_Target & rhs) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);

    Neurons neurons(_microcircuit.lock(), _label + " & " + rhs.label());

    for (Cell_Target::iterator i = rhs.begin(); i != rhs.end(); ++i)
    {
        Neuron_Map::const_iterator result = _elements.find(*i);
        if (result != _elements.end())
        {
            neurons._elements.insert(* result);
        }
    }
    return neurons;
}

// ----------------------------------------------------------------------------

const Neurons Neurons::operator | (Neurons & rhs) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Neurons result(_microcircuit.lock(), _label + " + " + rhs._label);
    std::set_union(_elements.begin(), _elements.end(),
                   rhs._elements.begin(), rhs._elements.end(),
                   std::inserter(result._elements, result._elements.begin()));
    return result;
}

// ----------------------------------------------------------------------------

const Neurons Neurons::operator - (Neurons & rhs) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Neurons result(_microcircuit.lock(), _label + " - " + rhs._label);
    std::set_difference(_elements.begin(), _elements.end(),
        rhs._elements.begin(), rhs._elements.end(),
        std::inserter(result._elements, result._elements.begin()));
    return result;
}

// ----------------------------------------------------------------------------

const Neurons Neurons::operator ^ (Neurons & rhs) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Neurons result(_microcircuit.lock(), _label + " XOR " + rhs._label);
    std::set_symmetric_difference(_elements.begin(), _elements.end(),
        rhs._elements.begin(), rhs._elements.end(),
        std::inserter(result._elements, result._elements.begin()));
    return result;
}

// ----------------------------------------------------------------------------

Neurons Neurons::operator & (Neurons & rhs)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Neurons result(_microcircuit.lock(), Label("(") + _label + " & " +
        rhs._label + Label(")"));
    std::set_intersection(_elements.begin(), _elements.end(),
        rhs._elements.begin(), rhs._elements.end(),
        std::inserter(result._elements, result._elements.begin()));
    return result;
}

// ----------------------------------------------------------------------------

Neurons Neurons::operator & (Cell_Target & rhs)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);

    Neurons neurons(_microcircuit.lock(), _label + " & " + rhs.label());

    for (Cell_Target::iterator i = rhs.begin(); i != rhs.end(); ++i)
    {
        Neuron_Map::const_iterator result = _elements.find(*i);
        if (result != _elements.end())
        {
            neurons._elements.insert(* result);
        }
    }
    return neurons;
}

// ----------------------------------------------------------------------------

Neurons Neurons::operator | (Neurons & rhs)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Neurons result(_microcircuit.lock(), _label + " + " + rhs._label);
    std::set_union(_elements.begin(), _elements.end(),
                   rhs._elements.begin(), rhs._elements.end(),
                   std::inserter(result._elements, result._elements.begin()));
    return result;
}

// ----------------------------------------------------------------------------

Neurons Neurons::operator - (Neurons & rhs)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Neurons result(_microcircuit.lock(), _label + " - " + rhs._label);
    std::set_difference(_elements.begin(), _elements.end(),
        rhs._elements.begin(), rhs._elements.end(),
        std::inserter(result._elements, result._elements.begin()));
    return result;
}

// ----------------------------------------------------------------------------

Neurons Neurons::operator ^ (Neurons & rhs)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Neurons result(_microcircuit.lock(), _label + " XOR " + rhs._label);
    std::set_symmetric_difference(_elements.begin(), _elements.end(),
        rhs._elements.begin(), rhs._elements.end(),
        std::inserter(result._elements, result._elements.begin()));
    return result;
}


// ----------------------------------------------------------------------------

//Neurons operator & (const Box<Micron> & rhs)
//{
//}

// ----------------------------------------------------------------------------

bool Neurons::contains(const Neuron & neuron) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    CHECK_MICROCIRCUIT_POINTER(neuron._microcircuit);

    if (neuron._microcircuit.lock() == _microcircuit.lock() &&
        find(neuron.gid()) != end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

Neurons::const_iterator Neurons::find(Cell_GID gid) const
{
    return const_iterator(_elements.find(gid));
}

// ----------------------------------------------------------------------------

Neurons::iterator Neurons::find(Cell_GID gid)
{
    return iterator(_elements.find(gid));
}

// ----------------------------------------------------------------------------

void Neurons::insert(const Neuron_Ptr & neuron)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    CHECK_MICROCIRCUIT_POINTER(neuron->_microcircuit);

    bbp_assert(_elements.find(neuron->gid()) == _elements.end() ||
           _elements.find(neuron->gid())->second == neuron);

    // If the container doesn't have a link yet, link it.
    if (_microcircuit.lock().get() == 0)
    {
        bbp_assert(_elements.size() == 0);
        _microcircuit = neuron->_microcircuit;
    }

    // Check if neuron is from the same microcircuit as the container.
    if (neuron->_microcircuit.lock() != _microcircuit.lock())
    {
        std::string error("Neuron is not from the same microcircuit"
                          " as the Neurons container.");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    // Insert neuron into container.
    _elements.insert(std::make_pair(neuron->gid(), neuron));
}

// ----------------------------------------------------------------------------

void Neurons::insert(const iterator & neuron)
{
    insert(neuron.ptr());
}

// ----------------------------------------------------------------------------

//void Neurons::insert(const Neuron & neuron)
//{
//  TODO: Implement and handle different microcircuits case. (TT)
//}

// ----------------------------------------------------------------------------

void Neurons::erase(Cell_GID gid)
{
    _elements.erase(gid);
}

// ----------------------------------------------------------------------------

void Neurons::clear()
{
    _elements.clear();
}

// ----------------------------------------------------------------------------

size_t Neurons::size() const
{
    return _elements.size();
}

// ----------------------------------------------------------------------------

Neurons & Neurons::operator += (Neurons & rhs)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    std::set_union(_elements.begin(), _elements.end(),
        rhs._elements.begin(), rhs._elements.end(),
        std::inserter(_elements, _elements.begin()));
    return * this;
}

// ----------------------------------------------------------------------------

const Neurons Neurons::operator + (const Neurons & rhs) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Neurons result(_microcircuit.lock(), _label + " + " + rhs._label);
    std::set_union(_elements.begin(), _elements.end(),
        rhs._elements.begin(), rhs._elements.end(),
        std::inserter(result._elements, result._elements.begin()));
    return result;
}

// ----------------------------------------------------------------------------

Neurons Neurons::operator + (Neurons & rhs)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Neuron containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Neurons result(_microcircuit.lock(), _label + " + " + rhs._label);
    std::set_union(_elements.begin(), _elements.end(),
        rhs._elements.begin(), rhs._elements.end(),
        std::inserter(result._elements, result._elements.begin()));
    return result;
}

// ----------------------------------------------------------------------------

const Label & Neurons::label() const
{
    return _label;
}

// ----------------------------------------------------------------------------

void Neurons::label(const Label & label)
{
    _label = label;
}

// ----------------------------------------------------------------------------

const Text & Neurons::description() const
{
    return _description;
}

// ----------------------------------------------------------------------------

void Neurons::description(const Text & text)
{
    _description = text;
}

// ----------------------------------------------------------------------------

void Neurons::print() const
{
  //  std::cout << *this << std::endl;
}

// ----------------------------------------------------------------------------

std::ostream & operator << (std::ostream & lhs, const Neurons & rhs)
{
    if (rhs.label().empty())
    {
        lhs << "Neurons";
    }
    else
    {
        lhs << rhs.label();
    }

    lhs << "(" << rhs._elements.size() << ") = { ";
    for (Neurons::const_iterator i = rhs.begin();
        i != rhs.end();++i)
    {
        lhs << * i << ' ';
    }
    lhs  << "}";
    return lhs;
}

// ----------------------------------------------------------------------------

#undef CHECK_MICROCIRCUIT_POINTER
#undef CHECK_CONTAINER_MICROCIRCUIT_POINTER

#endif // SWIG
}
#endif
