/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors: Sebastien Lasserre,
                 Rajnish Ranjan,
                 Thomas Traenkler

*/

#include <fstream>
#include <sstream>
#include <vector>

#include "BBP/Common/Types.h"
#include "BBP/Common/String/String.h"
#include "BBP/Model/Experiment/Readers/Parsers/Blue_Config_File_Parser.h"


using namespace bbp ;

//-----------------------------------------------------------------------------

Blue_Config_File_Parser::Blue_Config_File_Parser(std::string Path)
{
    BlueConfig_path = Path;
    if ( Path != "" )
    {
        std::list<std::string> runList ; 
        getSectionNameList("Run",runList);
        std::list<std::string>::iterator lstItr = runList.begin () ;
        _runName = (*lstItr) ;
    }
    else
      _runName = "" ;

}

//-----------------------------------------------------------------------------

void Blue_Config_File_Parser::getSectionValue(
    std::string SectionType, 
    std::string SectionName, 
    std::string strParamName,
    std::string &strValue) 

    throw (File_Parse_Error)

{
    std::ifstream       file;
    std::string         line, 
                        token,
                        value;
    std::stringstream   line_stream, 
                        value_stream;

    file.open(BlueConfig_path.c_str(), std::ios::in);

    if(file.is_open() == false)
    {
        throw_exception(
            File_Parse_Error("Blue_Config_File_Parser::getSectionValue() : "
                             "Could not open Config file: " + BlueConfig_path),
            FATAL_LEVEL, __FILE__, __LINE__);
    }    
    while (file.good() == true)
    {
        // read line from file
        std::getline(file, line);
   
        // if line empty exit loop
        if (line.empty() == true)
        {
            continue;
        }
        // if line not empty, read it
        else
        {
            // convert it to a string stream for the tokenizer
            line_stream.clear();
            line_stream.str(line);
            
            // for each token in this line
            while (line_stream.good() == true)
            {
                line_stream >> token;
                
                // if comment discard line
                if (token[0] == '#')
                {
                    break;
                }
                // if section type of interest
                else if (token == SectionType)
                {
                    line_stream >> token;
                    if (token == SectionName)
                    {
                        // read and interpret next lines as a single section
                        while (file.good())
                        {
                            // read line
                            getline(file, line);
                            // convert it to a string stream for the tokenizer
                            line_stream.clear();
                            line_stream.str(line);
                            // read token
                            while (line_stream.good())
                            {
                                line_stream >> token;
                                if (token == strParamName)
                                {
                                    getline(line_stream, value);
                                    strValue = bbp::trim_ends(value);
                                    file.close();
                                    file.clear();
                                    return;
                                }
                            }
                        }
                    }
                }
                
            }
        }
    }
    file.close();
    file.clear();
}

//-----------------------------------------------------------------------------

void Blue_Config_File_Parser::getRunSectionValue(
    std::string strParamName,
    std::string & strValue
) throw (File_Parse_Error)
{
    // can throw File_Parse_Error
    return getSectionValue(std::string("Run"), _runName, 
                           strParamName, strValue ) ; 
}

//-----------------------------------------------------------------------------

void Blue_Config_File_Parser::getSectionNameList(
    std::string SectionType,
    std::list<std::string> & SecNameList) 

    throw (File_Parse_Error)

{
    FILE * fp = fopen(BlueConfig_path.c_str(), "r");

    char Line[3000];    
    char Token[3000];
    char Value[3000];
    if(!fp)
    {
        throw_exception(
            File_Parse_Error("Blue_Config_File_Parser::getSectionNameList():"
                             " Could not open Config file: " + BlueConfig_path),
            FATAL_LEVEL, __FILE__, __LINE__);
    }    
    while(fgets(Line, 3000, fp))
    {
        sscanf(Line,"%s",Token);
        if ((Token[0] == '#') || ( Token[0] == '\n' )) {
            continue;
        }
        if(std::string(Token) == SectionType) {
            sscanf(Line,"%*s %s",Value);
            SecNameList.push_back(Value);
        }
        
    }
    fclose(fp);

    if (!SecNameList.size())
    {
        throw_exception(
            File_Parse_Error("Blue_Config_File_Parser::getSectionNameList():"
                             "Could not find in the config file the section "
                             + SectionType), FATAL_LEVEL, __FILE__, __LINE__);
    }
}

//-----------------------------------------------------------------------------
