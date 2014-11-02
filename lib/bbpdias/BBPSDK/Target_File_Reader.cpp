/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2008. All rights reserved.

                Authors: Thomas Traenkler
*/

#include <fstream>
#include <boost/filesystem.hpp>

#include "File/Target_File_Reader.h"
#include "Parsers/Lex_Yacc/ParsedTarget.h"
#include "BBP/Model/Microcircuit/Targets/Target.h"

namespace bbp {

// ----------------------------------------------------------------------------

Target_File_Reader::Target_File_Reader(const Filepath & target_filename) 
: _target_filename(target_filename)
{
    if (!boost::filesystem::exists(_target_filename))
    {
        throw_exception(File_Open_Error("Target file not found: " + 
                                        _target_filename.string()),
                        FATAL_LEVEL, __FILE__, __LINE__);
    }
    if (!boost::filesystem::is_regular(_target_filename))
    {
        throw_exception(File_Open_Error("Invalid file: " + 
                                        _target_filename.string()),
                        FATAL_LEVEL, __FILE__, __LINE__);
    }

}

// ----------------------------------------------------------------------------

void Target_File_Reader::load( Targets &targets )
{
    ParsedTargetManager manager;
    manager.parse( _target_filename.string() );
    
    //check for parse errors
    if( manager.nParseErrors > 0 )
    {
        std::cerr << _target_filename.string().c_str()
            << ": "<<manager.nParseErrors<<" detected.\n";
        std::cerr<<"Aborting target load\n";
        return;
    }
    
    convertTargets( targets, manager );
}

// ----------------------------------------------------------------------------

void Target_File_Reader::load(Targets & userTargets, 
                              const Targets & systemTargets )
{
    ParsedTargetManager manager;
    manager.parse( _target_filename.string() );
    
    //check for parse errors
    if( manager.nParseErrors > 0 )
    {
        std::cerr <<_target_filename.string().c_str()
             << ": "<<manager.nParseErrors<<" detected.\n";
        std::cerr<<"Aborting target load\n";
        return;
    }
    
    convertTargets( userTargets, systemTargets, manager );
}

// ----------------------------------------------------------------------------

void Target_File_Reader::convertTargets(Targets &targets, 
                                        ParsedTargetManager &manager )
{    
    std::vector< std::pair<ParsedTarget*, Target > > addedTargets;
    
    //print Targets in original order
    for( unsigned int targetIndex=0; 
        targetIndex < manager.targetsAsOriginallyOrdered.size(); 
        targetIndex++ )
    {
        ParsedTarget * parsedTarget = 
            manager.targetsAsOriginallyOrdered[targetIndex];
        // only need to instantiate targets that were fully defined in the 
        // target file
        if( !parsedTarget->isDefined )  
            
            continue;
        
        Target_Type convertedType = CELL_TARGET;
        switch( parsedTarget->type )
        {
            case ParsedTarget::Cell:
                convertedType = CELL_TARGET;
                break;
            case ParsedTarget::Section:
                convertedType = SECTION_TARGET;
                break;
            case ParsedTarget::Compartment:
                convertedType = COMPARTMENT_TARGET;
                break;
            case ParsedTarget::Synapse:
                convertedType = SYNAPSE_TARGET;
        }
        
	
//         Target addTarget = 
//             Target(new Target( parsedTarget->name, convertedType ));
        Target addTarget =
            Target(parsedTarget->name, convertedType);
        
        try 
        {
            targets.push_back( Target(addTarget,true) );
            addedTargets.push_back( 
                 std::pair<ParsedTarget*,Target >(
                  parsedTarget, Target(addTarget,true) ) );
        }
        catch ( std::runtime_error rte )
        {
            //so as not to consider this target again for next conversion stage
            parsedTarget->isDefined = false; 
            std::cerr<<rte.what()<<std::endl;
        }
    }
    
    // once place holders for all the targets are inserted, go back an add in 
    // their cell member and target member information
    for( unsigned int targetIndex=0; targetIndex<addedTargets.size(); 
         targetIndex++ )
    {
        ParsedTarget *parsedTarget = addedTargets[targetIndex].first;
        Target resultTarget = addedTargets[targetIndex].second;
        
        // convert subtargets vector<ParsedTarget*> to targetMembers 
        // vector<Target >
        std::vector<Target > targetMembers;
        for( unsigned int subIndex=0; 
            subIndex < parsedTarget->subtargets.size(); subIndex++ )
        {
            try
            {
                // const Target &located = 
                // targets.find( parsedTarget->subtargets[subIndex]->name );
                // targetMembers.push_back( &located );
		        // Modified to handle smart pointers - JLK 8.10.08
		        Targets::iterator located = targets.find( 
                    parsedTarget->subtargets[subIndex]->name );
                targetMembers.push_back( located.get_pointer() );
            } 
            catch ( Target_Not_Found )
            {
                // target *files* should be loaded such that targets 
                // should be wholly defined when a file concludes.
                // e.g. user.target can refer to targets in start.target, 
                // so start.target should be loaded first
                // I don't expect this error to occur, so just give a warning 
                // message in case it happens and be
                // ready to come up with a more fault tolerent solution
                std::cerr << "Warning.  Attempted to use target " 
                    << parsedTarget->subtargets[subIndex]->name 
                    << " which is not defined\n";
            }
        }
        
        std::vector<Target::Subset> subsetInfo;
        std::vector< std::vector<int> > synapseIDs;
        switch( parsedTarget->type )
        {
            case ParsedTarget::Cell:
                resultTarget.insert( std::vector<Cell_GID>( 
                    parsedTarget->cellMembers.begin(), 
                    parsedTarget->cellMembers.end() ) );
                resultTarget.insert( targetMembers );
                break;
            case ParsedTarget::Section: 
            case ParsedTarget::Compartment:
                resultTarget.insert( std::vector<Cell_GID>( 
                    parsedTarget->cellMembers.begin(), 
                    parsedTarget->cellMembers.end() ),
                    convertSubsetFlags( parsedTarget->cellMemberSubset, 
                    subsetInfo ),
                    parsedTarget->cellMemberExtraData );
                
                convertSubsetFlags( parsedTarget->subtargetSubset, subsetInfo );
                resultTarget.insert( targetMembers,
                    convertSubsetFlags( parsedTarget->cellMemberSubset, subsetInfo ),
                    parsedTarget->cellMemberExtraData );
                break;
            case ParsedTarget::Synapse:
                //note the synapse info is converted from doubles to int
                resultTarget.insert( std::vector<Cell_GID>( parsedTarget->cellMembers.begin(), parsedTarget->cellMembers.end() ),
                    convertSynpaseIDs( parsedTarget->cellMemberExtraData, synapseIDs ) );
                break;
        }
    }
}

// ----------------------------------------------------------------------------

void Target_File_Reader::convertTargets( Targets &targets, const Targets &systemTargets, ParsedTargetManager &manager )
{
    //This function is pretty much just a duplicate of the one for just system targets.  They can probably be
    // merged for easier maintainece.  For now, I just want to get this done.

    std::vector< std::pair<ParsedTarget*, Target > > addedTargets;
    
    //print Targets in original order
    for( unsigned int targetIndex=0; targetIndex<manager.targetsAsOriginallyOrdered.size(); targetIndex++ )
    {
        ParsedTarget *parsedTarget = manager.targetsAsOriginallyOrdered[targetIndex];
        
        if( !parsedTarget->isDefined )  //only need to instantiate targets that were fully defined in the target file
            continue;
        
        Target_Type convertedType = CELL_TARGET;
        switch( parsedTarget->type )
        {
            case ParsedTarget::Cell:
                convertedType = CELL_TARGET;
                break;
            case ParsedTarget::Section:
                convertedType = SECTION_TARGET;
                break;
            case ParsedTarget::Compartment:
                convertedType = COMPARTMENT_TARGET;
                break;
            case ParsedTarget::Synapse:
                convertedType = SYNAPSE_TARGET;
        }
        
        //Target addTarget = Target(new Target( parsedTarget->name, convertedType ));
        Target addTarget = Target( parsedTarget->name, convertedType );
        try 
        {
            targets.push_back( Target(addTarget,true) );
            addedTargets.push_back( std::pair<ParsedTarget*,Target >( parsedTarget, Target(addTarget,true) ) );
        }
        catch ( std::runtime_error rte )
        {
            parsedTarget->isDefined = false; //so as not to consider this target again for next conversion stage
            std::cerr<<rte.what()<<std::endl;
        }
    }
    
    //once place holders for all the targets are inserted, go back an add in their cell member and target member information
    for( unsigned int targetIndex=0; targetIndex<addedTargets.size(); targetIndex++ )
    {
        ParsedTarget *parsedTarget = addedTargets[targetIndex].first;
        Target resultTarget = addedTargets[targetIndex].second;
                
        //convert subtargets vector<ParsedTarget*> to targetMembers vector<Target >
        std::vector<Target> targetMembers;
        for( unsigned int subIndex=0; subIndex < parsedTarget->subtargets.size(); subIndex++ )
        {
            try{
                //check whether the target is in those defined, or those in the system targets
                if( targets.exists( parsedTarget->subtargets[subIndex]->name ) )
                {
                    //const Target &located = targets.find( parsedTarget->subtargets[subIndex]->name );
                    //targetMembers.push_back( &located );
		    // Modified to handle smart pointers - JLK 8.10.08
		    Targets::iterator located = targets.find( parsedTarget->subtargets[subIndex]->name );
                    targetMembers.push_back( located.get_pointer() );
                }
                else
                {
                    //const Target &located = systemTargets.find( parsedTarget->subtargets[subIndex]->name );
                    //targetMembers.push_back( &located );
		    // Modified to handle smart pointers - JLK 8.10.08
		    Targets::iterator located = systemTargets.find( parsedTarget->subtargets[subIndex]->name );
                    targetMembers.push_back( located.get_pointer() );
                }                
            } catch ( Target_Not_Found ) {
                //target *files* should be loaded such that targets should be wholly defined when a file concludes.
                // e.g. user.target can refer to targets in start.target, so start.target should be loaded first
                // I don't expect this error to occur, so just give a warning message in case it happens and be
                // ready to come up with a more fault tolerent solution
                std::cerr<<"Warning.  Attempted to use target "<<parsedTarget->subtargets[subIndex]->name<<" which is not defined\n";
            }
        }
        
        std::vector<Target::Subset> subsetInfo;
        std::vector< std::vector<int> > synapseIDs;
        switch( parsedTarget->type )
        {
            case ParsedTarget::Cell:
                resultTarget.insert( std::vector<Cell_GID>( parsedTarget->cellMembers.begin(), parsedTarget->cellMembers.end() ) );
                resultTarget.insert( targetMembers );
                break;
            case ParsedTarget::Section: case ParsedTarget::Compartment:
                resultTarget.insert( std::vector<Cell_GID>( parsedTarget->cellMembers.begin(), parsedTarget->cellMembers.end() ),
                    convertSubsetFlags( parsedTarget->cellMemberSubset, subsetInfo ),
                    parsedTarget->cellMemberExtraData );
                
                convertSubsetFlags( parsedTarget->subtargetSubset, subsetInfo );
                resultTarget.insert( targetMembers,
                    convertSubsetFlags( parsedTarget->cellMemberSubset, subsetInfo ),
                    parsedTarget->cellMemberExtraData );
                break;
            case ParsedTarget::Synapse:
                //note the synapse info is converted from doubles to int
                resultTarget.insert( std::vector<Cell_GID>( parsedTarget->cellMembers.begin(), parsedTarget->cellMembers.end() ),
                    convertSynpaseIDs( parsedTarget->cellMemberExtraData, synapseIDs ) );
                break;
        }
    }        
}

// ----------------------------------------------------------------------------

std::vector<Target::Subset>& Target_File_Reader::convertSubsetFlags( const std::vector<ParsedTarget::ParsedTargetSubset> &parsedSubsets, std::vector<Target::Subset> &resultSubsets )
{
    resultSubsets.resize( parsedSubsets.size() );
    for( unsigned int itemIndex=0; itemIndex<parsedSubsets.size(); itemIndex++ )
    {
        switch( parsedSubsets[itemIndex] )
        {
            case ParsedTarget::All:
                resultSubsets[itemIndex] = Target::All;
                break;
            case ParsedTarget::Soma:
                resultSubsets[itemIndex] = Target::Soma;
                break;
            case ParsedTarget::Axon:
                resultSubsets[itemIndex] = Target::Axon;
                break;
            case ParsedTarget::Dend:
                resultSubsets[itemIndex] = Target::Dend;
                break;
            case ParsedTarget::Apic:
                resultSubsets[itemIndex] = Target::Apic;
                break;
        }
    }
    
    return resultSubsets;
}

// ----------------------------------------------------------------------------

std::vector< std::vector<int> >& Target_File_Reader::convertSynpaseIDs( const std::vector< std::vector<double> > &parsedIDs, std::vector< std::vector<int> > &resultIDs )
{
    resultIDs.resize( parsedIDs.size() );
    for( unsigned int memberIndex=0; memberIndex<parsedIDs.size(); memberIndex++ )
    {
        resultIDs[memberIndex] = std::vector<int>();
        std::vector<int> & to = resultIDs[memberIndex];
        const std::vector<double> & from = parsedIDs[memberIndex];
        to.reserve(from.size());
        for (std::vector<double>::const_iterator i = from.begin(); i != from.end(); ++i)
            to.push_back((int)*i);
    }
    return resultIDs;
}

// ----------------------------------------------------------------------------

}
