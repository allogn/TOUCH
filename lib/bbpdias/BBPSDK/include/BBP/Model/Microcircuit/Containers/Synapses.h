/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors: Thomas Traenkler
                             Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SYNAPSES_H
#define BBP_SYNAPSES_H

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>

#include "BBP/Common/Dataset/Containers/detail/Index_Set.h"

#include "BBP/Common/Patterns/Singleton.h"
#include "BBP/Model/Microcircuit/Types.h"

#if defined SWIG || defined SWIG_IMPLEMENTATION               
#include "swig_iterator.h"
#else
template <typename T, typename U> class Swig_Iterator;
#endif

namespace bbp {

class Synapse;
class Neuron;
class Neurons;
class Microcircuit;
typedef boost::shared_ptr<Microcircuit> Microcircuit_Ptr;
typedef boost::weak_ptr<Microcircuit> Microcircuit_Weak_Ptr;

// ----------------------------------------------------------------------------

//! A set of synapses that are part of the same microcircuit.
/*!
    May reference all Synapses in the microcircuit or a subset.
    Provides boolean operations to e.g. intersect with
    another synapse target (cf. target concept of James King).

    This container stores references to the real synapse objects inside
    the raw dataset. When containers are assigned or copy constructed the
    internal storage will be shared between assigned and assignee.

    Set operations are only defined between Synapses containers associated to
    the same microcircuit dataset.
    This container has copy on write semantics to allow cheap copies.
    \todo In general the targets handling (inside the containers or external?)
    should be discussed.
    \todo Synapses::erase() function is not in sync with other containers - 
    how should it be implemented and when is a sync desirable? I would suggest
    to have a flag that states if this container is part of a Microcircuit
    or a standalone container. In the first case, synapses will be removed
    from the circuit in all microcircuit synapses containers, while in the
    latter it is only removed from this container. (TT/JH)
    \ingroup Microcircuit_Containers
*/
class Synapses
{
    friend class Neuron;
    friend class Microcircuit;

public:

    //! Create a container without a linked microcircuit.
    /*! The microcircuit link will be set when the first synapse is 
        inserted. After that, the container will be tied to that
        microcircuit (until it's placed as left hand side in an assigment). */
    Synapses(const Label & label = "");

protected:
    //! Create a container linked to a microcircuit
    /*! This container will be attached to that microcircuit during its 
        lifetime (unless it's placed as left hand side in an assigment). */
    Synapses(Microcircuit_Ptr microcircuit, const Label & label = "");

public:
    //! Get label of the container.
    inline const Label &    label() const;

    // ITERATOR _______________________________________________________________

    /*!
        \name Access
        For access and navigation inside the synapses container.
    */
    //@{

    //! Linear forward const iterator.
    /*!
        Iteration order is not guaranteed to be in any in particular.
    */
    class const_iterator;
    //! Access first synapse inside the container
    inline const_iterator    begin() const;
    /*!
      Returns the element succeeding the last element in the container.
      If the Synapses iterator exceeds the elements in the container, the
      iterator will equal this state.
    */
    inline const_iterator    end() const;

    //! Searchs synapse inside this container
    /*!
        If the synapse is not found the element succeeding the last element
        in the container will be returned.
        \return A read-only access iterator pointint to the synapse.
    */
    inline const_iterator    find(const Synapse & synapse) const;


    //! Linear forward iterator.
    /*!
      Iteration order is not guaranteed to be in any in particular.
    */
    class iterator;
    //! Access first synapse inside the container
    inline iterator    begin();
    /*!
      Returns the element succeeding the last element in the container.
      If the Synapses iterator exceeds the elements in the container, the
      iterator will equal this state.
    */
    inline iterator    end();
    //! Finds a synapse by index into the set.
    /*!
      If the synapse is not found the element succeeding the last element
      in the container will be returned.
      \return A read-only access iterator pointint to the synapse.
    */
    inline iterator    find(const Synapse & synapse);

    //! Get a container with a set of presynaptic neurons of these synapses.
    const Neurons presynaptic_neurons() const;
    //! Get a container with a set of postsynaptic neurons of these synapses.
    const Neurons postsynaptic_neurons() const;

    //! Get a container with a set of presynaptic neurons of these synapses.
    Neurons presynaptic_neurons();
    //! Get a container with a set of postsynaptic neurons of these synapses.
    Neurons postsynaptic_neurons();
	
    //@}

    // CONTAINER MANAGEMENT ___________________________________________________

    /*!
        \name Modification
        Insert or remove elements from the container.
    */
    //@{

