/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2008. All rights reserved.

        Responsible authors:    Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_INDEXED_CONTAINER_H
#define BBP_INDEXED_CONTAINER_H

#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/mpl/if.hpp>

#include "BBP/Common/Types.h"
#include "BBP/Common/Patterns/member_functions.h"
#include "detail/Index_Set.h"


namespace bbp
{

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
#ifndef SWIG //----------------------------------------------------------------

namespace indexed_container
{

template <typename T, typename Index>
class index_of_T
{
public:
    Index operator()(const T &t) const
    {
        return t.id();
    }
};

template <typename T>
struct remove_const_and_reference
{
    typedef typename boost::remove_const<
                typename boost::remove_reference<T>::type
            >::type type;
};

// ----------------------------------------------------------------------------

//! Adapter class for accessing a back storage class.
template <
    typename T, 
    typename Index,
    typename Storage,
    class Impl = void,
    class Indexer = index_of_T<typename remove_const_and_reference<T>::type, 
                               Index>
>
class Storage_Adapter
{
public:
    typedef T       return_type;
    typedef T       reference_type;
    typedef Index   index_type;
    typedef Storage storage_type;
    typedef Indexer indexer_type;

    //! Constructor taking the real storage object.
    /*! Default constructor only provided for cases where its needed */
    Storage_Adapter(const Storage & storage = Storage()) : _storage(storage) {}

    //! Allowing construction from compatiable adapters
    /*! A compatible adapter must fulfill that:
        Their Index parameter are compatabile.
        Their Storage and T parameters are the same except from const 
        qualifications. In precence of const only the following are allowed:
        If the source parameter is X the target can be both X and const X,
        if the source parameter is const X the target must be const X.
        For example Storage_Adapter<int, int, container, X, Y> is a valid
        argument for Storage_Adapter<const int, int, const container, X, Y> 
        but not the opposite.
    */
    template <typename U, typename Other_Storage, typename Other_Impl>
    Storage_Adapter(const Storage_Adapter<U, Index, Other_Storage,
                                          Other_Impl, Indexer> & compatible) :
        _storage(compatible._storage)
    {
        BOOST_STATIC_ASSERT((boost::is_convertible<U, T>::value &&
                             (boost::is_const<T>::value ||
                              !boost::is_const<U>::value)));
    }

    template <typename U, typename Other_Storage>
    bool operator ==(const Storage_Adapter<U, Index, Other_Storage, Indexer> & 
                     compatible) const
    {
        BOOST_STATIC_ASSERT((boost::is_convertible<U, T>::value));
        return _storage == compatible._storage;
    }
    
    T find(Index i) const
    {
        return static_cast<const Impl*>(this)->find_impl(i);
    }

    Index index_of
    (const typename remove_const_and_reference<T>::type & object) const
    {
        return Indexer()(object);
    }

    bool valid() const
    {
        // _storage can never be 0 since it is constructed from a reference
        return true;
    }

protected:
    template <typename U, typename I, typename St, typename Imp, typename Ix>
    friend class Storage_Adapter;

    Storage _storage;
};

// ----------------------------------------------------------------------------

}
}

#include "detail/storage_adapter_specializations.h"

namespace bbp
{
namespace indexed_container
{

// ----------------------------------------------------------------------------
 
template <typename T, 
          typename Index, 
          typename Storage,
          bool     reference = false,
          class Storage_Adapter = 
                indexed_container::Storage_Adapter<
                    typename boost::mpl::if_c<reference, T &, T>::type, 
                    Index, Storage *>,
          bool Ordered = false>
class traits
{
public:
    typedef T                                      stored_type;
    typedef Index                                  index_type;
    typedef typename Storage_Adapter::storage_type storage_type;
    typedef Storage_Adapter                        storage_adapter;
    static const bool                              ordered = Ordered;
};

// ----------------------------------------------------------------------------

template <typename Set>
class insert_iterator
{
public:
    typedef int difference_type;
    typedef std::input_iterator_tag iterator_category;
    typedef insert_iterator<Set> * pointer;
    typedef insert_iterator<Set> & reference;
    typedef insert_iterator<Set> value_type;

    insert_iterator(Set & set) : _set(set) {}
    
    template <typename Index>
    insert_iterator & operator = (Index value) 
    { 
        _set.insert(value); return *this;
    }

    insert_iterator & operator * () { return *this; }

    insert_iterator & operator ++ () { return *this; }

