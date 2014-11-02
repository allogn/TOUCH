/*

                Ecole Polytechnique Federale de Lausanne
                Brain Mind Institute,
                Blue Brain Project
                (c) 2006-2007. All rights reserved.

                Authors:    Juan Hernando Vieites
                            Thomas Traenkler
                            Sebastien Lasserre
                            

*/

#include "BBP/Model/Experiment/Readers/Experiment_Reader.h"
#include "BBP/Model/Experiment/Experiment.h"
#include "BBP/Common/String/String.h"
#include "BBP/Model/Microcircuit/Writers/File/Target_File_Writer.h"
#include "BBP/Common/System/environ.h"

namespace bbp {

// ----------------------------------------------------------------------------

void Experiment::open(const URI & source, 
                      const std::string & original_uri_prefix,
                      const std::string & replacement_uri_prefix)
{
    close();

    try 
    {
        // Use the ExperimentReader to load the target and store the 
        // report metadata.
        Experiment_Reader reader(
            source, original_uri_prefix, replacement_uri_prefix);
        reader.read(*this);

        // Initialize the mircrocircuit for loading.
        _microcircuit = Microcircuit::create();
        _microcircuit->open(*this);
        
        // If the experiment declares a user target file, read it in now
        reader.load_user_targets(*this);
        _is_open = true;
    } 
    catch (...) 
    {
        /*! \todo Log meaningful error message. (TT) The log message must be
            logged in the throwing code, since it has the complete context,
            and not here where we know nothing (reporting is already done in
            all the throw points as far as I'm concerned) (JH). */
        close();
        throw;
    }
}

// ----------------------------------------------------------------------------


void Experiment::open(const URI & source)
{
    std::string prefix;
    
    if(bbp::getenv("BBP_BGLSCRATCH_REPLACEMENT", prefix))
    {
        open(source, "/bglscratch", prefix);
        return;
    }
    else
    {
#ifdef BBP_BGLSCRATCH_REPLACEMENT_PREFIX
#define BBP_STRINGIFY2(x) # x
#define BBP_STRINGIFY(x) BBP_STRINGIFY2(x)
        open(source, "/bglscratch", 
            BBP_STRINGIFY(BBP_BGLSCRATCH_REPLACEMENT_PREFIX));
#undef BBP_STRINGIFY
#undef BBP_STRINGIFY2
#else        
        close();
        try 
        {
            // Use the ExperimentReader to load the target and store the 
            // report metadata.
            Experiment_Reader reader(source);
            reader.read(*this);

            // Initialize the mircrocircuit for loading.
            _microcircuit = Microcircuit::create();
            _microcircuit->open(*this);
            
            //If the experiment declares a user target file, read it in now
            // read user targets if any
            reader.load_user_targets(*this);
            _is_open = true;
        } 
        catch (...) 
        {
            close();
            throw;
        }
#endif
    }
}

// ----------------------------------------------------------------------------

void Experiment::close()
{
    _microcircuit.reset();
    //! \todo Clear report container once a report container is existing.
    //_reports.clear();
    _reports_specification.clear();
    _user_targets.clear();
    Experiment_Specification::clear();
    _is_open = false;
    
}

}
