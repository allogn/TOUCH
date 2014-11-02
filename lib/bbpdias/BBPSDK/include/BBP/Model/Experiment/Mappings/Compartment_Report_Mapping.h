/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Juan Hernando Vieites
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENT_REPORT_MAPPING_H
#define BBP_COMPARTMENT_REPORT_MAPPING_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp>

#include "BBP/Model/Microcircuit/Types.h"

namespace bbp {

typedef Compartment_ID Compartment_Count;


    
// ----------------------------------------------------------------------------

//! Mapping of compartments for a cell target inside a report frame.
/*!
  A Compartment_Report_Mapping is associated to a Cell_Target
 
  Every neuron from the cell target has an index. This indices are assigned
  from 0 to cell_target.size() - 1. Every cell takes the index as the position
  where is appears while iterating the Cell_Target with its forward iterator.

  The raw mapping is a vector of vectors of offsets within the simulation 
  buffer. Additionally a vector of vectors with compartment counts for each
  cell section can be provided.

  Each mapping row contains the offsets for the sections of the cell
  corresponding to that row (the cell associated to the row is the one
  that has the same index as the row). Each column in the row corresponds to 
  a section of the neuron. Sections are addressed by their section index in
  the same way as inside the morphology objects. The offsets are absolute
  array offsets for that section in the simulation data array. When a section
  doesn't have any simluation data it's offset will be 
  UNDEFINED_REPORT_FRAME_INDEX.

  For example compartments_offset_table[300][15] contains the section 
  offset for the section with index 15 of the neuron with internal index 
  300. The resulting section offset will allow direct access in the array
  pointed to by the current frame pointer.
  
  When no compartment counts are directly provided the chunk of simulation
  buffer for a cell has to enumerate sections in the same order they appear
  in the morphology data. When compartment counts are provided this 
  restriction doesn't apply. In the first case compartment counts are 
  internally stored to speed up the methods provided, in order to do so
  this object computes the compartment counts.

    \bug I continued to have compiling errors
    with GCC 4.x (either specialization after instantiation for the inlined
    compartment_counts() function that was in the .cpp file, or if I move
    swap_mapping behind its definition in the .cpp and remove the line
    modifier, which removes the error, I get a
    warning that swap_mapping is used but never defined. If I move the
    functions to the .h file, I get duplicate symbols error for swap_mapping
    with the precompute_compartment_counts = false , even if I remove
    the inlining. Therefore, I made a couple of changes to this and the .cpp
    file, removing swap_mapping with precom. = false. Review the problem
    and discuss why we need precompute false anyways. (TT)
    http://gcc.gnu.org/ml/gcc-prs/2000-q3/msg00091.html
    http://www-1.ibm.com/support/docview.wss?uid=swg21044588
 
*/
template <bool precompute_compartment_counts = true>
class Base_Compartment_Report_Mapping
{
public:
    typedef std::vector<std::vector<Report_Frame_Index> > Offset_Mapping;
    typedef std::vector<std::vector<Compartment_Count> > Compartment_Counts;
/*
	Base_Compartment_Report_Mapping() {}
	Base_Compartment_Report_Mapping(
		const Base_Compartment_Report_Mapping & rhs)
	{
		_offset_mapping = rhs._offset_mapping;
		_compartment_counts = rhs._compartment_counts;
		_ordered = rhs._ordered;
		_number_of_compartments_in_frame 
			= rhs._number_of_compartments_in_frame;
	}
*/
    //! returns compartment array offset for the specified neuron or section
    inline Report_Frame_Index section_offset
        (Cell_Index neuron_index, Section_ID section_index = 0) const;

    //! returns compartment array offsets for the specified neuron or section
    inline const std::vector<Report_Frame_Index> &
        sections_offsets(Cell_Index neuron_index) const;

	inline Cell_Index number_of_cells() const;
	