    insert_iterator & operator ++ (int) { return *this; }

protected:
    Set & _set;
};

// ----------------------------------------------------------------------------

template <typename Set>
insert_iterator<Set> inserter(Set & set)
{
    return insert_iterator<Set>(set);
}

}

#endif //SWIG -----------------------------------------------------------------
/** \endcond */ // SHOW_IN_DEVELOPER_REFERENCE

// ----------------------------------------------------------------------------

//! Generic container to store indices to objects contained in some back storage
/*!
  \section Synopsis
  This template class can be used to store efficiently indices that refer to
  objects stored in some external back storage. This container only stores
  references, not being possible to add or remove objects inside the real
  storage, but only allow read and modify access to those objects. 

  Depending on the instantiation parameters the indices can be stored ordered
  or unordered, only ordered sets support iterators that enumerate objects 
  in order and bidirectionally, while unordered containers will be more
  space and time efficient.

  Through the use of some optional adapter class, the query functions can
  transform the objects before returning them to the user or even create
  temporary objects that are not actually stored inside the back storage. An
  adapter class is specified as an optional template argument.  
  
  Copies of containers are performed with copy-on-write semantics, this
  means that copies will be cheap and the internal index set shared until
  one of the objects tries to insert or remove some index. In that case
  the set is copied before the operation is performed.
  Note that this only applies to the container itself, not the pointed
  objects. The same object can be referenced by different containers and 
  modified from any of those.
  
  \section Instantiation Template instantiation
  \arg <tt>typename T</tt>: The type of objects that the user will see. If an
    storage adapter is provided this may not be the type of objects actually
    stored inside the Storage class.
    The actual instantiation can be of type X or const X where X is a
    typename without reference modifiers (X &).
    \todo When the default storage adapter is used, to be able to insert
     an object in the index set by reference and not giving its index directly,
     T must declare a method <tt>Index T::id() const</tt>. (Unimplemented yet).
  \arg <tt>typename Index</tt>: The type used to index objects of type T inside
    the back storage. This index type must be an integral discrete type or class
  (i.e int, long, uint64_t).
  \arg <tt>typename Storage</tt>: The back storage class. By default, if no
    adapter class is provided, this class is expected to have a method with
    the following possible prototype: <tt>T & find(const Index & index)</tt>, 
    <tt>const T & find(const Index & index) const</tt>, 
    <tt>T find(const Index & index)</tt>, <tt>T find(const Index & index)
    const</tt>. The <tt>const</tt> qualified methods are required for 
    const_iterators and when T is actually const X (obviating the double const
    qualification). In the default instantiation only the methods returning
    by value are required.
    These requirements don't apply to user defined storage adapter 
    instantiations (see traits parameter for details).
    
  \arg <tt>typename Traits</tt>: By default this parameter doesn't need to
    be specified. This class must declare the following types and constants:
    * Traits::stored_type The same type as the T parameter.
    * Traits::index_type The same type as the Index parameter.
    * Traits::storage_type The same type as the Storage parameter.
    * Traits::storage_adapter Some user given class that acts as an adapter
    to interface the storage object.
    * Traits::ordered A bool constant that says if the index set must be 
    ordered or not.
    In the namespace indexed_container there is a template class traits
    defined that makes is easier to specify user defined Traits. See the
    instantiation examples or refer to the developer documentation for further
    details.

  Some instantiation examples are:

  \code
  struct Foo
  {
     std::string & find(int index);
     const std::string & find(int index) const;
  };

  Foo storage;

  // Default instantations
  // Iterators will dereference strings from Foo by value (they are copied).
  Indexed_Container<std::string, int, Foo> c1(storage);
  Indexed_Container<const std::string, int, const Foo> c2(storage);

  // Specifying that find functions return by reference.
  Indexed_Container<const std::string, int, const Foo,
      indexed_container::traits<
          const std::string, // Type that the user will access
          int,               // Index type
          const Foo,         // Back storage type
          true               // find functions in Foo return by reference
      >
  > c3(storage);
  Indexed_Container<std::string, int, Foo
                    indexed_container::traits<std::string, int, Foo, true> c4;

  // User given adapter. It implements a find_impl method that is used to
  // access the inherited attribute Foo * _storage and return the object 
  // referenced by a given index. Note that find_impl could be transforming
  // or creating a temporary object using information from _storage instead
  // of just fordwaring it.
  // And adapter must be copy constructible and assignable.
  // See src/Test/indexed_container.cpp to get a full example showing how
  // different adapters may become compatible.
  class My_Foo_Adapter : public indexed_container::Storage_Adapter<
      std::string,   // Type returned by the find_impl function.
      int,           // Index type.
      Foo *,         // Back storage.
      My_Foo_Adapter // This type name
                     // (Curiously Recursive Template Pattern applied).
  >
  {
  public:
      // Required constructor.
      My_Foo_Adapter(Foo & foo) :
          indexed_container<std::string, int, Foo, My_Foo_Adapter>(foo) {}
  private:
      friend class indexed_container<std::string, int, Foo, My_Foo_Adapter>;
      std::string find_impl(const int & index)
      {
          return _storage.find(index) + " (copy)";
      }
  };
  Indexed_Container<std::string, int, const Foo,
      indexed_container::traits<
          std::string, int, Foo, 
          false,          // this parameter doesn't matter if the default 
                          // adapter instantation is not used.
          My_Foo_Adapter, // User defined storage adapter.
          false           // Store an unordered index set.
      > 
  > c5(storage); // c5(My_Foo_Adapter(storage)) also allowed.

  \endcode

  \section indexed_container_set_operation_examples Set Operations

  This container supports boolean set operations that can mix compatible
  containers. Compatible containers are those whose storage adapters are 
  compatible, their index types the same, and object and storage types are 
  convertible. The return type of the set operations will have to most
  restrictive of the containers involved in the operations with regard to
  const qualifiers.

  Supported operations are intersection, union, difference and exclusive 
  disjunction (aka. exclusive or).
  
  Some example operations are:
  \code
  Indexed_Container<std::string, int, Foo> a, b;
  Indexed_Container<const std::string, int, const Foo> c;
  ...
  c = a & c;
  c = c | a;
  // b = a - c; Wrong, doesn't compile because b cannot be assigned from a
  // container of type c (which is the most restrictive between a and c).
  b = b ^ a;
  \endcode
*/
template <
    // Type of the elements stored
    typename T,
    // Type of the indices
    typename Index,
    // The type of the back storage
    class Storage,
    // Additional type traits
    class Traits = indexed_container::traits<T, Index, Storage>
>
class Indexed_Container
{
protected:
    /** \cond SHOW_IN_DEVELOPER_REFERENCE */
    typedef Traits traits;

public:
    template <typename Other_Traits>
    struct choose_container;
 
