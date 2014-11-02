/*

    Ecole Polytechnique Federale de Lausanne
    Brain Mind Institute,
    Blue Brain Project
    (c) 2006-2007. All rights reserved.

    Responsible author:	Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COPY_ON_WRITE
#define BBP_COPY_ON_WRITE

#include <list>
#include <boost/shared_ptr.hpp>

#include <BBP/Common/Exception/Exception.h>

/** \cond SHOW_IN_DEVELOPER_REFERENCE */

namespace bbp
{

template <typename Wrapper>
class copy_on_write;

class copy_on_write_core
{
protected:
public: // The SGI and Apples seem to think it needs public 
        // access to all of these functions - JLK
    template <typename Wrapper> friend class copy_on_write;
  
    template <typename Wrapper> 
    static size_t impl_use_count(const Wrapper & wrapper)
    {
        return wrapper._impl.use_count();
    }

    template <typename Wrapper> 
    static void duplicate_impl(Wrapper & wrapper)
    {
        wrapper._impl.reset
            (new typename Wrapper::implementation(*wrapper._impl));
    }
    template <typename Wrapper> 
    static void copy_impl_pointer(Wrapper & source, Wrapper & destination)
    {
        destination._impl = source._impl;
    }
};

//! Base class for classes implementing Copy On Write support.
/*!
  This class provides all the necessary mechanisms for a class to
  implement an advanced copy on write support.
  Objects deriving from this class support two copy policies that can
  be selected during copy construction. From the user perspective, these 
  policies are equivalent to shallow copy and deep copy of the object. 
  The shallow copy, creates a new reference to the original object's data. 
  That means that the new object becomes an alias of the previous one and any
  modification to any of the them is seen by the other. 
  Deep copy performs a cheap copy of the object data and leaves final data
  copy to the moment when a modification is going to happen to any of the
  instances temporatily sharing the data.
  The same object data can have references (real shallow copies) and fake
  deep copies without interfering each other. When an object decides to
  deep copy the data all the objects that are shallow copies are updated to
  point to the new location for the data.

  To enable copy on write the class inheriting from copy_on_write must
  pass itself as the template parameter of copy_on_write. Besides that, the
  derived class must fulfill the following requirements.
  - It must declare a class or typedef called
  - It must make copy_on_write_core a friend so the base class can access
    if members.
  - The internal data must be stored in an object called _impl of type
    boost::shared_ptr<T::implementation>.
  - The internal implementation object must have a public copy constructor.
  - The _impl object is never assumed to be a null pointer..

  Is user responsibility to call the inherited function check_and_copy
  before any modification of the data is about to happen. Nothing needs
  to be done in the constructor except creating a valid instance of _impl.
  The destructor can be empty. The copy constructor must call the base
  copy_on_write constructor and should take a bool parameter to pass to
  it unless one specific policy wants to be enforced.

  There is a small storage overhead due to the use of this class, which 
  is equivalent to a smart pointer plus a std::list of pointers for
  each independent set of references for the same data (to store the list
  of aliases of that data). The consistency requeriments for groups of 
  shallow copies have also a runtime penalty when copy on write needs to
  be performed for any of these groups.

  An example of a class using this pattern class follows:
  \code
  class Simple_Stack : protected bbp::copy_on_write<Simple_Stack>
  {
  public:
      Simple_Stack() :
          _impl(new Simple_Stack_Impl())
      {}

      Simple_Stack(const Simple_Stack & stack, bool reference = false) :
          bbp::copy_on_write<Simple_Stack>(stack, reference)
      {
          _impl = stack._impl;
      }

      void push(int i)
      {
          // Check if we need to copy the data and perform a deep copy
          // in that case.
          check_and_copy(this);
          _impl->_data.push_back(i);
      }

      void print() const
      {
          std::cout << "[ ";
          std::copy(_impl->_data.begin(), _impl->_data.end(), 
                    std::ostream_iterator<int>(std::cout, " "));
          std::cout << "]" << std::endl;
      }

  protected:
      // Make private members accessible to COW implementation.
      friend class bbp::copy_on_write_core;

    struct Simple_Stack_Impl
    {
        Simple_Stack_Impl()
        {}

        Simple_Stack_Impl(const Simple_Stack_Impl & other) :
            _data(other._data)
        {
            std::cout << "Copying " << this << std::endl;
        }
        std::vector<int> _data;
    };
    typedef Simple_Stack_Impl implementation;

  protected:
        boost::shared_ptr<Simple_Stack_Impl> _impl;
  };

  \endcode

  Notes: 
  - Exception safety of this class hasn't been evaluated.
  - The current implementation doesn't support virtual inheritance from
  this class.
  \todo Implement thread safety
*/
template <typename Wrapper>
class copy_on_write
{
public:
    copy_on_write() :
        _state(new state)
    {
        _cursor = _state->insert_reference(static_cast<Wrapper*>(this));
    }

    //! Copy constructor
    /*!
      @param other The source copy_on_write object of the object being
      copied.
      @param reference If true this object becomes an independent reference
      of the data (deep copy policy). Data will be shared until the original
      reference or this one call check_and_copy. If false this object shares
      the data with the source one.
    */
    copy_on_write(const copy_on_write &other, bool reference)
    {
        if (reference)
            _state = other._state;
        else
            _state.reset(new state());
        _cursor = _state->insert_reference(static_cast<Wrapper*>(this));
    }

    ~copy_on_write()
    {
        _state->remove_reference(_cursor);
    }

    //! Checks if a deep copy of the data needs to be performed.
    /*! A deep copy of the data is done when the total number of objects
      pointing to the internal data is larger than the number of objects
      that are an alias of this one. When a copy needs to be done the pointer
      stored by all alias objects is update to maintain consistency. Note
      the performance impact this operation may have when the number of 
      shallow references to the same data is large. */
    void check_and_copy(Wrapper * wrapper)
    {
        _state->check_and_copy(wrapper);
    }
    
protected:
    class state;
    boost::shared_ptr<state> _state;

    typename std::list<Wrapper*>::iterator _cursor;
};

//! Implementation class for copy_on_write
template <typename Wrapper>
class copy_on_write<Wrapper>::state
{
public:
    void check_and_copy(Wrapper * wrapper)
    {
        bbp_assert(_references.size() > 0);

        // Checking if the references count of the internal data is the
        // same as the number of non copy on write references this object
        // knows.
        if (_references.size() != copy_on_write_core::impl_use_count(*wrapper))
        {
            // There are other references to the data that are independent
            // from this. We need to copy the data and update the pointer
            // in all the dependent objects (which are the references that
            // are sharing the data.
            copy_on_write_core::duplicate_impl(*wrapper);

            // Replacing the data pointer in all the shared references.
            for (typename std::list<Wrapper*>::iterator i = 
                     _references.begin();
                 i != _references.end(); ++i)
                copy_on_write_core::copy_impl_pointer(*wrapper, **i);
        }
    }

    typename std::list<Wrapper*>::iterator insert_reference(Wrapper * cow)
    {
        _references.push_front(cow);
        return _references.begin();
    }

    void remove_reference(const typename std::list<Wrapper*>::iterator &cursor)
    {
        _references.erase(cursor);
    }

    std::list<Wrapper*> _references;
};

}
/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

#endif
