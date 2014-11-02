/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2008. All rights reserved.

                Authors: Thomas Traenkler
*/
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_TARGET_FILE_READER_H
#define BBP_TARGET_FILE_READER_H

#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"
#include "BBP/Model/Microcircuit/Readers/Target_Reader.h"

#include "../Parsers/Lex_Yacc/ParsedTarget.h"

namespace bbp {

class Targets;

// ----------------------------------------------------------------------------

//! Loads microcircuit targets from a file.
/*! 
    File loader for targets. Reads Blue Target ASCII files and 
    fills the interfaces with target data. 

    \todo The readers in general should be kept user friendly for users not
    aware of the implementation details as the readers might 
    also be used by end users that do not want to deal with a whole 
    Experiment object. Therefore I would recommend to remove implementation 
    internal functions from the public interface such as the convert functions,
    making them private. The load (userTargets, systemTargets) function should
    be better documented - it only makes sense when you know that the 
    user.target depends on the start.target. (TT)
    
    \todo I would also suggest to add an open(const Filepath & target_filename)
    /close() function pair instead of only being able 
    to specify a filepath in the constructor. This way the reader becomes 
    more flexible and reusable. (TT)
*/
class Target_File_Reader 
    : public Target_Reader
{
public:
        /*!
    Create a reader for the given filename.
    Exception thrown if file doesn't exist or it isn't a regular file.
    */
        Target_File_Reader(const Filepath & target_filename)
        /* throw IO_Error */;

        /*! 
    @sa Target_Reader::read
    */
    
        /**
         * Load targets from start.target.  These targets should be wholly 
         * contained in the file parsed.
         */
        void load(Targets & targets)
        /* throw IO_Error */;
        
        /**
         * Load targets from a user target file.  These may need to refer to 
         * targets from the system start.target file parsed.
         */
        void load(Targets & userTargets, const Targets &systemTargets);
private:        
        /*!
         * After initial temporary targets are extracted from the file, do
         * convert them to the concrete target classes while also doing
         * some additional data validation.  
         *
         * @param targets Storage container for final target objects
         * @param manager Collection of temporary targets parsed from file
           \todo Should not be in public interface. (TT)
         */
        void convertTargets( Targets &targets, ParsedTargetManager &manager );
        
        /*!
         * After initial temporary targets are extracted from the file, do
         * convert them to the concrete target classes while also doing
         * some additional data validation.  
         *
         * @param targets Storage container for final target objects
         * @param systemTargets Targets read from system target source 
           (i.e. start.target)
         * @param manager Collection of temporary targets parsed from file
         \todo Should not be in public interface. (TT)
         */
        void convertTargets( Targets &targets, const Targets &systemTargets, ParsedTargetManager &manager );
        
        /**
         * helper function to convert Subset tags used by parser to subset
         * flags used by Model
         \todo Should not be in public interface. (TT)
         */
        std::vector<Target::Subset>& convertSubsetFlags( const std::vector<ParsedTarget::ParsedTargetSubset> &parsedSubsets,
            std::vector<Target::Subset> &resultSubsets );

        /**
         * helper function to convert Synapses stored in parsed targets into 
         * those used by model targets; doubles to ints
         \todo Should not be in public interface. (TT)
         */            
        std::vector< std::vector<int> >& convertSynpaseIDs( 
            const std::vector<std::vector< double> > &parsedIDs,
            std::vector< std::vector<int> > &resultIDs );

private:
    Filepath _target_filename;
};

// ----------------------------------------------------------------------------

}
#endif
