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

#ifndef BBP_CONNECTION_SCALINGS_H
#define BBP_CONNECTION_SCALINGS_H

#include <iostream>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include "BBP/Model/Microcircuit/Types.h"
#include "../Connection_Scaling.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Connection_Scaling;
typedef boost::shared_ptr<Connection_Scaling> 
Connection_Scaling_Factor_Ptr;

// ----------------------------------------------------------------------------

//! A set of connection scaling factors for microcircuit synapses.
/*!
    \sa Connection_Scaling
    \ingroup Experiment_Containers
*/
class Connection_Scalings
{
public:
    inline Connection_Scalings();
    inline Connection_Scalings(const Label & label);

    //! Get label of this set of connections scalings (e.g. "spines_2.5")
    /*!
        \return text label for the container, whitespace are not allowed
    */
    inline const Label & label() const;
    //! Get content description text for this container.
    inline const Text & description() const;

    //! Set label of this set of experiments (e.g. "spines_2.5")
    /*!
        \param label text label for the container, whitespace are not allowed
    */
    inline void label(const Label & label);
    //! Set content description text for this container.
    inline void description(const Text & content_description);

    // ITERATOR ---------------------------------------------------------------

    //! Forward iterator for the container to visit all elements
    class iterator;
    //! Search for specified connection_scaling_factor identifier.
    /*! end iterator returned if not found */
    inline iterator find(const Label & connection_scalings_name);
    //! Returns the first element of the container for linear access
    inline iterator begin();
    //! Returns the iterator that indicates the end of the container
    inline const iterator end();

    //! Forward iterator for the container to visit all elements
    class const_iterator;
    //! Search for specified connection_scaling_factor identifier.
    /*! end iterator returned if not found */
    inline const_iterator find(const Label & connection_scalings_name) 
        const;
    //! Returns the first element of the container for const linear access
    inline const_iterator begin() const;
    //! Returns the const iterator that indicates the end of the container
    inline const const_iterator end() const;

    // INSERT/REMOVE CONTAINER ELEMENTS -----------------------------------------
 
    //! Insert a scaling factor into the set.
    inline void insert(const Connection_Scaling & scaling_factor);
    //! Erase scaling factor with specified label.
    /*!
        If the connection_scaling_factor label is not inside the target 
        nothing is done.
    */
    inline void erase(const Label & connection_scalings_name);
    //! Drop all neurons from microcircuit.
    inline void clear();
    //! Get number of neurons referenced in the container.
    inline size_t size() const;

    // STATUS INFORMATION -----------------------------------------------------
  
    //! Print names of experiments in this container
    inline void print() const;

protected:
    friend std::ostream & operator << 
            (std::ostream & lhs, const Connection_Scalings & rhs);

    //! connection scalings in the container
    std::map<Label, Connection_Scaling> _elements;
    //! name of the container
    Label                   _label;
    //! content description
    Text                    _description;
};

//! Output the connection scaling factor labels to the standard output.
inline std::ostream & operator << 
    (std::ostream & lhs, const Connection_Scalings & rhs);
}

// ----------------------------------------------------------------------------