    //! returns the number of compartments in the frame
    inline Report_Frame_Index number_of_compartments() const;

    //! returns the number of sections in the frame
    inline Report_Frame_Index number_of_sections(
                            Cell_Index neuron_index) const;

    //! returns number of compartments of a neuron
    inline Report_Frame_Index number_of_compartments(
                            Cell_Index neuron_index) const;

    //! returns number of compartments of a section
    inline Compartment_Count number_of_compartments 
        (Cell_Index neuron_index, Section_ID section_index) const;

    // MODIFY ACCESS FUNCTIONS

    //! Replaces the current mapping with new offsets
    /*! 
      Compartment counts will be computed internally, simulation buffer is
      required to be sorted per cell and section.
    */
    inline void swap_mapping(Offset_Mapping & offsets,
                             Report_Frame_Index total_compartments);

    //! Replaces the current mapping with new offsets and compartment counts.
    /*! 
      Simulation buffer is not required to be sorted at all.
    */
    inline void swap_mapping(Offset_Mapping & offsets, 
                             Compartment_Counts & counts,
                             Report_Frame_Index total_compartments);

protected:
    //! Computes compartment counts from some ordered section offsets.
    inline void compute_compartment_counts(Offset_Mapping & offsets);


protected:
    Offset_Mapping     _offset_mapping;
    Compartment_Counts _compartment_counts;
    bool               _ordered;