    struct incompatible_containers;

    /** \endcond */ // SHOW_IN_DEVELOPER_REFERENCE


public:
    inline Indexed_Container(Storage & storage, const Label & label = "");

    inline Indexed_Container(const typename Traits::storage_adapter & adapter,
                             const Label & label = "");

    //! Copy constructor.
    /**
       U can be T or const T
    */
    template <typename U, typename Other_Storage, typename Other_Traits>
    inline Indexed_Container
    (const Indexed_Container<U, Index, Other_Storage, Other_Traits> & other,
     const Label & label = "") :
        _label(label),
        _indices(other._indices),
        _storage(other._storage)
    {
        // Construction of containers of type T is only allowed from containers
        // of the same type and not from const T containers.
        BOOST_STATIC_ASSERT(!boost::is_const<U>::value || 
                            boost::is_const<T>::value);
    }

    //! Get label (identifier name) of the container.
    inline const Label & label() const;

    class iterator;

    class const_iterator;

    class reverse_iterator;

    class const_reverse_iterator;

    //! Returns an iterator to the first element of the container.
    inline iterator begin()
    {
        return iterator(_indices->begin(), _storage);
    }

    //! Returns the iterator that indicates the end of the container.
    inline iterator end()
    {
        return iterator(_indices->end(), _storage);
    }

    //! Get an iterator to the first contained element for const access.
    inline const_iterator begin() const
    {
        return const_iterator(_indices->begin(), _storage);
    }

    //! Get the const_iterator that indicates the end of the container.
    inline const_iterator end() const
    {
        return const_iterator(_indices->end(), _storage);
    }

    //! Returns an reverse iterator to the last element of the container.
    /*! Only available in ordered containers */
    inline reverse_iterator rbegin()
    {
        BOOST_STATIC_ASSERT((Traits::ordered));
        return reverse_iterator(iterator(_indices->end(), _storage));
    }

    //! Returns the reverse iterator that indicates the end of the container.
    /*! Only available in ordered containers */
    inline reverse_iterator rend()
    {
        BOOST_STATIC_ASSERT((Traits::ordered));
        return reverse_iterator(iterator(_indices->begin(), _storage));
    }

    //! Get an reverse iterator to the last contained element for const access.
    /*! Only available in ordered containers */
    inline const_reverse_iterator rbegin() const
    {
        BOOST_STATIC_ASSERT((Traits::ordered));
        return const_reverse_iterator(iterator(_indices->end(), _storage));
    }

    //! Get the reverse const iterator that indicates the end of the container.
    /*! Only available in ordered containers */
    inline const_reverse_iterator rend() const
    {
        BOOST_STATIC_ASSERT((Traits::ordered));
        return const_reverse_iterator(iterator(_indices->begin(), _storage));
    }

