/*
 *  ParsedTarget.h
 *  bc_extBBP-SDK
 *
 *  Created by James King on 4/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_PARSED_TARGET_H 
#define BBP_PARSED_TARGET_H

#include <vector>
#include <map>
#include <string>

extern void TargetParser( FILE *io );
extern int Target_parse(void);
extern int Target_lex(void);

/**
 * A parsed target is just a simplified version of the target class to be used as an intermediate
 * object.  Currently, it is intended to allow Lex/Yacc to compile without knowing about the whole
 * BBP-SDK system.  When and if Lex/Yacc can be integrated into the cmake build system, then this
 * object can probably go away.
 */
class ParsedTarget
{
    //All data is public!  be free!
    public:
        /*!
         * The available target types
         */
        enum ParsedTargetType { Cell, Section, Compartment, Synapse };
        
        //! The name of the target
        std::string name;
        
        //! The type of target
        ParsedTargetType type;
        
        /*!
         * This flag is used to indicate a target is defined by having its contents parsed.
         * Helps because targets may be used multiple times as subtargets before finally being
         * defined.
         */
        bool isDefined;
        
        std::vector<ParsedTarget*> subtargets;
        
        std::vector<int> cellMembers;
        
        enum ParsedTargetSubset { Soma, Axon, Dend, Apic, All };
        
        //! For section/compartment targets - location restrictions on a neuron
        std::vector< ParsedTargetSubset > cellMemberSubset;

        //! For section/compartment targets - location restrictions on a neuron
        std::vector< ParsedTargetSubset > subtargetSubset;
        
        //! for section/compartments/synapse targets: either normalized section info or synapse ids
        std::vector< std::vector<double> > cellMemberExtraData;
        
        //! for section/compartments/synapse targets: either normalized section info or synapse ids
        std::vector< std::vector<double> > subtargetExtraData;
        
        // ----------------------------------------------------------------------------
        // Member Functions
        // ----------------------------------------------------------------------------
        
        ParsedTarget( const std::string &pName, ParsedTargetType pType );
};

/**
 * Do basic validation and organization of targets as they are read in
 */
class ParsedTargetManager
{
    public:
        int nParseErrors;
    
        ParsedTarget *activeTarget;
        
        std::vector< ParsedTarget*> targetsAsOriginallyOrdered;
        
        std::map< std::string, ParsedTarget*> parsedTargets;
        
        bool lastSawGID;
        
        std::vector<double> activeValues;
        
        int parseLine;
        
        // ----------------------------------------------------------------------------
        // Member Functions
        // ----------------------------------------------------------------------------
        
        ParsedTargetManager();
        
        ~ParsedTargetManager();
        
        void prepare( const std::string &pName, ParsedTarget::ParsedTargetType pType );
        
        void parse( const std::string &pFile );
        
        void updateActiveTarget( const std::string &pName );
        
        void updateActiveTarget( const std::string &pName, ParsedTarget::ParsedTargetSubset pSubset );
        
        void updateActiveTarget( const std::string &pName, ParsedTarget::ParsedTargetSubset pSubset, std::vector<double> &pExtra );
        
        void updateActiveTarget( const std::string &pName, std::vector<double> &pExtra );
};

#endif
