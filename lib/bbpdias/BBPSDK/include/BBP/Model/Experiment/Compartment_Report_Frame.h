/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Juan Hernando Vieites
                                Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENT_REPORT_FRAME_H
#define BBP_COMPARTMENT_REPORT_FRAME_H

#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"
#include "BBP/Model/Experiment/Mappings/Compartment_Report_Mapping.h"
#include "BBP/Model/Experiment/Report_Specification.h"
#include "BBP/Model/Experiment/Readers/Report_Reader.h"

namespace bbp {

// Forward references ---------------------------------------------------------

template <typename T, typename Allocator>
class Compartment_Report_Reader;

class Report;
typedef boost::shared_ptr<Report> Report_Ptr;

// ----------------------------------------------------------------------------

//! A moment in time in the compartment report stream.
/*!
    The compartment report frame class is the memory data structure 
    and interface for simulation compartment reports reported from 
    NEURON simulator simulation runs.

    This object is provided with data by report reader classes that read the 
    report data from a file or network stream and point the report object to
    where to get the simulation data for the current timestep. 
    
    The class implements the interface specified in the Report for 
    a specific data type (e.g. millivolt) specified as the template parameter.

    The template parameter must be default constructable.

    The Allocator parameter is used to provide specific allocation routines
    for arrays of T. For simplicity (in specification not in implementation
    :) ) we will keep the STL allocator implementation. By default 
    std::allocator will be used (and should be ok in most cases.
    
    \ingroup Report
*/
template <typename T, typename Allocator = std::allocator<T> >
class Compartment_Report_Frame 
    : public Report_Frame
{
    friend class Compartment_Report_Reader<T, Allocator>;

public:
    class Context;

protected:
    typedef boost::shared_ptr<Context> Context_Ptr;

public:
	typedef boost::shared_ptr<Compartment_Report_Frame<T, Allocator> > Pointer;

    //! Const access current frame data array.
    inline const T * frame() const;
    //! Access current frame data array.
    inline T * frame();
    //! Get number of elements in this simulation data frame.
    inline size_t size() const;

    //! Access context information about this frame.
    inline Context & context();

    //! Access context information about this frame.
    inline const Context & context() const;

    //! Return whether a context has been already set or not.
    inline bool has_valid_context() const;

    //! Get the absolute timestamp for this frame.
    inline Millisecond time() const;

    /*! 
      Sets the number of elements in simulation data frame.
      Previous buffer will be replaced.
      As the internal buffer is a shared pointer, additional copies of
      the object can access thread-safely the old data
    */
    void allocate(size_t frameSize);

    /*! 
      Sets the frame data using a shared array.
    */
    void frame(const boost::shared_array<T> & frame);

    //! prints the content of the current compartment report frame to the console
    inline void print() const;

protected:
    //! pointer to the current simulation data frame
    boost::shared_array<T> _frame;
    //! pointer to context necessary for interpreting raw data such as mapping
    Context_Ptr            _context;
    //! time when this frame has been recorded
    Millisecond            _timestamp;

    Allocator _allocator;
};

template <typename T, typename Allocator>
inline std::ostream & operator << 
(std::ostream & lhs, const Compartment_Report_Frame<T, Allocator> & rhs);

// Compartment_Report_Frame::Context class ------------------------------------

/*!
    The report class is the memory data structure and interface for simulation 
    reports reported from NEURON simulator simulation runs.

    This object is provided with data by report reader classes that read the 
    report data from a file or network stream and point the report object to
    where to get the simulation data for the current timestep. 
    
    The class implements the interface specified in the Report for 
    a specific data type (e.g. millivolt) specified as the template parameter. 
*/

template<typename T, typename Allocator>
class Compartment_Report_Frame<T, Allocator>::Context
{
    friend class Compartment_Report_Reader<T, Allocator>;

public:
    /*! 
        \brief Mapping between neuron or section and corresponding compartments 
        in the current simulation data frame. 
        
        Stores both the offset where the compartments of a particular neuron or
        section start and the number of compartments.
    */
    inline const Const_Compartment_Report_Mapping_Ptr & mapping() const;

    //! Cells reported from in this frame
    inline const Cell_Target & cell_target() const;

