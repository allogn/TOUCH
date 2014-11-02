/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Sebastien Lasserre
        
        TODO :
        
        - use the container template to generate this container. (SL - 21/01/2008)

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MESHES_H
#define BBP_MESHES_H

#include <map>
#include <boost/shared_ptr.hpp>

#include "BBP/Model/Microcircuit/Types.h"

namespace bbp {

class Mesh;
typedef boost::shared_ptr<const Mesh> Const_Mesh_Ptr;
typedef boost::shared_ptr<Mesh> Mesh_Ptr;

//! A container of neuron meshes.
/*!
    The Mesh class is a container for neuron meshes.
    \sa Mesh
    \ingroup Microcircuit_Containers
*/
class Meshes
{
    friend class Microcircuit;

public:
    ////! Print global identifiers of meshes in this container
    //inline void print() const;

    // ITERATOR ---------------------------------------------------------------

    /*!
        \name Access
        For access and navigation inside the meshes container.
    */
    //@{

    //! Forward iterator for the Meshes container to visit all elements
    class iterator;

    //! Forward iterator for the Meshes container to visit all elements
    class const_iterator;

 
    //! Returns the first element of the container for linear access
    inline iterator begin();
    //! Returns the iterator that indicates the end of the container
    inline const iterator end();

    //! Returns the first element of the container for const linear access
    inline const_iterator begin() const;
    //! Returns the const iterator that indicates the end of the container
    inline const const_iterator end() const;

    //! Access mesh via label.
    /*!
        Iterator to end returned if morphology not found.
    */
    inline iterator find(const Label & name);
    //! Access mesh via label.
    /*!
        Iterator to end returned if morphology not found.
    */
    inline const_iterator find(const Label & name) const;

    //! Retrieve number of meshes in the container.
    inline size_t size() const;
    //@}

    /*!
        \name Modification
        Insert or remove elements from the container.
    */
    //@{
    inline void insert(const Label name, const Mesh_Ptr & mesh);

    //! Erase mesh with specified global identifier.
    inline void erase(const Label & name);

    //! Drop all meshes from microcircuit.
    inline void clear();
    //@}

protected:
    
    typedef std::map<Label, Mesh_Ptr> Mesh_Map;
    Mesh_Map        _elements;
};

//! Outputs the morpholgies to the output stream in ASCII character format
// inline std::ostream & operator << (std::ostream & lhs, 
//                                 const Meshes & rhs);
}
#include "BBP/Model/Microcircuit/Mesh.h"

namespace bbp
{

#ifndef SWIG

// ----------------------------------------------------------------------------

class Meshes::iterator
    : public std::iterator<std::forward_iterator_tag, Mesh>
{
    friend class Meshes;
    friend class Meshes::const_iterator;

public:
    iterator()
    {}

private:
    explicit iterator(Meshes::Mesh_Map::iterator mesh_iterator)
        : _current(mesh_iterator)
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
        ++ _current;
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
    Mesh & operator * ()
    {
        return * _current->second.get();
    }

    //! Get pointer to element at current iterator position.
    Mesh * operator -> ()
    {
        return & (this->operator *());
    }

    //! Get smart pointer to element at current iterator position.
    Mesh_Ptr ptr() const
    {
        return _current->second;
    }

    //! Get identifier name of the mesh.
    const Label & label() const
    {
        return _current->first;
    }

private:
    Meshes::Mesh_Map::iterator _current;

};

// ----------------------------------------------------------------------------

class Meshes::const_iterator
    : public std::iterator<std::forward_iterator_tag, const Mesh>
{
    friend class Meshes;
    friend class Meshes::iterator;

public:
    const_iterator()
    {}

    const_iterator(Meshes::iterator iterator)
        : _current(iterator._current)
    {}

private:
    explicit const_iterator (Meshes::Mesh_Map::const_iterator mesh_iterator)
        : _current(mesh_iterator)
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
    const Mesh & operator * () const
    {
        return *_current->second.get();
    }

    //! Get pointer to element at current iterator position.
    const Mesh * operator -> () const
    {
        return & (this->operator *());
    }

    //! Get const smart pointer to element at current iterator position.
    Const_Mesh_Ptr ptr() const
    {
        return _current->second;
    }

    //! Get identifier name of the mesh.
    const Label & label() const
    {
        return _current->first;
    }

private:
    Meshes::Mesh_Map::const_iterator _current;
};

// ----------------------------------------------------------------------------

Meshes::const_iterator Meshes::find(const Label & name) const
{
    return const_iterator(_elements.find(name));
}

// ----------------------------------------------------------------------------

Meshes::iterator Meshes::find(const Label & name)
{
    return iterator(_elements.find(name));
}

// ----------------------------------------------------------------------------

Meshes::iterator Meshes::begin(void)
{
    return iterator (_elements.begin());
}

// ----------------------------------------------------------------------------

const Meshes::iterator Meshes::end(void)
{
    return iterator (_elements.end());
}

// ----------------------------------------------------------------------------

Meshes::const_iterator Meshes::begin(void) const
{
    return const_iterator (_elements.begin());
}

// ----------------------------------------------------------------------------

const Meshes::const_iterator Meshes::end(void) const
{
    return const_iterator (_elements.end());
}

// ----------------------------------------------------------------------------

void Meshes::erase(const Label & name)
{
    _elements.erase(name);
}

// ----------------------------------------------------------------------------

void Meshes::clear()
{
    Mesh_Map _;
    _elements.swap(_);
}

// ----------------------------------------------------------------------------

size_t Meshes::size() const
{
    return _elements.size();
}

// ----------------------------------------------------------------------------

// void Meshes::print() const
// {
//     std::cout << *this;
// }

// ----------------------------------------------------------------------------

void Meshes::insert(Label name, const Mesh_Ptr & mesh)
{
    _elements.insert(std::make_pair(name, mesh));
}

// ----------------------------------------------------------------------------

// std::ostream & operator << (std::ostream & lhs, const Meshes & rhs)
// {
//     for (Meshes::const_iterator i = rhs.begin();
//         i != rhs.end();
//         ++i)
//     {
//         lhs << *i << std::endl;
//     }
//     return lhs;
// }

// ----------------------------------------------------------------------------

#endif
}
#endif
