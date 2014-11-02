/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

		Responsible author:		Thomas Traenkler
		Contributing authors:	Nikolai Chapochnikov
								Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SOMA_H
#define BBP_SOMA_H

#include "Types.h"
#include "BBP/Model/Microcircuit/Section.h"

namespace bbp {

class Morphology;
class Neuron;

namespace rw 
{
    class Soma;
}

// ----------------------------------------------------------------------------

//! Interface to the cell body of a neuron.
/*!
	The Soma class provides access to the soma part of a neuron.
	It provides access to the soma's center position (in global
	coordinates if referenced from a neuron) and the positions of
	the surface of the cell membrane defining points.
    \todo Should inherit from Section, but clarify first how the soma
    section is different from other sections (e.g. segments, diameters). (TT)
	\sa Neuron, Morphology
    \ingroup Microcircuit
*/
class Soma
    : public Section
{
    friend class Neuron;
    friend class Morphology;
    friend class bbp::rw::Soma;


public: 
    //! \todo Temporary default constructor to solve SWIG compilation issues (SL)
    Soma() {}

protected:
    //! Constructor for an isolated morphology.
	inline Soma(const Morphology & morphology);
	//! Constructor for soma of a neuron inside a microcircuit.
	inline Soma(const Neuron     & neuron);

public:
	//! Print soma status information.
	inline void print() const;

    /*!
        \name Structure
        For access of soma structure.
    */
    //@{
	//! Get position of the soma center (read-only).
	inline const Vector_3D<Micron> & position() const;
    //! Returns the mean distance of the soma points to the soma center
    inline Micron mean_radius() const;
    //! Returns the maximum distance of the soma points to the soma center
    inline Micron max_radius() const;

	//! Forward iterator to visit all elements (read only).
	class const_iterator;
	//! Forward iterator to visit all elements (read only).
	typedef const_iterator iterator;

	//! Get iterator to first of soma surface points.
	inline const_iterator begin(void) const;
	//! Get iterator that indicates the end of the container has been reached.
	inline const_iterator end(void) const;
	//! Get the number of points in the container.
	inline size_t size() const;
	//@}

protected:
    const Morphology * morphology_ptr() const
    {
        return _morphology;
    }

private:
	//! Center of the soma volume.
	Vector_3D<Micron>			_center;
    //! Pointer to vector of soma points.
	Morphology_Point_ID	        _end_point;
};

// ----------------------------------------------------------------------------

inline std::ostream & operator << (std::ostream & out, const Soma & soma);

}

#ifndef SWIG

#include "Morphology.h"
#include "Neuron.h"

namespace bbp
{

// ----------------------------------------------------------------------------

void Soma::print() const
{
    std::cout << *this << std::endl;
}

// ----------------------------------------------------------------------------

class Soma::const_iterator :
    public std::iterator<std::forward_iterator_tag, const Vector_3D<Micron> >
{
    friend class Soma;

private:
    explicit const_iterator(const Soma *soma, Morphology_Point_ID current = 0)
        : _soma(soma), _current(current)
    {}

public:
    const_iterator() {}

    //! Compare two iterators for equality.
    /*!
        This only works for iterators of the same container.
    */
    bool operator == (const const_iterator & other) const
    {
         return _soma == other._soma && _current == other._current;
    }

    //! Compare two iterators for inequality.
    /*!
        This only works for iterators of the same container.
    */
    bool operator != (const const_iterator & other) const
    {
        return !(_soma == other._soma && _current == other._current);
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
        const_iterator temp(* this);
        ++_current;
        return temp;
    }

    //! Get (dereference) current element (READ WARNING BEFORE USE!).
    /*!
        WARNING: Referenced elements are only created on the fly and are
        invalidated when the next element gets dereferenced!
    */
    const Vector_3D<Micron> & operator * () const
    {
        _global_point_position = 
            _soma->morphology_ptr()->point_positions()[_current] +
            _soma->_center;
        return _global_point_position;
    }

    //! Get pointer to current element (READ WARNING BEFORE USE!).
    /*!
        WARNING: Referenced elements are only created on the fly and are
        invalidated when the next element gets dereferenced!
    */
    const Vector_3D<Micron> * operator -> () const
    {
        return & (this->operator *());
    }

private:
    mutable Vector_3D<Micron>   _global_point_position;
    const Soma *                _soma;
    Morphology_Point_ID         _current;

};

// ----------------------------------------------------------------------------

/*!
    \bug Potential bug: Check if use of lock() is correct. (TT)
*/
Soma::Soma(const Neuron & neuron) 
:
    Section(& neuron.morphology(), & neuron, 
            neuron._microcircuit.lock().get(), 
            Rotation_3D<Micron>(neuron.orientation()), 0),
    _center(neuron.position()),
	_end_point(neuron.morphology().section_point_counts()[0])
{
}

// ----------------------------------------------------------------------------

Soma::Soma(const Morphology & morphology)
:
    Section(& morphology, 0),
    _center(0.0f, 0.0f, 0.0f),
	_end_point(morphology.section_point_counts()[0])
{
    Vector_3D<Micron> center;
    center.reset();
    for (const_iterator i = begin(); i != end(); ++i)
        center += *i;
    size_t count = _end_point;
    _center = center / (Micron) count;
#ifndef NDEBUG
    if (center.x() > 0.05 ||
        center.y() > 0.05 || 
        center.z() > 0.05)
    {
        std::stringstream message;
        message << "Soma for morphology " << morphology.label() << " found "
                << "to be centered at " << (center / (Micron) count);
        log_message(message.str(), SEVERE_LEVEL);
    }
#endif
}

// ----------------------------------------------------------------------------

const Vector_3D<Micron> & Soma::position() const
{
	return _center;
}

// ----------------------------------------------------------------------------

size_t Soma::size() const
{
	return (_end_point);
}

// ----------------------------------------------------------------------------

Micron Soma::mean_radius() const
{
    Micron radius = 0;
    for (const_iterator i = begin(); i != end(); ++i)
        radius += (*i - _center).length();
    return radius / size();
}

// ----------------------------------------------------------------------------

Micron Soma::max_radius() const
{
    Micron max_radius = 0;
    for (const_iterator i = begin(); i != end(); ++i)
    {
        Micron radius = (*i - _center).length();
        if (radius > max_radius)
            max_radius = radius;
        radius += i->length();
    }
    return max_radius;
}

// ----------------------------------------------------------------------------

Soma::const_iterator Soma::begin(void) const
{
    return const_iterator(this);
}

// ----------------------------------------------------------------------------

Soma::const_iterator Soma::end(void) const
{
    return const_iterator(this, _end_point);
}

// ----------------------------------------------------------------------------

std::ostream & operator << (std::ostream & out, const Soma & soma)
{
    out << "Soma: center " << soma.position() << ", radius " 
        << soma.mean_radius() << std::endl;

    return out;
}

}

#endif // SWIG
#endif
