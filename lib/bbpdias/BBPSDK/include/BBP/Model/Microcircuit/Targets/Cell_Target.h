/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_CELL_TARGET_H
#define BBP_CELL_TARGET_H

#include <iostream>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <boost/lexical_cast.hpp>


#include "BBP/Model/Microcircuit/Types.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Set of selected cells.
/*! 
    A cell target = set of cells. May reference all cells in the 
    microcircuit or a subset. Provides boolean operations to e.g. intersect with
    another cell target (cf. target concept of James King and Blue Target -
    will link into Blue Target once it becomes a library).
    This container has copy on write semantics to allow cheap copies.
    \ingroup Microcircuit_Target
*/
class Cell_Target
{
public:
    //! Forward ordered iterator for the cell container to visit all elements.
    /*! This iterator is read only as in std::set */
    class const_iterator;
    //! Forward ordered iterator for the cell container to visit all elements.
    /*! This iterator is read only as in std::set */
    typedef const_iterator iterator;

	// CONSTRUCTORS -----------------------------------------------------------

    inline Cell_Target();
    explicit inline Cell_Target(const Label & label);
    inline Cell_Target(const Label & label, const Text & description);
    inline Cell_Target(const Label & label, 
                       const Text & description, 
                       const Cell_Target & cell_target);
    inline ~Cell_Target()
    {
#ifdef BBP_DEBUG
        //! \todo remove this debug output.
        std::cout << "Cell_Target::~Cell_Target()" << std::endl;
#endif
    }

	// DESCRIPTION ------------------------------------------------------------

    /*!
        \name Access
        Access to cell target name and elements.
    */
    //@{
    //! Get label for this set of cells (e.g. "Layer4").
    inline const Label & label() const;
	//! Set label for this set of cells (e.g. "Layer 4")
    inline void label(const Label & text);
	//! Get the description for this set of cells.
	/*!
        (e.g. "all neurons in layer IV")
    */
	inline const Text & description() const;
    //! Set the description for this set of cells 
	/*!
        (e.g. "all neurons in layer IV")
    */
	inline void description(const Text & text);
 
	// ITERATOR ---------------------------------------------------------------

    //! Returns the first element of the container for linear access.
    inline const_iterator begin(void) const;
    //! indicates the end of the container has been reached.
    inline const_iterator end(void) const;
    //! Find a cell by gid.
    inline const_iterator find(Cell_GID gid) const;

    //! Retrieve number of cells referenced in the container.
    inline size_t size() const;
    //@}
            
    // INSERT / REMOVE --------------------------------------------------------

    /*!
        \name Modification
        Set cell target attributes.
    */
    //@{
    //! Insert a cell specified by its global identifier into the cell set.
    /*! Beware this call will invalidate iterators if copy-on-write is
        needed. */
    inline void insert (Cell_GID gid);
    //! Erase cell with specified global identifier.
    /*! Beware this call will invalidate iterators if copy-on-write is
        needed. Iterators to the given gid are also invalidated. */
    inline void erase(Cell_GID gid);
    //! Drop all cells from set.
    /*! All iterators will be invalidated */
    inline void clear();
    //! Insert a cell specified by its global identifier into the cell set.
    /*! \sa insert(Cell_GID) */
    inline Cell_Target & operator += (Cell_GID gid);
	//! Add another set of cells into this container
    /*! \sa insert(Cell_GID) */
    inline Cell_Target & operator += (const Cell_Target & rhs);
    //@}

	// LOGICAL OPERATIONS -----------------------------------------------------

    //! Equality operator
    inline bool operator == (const Cell_Target & rhs) const;

    //! Inequality operator
    inline bool operator != (const Cell_Target & rhs) const
    {
        return !(*this == rhs);
    }
    
    /*!
        \name Boolean operations
        Intersect, unify, substract or exclusive or between two cell targets,
        a volume, etc.
    */
    //@{
    //! Boolean intersection (AND) operator between two cell sets.
    inline Cell_Target operator & (const Cell_Target & rhs) const;
    //! Boolean union (OR) operator between two cell sets.
    inline Cell_Target operator | (const Cell_Target & rhs) const;
    //! Boolean difference (substract) operator between two cell sets.
    inline Cell_Target operator - (const Cell_Target & rhs) const;
    //! Boolean exclusive or (XOR) operator between two cell sets.
    inline Cell_Target operator ^ (const Cell_Target & rhs) const;
    //! Addition between two cell sets.
    inline Cell_Target   operator + (Cell_GID gid) const;
    //! select a random fraction of the cell target specified as a parameter
    inline Cell_Target fraction(Percentage percentage) const;
    //@}