    //! Insert a set of synapses into this set.
    inline Synapses       & operator += (const Synapses & rhs);
    //! Insert a synapse into the set (throws if from different microcircuits).
    inline iterator            insert(const Synapse & synapse);
    //! Insert all afferent and efferent synapses of a neuron into the set
    //!  (throws if from different microcircuits).
    inline void                insert(const Neuron & neuron);
    //! Remove a synapse from the set.
    inline void                erase(const Synapse & synapse);
    //! Drop all Synapses from this container
    /*! The container is dettached from its microcircuit, so this container
        can store synapses from any other microcircuit than the original. */
    inline void                clear();
    //@}

    // LOGICAL OPERATORS ______________________________________________________

    /*!
        \name Boolean Operations
        Intersection, union, or exclusion operations between containers
        or volumes.
    */
    //@{

    //! boolean intersection (AND) operator between two synapse sets
    inline Synapses            operator & (const Synapses & rhs) const;
    //! Boolean union (OR) operator between two synapse sets
    inline Synapses            operator | (const Synapses & rhs) const;
    //! Boolean difference (substract) operator between two synapse sets.
    inline Synapses            operator - (const Synapses & rhs) const;
    //! Boolean exclusive or (XOR) operator between two synapse sets.
    inline Synapses            operator ^ (const Synapses & rhs) const;

    //! boolean intersection (AND) operator between a synapse set and a volume
    //inline Synapses        operator & (const Volume<Millivolt, Micron> & rhs);
    //@}

    // CONTAINER STATUS _______________________________________________________

    //! Print status information about the container to the standard output.
    inline void                print() const;
    //! Retrieve number of Synapses referenced in the container.
    inline size_t              size() const;

protected:
    //! Get a synapse by its internal array index.
    Synapse synapse(Synapse_Index index);
    //! Get a synapse by its internal array index.
    const Synapse synapse(Synapse_Index index) const;

    friend class Synapses::iterator;
    friend class Synapses::const_iterator;

protected:
    //! Index set storing array indices to find all data related to a synapse.
    typedef detail::Index_Set<Synapse_Index, false> Synapse_Index_Set;

    //! Label for this set of cells.
    /*!
        (e.g. "Microcircuit", "Layer 4", "Excitatory", or "Center Minicolumn").
    */
    Label                                    _label;
    //! Synapses in this set.
    /*! This vector must be kept ordered by all operations. */
    boost::shared_ptr<Synapse_Index_Set>    _elements;
    Microcircuit_Weak_Ptr                   _microcircuit;
};

//! Outputs the synapses to the output stream in ASCII character format
inline std::ostream & operator << (std::ostream & lhs, const Synapses & rhs);

// ----------------------------------------------------------------------------

}

#ifndef SWIG

#include "BBP/Common/Patterns/Singleton.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"
#include "BBP/Model/Microcircuit/Synapse.h"
#include "BBP/Model/Microcircuit/Neuron.h"