    //! Gets the report specification associated to this context.
    inline const Report_Specification & report() const;

protected:
    Const_Compartment_Report_Mapping_Ptr _compartment_mapping;
    Cell_Target                          _cell_target;
    Report_Specification                 _report;
};

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
std::ostream & operator << (std::ostream & lhs, 
                            const Compartment_Report_Frame<T, Allocator> & rhs)
{
    if (!rhs.has_valid_context())
        lhs << " Frame (uninitialized)";
    else
    {
        lhs << " Frame (report: " << rhs.context().report().label() 
            << ", time: " << rhs.time()
            << ", compartments: " << rhs.size() << ") = { ";
        
        if (rhs.size() != 0)
        {
            for (Report_Frame_Index i = 0; i < 5 && i < rhs.size(); ++i)
            {
                lhs << rhs.frame()[i] << " ";
            }
            lhs << "... " << "}";
            //std::cout << "First element of cell target: " 
			//<< * rhs.context().cell_target.begin() << std::endl;
        }
        else
        {
            lhs << "no data }";
        }
    }

    return lhs;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
typename Compartment_Report_Frame<T, Allocator>::Context & 
Compartment_Report_Frame<T, Allocator>::context()
{
    return * _context;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
const typename Compartment_Report_Frame<T, Allocator>::Context & 
Compartment_Report_Frame<T, Allocator>::context() const
{
    return * _context;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
bool Compartment_Report_Frame<T, Allocator>::has_valid_context() const
{
    return _context.get() != 0;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
const T * Compartment_Report_Frame<T, Allocator>::frame() const
{
    return & _frame[0];
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
T * Compartment_Report_Frame<T, Allocator>::frame()
{
    return & _frame[0];
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
size_t Compartment_Report_Frame<T, Allocator>::size() const
{
#ifdef BBP_SAFETY_MODE
	if (_context.get() == 0)
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
    bbp_assert(_context.get() != 0);
    return (size_t) _context->mapping()->number_of_compartments();
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
Millisecond Compartment_Report_Frame<T, Allocator>::time() const
{
    return _timestamp;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Frame<T, Allocator>::print() const
{
    std::cout << * this << std::endl;
}

// ----------------------------------------------------------------------------

/** \cond SHOW_IN_DEVELOPER_REFERENCE */

/*! Helper class for wrapping STL allocator for its use as boost::smart_*
    deallocator types (see boost smart pointer documentation for details.)
*/
template<typename Allocator>
class Boost_Deallocator_Helper
{
public:
    Boost_Deallocator_Helper(Allocator& allocator, size_t size) throw() : 
        _allocator(allocator),
        _size(size)
    {}

    Boost_Deallocator_Helper(const Boost_Deallocator_Helper& h) : 
        _allocator(h._allocator),
        _size(h._size)
    {}

    void operator()(typename Allocator::pointer p)
    {
        // This loop has been profiled for allocator::pointer of type float
        // and the deallocation it's independent of the size of the array
        // (in gcc -O3, the loop must be being detected as empty).
        // If the array to deallocate has a non empty destructor things
        // are significantly worse, but this happens in any case as 
        // destructors must be called.
        // Without optimization there is a performance penalty even with
        // types with empty destructors.
        for (size_t i = 0; i < _size; ++i)
            _allocator.destroy(&p[i]);
        _allocator.deallocate(p, _size);
    }

protected:
    Allocator _allocator;
    size_t _size;
};

/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Frame<T, Allocator>::allocate(size_t frame_size)
{
    _frame.reset(_allocator.allocate(frame_size), 
                 Boost_Deallocator_Helper<Allocator>(_allocator, frame_size));
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
void Compartment_Report_Frame<T, Allocator>::frame
(const boost::shared_array<T> & frame)
{
    _frame = frame;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
const Const_Compartment_Report_Mapping_Ptr &
Compartment_Report_Frame<T, Allocator>::Context::mapping() const
{
    return _compartment_mapping;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
const Cell_Target & 
Compartment_Report_Frame<T, Allocator>::Context::cell_target() const
{
    return _cell_target;
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
const Report_Specification &
Compartment_Report_Frame<T, Allocator>::Context::report() const
{
    return _report;
}

// ----------------------------------------------------------------------------

}

#endif