    //! Search an object inside the container.
    /*!
      An iterator equal to end() is returned is object is not found.
    */
    inline iterator find(Index index)
    {
        return _storage.valid() ? 
               const_iterator(_indices->find(index) , _storage) :
               end();
    }
    inline const_iterator find(Index index) const
    {
        return _storage.valid() ? 
               const_iterator(_indices->find(index) , _storage) :
               end();
    }

    //! Search an object inside the container.
    /*!
      An iterator equal to end() is returned is object is not found.
    */
    inline iterator find(const T & object)
    {
        if (_storage.valid())
            return iterator
                (_indices->find(typename Traits::storage_adapter::indexer_type()
                                (object)),
                 _storage);
        else
            return end();
    }
    inline const_iterator find(const T & object) const
    {
        if (_storage.valid())
            return iterator
                (_indices->find(typename Traits::storage_adapter::indexer_type()
                                (object)), 
                 _storage);
        else
            return end();
    }

    //! Insert an object by providing its index inside the back storage.
    inline void insert(const Index & index);

    //! Insert an object inside the back storage.
    inline void insert(T & object);

    //! Insert several objects by using a closed interval of indices.
    inline void insert_range(Index start, Index end);

    //! Remove the object identified by the given index.
    inline void erase(Index index);

    //! Remove the given object from the container.
    inline void erase(T & object);

    //! Remove several objects by using a closed interval of indices.
    inline void erase_range(Index start, Index end);

    //! Remove all elements from the container.
    /*!
      Back storage is unmodified
    */
    inline void clear();

    //! Retrieve number of objects in the container.
    inline size_t size() const;

    //! Swaps this container with other of the same type
    void swap(Indexed_Container & other);

    //! Prints indices of elements in this container.
    inline void print() const;

#ifdef _DOXYGEN_
    /** \cond SHOW_IN_USER_REFERENCE */
    compatible Indexed_Container operator & 
    (const compatible Indexed_Container & other) const;

    compatible Indexed_Container operator |
    (const compatible Indexed_Container & other) const;

    compatible Indexed_Container operator ^ 
    (const compatible Indexed_Container & other) const;

    compatible Indexed_Container operator - 
    (const compatible Indexed_Container & other) const;

    compatible Indexed_Container operator +=
    (const compatible Indexed_Container & other);

    /** \endcond */ // SHOW_IN_USER_REFERENCE
#endif
    /** \cond SHOW_IN_DEVELOPER_REFERENCE */
    template<typename U, typename Other_Storage, typename Other_Traits>
    typename choose_container<Other_Traits>::type operator & 
    (const Indexed_Container<U, Index, Other_Storage, Other_Traits> & rhs)
    const
    {
        typedef typename choose_container<Other_Traits>::type Result_Type;
        BOOST_STATIC_ASSERT((!boost::is_same<incompatible_containers, 
                                             Result_Type>::value));
        bbp_assert(_storage == rhs._storage);

        Label name = "(" + _label + " & " + rhs._label + ")";
        Result_Type result(_storage, name);
        Index_Set indices;
        std::set_intersection(_indices->begin(), _indices->end(),
                              rhs._indices->begin(), rhs._indices->end(),
                              indexed_container::inserter(indices));
        result._indices->swap(indices);

        return result;
    }

    template<typename U, typename Other_Storage, typename Other_Traits>
    typename choose_container<Other_Traits>::type operator | 
    (const Indexed_Container<U, Index, Other_Storage, Other_Traits> & rhs)
    const
    {
        typedef typename choose_container<Other_Traits>::type Result_Type;
        BOOST_STATIC_ASSERT((!boost::is_same<incompatible_containers, 
                                             Result_Type>::value));
        bbp_assert(_storage == rhs._storage);
        Label name = "(" + _label + " | " + rhs._label + ")";
        Result_Type result(_storage, name);
        Index_Set indices;
        std::set_union(_indices->begin(), _indices->end(),
                       rhs._indices->begin(), rhs._indices->end(),
                       indexed_container::inserter(indices));
        result._indices->swap(indices);

        return result;
    }

    template<typename U, typename Other_Storage, typename Other_Traits>
    typename choose_container<Other_Traits>::type operator -
    (const Indexed_Container<U, Index, Other_Storage, Other_Traits> & rhs)
    const
    {
        typedef typename choose_container<Other_Traits>::type Result_Type;
        BOOST_STATIC_ASSERT((!boost::is_same<incompatible_containers, 
                                             Result_Type>::value));
        bbp_assert(_storage == rhs._storage);
        Label name = "(" + _label + " - " + rhs._label + ")";
        Result_Type result(_storage, name);
        Index_Set indices;
        std::set_difference(_indices->begin(), _indices->end(),
                            rhs._indices->begin(), rhs._indices->end(),
                            indexed_container::inserter(indices));
        result._indices->swap(indices);

        return result;
    }

