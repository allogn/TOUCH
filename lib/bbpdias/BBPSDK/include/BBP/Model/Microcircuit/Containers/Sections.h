/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:	Juan Hernando Vieites
                                Thomas Trankler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SECTIONS_H
#define BBP_SECTIONS_H

#include <boost/tuple/tuple.hpp>

#include <vector>

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Common/Dataset/Containers/Indexed_Container.h"
#include "BBP/Common/Math/Geometry/Vector_3D.h"
#include "BBP/Common/Math/Geometry/Rotation.h"

namespace bbp {

class Microcircuit;
class Morphology;
class Neuron;
class Section;

#ifndef SWIG

//-----------------------------------------------------------------------------

/** \cond SHOW_IN_DEVELOPER_REFERENCE */
typedef boost::tuple<const Morphology *, const Neuron *, const Microcircuit *> 
    Section_Storage;

class _Section_Storage_Adapter;
typedef indexed_container::Storage_Adapter<
    Section, Section_ID, Section_Storage, _Section_Storage_Adapter
> _Base_Section_Storage_Adapter;

//-----------------------------------------------------------------------------

class _Section_Storage_Adapter :
    public _Base_Section_Storage_Adapter
{
    friend class indexed_container::Storage_Adapter<
        Section, Section_ID, Section_Storage, _Section_Storage_Adapter>;
    friend class _Const_Section_Storage_Adapter;

public:
    inline _Section_Storage_Adapter(const Section_Storage & storage = 
                                    Section_Storage());

    bool operator == (const _Section_Storage_Adapter & other) const
    {
        return (_storage.get<0>() == other._storage.get<0>() &&
                _storage.get<1>() == other._storage.get<1>() &&
                _storage.get<2>() == other._storage.get<2>());
    }

    inline void check_section_id(Section_ID section) const;

    const Morphology & morphology() const
    {
        return *_storage.get<0>();
    }

protected:
    inline Section find_impl(Section_ID section) const;

protected:
    //! Stores the rotation matrix of the neuron to speed up computations
    Rotation_3D<Micron> _neuron_rotation;
};

/** \endcond SHOW_IN_DEVELOPER_REFERENCE */
#endif // SWIG ----------------------------------------------------------------

/** \if SHOW_IN_USER_REFERENCE */
// Simplified version to show up in user reference
#ifdef _DOXYGEN_
typedef Indexed_Container<
    Section, Section_ID, impl detail, impl detail
> _Section_Container;
#endif
/** \else */
typedef Indexed_Container<
    Section, Section_ID, Section_Storage,
    indexed_container::traits<Section, Section_ID, Section_Storage, false,
                              _Section_Storage_Adapter, true>
> _Section_Container;
/** \endif */

#ifdef SWIG
// Sorry for polluting this header with SWIG stuff but a template
// instantiation must be done before _Compartment_Container can be inherited.
// Otherwise it won't be wrapped.
%template(_Section_Container) Indexed_Container<
    Section, Section_ID, Section_Storage,
    indexed_container::traits<Section, Section_ID, Section_Storage, false,
                              _Section_Storage_Adapter, true>
>;
#endif

//-----------------------------------------------------------------------------

//! A set of sections from a single morphology of the microcircuit.
/*!
    This container stores section from any neuron or morphology 
    of the microcircuit.
    If a neuron is given in the constructor sections coordinates will be 
    reported in the global coordinate frame, otherwise theyt will be local
    to the morphology.

	Can be all sections or a certain type of section
    (e.g. only dendrites, axon, or apical dendrites). (Be aware a soma is not 
    a real section in the sense that its points do not form a tube. Use the
    soma class to access the soma section instead.)

    This container inherits from Indexed_Container, where the Index template
    parameter is Section_ID and the type returned is Section.

    Warning: Don't use any iterator obtained from this container if the 
    container has been deallocated. If a function returns this container
    by value that means it's a temporary object, so the following code is
    wrong and may fail unexpectedly:
    \code
    Sections::const_iterator i = some_function_returning_sections().begin();
    i->print();
    \endcode
    \todo The fact that using begin() on a temporary container is wrong
    should be checked in debug mode to avoid pitfalls. (TT)
	\sa Sections, Section, Neuron, Compartment
    \ingroup Microcircuit_Containers
*/
class Sections 
    : public _Section_Container
{
public:

    typedef _Section_Container::iterator iterator;
    typedef _Section_Container::const_iterator const_iterator;


    //! Create e new container.
    /*! Section coordinates will be in the global circuit coordinate frame. */
    Sections(const Morphology   * morphology,
             const Neuron       * neuron,
             const Microcircuit * microcircuit) 
    :
        _Section_Container(Section_Storage(morphology, neuron, microcircuit))
    {}

    //! Create e new container.
    /*! Section coordinates will be local to the morphology. */
    Sections(const Morphology * morphology) 
    :
        _Section_Container(Section_Storage(morphology, 0, 0))
    {}

    Sections(const _Section_Container & other) 
    : 
        _Section_Container(other) 
    {}

    /*! Returns an iterator to the parent of the given section.
        The iterator equals end() if the given id is out of bounds,
        the section doesn't have any parent or the parent is outside this
        section set. */
    inline const_iterator parent
        (Section_ID section_id) const;

    inline const Sections children(Section_ID section_id) const;

#ifndef SWIG // To suppres spurious warning
    // Solving partial overriding issues.
    using _Section_Container::insert;
#endif

    inline std::vector<Section> as_vector();

    // Hiding the following functions to the user
private:
    void insert(Section & section);
};

// Specilization declarations

template<>
inline void _Section_Container::insert(const Section_ID & section);

template<>
inline void _Section_Container::insert_range(Section_ID start, Section_ID end);

template <>
inline std::ostream & operator << (std::ostream & lhs, 
                                   const _Section_Container & rhs);

//-----------------------------------------------------------------------------

}
#include "BBP/Model/Microcircuit/Section.h"
#include "BBP/Model/Microcircuit/Neuron.h"

