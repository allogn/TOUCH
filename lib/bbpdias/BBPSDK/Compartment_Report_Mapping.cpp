
/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Juan Hernando Vieites
*/

#include "BBP/Model/Experiment/Mappings/Compartment_Report_Mapping.h"

namespace bbp
{
    
// Computes compartment counts for a section within a cell inside an ordered
// offset based compartment mapping (see Compartment_Report_Mapping).
/*! 
    \todo This cannot be inlined cause otherwise the template compilation
    on GCC screws up (either specialization after instantiation when
    compute_compartment_counts is in the .cpp or duplicate
    symbol when compartment_counts is inline in the .h). (TT)
*/
Compartment_Count compartment_counts(
            Compartment_Report_Mapping::Offset_Mapping::const_iterator cell,
            std::vector<Report_Frame_Index>::const_iterator section,
            const Compartment_Report_Mapping::Offset_Mapping & mapping, 
            Report_Frame_Index total_compartments)
{
    Report_Frame_Index first_compartment_index;
    Report_Frame_Index last_compartment_index = UNDEFINED_REPORT_FRAME_INDEX;
    
    // Soma sections of neurons that are present have always 1 compartment
    //! \todo check if this statement is correct for unsorted Garik's parser.
    if (section == cell->begin())
        return 1;
    
    first_compartment_index = *section;
    
    // Detecting empty sections.
    if (first_compartment_index == UNDEFINED_REPORT_FRAME_INDEX)
        return 0;
    
    // Skipping sections that don't appear in the report. Right now,
    // this is not very bad since we will only have to skip sections
    // when we ask for the second axon section, due to the remaining
    // axon sections not being reported.
    for (++section;
         section != cell->end() &&
         last_compartment_index == UNDEFINED_REPORT_FRAME_INDEX;
         ++section)
    {
        last_compartment_index = *section;
    }
    
    if (last_compartment_index == UNDEFINED_REPORT_FRAME_INDEX)
    {
        // We reached the end of the neuron without finding a section with
        // valid offset. We need to search for the first offset of the next
        // neuron (if any). For simplicity we assume that first
        // section (soma) always has a defined offset.
        while (++cell != mapping.end() && cell->size() == 0)
            ;
        
        if (cell != mapping.end())
        {
            last_compartment_index = *cell->begin();
            bbp_assert(last_compartment_index != UNDEFINED_REPORT_FRAME_INDEX);
        }
        else
        {
            // No cell with mapping found after the initial one.
            last_compartment_index = total_compartments;
        }
    }
    
    bbp_assert(last_compartment_index - first_compartment_index > 0 &&
           last_compartment_index - first_compartment_index <=
           std::numeric_limits<boost::uint8_t>::max());
    return (boost::uint8_t) (last_compartment_index - first_compartment_index);
}    
    
}