	// STATUS INFORMATION -----------------------------------------------------

    //! Print global identifiers of cells in this container
    inline void print() const;

private:

    //! cells in this set specified by their global identifier
    typedef std::set<Cell_GID>	Cell_GID_Set;

	//! Brief name for the cell target without spaces
	Label                           _label;
	//! Longer description of the cell target in words
    Text                            _description;
	//! The elements of the cell target. 
    /*! The pointed object is copied on modification if the pointer is not
        unique. This class provides thread-safe copy on write semantics
        in the sense that different objects are not affected by operations
        in other objects. What is not guaranteed is thread-safe read/modify
        access. */
    boost::shared_ptr<Cell_GID_Set> _elements;
};



//! Outputs the cell GIDs to the output stream in ASCII character format
inline std::ostream & operator << (std::ostream & lhs, const Cell_Target & rhs);

#ifndef SWIG

// ----------------------------------------------------------------------------

Cell_Target::Cell_Target() :
    _elements(new Cell_GID_Set)
{}

// ----------------------------------------------------------------------------

Cell_Target::Cell_Target(const Label & label, const Text & description) :
    _label(label), 
    _description(description),
    _elements(new Cell_GID_Set)
{}

// ----------------------------------------------------------------------------

Cell_Target::Cell_Target(const Label & label) :
    _label(label),
    _elements(new Cell_GID_Set)
{}

// ----------------------------------------------------------------------------

Cell_Target::Cell_Target(const Label & label, const Text & description, 
                         const Cell_Target & cell_target) :
    _label(label),
    _description(description),
    _elements(cell_target._elements)
{}

// ----------------------------------------------------------------------------

//! Forward iterator for the cell container to visit all elements
class Cell_Target::const_iterator 
    : public std::iterator<std::forward_iterator_tag, const Cell_GID>
{
    friend class Cell_Target;

public:
    const_iterator()
    {}

private:
    explicit const_iterator(Cell_GID_Set::const_iterator gid_iterator)
        : _current(gid_iterator)
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
    const Cell_GID & operator * ()
    {
        return * _current;
    }

    //! Get pointer to element at current iterator position.
    const Cell_GID * operator -> ()
    {
        return & (this->operator *());
    }

private:
    Cell_GID_Set::const_iterator _current;

};

// ----------------------------------------------------------------------------

Cell_Target::const_iterator Cell_Target::begin(void) const
{
    return const_iterator(_elements->begin());
}

// ----------------------------------------------------------------------------

Cell_Target::const_iterator Cell_Target::end(void) const
{
    return const_iterator(_elements->end());
}

// ----------------------------------------------------------------------------

bool Cell_Target::operator == (const Cell_Target & rhs) const
{
    if (_elements.get() == rhs._elements.get())
        return true;
    else
        return *_elements == *rhs._elements;
}

// ----------------------------------------------------------------------------

Cell_Target Cell_Target::operator & (const Cell_Target & rhs) const
{
    Cell_Target result(_label + "_INTERSECTION_" + rhs._label);
    std::set_intersection(_elements->begin(), _elements->end(),
        rhs._elements->begin(), rhs._elements->end(), 
        std::inserter(*result._elements, result._elements->begin()));
    return result;
}

// ----------------------------------------------------------------------------

Cell_Target Cell_Target::operator | (const Cell_Target & rhs) const
{
    Cell_Target result(_label + "_UNION_" + rhs._label);
    std::set_union(_elements->begin(), _elements->end(),
        rhs._elements->begin(), rhs._elements->end(), 
        std::inserter(*result._elements, result._elements->begin()));
    return result;
}

// ----------------------------------------------------------------------------

Cell_Target Cell_Target::operator - (const Cell_Target & rhs) const
{
    Cell_Target result(_label + "_DIFFERENCE_" + rhs._label);
    std::set_difference(_elements->begin(), _elements->end(),
        rhs._elements->begin(), rhs._elements->end(), 
        std::inserter(*result._elements, result._elements->begin()));
    return result;
}

// ----------------------------------------------------------------------------

