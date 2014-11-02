/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors:        Sebastien Lasserre
                        Rajnish Ranjan

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_BLUE_CONFIG_FILE_PARSER_H
#define BBP_BLUE_CONFIG_FILE_PARSER_H

#include <string>
#include <list>
#include "BBP/Model/Microcircuit/Exceptions.h"

namespace bbp 
{

//-----------------------------------------------------------------------------

/*!
    Blue_Config_File_Parser class provides access to the Blueconfig files 
    which contain all the informations to access to the data for a microcircuit
    visualization or analyzis. Using the getSectionValue() method we can get 
    all the paths stored in the Blue_Config_File_Parser file.
*/
class Blue_Config_File_Parser
{
    
public:
    
    /**
        Constructor to setup the path of the blueConfig File and init the name of the run section.
    */
    Blue_Config_File_Parser(std::string Path); 

    /**provides value of a particular variable stored in the Blue_Config_File_Parser file
     * @param[in] SectionType The type of the section like "Run, Report, Stimulus, ..."
     * @param[in] SectionName The name of the section
     * @param[in] strParamName The name of the paramater we want to query in a particular section
     * @param[out] strValue The value of the variable.
     */
    void getSectionValue(
                    std::string SectionType,
                    std::string SectionName,
                    std::string strParamName,
                    std::string &strValue
                ) 

                throw (File_Parse_Error);

                
    /**
        get the value but only in the current Run section(use  getSectionValue with _runName)
    */
    void getRunSectionValue(
                    std::string strParamName,
                    std::string &strValue
                ) 

                throw (File_Parse_Error) ;

                
    /**
        get the list of strings for all the sections 
    */
    void getSectionNameList(
                    std::string SectionType,
                    std::list<std::string> &SecNameList
                ) 

                throw (File_Parse_Error);


public :
        
        //! path to the BlueConfig file file
        std::string BlueConfig_path ;
        //! list of all the section names
        std::list<std::string> SectionNameList ;    
        //! name of the Run section
        std::string _runName ;
};

//-----------------------------------------------------------------------------

}
#endif /*BDB_BLUECONFIGREADER_H_*/