namespace bbp
{

//-----------------------------------------------------------------------------

_Section_Storage_Adapter::_Section_Storage_Adapter
(const Section_Storage & storage) :
    _Base_Section_Storage_Adapter(storage),
    _neuron_rotation(storage.get<1>() == 0 ? // Any neuron given?
                     Rotation_3D<Micron>() :
                     Rotation_3D<Micron>(storage.get<1>()->orientation()))
{}

//-----------------------------------------------------------------------------

    void _Section_Storage_Adapter::check_section_id(Section_ID section __attribute__((__unused__))) const
{
#ifndef NDEBUG
    if (_storage.get<0>()->number_of_sections() <= section)
    {
        std::runtime_error exc("Section id " +
                               boost::lexical_cast<std::string>(section) + 
                               " out of range for morphology " +
                               _storage.get<0>()->label());
        throw_exception(exc, FATAL_LEVEL);
    }
#endif
}

//-----------------------------------------------------------------------------

Section _Section_Storage_Adapter::find_impl(Section_ID section) const
{
    if (_storage.get<1>() != 0)
    {
        // Section from a neuron, circuit coordinates will be used.
        return Section(_storage.get<0>(), _storage.get<1>(), _storage.get<2>(),
                       _neuron_rotation, section);
    } 
    else
    {
        // Section from a morphology, local coordinates will be used.
        return Section(_storage.get<0>(), section);
    }
}

//-----------------------------------------------------------------------------

template<>
inline void _Section_Container::insert(const Section_ID & section)
{
    bbp_assert(_storage.valid());
    _storage.check_section_id(section);
    if (!_indices.unique())
        _indices.reset(new Index_Set(*_indices));
    _indices->insert(section);
}

//-----------------------------------------------------------------------------

template<>
inline void _Section_Container::insert_range(Section_ID start, Section_ID end)
{
    bbp_assert(_storage.valid());
    // Only need to check the start and end positions since all the other
    // must be on the range.
    _storage.check_section_id(start);
    _storage.check_section_id(end);
    if (!_indices.unique())
        _indices.reset(new Index_Set(*_indices));
    _indices->insert_interval(start, end);
}

//-----------------------------------------------------------------------------

template <>
std::ostream & operator << (std::ostream & lhs, const _Section_Container & rhs)
{
    lhs << rhs.label() << " = { ";
    for (_Section_Container::const_iterator i = rhs.begin(); 
         i != rhs.end();
         ++i)
    {
        lhs << i->segments() << ' ';
    }
    lhs << "}";
    return lhs;
}

//-----------------------------------------------------------------------------

Sections::const_iterator Sections::parent(Section_ID section_id) const
{
    bbp_assert(find(section_id) != end());
    return find(_storage.morphology().parent_section(section_id));
}


//-----------------------------------------------------------------------------

const Sections Sections::children(Section_ID section_id) const
{
    bbp_assert(find(section_id) != end());
    typedef std::vector<Section_ID> Section_ID_List;
    const Section_ID_List & children =
        _storage.morphology().children_sections(section_id);

    // We copy the initial result from this class to get the same neuron
    // and microcircuit references.
    Sections result(*this);
    result.clear();

    for (Section_ID_List::const_iterator i = children.begin();
         i != children.end(); 
         ++i)
    {
        if (find(*i) != end())
            result.insert(*i);
    }

    return result;
}

//-----------------------------------------------------------------------------

inline std::vector<Section> Sections::as_vector()
{
    std::vector<Section> ret;
    for(Sections::iterator i = begin(); i != end(); i++)
    {
        ret.push_back(*i);
    }
    return ret;
}


//-----------------------------------------------------------------------------

}
#endif
