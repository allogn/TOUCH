/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SEGMENTS_H
#define BBP_SEGMENTS_H

#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Common/Math/Geometry/Rotation.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Morphology;
class Segment;

// ----------------------------------------------------------------------------

//! A set of segments of the same neuron.
/*!
    This container currently stores only segments of the same neuron.

    Warning: Don't use any iterator obtained from this container if the 
    container has been deallocated. For example Section::segments() returns
    a temporary object, which means that the following code is wrong and
    may fail unexpectedly:
    \code
    Segments::const_iterator i = section.segments().begin;
    i->print();
    \endcode

    \sa Segment
    \ingroup Microcircuit_Containers
*/
class Segments
{
    friend class Section;

public:
    //! Forward const iterator for the Segments container.
    class const_iterator;
    
    //! Backward const iterator for the Segments container.
    class const_reverse_iterator;

    Segments() {}
protected:
    /*!
      \param morphology
      \param soma_position
      \param neuron_rotation
      \param begin_point The first morphological point of the section.
      \param end_point The last morphological point of the section.
    */
    inline Segments(const Morphology * morphology, 
                    const Vector_3D<Micron> & soma_position,
                    const Rotation_3D<Micron> & neuron_rotation, 
                    Morphology_Point_ID begin_point, 
                    Morphology_Point_ID end_point);
    inline Segments(const Morphology * morphology, 
                    Morphology_Point_ID begin_point, 
                    Morphology_Point_ID end_point);
    
public:
    
    //! Retrieve number of segments referenced in the container.
    inline size_t size () const;

    //! Print global identifiers of neurons in this container
    inline void print() const;

    //! Returns the first element of the container for linear access
    inline const_iterator begin() const;

    //! indicates the end of the container has been reached
    inline const_iterator end() const;

    //! Returns the last element of the container for backwards iteration
    inline const_reverse_iterator rbegin() const;

    //! indicates the end of the container reverse iteration has been reached
    inline const_reverse_iterator rend() const;

    //! searches for a specific segment inside the neuron
	/*!
		@return iterator pointing to the segment in the container, 
		equals to end if not found
	*/
    inline const_iterator find(Segment_ID segment) const;

    //! Compares to containers for equality
    inline bool operator == (const Segments & other) const;

    /** \cond */
    // /*! returns the soma relative position of a section point
    //     @param point_id A id between 0 and size() - 1
    //     @return A point in global or local coordinates depending on the 
    //             procedence of the father container (Neuron or Morphology).
    // */
    // inline const Vector_3D<Micron> & 
    //     position(Morphology_Point_ID point_id) const;
    /** \endcond */
    

    inline std::vector<Segment> as_vector();

protected:
    //! Get the segment by its point id from the morphology it belongs to.
	inline const Segment segment(Morphology_Point_ID id) const;

private:
    mutable Morphology         * _morphology;
    mutable bool                 _use_global_coordinates;
    Vector_3D<Micron>    _soma_position;
    Rotation_3D<Micron>  _neuron_rotation; 
    Morphology_Point_ID  _begin_point;
    Morphology_Point_ID  _end_point;
};

// ----------------------------------------------------------------------------

//! Outputs the segment GIDs to the output stream in ASCII character format
inline std::ostream & operator << (std::ostream & lhs, const Segments & rhs);

}
#include "BBP/Model/Microcircuit/Segment.h"

