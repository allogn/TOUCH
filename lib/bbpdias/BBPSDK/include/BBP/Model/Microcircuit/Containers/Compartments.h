/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Authors: Juan Hernando,
                 Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENTS_H
#define BBP_COMPARTMENTS_H

#include "BBP/Common/Dataset/Containers/Indexed_Container.h"
#include "BBP/Model/Microcircuit/Compartment.h"

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
namespace bbp
{

/**
   This is the class used as internal storage for the Compartments container.
*/
struct Compartment_Index
{
    Compartment_Index() :
        cell_index(UNDEFINED_CELL_INDEX),
        offset(UNDEFINED_REPORT_FRAME_INDEX)
    {}

    Compartment_Index(Report_Frame_Index offset,
                      Cell_Index index) :
        cell_index(index),
        offset(offset)
    {}

    Compartment_Index & operator ++ ()
    {
        ++offset;
        return *this;
    }

    Compartment_Index operator ++ (int) const
    {
        Compartment_Index index;
        index.cell_index = cell_index;
        index.offset = offset + 1;
        return index;
    }

    Compartment_Index & operator -- ()
    {
        --offset;
        return *this;        
    }

    Compartment_Index operator -- (int) const
    {
        Compartment_Index index;
        index.cell_index = cell_index;
        index.offset = offset - 1;
        return index;
    }
    
    Compartment_Index operator + (int increment) const
    {
        Compartment_Index index;
        index.cell_index = cell_index;
        index.offset = offset + increment;
        return index;
    }

    Compartment_Index operator - (int decrement) const
    {
        Compartment_Index index;
        index.cell_index = cell_index;
        index.offset = offset - decrement;
        return index;
    }

    std::ptrdiff_t operator - (Compartment_Index index) const
    {
        if (cell_index == index.cell_index)
            return static_cast<std::ptrdiff_t>(offset - index.offset);
        else
            return std::numeric_limits<std::ptrdiff_t>::max();
    }

    bool operator > (const Compartment_Index & index) const
    {
        if (cell_index == index.cell_index)
            return offset > index.offset;
        else
            return cell_index > index.cell_index;        
    }

    bool operator < (const Compartment_Index & index) const
    {
        if (cell_index == index.cell_index)
            return offset < index.offset;
        else
            return cell_index < index.cell_index;
    }

    bool operator <= (const Compartment_Index & index) const
    {        
        return *this < index || *this == index;

    }

    bool operator >= (const Compartment_Index & index) const
    {        
        return *this > index || *this == index;

    }

    bool operator == (const Compartment_Index & index) const
    {
        return cell_index == index.cell_index && offset == index.offset;
    }

    Cell_Index         cell_index;
    Report_Frame_Index offset;

};    

// ----------------------------------------------------------------------------

class Compartment_Indexer
{
public:
    Compartment_Index operator()(const Compartment & compartment) const
    {
        return Compartment_Index(compartment._report_frame_index,
                                 compartment._cell_index);
    }
};

}

// ----------------------------------------------------------------------------

namespace boost
{
template <typename Target, typename Source>
Target lexical_cast(const Source &);

template <>
inline std::string lexical_cast<std::string, bbp::Compartment_Index>
(const bbp::Compartment_Index & index)
{
    std::stringstream result;
    result << index.offset;
    return result.str();
}

template <>
inline std::size_t hash<bbp::Compartment_Index>::operator()
(bbp::Compartment_Index const & index) const
{
    // Cast to longs avoid an ambiguous overload in 32-bit machines.
    return hash_value((long) index.offset);
}

}
/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

namespace bbp {

#ifndef SWIG // ---------------------------------------------------------------

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
class _Compartment_Storage_Adapter;
typedef indexed_container::Storage_Adapter<
    Compartment, Compartment_Index, const Microcircuit *, 
    _Compartment_Storage_Adapter, Compartment_Indexer
> _Base_Compartment_Storage_Adapter;

//-----------------------------------------------------------------------------

class _Compartment_Storage_Adapter :
    public _Base_Compartment_Storage_Adapter
{
    friend class indexed_container::Storage_Adapter<
        Compartment, Compartment_Index, const Microcircuit *, 
        _Compartment_Storage_Adapter, Compartment_Indexer>;
    friend class _Const_Section_Storage_Adapter;

public:
    _Compartment_Storage_Adapter(const Microcircuit & microcircuit) :
        _Base_Compartment_Storage_Adapter(microcircuit)
    {}

    const Microcircuit & microcircuit() const
    {
        return *_storage;
    }

    bool operator == (const _Compartment_Storage_Adapter & other) const
    {
        return _storage == other._storage;
    }

protected:
    inline Compartment find_impl(const Compartment_Index & section) const;
};

#endif // SWIG -----------------------------------------------------------------
/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

/** \if SHOW_IN_USER_REFERENCE */
// Simplified version to show up in user reference
#ifdef _DOXYGEN_
typedef Indexed_Container<
    Compartment, Compartment_Index, impl detail, impl detail
> _Compartment_Container;
#endif
/** \else */
typedef Indexed_Container<
    Compartment, Compartment_Index, const Microcircuit,
    indexed_container::traits<Compartment, Compartment_Index,
                              const Microcircuit, false,
                              _Compartment_Storage_Adapter, true>
> _Compartment_Container;
/** \endif */


#ifdef SWIG
// Sorry for polluting this header with SWIG stuff but a template
// instantiation must be done before _Compartment_Container can be inherited.
// Otherwise it won't be wrapped.
%template(_Compartment_Container) Indexed_Container<
    Compartment, Compartment_Index, const Microcircuit,
    indexed_container::traits<Compartment, Compartment_Index,
                              const Microcircuit, false,
                              _Compartment_Storage_Adapter, true>
>;
#endif

//-----------------------------------------------------------------------------

//! A container for neuron volume compartments.
/*!
    This container stores global compartment identifiers to refer to 
    compartments from a Microcircuit. The Compartment objects returned by
    functions inherited from Indexed_Container are temporary objects that
    store the references to the data they represent (i.e. voltage buffers).
    The Compartment_Index class used to index the compartments internally is
    an implementation detail omitted from the user reference on purpose.
    \ingroup Microcircuit_Containers
*/
class Compartments : public _Compartment_Container
{
public:
    explicit Compartments(const Microcircuit & microcircuit,
                          const Label & label = "") :
        _Compartment_Container(_Compartment_Storage_Adapter(microcircuit),
                               label)
    {}

    Compartments(const _Compartment_Container & other) : 
        _Compartment_Container(other) 
    {}    

public:
#ifndef SWIG // To suppres spurious warning
    // Solving partial overriding issues.
    using _Compartment_Container::insert;
#endif

    //! Inserts a compartment inside the container
    /*! The Cell_GID, Section_ID and compartment ID inside the
        compartment GID must point to a valid compartment, otherwise the 
        result is undefined. */
    void insert(const Compartment_GID & gid);
};

//-----------------------------------------------------------------------------

}
#include "../../../Model/Microcircuit/Microcircuit.h"
#include "../../../Model/Experiment/Compartment_Report_Frame.h"

namespace bbp
{

//-----------------------------------------------------------------------------

Compartment _Compartment_Storage_Adapter::find_impl
(const Compartment_Index & compartment) const
{
    return Compartment(_storage, compartment.offset, compartment.cell_index);
}

//-----------------------------------------------------------------------------

}
#endif
