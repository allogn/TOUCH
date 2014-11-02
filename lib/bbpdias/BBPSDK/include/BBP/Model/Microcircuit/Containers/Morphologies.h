/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

        Responsible authors:	Thomas Traenkler
								Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MORPHOLOGIES_H
#define BBP_MORPHOLOGIES_H

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include "BBP/Model/Microcircuit/Types.h"

namespace bbp {

class Morphology;
class Morphology;
typedef boost::shared_ptr<const Morphology> Const_Morphology_Ptr;
typedef boost::shared_ptr<Morphology> Morphology_Ptr;
//! A set of neuron morphologies.
/*! 
	The Morphologies class is a container for neuron morphologies.
	\sa Morphology
    \ingroup Microcircuit_Containers
*/
class Morphologies
{
    friend class Microcircuit;

public:

    /*!
        \name Access
        For access and navigation inside the morphologies container.
    */
    //@{
    //! Forward iterator for the Morphologies container to visit all elements
    class iterator;
     //! Forward iterator for the Morphologies container to visit all elements
    class const_iterator;
 
    //! Returns the first element of the container for linear access
    inline iterator begin();
    //! Returns the iterator that indicates the end of the container
    inline iterator end(); 

   //! Returns the first element of the container for const linear access
    inline const_iterator begin() const;
    //! Returns the const iterator that indicates the end of the container
    inline const_iterator end() const;

    //! Access morphology via label.
    /*!
        Iterator to end returned if morphology not found
    */
    inline iterator find(const Label & name);
    //! Access morphology via label.
    /*!
        Iterator to end returned if morphology not found
    */
    inline const_iterator find(const Label & name) const;

    //! Retrieve number of morphologies in the container.
    inline size_t size() const;
    //@}

    /*!
        \name Modification
        Insert or remove elements from the container.
    */
    //@{

    inline void insert(const Morphology_Ptr & morphology);

    //! Erase morphologies with specified global identifier.
    inline void erase(const Label & name);

    //! Drop all morphologies from microcircuit.
    inline void clear();
    //@}

    //! Print global identifiers of morphologies in this container
    inline void print() const;

protected:
    
    typedef std::map<Label, Morphology_Ptr> Morphology_Map;
    Morphology_Map        _elements;
};

//! Outputs the morpholgies to the output stream in ASCII character format
inline std::ostream & operator << (std::ostream & lhs, 
                                   const Morphologies & rhs);
}
#include "BBP/Model/Microcircuit/Morphology.h"

namespace bbp
{

#ifndef SWIG

// ----------------------------------------------------------------------------

class Morphologies::iterator
    : public std::iterator<std::forward_iterator_tag, Morphology>
{
    friend class Morphologies;
    friend class Morphologies::const_iterator;

public:
    iterator()
    {}

private:
    explicit iterator(Morphologies::Morphology_Map::iterator iterator)
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

    //! Advance to next neuron in the container.
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
    Morphology & operator * ()
    {
        return * _current->second.get();
    }

    //! Get pointer to element at current iterator position.
    Morphology * operator -> ()
    {
        return & (this->operator *());
    }

    //! Get smart pointer to element at current iterator position.
    /*! If the iterator is at the end the returned value is undefined */
    Morphology_Ptr ptr() const
    {
        return _current->second;
    }

private:
    Morphologies::Morphology_Map::iterator _current;
};

// ----------------------------------------------------------------------------

class Morphologies::const_iterator
    : public std::iterator<std::forward_iterator_tag, const Morphology>
{
    friend class Morphologies;
    friend class Morphologies::iterator;

public:
    const_iterator()
    {}

    const_iterator(Morphologies::iterator iterator)
        : _current(iterator._current)
    {}

private:
    explicit const_iterator
    (Morphologies::Morphology_Map::const_iterator iterator)
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
    const Morphology & operator * () const
    {
        return *_current->second.get();
    }

    //! Get pointer to element at current iterator position.
    const Morphology * operator -> () const
    {
        return & (this->operator*());
    }

private:
    Morphologies::Morphology_Map::const_iterator _current;
};

// ----------------------------------------------------------------------------

Morphologies::const_iterator Morphologies::find(const Label & name) const
{
    return const_iterator(_elements.find(name));
}

// ----------------------------------------------------------------------------

Morphologies::iterator Morphologies::find(const Label & name)
{
    return iterator(_elements.find(name));
}

// ----------------------------------------------------------------------------

Morphologies::iterator Morphologies::begin(void)
{
    return iterator (_elements.begin());
}

// ----------------------------------------------------------------------------

Morphologies::iterator Morphologies::end(void)
{
    return iterator (_elements.end());
}

// ----------------------------------------------------------------------------

Morphologies::const_iterator Morphologies::begin(void) const
{
    return const_iterator (_elements.begin());
}

// ----------------------------------------------------------------------------

Morphologies::const_iterator Morphologies::end(void) const
{
    return const_iterator (_elements.end());
}

// ----------------------------------------------------------------------------

void Morphologies::erase(const Label & name)
{
    _elements.erase(name);
}

// ----------------------------------------------------------------------------

void Morphologies::clear()
{
    Morphology_Map _;
    _elements.swap(_);
}

// ----------------------------------------------------------------------------

size_t Morphologies::size() const
{
    return _elements.size();
}

// ----------------------------------------------------------------------------

void Morphologies::print() const
{
    std::cout << *this;
}

// ----------------------------------------------------------------------------

void Morphologies::insert(const Morphology_Ptr & morphology)
{
    _elements.insert(std::make_pair(morphology->label(), morphology));
}

// ----------------------------------------------------------------------------

std::ostream & operator << (std::ostream & lhs, const Morphologies & rhs)
{
    for (Morphologies::const_iterator i = rhs.begin();
         i != rhs.end();
         ++i)
    {
        lhs << *i << std::endl;
    }
    return lhs;
}

// ----------------------------------------------------------------------------
#endif // ifndef SWIGJAVA

}
#endif
