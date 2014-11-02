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

#ifndef BBP_STORAGE_ADAPTER_SPECIALIZATIONS_H
#define BBP_STORAGE_ADAPTER_SPECIALIZATIONS_H

#include <boost/shared_ptr.hpp>
#include "BBP/Common/Dataset/Containers/Indexed_Container.h"

namespace bbp
{
namespace indexed_container
{

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
template <
    typename T, 
    typename Index,
    typename Storage,
    class Impl,
    class Indexer
>
class Storage_Adapter;

/** \cond SHOW_IN_DEVELOPER_REFERENCE */

// Specialization for void implementation class.
// It only changes the implementation of the find function.
template <
    typename T, 
    typename Index,
    typename Storage,
    class Indexer
>
class Storage_Adapter<T, Index, Storage, void, Indexer>
{
public:
    typedef T       return_type;
    typedef T       reference_type;
    typedef Index   index_type;
    typedef Storage storage_type;
    typedef Indexer indexer_type;

    Storage_Adapter(const Storage & storage) : 
        _storage(storage) {}

    template <typename U, typename Other_Storage>
    Storage_Adapter(const Storage_Adapter<U, Index, Other_Storage, void,
                                          Indexer> & compatible);

    template <typename U, typename Other_Storage>
    bool operator ==(const Storage_Adapter<U, Index, Other_Storage, void,
                                           Indexer> & compatible) const;
    
    T find(Index i) const
    {
        return _storage.find(i);
    }

    Index index_of
    (const typename remove_const_and_reference<T>::type & object) const
    {
        return Indexer()(object);
    }

    bool valid() const
    {
        return true;
    }

protected:
    template <typename U, typename I, typename St, typename Imp, typename Ix>
    friend class Storage_Adapter;

    Storage _storage;
};

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
template <
    typename T, 
    typename Index,
    typename Storage,
    class Impl,
    class Indexer
>
class Storage_Adapter;

// Specialization for simple pointer storage class.
template <
    typename T, 
    typename Index,
    typename Storage,
    class Impl,
    class Indexer
>
class Storage_Adapter<T, Index, Storage *, Impl, Indexer>
{
public:
    typedef T         return_type;
    typedef T         reference_type;
    typedef Index     index_type;
    typedef Storage * storage_type;
    typedef Indexer   indexer_type;

    Storage_Adapter(Storage & storage) : 
        _storage(&storage) {}

    template <typename U, typename Other_Storage, typename Other_Impl>
    Storage_Adapter(const Storage_Adapter<U, Index, Other_Storage *, 
                                          Other_Impl, Indexer> & compatible) :
        _storage(compatible._storage)
    {
        BOOST_STATIC_ASSERT((boost::is_convertible<U, T>::value &&
                             (boost::is_const<T>::value ||
                              !boost::is_const<U>::value)));
    }


    template <typename U, typename Other_Storage, typename Other_Impl>
    bool operator ==(const Storage_Adapter<U, Index, Other_Storage *, 
                                            Other_Impl, Indexer> & compatible) 
        const
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
        return _storage != 0;
    }

protected:
    template <typename U, typename I, typename St, typename Imp, typename Ix>
    friend class Storage_Adapter;

    Storage * _storage;
};

// Specialization for simple pointer storage class with void implementation
// class.
template <
    typename T, 
    typename Index,
    typename Storage,
    class Indexer
>
class Storage_Adapter<T, Index, Storage *, void, Indexer>
{
public:
    typedef T         return_type;
    typedef T         reference_type;
    typedef Index     index_type;
    typedef Storage * storage_type;
    typedef Indexer   indexer_type;

    Storage_Adapter(Storage & storage) : 
        _storage(&storage) {}

    template <typename U, typename Other_Storage>
    Storage_Adapter(const Storage_Adapter<U, Index, Other_Storage *, void,
                                          Indexer> & compatible) :
        _storage(compatible._storage)
    {
        BOOST_STATIC_ASSERT((boost::is_convertible<U, T>::value &&
                             (boost::is_const<T>::value ||
                              !boost::is_const<U>::value)));
    }

    template <typename U, typename Other_Storage>
    bool operator ==(const Storage_Adapter<U, Index, Other_Storage *, void,
                                           Indexer> & compatible) const
    {
        return _storage == compatible._storage;
    }

    T find(Index i) const
    {
        return _storage->find(i);
    }

    Index index_of
    (const typename remove_const_and_reference<T>::type & object) const
    {
        return Indexer()(object);
    }

    bool valid() const
    {
        return _storage != 0;
    }

protected:
    template <typename U , typename I, typename St, typename Imp, typename Ix>
    friend class Storage_Adapter;

    Storage * _storage;
};

// Specialization for boost::shared_ptr storage and non void 
// implementation class.
template <
    typename T, 
    typename Index,
    typename Storage,
    class Impl,
    class Indexer
>
class Storage_Adapter<T, Index, boost::shared_ptr<Storage>, Impl, Indexer>
{
public:
    typedef T                          return_type;
    typedef T                          reference_type;
    typedef Index                      index_type;
    typedef boost::shared_ptr<Storage> storage_type;
    typedef Indexer                    indexer_type;

