/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors: Sebastien Lasserre, 
                 Konstantinos Sfyrakis,
                 Juan Hernando

*/

#include <BBP/Common/Exception/Exception.h>

#include <cassert>
#include <cctype>

#include <fstream>
#include <sstream>
#include <iomanip>

#include <boost/multi_index/identity.hpp>
#include <boost/filesystem.hpp>

#include "MVD_File_Parser.h"

namespace bbp
{

struct skip_white_space
{
    skip_white_space(size_t &line_count) :
        _line_count(line_count)
    {}

    std::istream & operator()(std::istream &in)
    {
        while (isspace(in.peek()))
        {
            if (in.get() == '\n')
            {
                ++_line_count;
            }
        }
        return in;
    }

    size_t _line_count;
};

std::istream & operator << (std::istream & in, struct skip_white_space ws)
{
    return ws(in);
}

//---------------------------------------------------------------------------

MVD_File_Parser::MVD_File_Parser(const boost::filesystem::path & filename) :
    _filename(filename)
{
    if (!boost::filesystem::exists(_filename))
    {
        throw_exception(
            File_Open_Error("File not found: " + _filename.string()),
            FATAL_LEVEL, __FILE__, __LINE__);
    }
    if (!boost::filesystem::is_regular(_filename))
    {
        throw_exception(
            File_Open_Error("Invalid file: " + _filename.string()),
            FATAL_LEVEL, __FILE__, __LINE__);
    }
}

//---------------------------------------------------------------------------

MVD_Cell_Index MVD_File_Parser::total_neuron_count()  /* throw (IO_Error) */
{    
    MVD_Cell_Index neuron_count = 0;
    
    //check if the file exists and open it
    std::ifstream file(_filename.string().c_str(), std::ios_base::in);
    if (!file)
    {
        std::stringstream stmp;
        stmp << "Could not find the mvd file: '" 
             << _filename.string() << "'" << std::endl;
        throw_exception(File_Access_Error(stmp.str()), 
                        FATAL_LEVEL, __FILE__, __LINE__);
    } 
    // get the number of neurons
    else 
    {
        enum SectionType {Neurons, None} type = None;
        
        file >> std::ws;
        std::string line;
        while (!file.eof())
        {            
            std::getline(file, line);
            file >> std::ws;
            
            // Skipping comments
            if (line[0] == '#')
                continue;

            bool section_header = false;

            if (line == "Neurons Loaded")
            { 
                type = Neurons;
                section_header = true;
            }
            else if (line == "MicroBox Data")
            {
                type = None;
                section_header = true;
            } 
            else if (line == "Layers Positions Data")
            {
                type = None;
                section_header = true;
            } 
            else if (line == "Axon-Dendrite Synapses")
            {
                type = None;
                section_header = true;
            }

            if (type == Neurons && !section_header)
            {
                neuron_count++;
            }   
        }
    }

    return neuron_count;
}

//---------------------------------------------------------------------------

void MVD_File_Parser::parse_file(const Cell_Target & cells)
   /* throw (IO_Error) */
{
    MVD_Cell_Index set_size = 
        cells.size() != 0 ? std::min(cells.size(), total_neuron_count()) :
                            total_neuron_count();

    // allocate memory for the temporary structures
    //_neuron_names.reset(new Label[set_size]);
    //_mvd_names.reset(new Label[set_size]);
    _cell_gids.reset(new Cell_GID[set_size]);
    _morphology_names.reset(new Label[set_size]);
    _column_gids.reset(new Column_GID[set_size]);
    _minicolumn_gids.reset(new Minicolumn_GID[set_size]);
    _layer_numbers.reset(new Cortical_Layer_Number[set_size]);
    _morphology_origins.reset(new Morphology_Reconstruction_Origin[set_size]);
    _morphology_types.reset(new Morphology_Type[set_size]);
    _electrophysiology_types.reset(new Electrophysiology_Type[set_size]);
    _neuron_positions.reset(new Vector_3D<Micron>[set_size]);
    _neuron_rotations.reset(new Degree[set_size]);

    //check if the file exists and open it
    std::ifstream file(_filename.string().c_str(), std::ios_base::in);
    if (!file)
    {
        std::stringstream stmp;
        stmp << "Could not find the mvd file: '" 
             << _filename.string() << "'" << std::endl;
        throw_exception(File_Access_Error(stmp.str()), 
                        FATAL_LEVEL, __FILE__, __LINE__);
    } 

    // get the number of neurons
    enum SectionType {Neurons, Mini_Columns_Positions, Microbox,ElectroTypes,MorphTypes, None}
    type = None;

    size_t line_count = 0;
    size_t neuron_count = 0;
    Cell_GID current_cell_gid = 0;
    Cell_Target::iterator next_cell_gid = cells.begin();

    size_t minicolumn_count = 0;
    Minicolumn_GID current_minicolumn_gid = 0;
    std::set<Minicolumn_GID> minicolumns;
    std::set<Minicolumn_GID>::iterator next_minicolumn_gid;

    std::string line;

    file << skip_white_space(line_count);
    while (!file.eof())
    {       
        std::getline(file, line);

        bool section_header = false;

        if (line == "Neurons Loaded")
        { 
            type = Neurons;
            section_header = true;
        }
        else if (line == "MicroBox Data")
        {
            type = Microbox;
            section_header = true;
        } 
        else if (line == "Layers Positions Data")
        {
            type = None;
            section_header = true;
        } 
		else if (line == "Axon-Dendrite Synapses")
		{
			type = None;
			section_header = true;
		}
		else if (line == "ElectroTypes")
		{
			type = ElectroTypes;
			section_header = true;
		}
		else if (line == "MorphTypes")
		{
			type = MorphTypes;
			section_header = true;
		}
		else if (line == "MiniColumnsPosition")
        {
            next_minicolumn_gid = minicolumns.begin();
            _minicolumn_positions.reset
                (new Vector_3D<Micron>[minicolumns.size()]);
            type = Mini_Columns_Positions;
            section_header = true;
        }

        if (type == Neurons && !section_header)
        {
            // cell_gids start at 1
            ++current_cell_gid;

            bool load_next = ((next_cell_gid != cells.end() &&
                               current_cell_gid == *next_cell_gid));
            if (load_next)
            {
                if (next_cell_gid != cells.end())
                    ++next_cell_gid;

                std::stringstream data(line);
                
                // Storing local index in gid to index map
                _cell_gid_index_map.insert(std::make_pair(current_cell_gid,
                                                          neuron_count));
                // Extracting data from the line read
                Morphology_Type_ID m_type;
                Electrophysiology_Type_ID e_type;
                _cell_gids[neuron_count] = current_cell_gid;
                data >> _morphology_names[neuron_count]
                     >> _morphology_origins[neuron_count]
                     >> _column_gids[neuron_count]
                     >> _minicolumn_gids[neuron_count]
                     >> _layer_numbers[neuron_count]
                     >> m_type
                     >> e_type
                     >> _neuron_positions[neuron_count].x()
                     >> _neuron_positions[neuron_count].y()
                     >> _neuron_positions[neuron_count].z()
                     >> _neuron_rotations[neuron_count];
                _morphology_types[neuron_count].id(m_type);
                _electrophysiology_types[neuron_count].id(e_type);

                minicolumns.insert(_minicolumn_gids[neuron_count]);
                ++neuron_count;
                // store the number values in the flat arrays
                if (data.fail())
                {
                    std::stringstream msg ;
                    msg << "Error parsing mvd file: '"
                        << _filename << "'. Bad format in line " 
                        << line_count << std::endl;
                    throw_exception(File_Parse_Error(msg.str()), 
                                    FATAL_LEVEL, __FILE__, __LINE__);
                }
            }
// REVIEW What to do with this original piece of code
//			_neuronName.push_back(std::string(nname));
//              // increment the nb of neurons loaded
//              nbNeurons++ ; 
//              // store the MVD name
//              sprintf(buf, "a%d", nbNeurons); // start from a1 to a10000
//              _mvdName.push_back(buf) ;
//
//              // store the morphology name
//              _vectMorphologyName.push_back(nname) ;
        }

        if (type == Microbox && !section_header)
        {
            std::stringstream data(line);
            
            //! \todo 
            float _;
            data >> _ >> _ >> _ >> _ >> _ >> _ >> _ >> _;
            
            if (data.fail())
            {
                std::stringstream msg ;
                msg << "Error parsing mvd file: '"
                    << _filename << "'. Bad format in MicroBox Data in line " 
                    << line_count << std::endl;
                throw_exception(File_Parse_Error(msg.str()), 
                                FATAL_LEVEL, __FILE__, __LINE__);
            }
        }

        if (type == Mini_Columns_Positions && !section_header)
        {
            if (next_minicolumn_gid != minicolumns.end() &&
                current_minicolumn_gid == *next_minicolumn_gid)
            {
                ++next_minicolumn_gid;
                std::stringstream data(line);

                // Storing local index in minicolumn gid to index map
                _minicol_gid_index_map.insert
                    (std::make_pair(current_minicolumn_gid, minicolumn_count));
                // Extracting data from the line read
                data >> _minicolumn_positions[minicolumn_count].x()
                     >> _minicolumn_positions[minicolumn_count].y()
                     >> _minicolumn_positions[minicolumn_count].z();
                ++minicolumn_count;
                if (data.fail())
                {
                    std::stringstream msg ;
                    msg << "Error parsing mvd file: '"
                        << _filename << "'. Bad format in MiniColumns in line " 
                        << line_count << std::endl;
                    throw_exception(File_Parse_Error(msg.str()),
                                    FATAL_LEVEL, __FILE__, __LINE__);
                }
            }

            // minicolumn_gids start with 0
            ++current_minicolumn_gid;
        }
		
		if (type == MorphTypes && !section_header)
        {
            std::stringstream data(line);
            
            //! \todo 
            std::string name,pyr,inh;
            data >> name >> pyr >> inh;
            
            if (data.fail())
            {
                std::stringstream msg ;
                msg << "Error parsing mvd file: '"
                    << _filename << "'. Bad format in MorphTypes Data in line " 
                    << line_count << std::endl;
                throw_exception(File_Parse_Error(msg.str()), 
                                FATAL_LEVEL, __FILE__, __LINE__);
            }
            _morphtypes.push_back(name);
            _excitatory.push_back(inh);
            _pyramidal.push_back(pyr);
        }

		if (type == ElectroTypes && !section_header)
        {
            std::stringstream data(line);
            
            //! \todo 
            std::string name;
            data >> name; 
            
            if (data.fail())
            {
                std::stringstream msg ;
                msg << "Error parsing mvd file: '"
                    << _filename << "'. Bad format in ElectroTypes Data in line " 
                    << line_count << std::endl;
                throw_exception(File_Parse_Error(msg.str()), 
                                FATAL_LEVEL, __FILE__, __LINE__);
            }
            _electrotypes.push_back(name);
        }
        file << skip_white_space(line_count);
    }

    // Checking if morphology types or electrophysiology types are not
    // present in the mvd file and initilizing the arrays with some default
    // values. This code is provided for backwards compatibility with old
    // mvd files.
    if (_electrotypes.size() == 0)
    {
        const char *types[] = 
            {"cAD", "bAD", "dAD", "cFS", "bFS", "dFS", "cST", "bST", "dST",
             "cIS", "bIS", "IBS", "rBS", "tBS", "cAL", "dAL", "bAL", "cNA",
             "bNA", "dNA"};
        for (size_t i = 0; i < sizeof(types) / sizeof(char*); ++i)
            _electrotypes.push_back(types[i]);
        std::cerr << "Warning: electrophysiology types table not found in"
            " mvd file. Using old list by default" << std::endl;
    }

    if (_morphtypes.size() == 0)
    {
        
        const char *morph_type[] = 
            {"L2PC",   "L3PC",   "L4PC",  "L4SP",   "L4SS", "L5CSPC", "L5CHPC",
             "L6CTPC", "L6CCPC", "L6CSPC","L6FFPC", "HC",   "CRC",    "MC", 
             "BTC",    "DBC",    "BP",    "NGC",    "LBC",  "NBC",    "SBC",
             "ChC",    "AHC",    "ADC",   "SAC"};
        const char *morph_class[] =
            {"PYR",    "PYR",     "PYR",  "INT",    "INT",  "PYR",    "PYR",
             "PYR",    "PYR",     "PYR",  "PYR",    "INT",  "INT",    "INT",
             "INT",    "INT",     "INT",  "INT",    "INT",  "INT",    "INT",
             "INT",    "INT",     "INT",  "INT"};
        const char *function_class[] =
            {"EXC",    "EXC",     "EXC",  "EXC",    "EXC",  "EXC",    "EXC",
             "EXC",    "EXC",     "EXC",  "EXC",    "INH",  "INH",    "INH",
             "INH",    "INH",     "INH",  "INH",    "INH",  "INH",    "INH",
             "INH",    "INH",     "INH",  "INH"};
        for (size_t i = 0; i < sizeof(morph_type) / sizeof(char*); ++i) 
        {
            _morphtypes.push_back(morph_type[i]);
            _excitatory.push_back(function_class[i]);
            _pyramidal.push_back(morph_class[i]);
        }
        std::cerr << "Warning: morphological types table not found in"
            " mvd file. Using old list by default" << std::endl;
    }

}

//---------------------------------------------------------------------------

}