namespace bbp
{

#ifdef BBP_SAFETY_MODE
#  define CHECK_MICROCIRCUIT_POINTER(m)              \
    if ((m).lock().get() == 0)                       \
    {                                                \
        throw_exception(Microcircuit_Access_Error(), \
            FATAL_LEVEL, __FILE__, __LINE__);        \
    }
#  define CHECK_CONTAINER_MICROCIRCUIT_POINTER(c)                          \
    if ((c)._microcircuit.lock().get() == 0 && (c)._elements->size() != 0) \
    {                                                                      \
        throw_exception(Microcircuit_Access_Error(),                       \
            FATAL_LEVEL, __FILE__, __LINE__);                              \
    }
#else
#  define CHECK_MICROCIRCUIT_POINTER(m) bbp_assert((m).lock().get() != 0)
#  define CHECK_CONTAINER_MICROCIRCUIT_POINTER(c) \
    bbp_assert((c)._microcircuit.lock().get() != 0 || (c)._elements->size() == 0)
#endif

// ----------------------------------------------------------------------------

class Synapses::iterator
    : public std::iterator<std::forward_iterator_tag, Synapse>,
      private Synapse_Accessor
{
    friend class Synapses;
    friend class Synapses::const_iterator;

public:
    iterator()
      : _synapse(UNDEFINED_SYNAPSE_INDEX)
    {
#ifndef NDEBUG
        _synapses = 0;
#endif
    }

private:
    iterator(Synapse_Index_Set::const_iterator start_point, 
             Synapses * synapses) :
        _current(start_point),
        _synapses(synapses),
        /* Consistency check for microcircuit pointer performed by Synapses
           container itself when it applies. It is valid for 
           _microcircuit.lock to be 0 when the container is empty. */
        _synapse(*synapses->_microcircuit.lock(), UNDEFINED_SYNAPSE_INDEX)
    {
    }

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

    //! Get (dereference) current element (READ WARNING BEFORE USE!).
    /*!
        WARNING: Referenced elements are only created on the fly and are
        invalidated when the next element gets dereferenced!
    */
    Synapse & operator * () const
    {
        synapse_index(_synapse) = * _current;
        return _synapse;
    }

    //! Get pointer to current element (READ WARNING BEFORE USE!).
    /*!
        WARNING: Referenced elements are only created on the fly and are
        invalidated when the next element gets dereferenced!
    */
    Synapse * operator -> () const
    {
        return & (this->operator *());
    }

private:
    Synapse_Index_Set::const_iterator _current;
    Synapses *                        _synapses;
    mutable Synapse                   _synapse;
};

// ----------------------------------------------------------------------------

class Synapses::const_iterator
    : public std::iterator<std::forward_iterator_tag, const Synapse>,
      private Synapse_Accessor
{
    friend class Synapses;
    friend class Synapses::iterator;

public:
    const_iterator()
      : _synapse(UNDEFINED_SYNAPSE_INDEX)
    {
#ifndef NDEBUG
        _synapses = 0;
#endif
    }

    const_iterator(Synapses::iterator iterator) :
        _current(iterator._current),
        _synapses(iterator._synapses),
        _synapse(iterator._synapse)
    {}

private:
    const_iterator(Synapse_Index_Set::const_iterator start_point,
                   const Synapses * synapses) :
        _current(start_point),
        _synapses(synapses),
        /* Consistency check for microcircuit pointer performed by Synapses
           container itself when it applies. It is valid for 
           _microcircuit.lock to be 0 when the container is empty. */
        _synapse(*synapses->_microcircuit.lock(), UNDEFINED_SYNAPSE_INDEX)
    {
    }

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

    //! Get (dereference) current element (READ WARNING BEFORE USE!).
    /*!
        WARNING: Referenced elements are only created on the fly and are
        invalidated when the next element gets dereferenced!
    */
    const Synapse & operator * () const
    {
        synapse_index(_synapse) = * _current;
        return _synapse;
    }

    //! Get pointer to current element (READ WARNING BEFORE USE!).
    /*!
        WARNING: Referenced elements are only created on the fly and are
        invalidated when the next element gets dereferenced!
    */
    const Synapse * operator -> () const
    {
        return & (this->operator *());
    }

private:
    Synapse_Index_Set::const_iterator _current;
    const Synapses *                  _synapses;
    mutable Synapse                   _synapse;
};

// ----------------------------------------------------------------------------

Synapses::const_iterator Synapses::begin() const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    return const_iterator(_elements->begin(), this);
}

// ----------------------------------------------------------------------------

Synapses::const_iterator Synapses::end() const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    return const_iterator(_elements->end(), this);
}

// ----------------------------------------------------------------------------

Synapses::iterator Synapses::begin()
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    return iterator(_elements->begin(), this);
}

// ----------------------------------------------------------------------------

Synapses::iterator Synapses::end()
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    return iterator(_elements->end(), this);
}

// ----------------------------------------------------------------------------

size_t Synapses::size() const
{
    return _elements->size();
}

// ----------------------------------------------------------------------------

const Label & Synapses::label() const
{
    return _label;
}

// ----------------------------------------------------------------------------

inline const Synapse Synapses::synapse(Synapse_Index index) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    return Synapse(*_microcircuit.lock(), index);
}

// ----------------------------------------------------------------------------

inline Synapse Synapses::synapse(Synapse_Index index)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    return Synapse(*_microcircuit.lock(), index);
}

// ----------------------------------------------------------------------------

Synapses::const_iterator Synapses::find(const Synapse & synapse) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    if (synapse._microcircuit != _microcircuit.lock())
        return end();
    else
        return const_iterator(_elements->find(synapse._index), this);
}

// ----------------------------------------------------------------------------

Synapses::iterator Synapses::find(const Synapse & synapse)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    if (synapse._microcircuit != _microcircuit.lock())
        return end();
    else
        return iterator(_elements->find(synapse._index), this);
}

// ----------------------------------------------------------------------------

Synapses::iterator Synapses::insert(const Synapse & synapse)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);

    // Setting microcircuit link if it's null.
    if (_microcircuit.lock().get() == 0)
        _microcircuit = synapse._microcircuit;

    // Checking synapse and container microcircuit links for equality.
    if (synapse._microcircuit != _microcircuit.lock())
    {
        std::string error ("Synapse insertion failed (not part of the same"
                           " microcircuit).");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    if (!_elements.unique())
        _elements.reset(new Synapse_Index_Set(*_elements));
    //! \todo Provide an insert function in Index_Set that returns an 
    //        iterator
    _elements->insert(synapse._index);
    return iterator(_elements->find(synapse._index), this);
}

// ----------------------------------------------------------------------------

