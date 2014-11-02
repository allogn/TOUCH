/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors: Sebastien Lasserre
                 Konstantinos Sfyrakis
                 Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MVD_FILE_PARSER_H
#define BBP_MVD_FILE_PARSER_H

#include <boost/filesystem/path.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

#include "BBP/Common/Exception/Exception.h"
#include "BBP/Common/Types.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"
#include "BBP/Model/Microcircuit/Morphology_Type.h"
#include "BBP/Model/Microcircuit/Electrophysiology_Type.h"
#include <string>
#include <vector>

namespace bbp 
{

// workaround for Windows 32 bit platform
#ifdef WIN32
#undef min
#undef max
#endif

typedef boost::shared_array<Label> Label_Array;
typedef boost::shared_array<Cell_GID> Cell_GID_Array;
typedef boost::shared_array<Column_GID> Column_GID_Array;
typedef boost::shared_array<Minicolumn_GID> Minicolumn_GID_Array;
typedef boost::shared_array<Cortical_Layer_Number> Cortical_Layer_Number_Array;
typedef boost::shared_array<Morphology_Reconstruction_Origin> 
    Morphology_Reconstruction_Origin_Array;
typedef boost::shared_array<Morphology_Type> Morphology_Type_Array;
typedef boost::shared_array<Electrophysiology_Type> Electro_Type_Array;
typedef boost::shared_array<Degree> Degree_Array;

typedef std::vector<std::string> Names_Table;
typedef size_t MVD_Cell_Index;
typedef size_t Minicolumn_Index;

const MVD_Cell_Index       UNDEFINED_MVD_CELL_INDEX = 
    std::numeric_limits<MVD_Cell_Index>::max();
const Minicolumn_Index UNDEFINED_MINICOLUMN_INDEX =
    std::numeric_limits<Minicolumn_Index>::max();

// ----------------------------------------------------------------------------

/** 
    This class provides access to the data stored in the MVD file.
    Basically it extracts from the file the raw data and return them.
*/
class MVD_File_Parser
{
protected:
    template <typename GID, typename Index> class Index_Map;

public:
    typedef Index_Map<Cell_GID, MVD_Cell_Index> Cell_GID_MVD_Index_Map;
    typedef Index_Map<Minicolumn_GID, Minicolumn_Index> Minicolumn_GID_Index_Map;

public:
    /*!
      Constructor with the path for the MVD file
      Exception thrown if file is not found or it is not regular file.
    */
    MVD_File_Parser(const boost::filesystem::path & filename)
        /* throw (IO_Error) */;
    
    //! get the number of neurons listed in the MVD file
    size_t total_neuron_count() /* throw (IO_Error) */;

    //! Extracts the information from the MVD file for all the selected neurons
    /*!
      If the target is empty (the default argument) the information from all
      the neurons is retreived.
      \todo Add an optional flag to parse only subsets of the information from
      the MVD file.
      The status of the internal data will be undefined if an exception 
      is thrown.
    */
    void parse_file(const Cell_Target & cells = Cell_Target())
        /* throw (IO_Error) */;

    /*!
       Returns the array index for a given Cell_GID.
       If the cell hasn't been loaded the result is UNDEFINED_MVD_CELL_INDEX
    */
    inline MVD_Cell_Index cell_index(Cell_GID cell_id) const;

    /*!
       Returns the array index for a given Cell_GID.
       If the cell hasn't been loaded the result is UNDEFINED_MINICOLUMN_INDEX
    */
    inline Minicolumn_Index minicolumn_index(Minicolumn_GID cell_id) const;

    /*!
      Returns the mapping from cell GIDs to indices in the data arrays.
    */
    inline const Cell_GID_MVD_Index_Map & cell_gid_index_map() const;

    inline size_t cells_loaded() const
    {
        return cell_gid_index_map().size();
    }

    /*!
      Returns the mapping from minicolumn GIDs to indices in the positions 
    */
    inline const Minicolumn_GID_Index_Map & minicolumn_gid_index_map() const;

    inline size_t minicolumns_loaded() const
    {
        return cell_gid_index_map().size();
    }


    const Cell_GID_Array & cell_gids() const
    {
        return _cell_gids;
    }