namespace bbp {

// ----------------------------------------------------------------------------

class Connection_Scalings::iterator :
    public std::iterator<std::forward_iterator_tag, Connection_Scaling>
{
    friend class Connection_Scalings;
    friend class Connection_Scalings::const_iterator;

public:
    iterator()
    {}

private:
    explicit iterator(
        std::map<Label, Connection_Scaling>::iterator            
            connection_scaling_iterator)
            : _current(connection_scaling_iterator)
    {}

public:
    //! Compare two iterators for equality.
    bool operator == (const iterator & other) const
    {
        return (_current == other._current);
    }

    //! Compare two iterators for inequality.
    bool operator != (const iterator & other) const
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
    Connection_Scaling & operator * ()
    {
        return _current->second;
    }

    //! Get pointer to element at current iterator position.
    Connection_Scaling * operator -> ()
    {
        return & (this->operator *());
    }

private:
    std::map<Label, Connection_Scaling>::iterator _current;

};


// ----------------------------------------------------------------------------

class Connection_Scalings::const_iterator
    : public std::iterator<std::forward_iterator_tag, 
                           Connection_Scaling>
{
    friend class Connection_Scalings;
    friend class Connection_Scalings::iterator;

public:
    const_iterator()
    {}

    const_iterator(Connection_Scalings::iterator iterator)
        : _current(iterator._current)
    {}

private:
    explicit const_iterator(std::map<Label, Connection_Scaling>::
        const_iterator current)
    : _current(current)
    {}

public:
    //! Compare two iterators for equality.
    bool operator == (const const_iterator & other) const
    {
        return (_current == other._current);
    }

    //! Compare two iterators for inequality.
    bool operator != (const const_iterator & other) const
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
    const Connection_Scaling & operator * () const
    {
        return _current->second;
    }

    //! Get pointer to element at current iterator position.
    const Connection_Scaling * operator -> () const
    {
        return & (this->operator *());
    }

private:

    std::map<Label, Connection_Scaling>::const_iterator _current;
};

// ----------------------------------------------------------------------------

Connection_Scalings::Connection_Scalings()
{
}

// ----------------------------------------------------------------------------

Connection_Scalings::Connection_Scalings(const Label & label)
: _label(label)
{
}

// ----------------------------------------------------------------------------

Connection_Scalings::iterator Connection_Scalings::begin()
{
    return iterator(_elements.begin());
}

// ----------------------------------------------------------------------------

const Connection_Scalings::iterator Connection_Scalings::end()
{
    return iterator(_elements.end());
}

// ----------------------------------------------------------------------------

Connection_Scalings::const_iterator Connection_Scalings::
    begin() const
{
    return const_iterator(_elements.begin());
}

// ----------------------------------------------------------------------------

const Connection_Scalings::const_iterator Connection_Scalings::
    end() const
{
    return const_iterator(_elements.end());
}

// ----------------------------------------------------------------------------

Connection_Scalings::const_iterator Connection_Scalings::
    find(const Label & connection_scalings_name) const
{
    return const_iterator(_elements.find(connection_scalings_name));
}

// ----------------------------------------------------------------------------

Connection_Scalings::iterator Connection_Scalings::
    find(const Label & connection_scalings_name)
{
    return iterator(_elements.find(connection_scalings_name));
}

// ----------------------------------------------------------------------------

void Connection_Scalings::
    insert(const Connection_Scaling & connection_scaling)
{
    bbp_assert(_elements.find(connection_scaling.label()) == _elements.end() );
    _elements.insert(
        std::make_pair(connection_scaling.label(), connection_scaling));
}

// ----------------------------------------------------------------------------

void Connection_Scalings::erase
    (const Label & connection_scaling_factor_label)
{
    _elements.erase(connection_scaling_factor_label);
}

// ----------------------------------------------------------------------------

void Connection_Scalings::clear()
{
    _elements.clear();
}

// ----------------------------------------------------------------------------

size_t Connection_Scalings::size() const
{
    return _elements.size();
}

// ----------------------------------------------------------------------------

const Label & Connection_Scalings::label() const
{
    return _label;
}

// ----------------------------------------------------------------------------


void Connection_Scalings::label(const Label & label)
{
    _label = label;
}

// ----------------------------------------------------------------------------

const Text & Connection_Scalings::description() const
{
    return _description;
}

// ----------------------------------------------------------------------------


void Connection_Scalings::description(const Text & content_description)
{
    _description = content_description;
}

// ----------------------------------------------------------------------------

std::ostream & operator << 
    (std::ostream & lhs, const Connection_Scalings & rhs)
{
    lhs << "{ ";
    for (Connection_Scalings::const_iterator i = rhs.begin(); 
        i != rhs.end(); ++i)
    {
        lhs << i->label()<< " ";
    }
    lhs  << "}";
    return lhs;
}

// ----------------------------------------------------------------------------

}
#endif