void Synapses::insert(const Neuron & neuron)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);

    // Setting microcircuit link if it's null.
    if (_microcircuit.lock().get() == 0)
        _microcircuit = neuron._microcircuit;

    if (neuron._microcircuit.lock() != _microcircuit.lock())
    {
        std::string error ("Insertion of all synapses of the specified "
            "neuron into the Synapses container failed (neuron not part "
            "of the same microcircuit).");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    // copy on write
    if (!_elements.unique())
        _elements.reset(new Synapse_Index_Set(*_elements));

    //! \todo Make this more storage efficient without intermediate copy.
    //! Might be a problem for workstation case in some instances. (TT)

    // add all afferent and efferent synapses to container
    * this  += neuron.afferent_synapses();
    * this  += neuron.efferent_synapses();
}

// ----------------------------------------------------------------------------

void Synapses::erase(const Synapse & synapse)
{
    // Checking synapse and container microcircuit links for equality.
    if (synapse._microcircuit != _microcircuit.lock())
    {
        std::string error ("Synapse erase failed (not part of the same"
                           " microcircuit).");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    if (!_elements.unique())
        _elements.reset(new Synapse_Index_Set(*_elements));
    _elements->remove(synapse._index);
}

// ----------------------------------------------------------------------------

Synapses Synapses::operator & (const Synapses & rhs) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Synapses containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Synapses result(_label + " & " + rhs._label);
    result._microcircuit = _microcircuit;

    Synapse_Index_Set::const_iterator i;
    for (i = _elements->begin(); i != _elements->end(); ++i) {
        if (rhs._elements->find(*i) != rhs._elements->end())
            // Inserting elements from this existing in rhs.
            result._elements->insert(*i);
    }

    return result;
}

// ----------------------------------------------------------------------------

Synapses Synapses::operator | (const Synapses & rhs) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Synapses containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Synapses result(_label + " | " + rhs._label);
    result._microcircuit = _microcircuit;

    Synapse_Index_Set::const_iterator i;
    for (i = _elements->begin(); i != _elements->end(); ++i)
        result._elements->insert(*i);
    for (i = rhs._elements->begin(); i != rhs._elements->end(); ++i)
        result._elements->insert(*i);

    return result;
}

// ----------------------------------------------------------------------------

Synapses Synapses::operator - (const Synapses & rhs) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Synapses containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Synapses result(_label + " - " + rhs._label);
    result._microcircuit = _microcircuit;

    Synapse_Index_Set::const_iterator i;
    for (i = _elements->begin(); i != _elements->end(); ++i) {
        if (rhs._elements->find(*i) == rhs._elements->end())
            // Inserting elements from this not existing in rhs.
            result._elements->insert(*i);
    }

    return result;
}

// ----------------------------------------------------------------------------

Synapses Synapses::operator ^ (const Synapses & rhs) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Synapses containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }

    Synapses result(_label + " XOR " + rhs._label);
    result._microcircuit = _microcircuit;

    Synapse_Index_Set::const_iterator i;
    for (i = _elements->begin(); i != _elements->end(); ++i) {
        if (rhs._elements->find(*i) == rhs._elements->end())
            // Inserting elements from this not existing in rhs.
            result._elements->insert(*i);
    }
    for (i = rhs._elements->begin(); i != rhs._elements->end(); ++i) {
        if (_elements->find(*i) == _elements->end())
            // Inserting elements from rhs not existing in this.
            result._elements->insert(*i);
    }

    return result;
}

// ----------------------------------------------------------------------------

//Synapses Synapses::operator & (const Volume<Millivolt, Micron> & rhs)
//{
//}

// ----------------------------------------------------------------------------

Synapses & Synapses::operator += (const Synapses & rhs)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    // Check if neuron is from the same microcircuit as the container.
    if (_microcircuit.lock() != rhs._microcircuit.lock())
    {
        std::string error("Synapses containers are not from the same"
                          "microcircuit");
        throw_exception(Bad_Data(error), SEVERE_LEVEL);
    }
    
    Synapse_Index_Set::const_iterator i;
    for (i = rhs._elements->begin(); i != rhs._elements->end(); ++i)
        _elements->insert(*i);
    return *this;
}

// ----------------------------------------------------------------------------

void Synapses::clear()
{
    _elements.reset(new Synapse_Index_Set());
}

// ----------------------------------------------------------------------------

void Synapses::print() const
{
    std::cout << *this;
}

// ----------------------------------------------------------------------------

std::ostream & operator << (std::ostream & lhs, const Synapses & rhs)
{
    if (rhs.label().empty())
    {
        lhs << "Synapses";
    }
    else
    {
        lhs << rhs.label();
    }
    lhs << " = { " << std::endl;
    for (Synapses::const_iterator i = rhs.begin(); i != rhs.end(); ++i)
    {
        lhs << "     " << * i << std::endl;
    }
    lhs << "}" << std::endl;
    return lhs;
}

// ----------------------------------------------------------------------------

#undef CHECK_MICROCIRCUIT_POINTER
#undef CHECK_CONTAINER_MICROCIRCUIT_POINTER

}
#endif // SWIG

#endif