    template<typename U, typename Other_Storage, typename Other_Traits>
    typename choose_container<Other_Traits>::type operator ^ 
    (const Indexed_Container<U, Index, Other_Storage, Other_Traits> & rhs)
    const
    {
        typedef typename choose_container<Other_Traits>::type Result_Type;
        BOOST_STATIC_ASSERT((!boost::is_same<incompatible_containers, 
                                             Result_Type>::value));
        bbp_assert(_storage == rhs._storage);
        Label name = "(" + _label + " XOR " + rhs._label + ")";
        Result_Type result(_storage, name);
        Index_Set indices;
        std::set_symmetric_difference
            (_indices->begin(), _indices->end(),
             rhs._indices->begin(), rhs._indices->end(),
             indexed_container::inserter(indices));
        result._indices->swap(indices);

        return result;
    }

    template<typename U, typename Other_Storage, typename Other_Traits>
    Indexed_Container & operator +=
    (const Indexed_Container<U, Index, Other_Storage, Other_Traits> & rhs)
    {
        BOOST_STATIC_ASSERT((boost::is_convertible<U, T>::value));
        BOOST_STATIC_ASSERT((boost::is_convertible<Other_Storage, 
                                                   Storage>::value));
        bbp_assert(_storage == rhs._storage);

        if (!_indices.unique())
            _indices.reset(new Index_Set(*_indices));
        _indices->insert(*rhs._indices);

        return * this;
    }
    /** \endcond */ // SHOW_IN_DEVELOPER_REFERENCE

protected:
    template <typename U, typename Other_Index,
              typename Other_Storage, typename Other_Adapter> 
    friend class Indexed_Container;

protected:
    Label _label;

    typedef detail::Index_Set<Index, Traits::ordered> Index_Set;
    boost::shared_ptr<Index_Set>     _indices;
    typename Traits::storage_adapter _storage;
};

//-----------------------------------------------------------------------------

template <typename T, typename Ix, typename S, typename t>
inline std::ostream & operator << (std::ostream & out,
                                   const Indexed_Container<T, Ix, S, t> & c);

#ifndef SWIG //----------------------------------------------------------------

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
template <typename T, typename Index, typename Storage, typename Traits>
struct Indexed_Container<T, Index, Storage, Traits>::incompatible_containers {};

//! Class that adds const qualifiers to a parametrization depending
template <typename T, typename Index, typename Storage, typename Traits>
template <typename Other_Traits>
struct Indexed_Container<T, Index, Storage, Traits>::choose_container
{
protected:
    typedef typename Other_Traits::stored_type U;
    typedef typename Other_Traits::storage_type Other_Storage;
    typedef typename Traits::storage_adapter Adapter;
    typedef typename Other_Traits::storage_adapter Other_Adapter;

    typedef typename Adapter::storage_type My_Real_Storage;
    typedef typename Other_Adapter::storage_type Other_Real_Storage;


public:
    typedef typename boost::mpl::if_<
    boost::is_same<Traits, Other_Traits>, 
    // Containers are of the same type
    Indexed_Container<T, Index, Storage, Traits>,

    // Checking if the other container has const qualifiers and compatible 
    // types with this.
    typename boost::mpl::if_c<
    boost::is_same<typename boost::add_const<T>::type, U>::value &&
    boost::is_convertible<My_Real_Storage, Other_Real_Storage>::value &&
    Traits::ordered == Other_Traits::ordered &&
    boost::is_same<typename Adapter::indexer_type,
                   typename Other_Adapter::indexer_type>::value,
    // The other container has const, the final type has const then.
    Indexed_Container<U, Index, Other_Storage, Other_Traits>,

    // Checking if this container has const qualifiers and is compatible with
    // the other.
    typename boost::mpl::if_c<
    boost::is_same<T, typename boost::add_const<U>::type>::value &&
    boost::is_convertible<Other_Real_Storage, My_Real_Storage>::value &&
    Traits::ordered == Other_Traits::ordered &&
    boost::is_same<typename Adapter::indexer_type,
                   typename Other_Adapter::indexer_type>::value,
    Indexed_Container<T, Index, Storage, Traits>,

    // Incompatible containers 
    incompatible_containers

