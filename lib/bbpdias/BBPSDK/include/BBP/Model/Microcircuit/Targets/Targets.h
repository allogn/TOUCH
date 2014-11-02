/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Authors:    Thomas Traenkler
                    James King

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_TARGETS_H
#define BBP_TARGETS_H

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include "BBP/Common/Exception/Exception.h"
#include "Target.h"
#include "Cell_Target.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! Targets is a container for microcircuit targets. 
/*! See Target class for more information on what is a target.
    \ingroup Microcircuit_Target
    \sa Target Cell_Target
*/
class Targets
{
public:

    //! Destructor
    ~Targets();

    //! Forward const iterator
    class const_iterator;
    //! Forward (read-only!) const iterator.
    /*!
      \todo Modification of elements is currently not allowed to avoid changing
      the target name and break targets container key consistency. To change 
      the content of a target you currently need to replace it, copy the 
      target, remove it from the container, and add the changed target back 
      into the container. (TT)
    */
    typedef const_iterator iterator;

    inline const_iterator begin() const;
    inline const_iterator end() const;

    //! access a target by its name
    Target & get_target(const Label & label)
        /* throw (Target_Not_Found) */;
    // \todo This should return an iterator. (TT)
    //! access a target by its name
    const Target & get_target(const Label & label) const
        /* throw (Target_Not_Found) */;
    
    //! access a target by its name, retrieving an iterator
    const_iterator find(const Label & label) const
	/* throw (Target_Not_Found) */;
    
    /*! 
     Add the given target to the end of the primary target container, 
     validating a target with the same name does not already exist. This 
     Targets object takes ownership of the target and is responsible for 
     deallocating it when it is destroyed.
     
     @param pTarget Pointer to new target
     
     \todo Conforming to STL conventions and other containers and for SWIG
     wrapping, probably this function is better avoiding pointers and called
     void push_back(Target & target); (TT)
     \todo A brief comment should be shown for the function for the user
     to quickly see what the purpose of the function is - see the doxygen 
     documentation. (TT)
     \bug This function cannot be wrapped safely due to the memory 
     management specifications. Thus, ignored (JH).
     */
    void push_back(const Target & target);
    
    /*!
      insert an empty target specifying its label and type.  This function is 
      not really supported at the current time.  It allocates the target which
      will be deallocate when the Targets object is destroyed
      \todo Would prefer this to return a reference or an iterator 
      instead of a pointer to match rest of library and also to ease
      SWIG wrapping. (TT)
      \bug Wrapping this function with SWIG is troublesome due to memory 
      allocation issues. Thus, ignored (JH).
     */
    Target  insert(const Label & label, Target_Type type);
    
    /*!
     Test if a target with a name exists in the container
     
     @param label Name of the target to look for
     @return True if the target with given name exist; otherwise, false
    */
    bool exists(const Label & label) const;
    
    //! drop a target from the targets container.  
    void erase(const Label & label);
    //! drop all currently loaded targets
    void clear();
    //! return number of targets in the container
    size_t size() const;
    //! return a cell target (resolves and flattens nested targets)
    Cell_Target cell_target(const Label & label) const 
        /* throw (Target_Not_Found) */;
    
private:
    /*! 
      \brief All Targets in a human defined order (as read from files/ built 
      from guis, etc)
    */
    std::vector<Target> _targets;

    /*!
      All Targets ordered for faster look up - but very sensitive to insertions
      and deletions not at the end of the vector If storing the index of the
      target object proves inefficient, then it might be better to store a
      pointer.
     */
    std::map<Label, unsigned int> _target_lookup;
};

#ifndef SWIG //----------------------------------------------------------------

class Targets::const_iterator 
    : public std::iterator<std::forward_iterator_tag, Target >
{
    friend class Targets;

public:
    const_iterator()
    {}

private:
    explicit const_iterator(std::vector<Target >::const_iterator iterator)
        : _current(iterator)
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
    Target & operator * () const
    {
		//! \todo Review const_cast. (TT)
        // This is a hideous hack, possibly a crime against decency, done to 
        // allow Boost::Python to wrap the iterators for the Targets class
        // JLKenyon - Tue Jan  6 19:05:21 CET 2009
        return const_cast<Target &>(*_current);
    }
    
    //! Get pointer to element at current iterator position.
    const Target  operator -> () const
    {
        //Manually modified during transition to boost smart pointers
        //return & (this->operator *());
        return *_current;
    }

    //! Return the stored pointer value instead of the reference
    //  Direct access to the pointer is needed when using smart
    //  pointers, to allow us to copy the smart pointer instead
    //  of creating a new one (which will cause a serious error)
    //    This is required because the operator * was overloaded
    //  to behave as an operator **, denying a simple access to
    //  the normal operator *.  This solution may not be ideal.
    //  				- JLK 8.10.08
    const Target get_pointer() const
    {
	return *_current;
    }
    

private:
    std::vector<Target >::const_iterator _current;

};

//-----------------------------------------------------------------------------

Targets::const_iterator Targets::begin() const
{
    return const_iterator(_targets.begin());
}

//-----------------------------------------------------------------------------

Targets::const_iterator Targets::end() const
{
    return const_iterator(_targets.end());
}

#endif // SWIG ----------------------------------------------------------------

}
#endif