namespace bbp
{

// ----------------------------------------------------------------------------

#ifndef SWIG

class Segments::const_iterator
    : public std::iterator<std::forward_iterator_tag, const Segment>,
      private Segment
{
    friend class Segments;

public:
    const_iterator() :
        _segments(0)
    {
#ifndef NDEBUG
        _current = UNDEFINED_MORPHOLOGY_POINT_ID;
#endif
    }

private:
    explicit const_iterator(const Segments * segments,
                            Morphology_Point_ID start_point) :
        _segments(segments),
        _current(start_point)
    {
        if (_current < _segments->_morphology->number_of_points() - 1)
            * static_cast<Segment *>(this) = _segments->segment(_current);
    }

public:
    //! Compare two iterators for equality.
    /*!
        This only works for iterators of the same container.
    */
    bool operator == (const const_iterator & other) const
    { 
         return _current == other._current && 
             (_segments == other._segments ||
              (_segments != 0 && other._segments != 0 &&
               *_segments == *other._segments));
    }

    //! Compare two iterators for inequality.
    /*!
        This only works for iterators of the same container.
    */
    bool operator != (const const_iterator & other) const
    {
        return !(*this == other);
    }

    //! Advance to next element in the container.
    const_iterator & operator ++()
    {
        /*
			\todo This can be helpful for adding support for segments that
			are not part of the same neuron. Only the container can know
			what has to be done and if segments are from the same container.
			(TT)
		*/
        if (++_current < _segments->_morphology->number_of_points() - 1)
            * static_cast<Segment *>(this)  = _segments->segment(_current);
        return * this;
    }

    //! Advance to next element in the container, but return original.
    const_iterator operator ++(int)
    {
        const_iterator temp(* this);
        if (++_current < _segments->_morphology->number_of_points() - 1)
            * static_cast<Segment *>(this)  = _segments->segment(_current);
        return temp;
    }

    //! Advance to previous element in the container.
    const_iterator & operator --()
    {
        /*
			\todo This can be helpful for adding support for segments that
			are not part of the same neuron. Only the container can know
			what has to be done and if segments are from the same container.
			(TT)
		*/
        if (--_current != 0)
            * static_cast<Segment *>(this)  = _segments->segment(_current);
        return * this;
    }

	//! Advance to previous element in the container, but return original.
    const_iterator operator --(int)
    {
        const_iterator temp(* this);
        if (--_current != 0)
            * static_cast<Segment *>(this)  = _segments->segment(_current);
        return temp;
    }

    //! Get (dereference) current element (READ WARNING BEFORE USE!).
    /*!
        WARNING: Referenced elements are only created on the fly and are
        invalidated when the next element gets dereferenced!
    */
    const Segment & operator * () const
    {
        bbp_assert(_current < _segments->_morphology->number_of_points() - 1);
        return * static_cast<const Segment *>(this);
    }

    //! Get pointer to current element (READ WARNING BEFORE USE!).
    /*!
        WARNING: Referenced elements are only created on the fly and are
        invalidated when the next element gets dereferenced!
    */
    const Segment * operator -> () const
    {
        return & (this->operator *());
    }

private:
    const Segments *     _segments;
    Morphology_Point_ID  _current;
};

// ----------------------------------------------------------------------------

class Segments::const_reverse_iterator : 
    public boost::reverse_iterator<Segments::const_iterator>
{
public:
    const_reverse_iterator()
    {}

    const_reverse_iterator(const const_iterator &i) :
        boost::reverse_iterator<Segments::const_iterator>(i)
    {}
};

// ----------------------------------------------------------------------------

inline Segments::Segments(const Morphology * morphology, 
                          const Vector_3D<Micron> & soma_position, 
                          const Rotation_3D<Micron> & neuron_rotation,
                          Morphology_Point_ID begin_point,
                          Morphology_Point_ID end_point) 
 :  _morphology(const_cast<Morphology*>(morphology)),
    _use_global_coordinates(true),
    _soma_position(soma_position),
    _neuron_rotation(neuron_rotation),
    _begin_point(begin_point),
    _end_point(end_point)
{}

// ----------------------------------------------------------------------------

inline Segments::Segments(const Morphology * morphology, 
                          Morphology_Point_ID begin_point,
                          Morphology_Point_ID end_point) 
 :  _morphology(const_cast<Morphology*>(morphology)), 
    _use_global_coordinates(false),
    _begin_point(begin_point),
    _end_point(end_point)
{}

// ----------------------------------------------------------------------------

size_t Segments::size() const
{
    return _end_point - _begin_point;
}

// ----------------------------------------------------------------------------

inline Segments::const_iterator Segments::find(Segment_ID segment) const
{
	if (segment >= _begin_point && segment < _end_point)
		return const_iterator(this, segment - _begin_point);
	else
		return this->end();
}

// ----------------------------------------------------------------------------

const Segment Segments::segment(Morphology_Point_ID id) const
{
    if (_use_global_coordinates)
    {
        return Segment(_morphology, _soma_position, _neuron_rotation, id);
    }
    else
    {
        return Segment(_morphology, id);
    }
}

// ----------------------------------------------------------------------------

Segments::const_iterator Segments::begin() const
{
    return const_iterator(this, _begin_point);
}

// ----------------------------------------------------------------------------

Segments::const_iterator Segments::end() const
{
    // The last point of the section is not the begin of any segment so it's 
    // used to represent the end of the storage.
    return const_iterator(this, _end_point);
}

// ----------------------------------------------------------------------------

Segments::const_reverse_iterator Segments::rbegin() const
{
    return const_reverse_iterator(end());
}

// ----------------------------------------------------------------------------

Segments::const_reverse_iterator Segments::rend() const
{
    return const_reverse_iterator(begin());
}

// ----------------------------------------------------------------------------

bool Segments::operator == (const Segments & other) const
{
    return (_morphology == other._morphology &&
            _begin_point == other._begin_point &&
            _end_point == other._end_point &&
            (// Either both have local coordinates
             (!_use_global_coordinates && !other._use_global_coordinates) ||
             // or they have global coordiantes and are the same
             (_use_global_coordinates == other._use_global_coordinates &&
              _soma_position == other._soma_position &&
              _neuron_rotation == other._neuron_rotation)));
}


inline std::vector<Segment> Segments::as_vector()
{
    std::vector<Segment> ret;
    for(Segments::const_iterator i = begin(); i != end(); i++)
    {
        ret.push_back(*i);
    }
    return ret;
}


// ----------------------------------------------------------------------------
std::ostream & operator << (std::ostream & lhs, const Segments & rhs)
{
    lhs << "{ ";
    for (Segments::const_iterator i = rhs.begin(); 
        i != rhs.end();++i)
    {
        lhs << * i << /* ((i + 1) != rhs.end() ? ", " : */ " ";
    }
    lhs  << "}";
    return lhs;
}

// ----------------------------------------------------------------------------

void Segments::print() const
{
    std::cout << *this << std::endl;
}

// ----------------------------------------------------------------------------
#endif // NOT SWIG

}

#endif