    >::type>::type>::type type;
};
/** \endcond */ // SHOW_IN_DEVELOPER_REFERENCE

//-----------------------------------------------------------------------------

#ifdef _DOXYGEN_
namespace boost
{
    /** \cond */
    namespace mpl
    {
        template <typename T, typename U, typename V> class if_c;
    }
    /** \endcond */
}
#endif

#ifdef _DOXYGEN_
/** \cond SHOW_IN_USER_REFERENCE */
//! Mutable iterator to type T
/*!
  Iterator will be bidirectional if the container is ordered and forward only
  if it's unordered.
  See also the \ref iterator_tutorial_forward_iterator "iterators" reference.
*/
template <typename T, typename Index, 
          typename Storage, typename Traits>
class Indexed_Container<T, Index, Storage, Traits>::iterator
{
public:
    //! Compare two iterators for equality.
    bool operator ==(const iterator &) const;
    //! Compare two iterators for equality.
    bool operator ==(const const_iterator &) const;
    //! Advance to next element in the container, prefix notation.
    iterator & operator ++();
    //! Advance to next element in the container, postfix notation.
    iterator operator ++(int);
    //! Step back to previous element in the container, prefix notation.
    iterator & operator --();
    //! Step back to previous element in the container, postfix notation.
    iterator operator --(int);
    //! Dereference the current interval pointed by the iterator.
    Traits::storage_adapter::reference_type operator * () const;
    //! Dereference the current interval pointed by the iterator.
    Traits::storage_adapter::reference_type * operator -> () const;
};

//! Mutable reverse_iterator to type T
/*!
  The iterator will be available only if the container is ordered.
*/
template <typename T, typename Index, 
          typename Storage, typename Traits>
class Indexed_Container<T, Index, Storage, Traits>::reverse_iterator
{
public:
    //! Compare two iterators for equality.
    bool operator ==(const reverse_iterator &) const;
    //! Compare two iterators for equality.
    bool operator ==(const const_reverse_iterator &) const;
    //! Advance to previous element in the container, prefix notation.
    reverse_iterator & operator ++();
    //! Advance to previous element in the container, postfix notation.
    reverse_iterator operator ++(int);
    //! Step back to next element in the container, prefix notation.
    reverse_iterator & operator --();
    //! Step back to next element in the container, postfix notation.
    reverse_iterator operator --(int);
    //! Dereference the current interval pointed by the reverse_iterator.
    Traits::storage_adapter::reference_type operator * () const;
    //! Dereference the current interval pointed by the reverse_iterator.
    Traits::storage_adapter::reference_type * operator -> () const;
    //! Converts this iterator into a forward iterator
    iterator base() const;
};
/** \endcond */ // SHOW_IN_USER_REFERENCE
#endif

// Preventing doxygen to parse inheritance declarations because there is 
// something confusing it there.
#ifdef _DOXYGEN_
/** \cond SHOW_IN_DEVELOPER_REFERENCE */
//! Mutable iterator to type T
/*!
  Iterator will be bidirectional if the container is ordered and forward only
  if it's unordered.
  See also the \ref iterator_tutorial_forward_iterator "iterators" reference.
*/
template <typename T, typename Index, typename Storage, typename Traits>
class Indexed_Container<T, Index, Storage, Traits>::iterator
#else
/* This guard is needed because even if doxygen is supposed to not be
   parsing this, actually something nasty is making it to fall in an infinite
   loop if not guarded. */
/** \cond */
template <typename T, typename Index, typename Storage, typename Traits>
class Indexed_Container<T, Index, Storage, Traits>::iterator :
    public boost::iterator_facade<
        typename Indexed_Container::iterator,
        T,
        // Bidirection for ordered containers and forward for unordered.
        typename boost::mpl::if_c<Traits::ordered,
                                  std::bidirectional_iterator_tag,
                                  std::forward_iterator_tag>::type,
        typename Traits::storage_adapter::reference_type
    >
/** \endcond */
#endif
{
    friend class Indexed_Container;
    /** \cond */
    friend class boost::iterator_core_access;
    /** \endcond */
    friend class Indexed_Container::const_iterator;

public:
    iterator()
    {}

private:
    explicit iterator(
        typename Indexed_Container::Index_Set::const_iterator iterator,
        const typename Traits::storage_adapter & adapter) :
        _current(iterator),
        _storage_adapter(adapter)
    {}

public:
    ///*
    //    \bug Potential bug: This was needed to assign a Sections::iterator
    //    with sections.begin(). However this way the _storage_adapter is 
    //    not checked. (TT)
    //*/
    //iterator & operator = (const iterator & rhs)
    //{
    //    _current = rhs._current;
    //    return * this;
    //}
private:


    void increment()
    {
        ++_current;
    }

    // This method is not available for unordered containers
    void decrement()
    {
        --_current;
    }

    template <class Iterator_Type>
    bool equal(Iterator_Type const & other) const
    {
        return _current == other._current;
    }

    typename Traits::storage_adapter::reference_type dereference() const
    {
        return _storage_adapter.find(*_current);
    }

    typename Indexed_Container::Index_Set::const_iterator _current;
    typename Traits::storage_adapter                      _storage_adapter;
};

//! Mutable reverse iterator to type T
/*!
  The iterator will be available only if the container is ordered.
*/
template <typename T, typename Index, typename Storage, typename Traits>
class Indexed_Container<T, Index, Storage, Traits>::reverse_iterator :
    public boost::reverse_iterator<typename Indexed_Container::iterator>
{
public:
    reverse_iterator()
    {}

    reverse_iterator(const typename Indexed_Container::iterator & i) :
        boost::reverse_iterator<typename Indexed_Container::iterator>(i)
    {}
};

/** \endcond */ // SHOW_IN_DEVELOPER_REFERENCE

// ----------------------------------------------------------------------------

#ifdef _DOXYGEN_
/** \cond SHOW_IN_USER_REFERENCE */
//! Const iterator to type T
/*!
  Iterator will be bidirectional if the container is ordered and forward only
  if it's unordered.
  See also the \ref iterator_tutorial_forward_iterator "iterators" reference.
*/
template <typename T, typename Index, typename Storage, typename Traits>
class Indexed_Container<T, Index, Storage, Traits>::const_iterator
{
public:
    const_iterator(const iterator & other);
    //! Compare two iterators for equality.
    bool operator ==(const const_iterator &) const;
    //! Compare two iterators for equality.
    bool operator ==(const iterator &) const;
    //! Advance to next element in the container, prefix notation.
    const_iterator & operator ++();
    //! Advance to next element in the container, postfix notation.
    const_iterator operator ++(int);
    //! Step back to previous element in the container, prefix notation.
    const_iterator & operator --();
    //! Step back to previous element in the container, postfix notation.
    const_iterator operator --(int);
    //! Dereference the current object pointed by the iterator.
    Traits::storage_adapter::reference_type operator * () const;
    //! Dereference the current object pointed by the iterator.
    Traits::storage_adapter::reference_type * operator -> () const;
};

//! Const reverse_iterator to type T
/*!
  The iterator will be available only if the container is ordered.
*/
template <typename T, typename Index, 
          typename Storage, typename Traits>
class Indexed_Container<T, Index, Storage, Traits>::const_reverse_iterator
{
public:
    //! Compare two iterators for equality.
    bool operator ==(const reverse_iterator &) const;
    //! Compare two iterators for equality.
    bool operator ==(const const_reverse_iterator &) const;
    //! Advance to previous element in the container, prefix notation.
    const_reverse_iterator & operator ++();
    //! Advance to previous element in the container, postfix notation.
    const_reverse_iterator operator ++(int);
    //! Step back to next element in the container, prefix notation.
    const_reverse_iterator & operator --();
    //! Step back to next element in the container, postfix notation.
    const_reverse_iterator operator --(int)
    //! Dereference the current interval pointed by the const_reverse_iterator.
    Traits::storage_adapter::reference_type operator * () const;
    //! Dereference the current interval pointed by the const_reverse_iterator.
    Traits::storage_adapter::reference_type * operator -> () const;
    //! Converts this iterator into a forward iterator
    const_iterator base() const;
};
/** \endcond */ // SHOW_IN_USER_REFERENCE
#endif

// Preventing doxygen to parse inheritance declarations because there is 
// something confusing it there.
#ifdef _DOXYGEN_
/** \cond SHOW_IN_DEVELOPER_REFERENCE */
//! Const iterator to type T
/*!
  Iterator will be bidirectional if the container is ordered and forward only
  if it's unordered.
  See also the \ref iterator_tutorial_forward_iterator "iterators" reference.
*/
template <typename T, typename Index, typename Storage, typename Traits>
class Indexed_Container<T, Index, Storage, Traits>::const_iterator
#else
/** \cond */
template <typename T, typename Index, typename Storage, typename Traits>
class Indexed_Container<T, Index, Storage, Traits>::const_iterator :
    public boost::iterator_facade<
        typename Indexed_Container::const_iterator,
        typename boost::add_const<T>::type,
        // Bidirection for ordered containers and forward for unordered.
        typename boost::mpl::if_c<Traits::ordered,
                                  std::bidirectional_iterator_tag,
                                  std::forward_iterator_tag>::type,
        typename boost::add_const<
            typename Traits::storage_adapter::reference_type>::type
    >
/** \endcond */
#endif
{
    friend class Indexed_Container;
    /** \cond */
    friend class boost::iterator_core_access;
    /** \endcond */
    friend class Indexed_Container::iterator;

public:
    const_iterator()
    {}

    template <class Iterator_Type>
    const_iterator(const Iterator_Type & iterator) :
        _current(iterator._current),
        _storage_adapter(iterator._storage_adapter)
    {}

private:
    explicit const_iterator(
        typename Indexed_Container::Index_Set::const_iterator iterator,
        const typename Traits::storage_adapter & adapter) :
        _current(iterator),
        _storage_adapter(adapter)
    {}

public:
    ///*
    //    \bug Potential bug: This was needed to assign a Sections::iterator
    //    with sections.begin(). However this way the _storage_adapter is 
    //    not checked. (TT)
    //*/
    //const_iterator & operator = (const const_iterator & rhs)
    //{
    //    _current = rhs._current;
    //    return * this;
    //}

private:
    void increment()
    {
        ++_current;
    }

    // This method is not available for unordered containers
    void decrement()
    {
        --_current;
    }

    template <class Iterator_Type>
    bool equal(Iterator_Type const& other) const
    {
        return _current == other._current;
    }

    typename boost::add_const<
        typename Traits::storage_adapter::reference_type>::type
    dereference() const
    { 
        return _storage_adapter.find(*_current);
    }

    typename Indexed_Container::Index_Set::const_iterator _current;
    typename Traits::storage_adapter                _storage_adapter;
};

//! Const reverse iterator to type T
/*!
  The iterator will be available only if the container is ordered.
*/
template <typename T, typename Index, typename Storage, typename Traits>
class Indexed_Container<T, Index, Storage, Traits>::const_reverse_iterator :
    public boost::reverse_iterator<typename Indexed_Container::const_iterator>
{
public:
    const_reverse_iterator()
    {}

    const_reverse_iterator
    (const typename Indexed_Container::const_iterator & i) :
        boost::reverse_iterator<typename Indexed_Container::const_iterator>(i)
    {}
};

/** \endcond */ // SHOW_IN_DEVELOPER_REFERENCE

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
Indexed_Container<T, Index, Storage, Traits>::Indexed_Container
(Storage & storage, const Label & label) :
    _label(label),
    _indices(new Index_Set()),
    _storage(storage)
{}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
Indexed_Container<T, Index, Storage, Traits>::Indexed_Container
(const typename Traits::storage_adapter & adapter, const Label & label) :
    _label(label),
    _indices(new Index_Set()),
    _storage(adapter)
{}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
const Label & Indexed_Container<T, Index, Storage, Traits>::label() const
{
    return _label;
}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
void Indexed_Container<T, Index, Storage, Traits>::insert(const Index & index)
{
    bbp_assert(_storage.valid());
    if (!_indices.unique())
        _indices.reset(new Index_Set(*_indices));
    _indices->insert(index);
}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
void Indexed_Container<T, Index, Storage, Traits>::insert(T & object)
{
    bbp_assert(_storage.valid());
    if (!_indices.unique())
        _indices.reset(new Index_Set(*_indices));
    _indices->insert(typename Traits::storage_adapter::indexer_type()(object));
}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
void Indexed_Container<T, Index, Storage, Traits>::insert_range
(Index start, Index end)
{
    bbp_assert(_storage.valid());
    if (!_indices.unique())
        _indices.reset(new Index_Set(*_indices));
    _indices->insert_interval(start, end);
}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
void Indexed_Container<T, Index, Storage, Traits>::erase(Index index)
{
    if (!_indices.unique())
        _indices.reset(new Index_Set(*_indices));
    _indices->remove(index);
}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
void Indexed_Container<T, Index, Storage, Traits>::erase(T & object)
{
    if (!_indices.unique())
        _indices.reset(new Index_Set(*_indices));
    _indices->remove(typename Traits::storage_adapter::indexer_type()(object));
}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
void Indexed_Container<T, Index, Storage, Traits>::erase_range
(Index start, Index end)
{
    if (!_indices.unique())
        _indices.reset(new Index_Set(*_indices));
    _indices->remove_interval(start, end);
}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
void Indexed_Container<T, Index, Storage, Traits>::clear()
{
    _indices.reset(new Index_Set());
}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
size_t Indexed_Container<T, Index, Storage, Traits>::size() const
{
    return _indices->size();
}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
void Indexed_Container<T, Index, Storage, Traits>::swap
(Indexed_Container & other)
{
    _indices.swap(other._indices);
    std::swap(_storage, other._storage);
}

//-----------------------------------------------------------------------------

template <typename T, typename Index, typename Storage, typename Traits>
void Indexed_Container<T, Index, Storage, Traits>::print() const
{
    std::cout << *this << std::endl;
}


//-----------------------------------------------------------------------------

template <typename T, typename Ix, typename S, typename t>
std::ostream & operator << (std::ostream & lhs,
                            const Indexed_Container<T, Ix, S, t> & rhs)
{
    lhs << rhs.label() << " = { ";
    for (typename Indexed_Container<T, Ix, S, t>::const_iterator i = 
		rhs.begin(); i != rhs.end(); ++i)
    {
        lhs << *i << ' ';
    }
    lhs << "}";
    return lhs;
}

#endif // ----------------------------------------------------------------------

}
#endif