    //! number of compartments in a frame
    Report_Frame_Index _number_of_compartments_in_frame;

private:
};

// ----------------------------------------------------------------------------

typedef Base_Compartment_Report_Mapping<true> Compartment_Report_Mapping;

typedef boost::shared_ptr<Compartment_Report_Mapping>
    Compartment_Report_Mapping_Ptr;
typedef boost::shared_ptr<Compartment_Report_Mapping> 
    Const_Compartment_Report_Mapping_Ptr;


// Computes compartment counts for a section within a cell inside an ordered
// offset based compartment mapping (see Compartment_Report_Mapping).
Compartment_Count compartment_counts(
             Compartment_Report_Mapping::Offset_Mapping::const_iterator cell,
             std::vector<Report_Frame_Index>::const_iterator section,
             const Compartment_Report_Mapping::Offset_Mapping & mapping, 
             Report_Frame_Index total_compartments);

// ----------------------------------------------------------------------------

template <bool precompute_counts>
Report_Frame_Index 
Base_Compartment_Report_Mapping<precompute_counts>::section_offset 
(Cell_Index neuron_index, Section_ID section_index) const
{
    bbp_assert(neuron_index < _offset_mapping.size());
    bbp_assert(section_index < _offset_mapping[neuron_index].size());
    return _offset_mapping[neuron_index][section_index];
}

// ----------------------------------------------------------------------------

template <bool precompute_counts>
const std::vector<Report_Frame_Index> & 
Base_Compartment_Report_Mapping<precompute_counts>::sections_offsets
(Cell_Index neuron_index) const
{
    bbp_assert(neuron_index < _offset_mapping.size());
    return _offset_mapping[neuron_index];
}

// ----------------------------------------------------------------------------

template <bool precompute_counts>
Report_Frame_Index
Base_Compartment_Report_Mapping<precompute_counts>::number_of_sections
(Cell_Index neuron_index) const
{
    bbp_assert(neuron_index < _offset_mapping.size());
    return _offset_mapping[neuron_index].size();
}

// ----------------------------------------------------------------------------

template <bool precompute_counts>
Cell_Index
Base_Compartment_Report_Mapping<precompute_counts>::number_of_cells() 
const
{
	return _offset_mapping.size();
}
	
// ----------------------------------------------------------------------------

template <bool precompute_counts>
Report_Frame_Index
Base_Compartment_Report_Mapping<precompute_counts>::number_of_compartments() 
const
{
#ifdef BBP_SAFETY_MODE
	if (_number_of_compartments_in_frame == UNDEFINED_REPORT_FRAME_INDEX)
	{
		throw_exception(Microcircuit_Access_Error(), 
			SEVERE_LEVEL, __FILE__, __LINE__);
	}
#endif
    // Total number of compartments has not been initialized if assert fails.
    bbp_assert(_number_of_compartments_in_frame != UNDEFINED_REPORT_FRAME_INDEX);
    return _number_of_compartments_in_frame;
}

// ----------------------------------------------------------------------------

template <bool precompute_counts>
Report_Frame_Index
Base_Compartment_Report_Mapping<precompute_counts>::number_of_compartments
(Cell_Index neuron_index) const
{
    if (_offset_mapping[neuron_index].size() == 0)
        return 0;
    
    if (_ordered)
    {
        Report_Frame_Index first_compartment_index;
        Report_Frame_Index last_compartment_index;
    
        first_compartment_index = _offset_mapping[neuron_index][0];
    
        // Skipping neurons that appear in the mapping but don't have
        // any simulation compartment.
        //
        // This is a bit inefficient, but the only simple alternative is 
        // having a map instead of a vector. We cannot use a simple hash_map
        // for this because it cannot be iterated in order.
        // It't possible to have an ordered hash_map with nlogn insertion
        // and O(1) query and ordered iteration (something we have to do
        // almost be hand or combining a hash_map and a map. The hash map
        // stores iterators to the map, so the access and iteration is O(1),
        // but we have to insert in the map, and then in the hash_map (which
        // is not really and issue since we only insert once).
        // boost::multi_index data structure can also solve the problem.
        Cell_Index next_neuron;
        for (next_neuron = neuron_index + 1;
             (next_neuron < _offset_mapping.size() && 
              _offset_mapping[next_neuron].size() == 0);
             ++next_neuron)
            ;
        if (next_neuron < _offset_mapping.size())
        {
            last_compartment_index = _offset_mapping[next_neuron][0];
            bbp_assert(last_compartment_index != UNDEFINED_REPORT_FRAME_INDEX);
        }
        else
        {
            last_compartment_index = 
                (Report_Frame_Index) _number_of_compartments_in_frame;
        }
     
        return last_compartment_index - first_compartment_index;
    }
    else
    {
        const std::vector<Compartment_Count> & counts =
            _compartment_counts[neuron_index];
        Report_Frame_Index count = 0;
        for (std::vector<Compartment_Count>::const_iterator i = counts.begin();
             i != counts.end();
             ++i)
            count += *i;
        return count;
    }
}

// ----------------------------------------------------------------------------

template <>
inline Compartment_Count
Base_Compartment_Report_Mapping<true>::number_of_compartments 
(Cell_Index neuron_index, Section_ID section_index) const
{
    // Precomputed compartment counts specialization
    bbp_assert(neuron_index < _compartment_counts.size());
    bbp_assert(section_index < _compartment_counts[neuron_index].size());
    return _compartment_counts[neuron_index][section_index];
}

// ----------------------------------------------------------------------------

template <>
inline Compartment_Count
Base_Compartment_Report_Mapping<false>::number_of_compartments 
(Cell_Index neuron_index, Section_ID section_index) const
{
    // Not precomputed compartment counts specialization
    bbp_assert(neuron_index < _compartment_counts.size());
    bbp_assert(section_index < _compartment_counts[neuron_index].size());

    size_t section_count = _offset_mapping[neuron_index].size();

    Report_Frame_Index first_compartment_index;
    Report_Frame_Index last_compartment_index = UNDEFINED_REPORT_FRAME_INDEX;

    // Soma sections of neurons that are present have always 1 compartment
    /*!
        \todo check if this statement is correct for unsorted Garik's parser.
        \bug Not sure if this must always be true, while currently it is. (TT)
    */
    if (section_index == 0 && number_of_compartments(neuron_index) != 0)
        return 1;

    first_compartment_index = section_offset(neuron_index, section_index);

    // Detecting empty sections.
    if (first_compartment_index == UNDEFINED_REPORT_FRAME_INDEX)
        return 0;

    // Skipping sections that don't appear in the report. Right now,
    // this is not very bad since we will only have to skip sections
    // when we ask for the second axon section, due to the remaining
    // axon sections not being reported.
    Section_ID next_section = section_index + 1;
    while (next_section < section_count && 
           last_compartment_index == UNDEFINED_REPORT_FRAME_INDEX)
    {
        last_compartment_index = section_offset(neuron_index, next_section++);
    }
    
    if (last_compartment_index == UNDEFINED_REPORT_FRAME_INDEX)
    {
        // We reached the end of the neuron without finding a section with
        // valid offset. We need to search for the first offset of the next
        // neuron (if any). For simplicity we assume that first
        // section (soma) always has a defined offset.

        // See comment in number_of_compartments(neuron_index)
        Cell_Index next_neuron;
        for (next_neuron = neuron_index + 1;
             (next_neuron < _offset_mapping.size() &&
              _offset_mapping[next_neuron].size() == 0);
             ++next_neuron)
            ;
        if (next_neuron < _offset_mapping.size())
        {
            last_compartment_index = _offset_mapping[next_neuron][0];
            bbp_assert(last_compartment_index != UNDEFINED_REPORT_FRAME_INDEX);
        }
        else
        {
            last_compartment_index = 
                (Report_Frame_Index) _number_of_compartments_in_frame;
        }
    }
    return (Compartment_Count) (last_compartment_index - 
                                first_compartment_index);
}

// ----------------------------------------------------------------------------

template<>
inline void Base_Compartment_Report_Mapping<true>::compute_compartment_counts
(Offset_Mapping & offsets __attribute__((__unused__)))
{
    _compartment_counts.resize(_offset_mapping.size());
    
    Offset_Mapping::const_iterator cell_offsets;
    Compartment_Counts::iterator cell_counts;
    for (cell_offsets = _offset_mapping.begin(),
         cell_counts = _compartment_counts.begin();
         cell_offsets != _offset_mapping.end();
         ++cell_offsets, ++cell_counts)
    {
        cell_counts->reserve(cell_offsets->size());
        
        std::vector<Report_Frame_Index>::const_iterator section_offset;
        for (section_offset = cell_offsets->begin();
             section_offset != cell_offsets->end();
             ++section_offset)
        {
            cell_counts->push_back
            (compartment_counts(cell_offsets, section_offset,
                                _offset_mapping, 
                                _number_of_compartments_in_frame));
        }
    }
}

// ----------------------------------------------------------------------------

template <>
inline void Base_Compartment_Report_Mapping<true>::swap_mapping
(Offset_Mapping & offsets, Report_Frame_Index compartments)
{
    _ordered = true;
    _offset_mapping.swap(offsets);
    _number_of_compartments_in_frame = compartments;
    compute_compartment_counts(offsets);
}

/*
 
 If you need to know why this is commented out, check the \bug doxygen
 comment at the top of the class declaration at the beginning of this file. 
 (TT)
 
template <>
void Base_Compartment_Report_Mapping<false>::swap_mapping
(Offset_Mapping & offsets, Report_Frame_Index compartments)
{
    _ordered = true;
    _offset_mapping.swap(offsets);
    _number_of_compartments_in_frame = compartments;
}
*/

//-----------------------------------------------------------------------------

template <bool precompute_counts>
void Base_Compartment_Report_Mapping<precompute_counts>::swap_mapping
(Offset_Mapping & offsets, Compartment_Counts & counts,
 Report_Frame_Index compartments)
{
    _ordered = false;
    _offset_mapping.swap(offsets);
    _compartment_counts.swap(counts);
    _number_of_compartments_in_frame = compartments;
}

// ----------------------------------------------------------------------------

}
#endif
