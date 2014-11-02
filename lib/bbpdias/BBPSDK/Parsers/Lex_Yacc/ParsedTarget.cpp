/*
 *  ParsedTarget.cpp
 *  bc_extBBP-SDK
 *
 *  Created by James King on 4/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ParsedTarget.h"
#include <iostream>

// ----------------------------------------------------------------------------
// Global Instance of ParsedTargetManager for parser to access
// ----------------------------------------------------------------------------

ParsedTargetManager *ptManager = NULL;

int* activeLineNumber = NULL;


// ----------------------------------------------------------------------------
// ParsedTarget
// ----------------------------------------------------------------------------

ParsedTarget::ParsedTarget( const std::string &pName, ParsedTargetType pType ) : name(pName), type(pType), isDefined(false)
{}

// ----------------------------------------------------------------------------
// ParsedTargetManager
// ----------------------------------------------------------------------------

ParsedTargetManager::ParsedTargetManager() : nParseErrors(0), lastSawGID(false)
{}

// ----------------------------------------------------------------------------

ParsedTargetManager::~ParsedTargetManager()
{
    for( unsigned int targetIndex=0; targetIndex<targetsAsOriginallyOrdered.size(); targetIndex++ )
        delete targetsAsOriginallyOrdered[targetIndex];
    
    targetsAsOriginallyOrdered.clear();
    parsedTargets.clear();
}

// ----------------------------------------------------------------------------

void ParsedTargetManager::parse( const std::string &pFile )
{
    ptManager = this;
    parseLine = 1;
    activeLineNumber = &parseLine;

    FILE *fin = fopen( pFile.c_str(), "r" );
    if( !fin )
    {
        std::cerr<<"Error.  Unable to open target file '"<<pFile<<"'\n";
        return;
    }
    
    TargetParser( fin );
    
    //after parsing is complete, some final data validation would be good
    //  do this later, though, so targets from other files can be accessed, too
    
    ptManager = NULL;
    activeLineNumber = NULL;
}

// ----------------------------------------------------------------------------

void ParsedTargetManager::prepare( const std::string &pName, ParsedTarget::ParsedTargetType pType )
{
    std::map<std::string,ParsedTarget*>::iterator locator = parsedTargets.find( pName );
    if( locator != parsedTargets.end() )
    {
        activeTarget = locator->second;
        activeTarget->type = pType;
        if( activeTarget->isDefined ) //trying to define target with same name twice
        {
            std::cerr<<"Error.  Target "<<pName.c_str()<<" has multiple definitions.\n";
            nParseErrors++;
        }
    }
    else
    {
        activeTarget = new ParsedTarget( pName, pType );
        parsedTargets[ pName ] = activeTarget;
    }
    
    activeTarget->isDefined = true;
    targetsAsOriginallyOrdered.push_back( activeTarget );
}

// ----------------------------------------------------------------------------

void ParsedTargetManager::updateActiveTarget( const std::string &pName )
{
    //If this name is for a GID, just convert to int and add to target's list.
    if( lastSawGID )
    {
        activeTarget->cellMembers.push_back( atoi(pName.c_str()+1) );
        
        //for non-cell targets, add an empty entry to their extra data
        if( activeTarget->type != ParsedTarget::Cell )
        {
            activeTarget->cellMemberSubset.push_back( ParsedTarget::All );
            activeTarget->cellMemberExtraData.push_back( std::vector<double>() );
        }
    }
    else
    {
        //otherwise, get pointer to target (making new target if necessary)
        std::map<std::string,ParsedTarget*>::iterator locator = parsedTargets.find( pName );
        if( locator != parsedTargets.end() )
            activeTarget->subtargets.push_back( locator->second );
        else
        {
            activeTarget->subtargets.push_back( new ParsedTarget( pName, ParsedTarget::Cell ) );
            parsedTargets[ pName ] = activeTarget->subtargets.back();
        }
        
        //for non-cell targets, add an empty entry to their extra data
        if( activeTarget->type != ParsedTarget::Cell )
        {
            activeTarget->subtargetSubset.push_back( ParsedTarget::All );
            activeTarget->subtargetExtraData.push_back( std::vector<double>() );
        }
    }
}

// ----------------------------------------------------------------------------

void ParsedTargetManager::updateActiveTarget( const std::string &pName, ParsedTarget::ParsedTargetSubset pSubset )
{
    updateActiveTarget( pName );
    
    if( lastSawGID )
        activeTarget->cellMemberSubset.back() = pSubset;
    else
        activeTarget->subtargetSubset.back() = pSubset;
}

// ----------------------------------------------------------------------------

void ParsedTargetManager::updateActiveTarget( const std::string &pName, ParsedTarget::ParsedTargetSubset pSubset, std::vector<double> &pExtra )
{
    updateActiveTarget( pName, pSubset );
    unsigned int lastSize = pExtra.size();
    
    if( lastSawGID )
        activeTarget->cellMemberExtraData.back().swap( pExtra );
    else
        activeTarget->subtargetExtraData.back().swap( pExtra );
        
    pExtra.resize( lastSize );
}

// ----------------------------------------------------------------------------

void ParsedTargetManager::updateActiveTarget( const std::string &pName, std::vector<double> &pExtra )
{
    updateActiveTarget( pName, ParsedTarget::All, pExtra );
}
