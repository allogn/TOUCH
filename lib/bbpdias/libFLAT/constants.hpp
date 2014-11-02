// Useful definitions that depent on SpatialIndex but not on BBP SDK
#ifndef SPATIALINDEX_TOOLS_HPP_
#define SPATIALINDEX_TOOLS_HPP_

#include <SpatialIndex.h>

//#include "tools.hpp"

namespace FLAT {

    const float fill_factor = 0.7F;

    const unsigned default_page_size = 4096;
    const unsigned data_len_in_the_tree = 0; // TODO change

// Index and leaf node capacities given the page size and asssuming double as
// a basic data type for MBRs
// According to Node::getByteArraySize
    const unsigned node_header_double = 3 * sizeof(size_t)
        + 2 * 3 * sizeof(double);
    const unsigned node_space_for_entries_double = default_page_size
        - node_header_double;
    const unsigned index_entry_size_double = 2 * 3 * sizeof(double)
        + sizeof(SpatialIndex::id_type) + sizeof(size_t);
    const unsigned leaf_entry_size_double = index_entry_size_double
        + data_len_in_the_tree;
    const unsigned index_node_capacity_double = node_space_for_entries_double
        / index_entry_size_double;
    const unsigned leaf_node_capacity_double = node_space_for_entries_double
        / leaf_entry_size_double;

// Same for float

    const unsigned node_header_float = 3 * sizeof(size_t)
        + 2 * 3 * sizeof(float);
    const unsigned node_space_for_entries_float = default_page_size
        - node_header_float;
    const unsigned index_entry_size_float = 2 * 3 * sizeof(float)
        + sizeof(SpatialIndex::id_type) + sizeof(size_t);
    const unsigned leaf_entry_size_float = index_entry_size_float
        + data_len_in_the_tree;
    const unsigned index_node_capacity_float = node_space_for_entries_float
        / index_entry_size_float;
    const unsigned leaf_node_capacity_float = node_space_for_entries_float
        / leaf_entry_size_float;

// Float with points indexed
    const unsigned leaf_entry_size_float_point = 3 * sizeof (float)
        + sizeof (SpatialIndex::id_type) + sizeof (size_t)
        + data_len_in_the_tree;
    const unsigned leaf_node_capacity_float_point
        = node_space_for_entries_float / leaf_entry_size_float_point;

// And defaults
    const unsigned index_node_capacity = index_node_capacity_float;
    const unsigned leaf_node_capacity = leaf_node_capacity_float_point;
}

#endif