    const Column_GID_Array & column_gids() const
    {
        return _column_gids;
    }

//    const Label_Array & names() const
//    {
//        return _neuron_names;
//    }

    const Label_Array & morphology_names() const
    {
        return _morphology_names;
    }

    const Minicolumn_GID_Array & minicolumn_gids() const
    {
        return _minicolumn_gids;
    }

    const Vector_3D_Micron_Array & minicolumn_positions() const
    {
        return _minicolumn_positions;
    }

    const Cortical_Layer_Number_Array & layers() const
    {
        return _layer_numbers;
    }

    const Morphology_Reconstruction_Origin_Array & origins() const
    {
        return _morphology_origins;
    }

    const Morphology_Type_Array & morphology_types() const
    {
        return _morphology_types;
    }

    const Electro_Type_Array & electrophysiology_types() const
    {
        return _electrophysiology_types;
    }
    
    const Vector_3D_Micron_Array & positions() const
    {
        return _neuron_positions;
    }
    
    const Degree_Array & y_axis_rotations() const
    {
        return _neuron_rotations;
    }

	const Names_Table & morphology_types_array() const
	{
		return _morphtypes;
	}

	const Names_Table & electrophysiology_types_array() const
	{
		return _electrotypes;
	}

	const Names_Table & excitatory_types_array() const
	{
		return _excitatory;
	}

	const Names_Table & pyramidal_types_array() const
	{
		return _pyramidal;
	}

protected:
    template <typename ID, typename Index>
    class Index_Map : public boost::multi_index_container<
        std::pair<ID, Index>, // Type stored
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                boost::multi_index::member<std::pair<ID, Index>, ID,
                                           &std::pair<ID, Index>::first>
            > // Indexed using operator< std::pair<ID, Index>::first type
        >
    >
    {
        typedef typename std::pair<ID, Index> pair_type;
        typedef ID id_type;
        typedef Index index_type;
    };

protected:
    boost::filesystem::path _filename;

    Cell_GID_MVD_Index_Map _cell_gid_index_map;
    Minicolumn_GID_Index_Map _minicol_gid_index_map;

    // Data extracted from the MVD file for all the neurons
//    Label_Array             _mvd_names;
//    Label_Array             _neuron_names;
    Cell_GID_Array                         _cell_gids;
    Column_GID_Array                       _column_gids;
    Label_Array                            _morphology_names;
    Minicolumn_GID_Array                   _minicolumn_gids;
    Cortical_Layer_Number_Array                     _layer_numbers;
    Morphology_Reconstruction_Origin_Array _morphology_origins;
    Morphology_Type_Array                  _morphology_types;
    Electro_Type_Array                     _electrophysiology_types;
    Vector_3D_Micron_Array                   _neuron_positions; 
    Degree_Array                           _neuron_rotations;
    Vector_3D_Micron_Array                   _minicolumn_positions; 
    Names_Table 			   _morphtypes;
    Names_Table 			   _electrotypes;
    Names_Table 			   _excitatory;
    Names_Table 			   _pyramidal;
	
};

const MVD_File_Parser::Cell_GID_MVD_Index_Map & 
MVD_File_Parser::cell_gid_index_map() const
{
    return _cell_gid_index_map;
}

const MVD_File_Parser::Minicolumn_GID_Index_Map & 
MVD_File_Parser::minicolumn_gid_index_map() const
{
    return _minicol_gid_index_map;
}

MVD_Cell_Index MVD_File_Parser::cell_index(Cell_GID cell_id) const
{
    Cell_GID_MVD_Index_Map::const_iterator index = 
        _cell_gid_index_map.find(cell_id);
    if (_cell_gid_index_map.find(cell_id) == _cell_gid_index_map.end())
        return UNDEFINED_MVD_CELL_INDEX;
    else
        return index->second;
}

Minicolumn_Index 
MVD_File_Parser::minicolumn_index(Minicolumn_GID minicolumn_id) const
{
    Minicolumn_GID_Index_Map::const_iterator index = 
        _minicol_gid_index_map.find(minicolumn_id);
    if (_minicol_gid_index_map.find(minicolumn_id) ==
        _minicol_gid_index_map.end())
        return UNDEFINED_MINICOLUMN_INDEX;
    else
        return index->second;
}

}
#endif // MVD_FILE_PARSER_H