    Storage_Adapter(const boost::shared_ptr<Storage> & storage) : 
        _storage(storage) {}

    template <typename U, typename Other_Storage, typename Other_Impl>
    Storage_Adapter(const Storage_Adapter<U, Index, Other_Storage, Other_Impl,
                                          Indexer> & compatible);

    template <typename U, typename Other_Storage, typename Other_Impl>
    bool operator ==(const Storage_Adapter<U, Index, Other_Storage, Other_Impl,
                                           Indexer> & compatible) const;
    
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
        return _storage.get() != 0;
    }

protected:
    template <typename U, typename I, typename St, typename Imp, typename Ix>
    friend class Storage_Adapter;

    boost::shared_ptr<Storage> _storage;
};

// Specialization for boost::shared_ptr storage and void implementation class
template <
    typename T, 
    typename Index,
    typename Storage,
    class Indexer
>
class Storage_Adapter<T, Index, boost::shared_ptr<Storage>, void, Indexer>
{
public:
    typedef T                          return_type;
    typedef T                          reference_type;
    typedef Index                      index_type;
    typedef boost::shared_ptr<Storage> storage_type;
    typedef Indexer                    indexer_type;

    Storage_Adapter(const boost::shared_ptr<Storage> & storage) : 
        _storage(&storage) {}

    template <typename U, typename Other_Storage>
    Storage_Adapter(const Storage_Adapter<U, Index, Other_Storage, void,
                                          Indexer> & compatible);

    template <typename U, typename Other_Storage>
    bool operator ==(const Storage_Adapter<U, Index, Other_Storage, void,
                                           Indexer> & compatible) const;
    
    T find(Index i) const
    {
        return _storage->find(i);
    }

    Index index_of
    (const typename remove_const_and_reference<T>::type & object) const
    {
        return Indexer()(object);
    }

    bool valid() const
    {
        return _storage.get() != 0;
    }

protected:
    template <typename U, typename I, typename St, typename Imp, typename Ix>
    friend class Storage_Adapter;

    boost::shared_ptr<Storage> _storage;
};

// Specialization for boost::weak_ptr storage and non void 
// implementation class.
template <
    typename T, 
    typename Index,
    typename Storage,
    class Impl,
    class Indexer
>
class Storage_Adapter<T, Index, boost::weak_ptr<Storage>, Impl, Indexer>
{
public:
    typedef T                        return_type;
    typedef T                        reference_type;
    typedef Index                    index_type;
    typedef boost::weak_ptr<Storage> storage_type;
    typedef Indexer                  indexer_type;

    Storage_Adapter(const boost::weak_ptr<Storage> & storage) : 
        _storage(storage) {}

    template <typename U, typename Other_Storage, typename Other_Impl>
    Storage_Adapter(const Storage_Adapter<U, Index, Other_Storage, Other_Impl,
                                          Indexer> & compatible);

    template <typename U, typename Other_Storage, typename Other_Impl>
    bool operator ==(const Storage_Adapter<U, Index, Other_Storage, Other_Impl,
                                           Indexer> & compatible) const;
    
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
        return _storage.lock().get() != 0;
    }

protected:
    template <typename U, typename I, typename St, typename Imp, typename Ix>
    friend class Storage_Adapter;

    boost::weak_ptr<Storage> _storage;
};

// Specialization for boost::weak_ptr storage and void implementation class
template <
    typename T, 
    typename Index,
    typename Storage,
    class Indexer
>
class Storage_Adapter<T, Index, boost::weak_ptr<Storage>, void, Indexer>
{
public:
    typedef T                        return_type;
    typedef T                        reference_type;
    typedef Index                    index_type;
    typedef boost::weak_ptr<Storage> storage_type;
    typedef Indexer                  indexer_type;

    Storage_Adapter(const boost::weak_ptr<Storage> & storage) : 
        _storage(&storage) {}

    template <typename U, typename Other_Storage>
    Storage_Adapter(const Storage_Adapter<U, Index, Other_Storage, void,
                                          Indexer> & compatible);

    template <typename U, typename Other_Storage>
    bool operator ==(const Storage_Adapter<U, Index, Other_Storage, void,
                                           Indexer> & compatible) const;
    
    T find(Index i) const
    {
        bbp_assert(valid());
        return _storage.lock()->find(i);
    }

    Index index_of
    (const typename remove_const_and_reference<T>::type & object) const
    {
        return Indexer()(object);
    }

    bool valid() const
    {
        return _storage.lock().get() != 0;
    }

protected:
    template <typename U, typename I, typename St, typename Imp, typename Ix>
    friend class Storage_Adapter;

    boost::weak_ptr<Storage> _storage;
};

/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

}
}
#endif