Cell_Target Cell_Target::operator ^ (const Cell_Target & rhs) const
{
    Cell_Target result(_label + "_XOR_" + rhs._label);
    std::set_symmetric_difference(_elements->begin(), _elements->end(),
        rhs._elements->begin(), rhs._elements->end(), 
        std::inserter(*result._elements, result._elements->begin()));
    return result;
}

// ----------------------------------------------------------------------------

Cell_Target Cell_Target::fraction(Percentage percentage) const
{
    Cell_Target result;
    result._label = _label + "_" + boost::lexical_cast<Label>(percentage) 
        + "_Percent";

    /*!
		\todo NOTE: to get reproducible results the random number seed must
		be set outside. Commenting srand sentence. JH 
		Reply: Being reproducible may be important in some cases, but in the
		common case I would imagine the user is interested to have different
		fractions every time the function is called, so I put it back in. In
		any case to be reproducible the target can be stored for next time (TT)
	*/
	// set seed of random generator
    srand((unsigned) time(0));

    for (std::set<Cell_GID>::const_iterator i = _elements->begin();
         i != _elements->end();
         ++i)
    {
        // Rolling the dice
        double p = rand() / (double) RAND_MAX;
        if (p <= percentage)
        {
            // Adding neuron to result
            result._elements->insert(*i);
        }
    }

    return result;
}

// ----------------------------------------------------------------------------

const Label & Cell_Target::label() const
{
    return _label;
}

// ----------------------------------------------------------------------------

const Text & Cell_Target::description() const
{
    return _description;
}

// ----------------------------------------------------------------------------

void Cell_Target::label(const Label & text)
{
    _label = text;
}

// ----------------------------------------------------------------------------

void Cell_Target::description(const Text & text)
{
    _description = text;
}


// ----------------------------------------------------------------------------

void Cell_Target::insert (Cell_GID gid)
{
    if (!_elements.unique())
        _elements.reset(new Cell_GID_Set(*_elements));
    _elements->insert(gid);
}

// ----------------------------------------------------------------------------

void Cell_Target::erase(Cell_GID gid)
{
    if (!_elements.unique())
        _elements.reset(new Cell_GID_Set(*_elements));
    _elements->erase(gid);
}

// ----------------------------------------------------------------------------

Cell_Target::const_iterator Cell_Target::find(Cell_GID gid) const
{
    return const_iterator(_elements->find(gid));
}

// ----------------------------------------------------------------------------

void Cell_Target::clear()
{
    _elements.reset(new Cell_GID_Set);
}

// ----------------------------------------------------------------------------

void Cell_Target::print() const
{
    std::cout << * this;
}

// ----------------------------------------------------------------------------

size_t Cell_Target::size() const
{
    return _elements->size();
}

// ----------------------------------------------------------------------------

Cell_Target & Cell_Target::operator += (const Cell_Target & rhs)
{
    if (!_elements.unique())
        _elements.reset(new Cell_GID_Set(*_elements));

    _label = _label + "_UNION_" + rhs._label;
    std::set_union(_elements->begin(), _elements->end(),
        rhs._elements->begin(), rhs._elements->end(), 
        std::inserter(*_elements, _elements->begin()));
    return * this;
}

// ----------------------------------------------------------------------------

Cell_Target & Cell_Target::operator += (Cell_GID gid)
{
    insert(gid);
    return * this;
}

// ----------------------------------------------------------------------------

Cell_Target Cell_Target::operator + (Cell_GID gid) const
{
    Cell_Target result(*this);
    result.insert(gid);
    return result;
}

// ----------------------------------------------------------------------------

std::ostream & operator << (std::ostream & lhs, const Cell_Target & rhs)
{
    if (rhs.label().empty())
    {
        lhs << "# " << rhs.description() << " (" << rhs.size() << " cells)" 
            << std::endl 
            << "Target Cell Unnamed" << std::endl
            << "{ " << std::endl << " ";
    }
    else
    {
        lhs << "# " << rhs.description() << " (" << rhs.size() << " cells)" 
            << std::endl 
            << "Target Cell " << rhs.label() << std::endl
            << "{ " << std::endl << " ";
    }

    for (Cell_Target::const_iterator i = rhs.begin(); 
        i != rhs.end();++i)
    {
        lhs << "a" << * i << " ";
    }
    lhs  << std::endl << "}" << std::endl << std::endl;

    return lhs;
}

// ----------------------------------------------------------------------------

#endif // ifndef SWIG

}
#endif
