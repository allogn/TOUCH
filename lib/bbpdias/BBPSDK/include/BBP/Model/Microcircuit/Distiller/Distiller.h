/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2007-2008. All rights reserved.

        Responsible authors:   John Kenyon 
                                
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_DISTILLER_H
#define BBP_DISTILLER_H

namespace bbp {

class Distiller;

}


#include <cctype>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

#include "BBP/Common/Types.h"
#include "BBP/Common/Exception/Exception.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"
#include "BBP/Model/Microcircuit/Microcircuit.h"
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Microcircuit/Targets/Target.h"
#include "BBP/Model/Microcircuit/Targets/Targets.h"
#include "BBP/Model/Microcircuit/Neuron.h"
#include "BBP/Model/Microcircuit/Distiller/Volume_Comparators.h"

namespace bbp {

/*! The Distiller is an object which provides many tools for manipulating
    Target objects, such as boolean/set operations, random selections,
    volumetric searches, and various others.  It is designed to be used
    directly by the user (either in C++ or other language bindings) as a means
    of searching for specific cells or patterns of cells within a microcircuit.
*/
    class Distiller {
    public:
        Distiller(Microcircuit &microcircuit);

        ~Distiller();

        //! \name Utility functions
        //@{
        /*! Looks up a target by name. This is a shortcut to 
            microcircuit.targets.find(). */
        /*! This function is meant mainly for internal use, in the methods
            that accept either targets or names of targets.  However, it may
            be useful for some users in some cases, and it allows the user
            to do most of their target work through the distiller object
            which helps keep their code isomorphic.*/
        Target get_target                            (std::string name);
        //@}


        //! \name Split operations
        //@{
        /*! Creates a target for each cell represented in the input target, and
            returns all of these targets in a Targets object */
        Targets split_cells                          (const Target & SRC);

        /*! Creates a target for each cell represented in the input target, and
            returns all of these targets in a Targets object */
        Targets split_cells                          (std::string SRC);

        /*! Extracts subtargest from the input target object, and collects them
            into a Targets object */
        Targets split_subtargets                     (const Target & SRC);

        /*! Extracts subtargest from the input target object, and collects them
            into a Targets object */
        Targets split_subtargets                     (std::string SRC);
        //@}


        //! \name Synapse searches
        //@{
        /*! Create a target of all cells that connect to the source target
            cells.  Or, find presynaptic cells. */
        Target find_source_synapses                  (const Target & SRC);
            
        /*! Create a target of all cells that connect to the source target
            cells.  Or, find presynaptic cells. */
        Target find_source_synapses                  (std::string SRC);

        /*! Create a target of all cells that are connected to by the 
            source target cells.  Or, find post-synaptic cells. */
        Target find_destination_synapses             (const Target & SRC);
        
        /*! Create a target of all cells that are connected to by the 
            source target cells.  Or, find post-synaptic cells. */
        Target find_destination_synapses             (std::string SRC);

        /*! Create a target of all the cells that are both connected to
            and receive connections from the source target cells. */
        Target find_reciprocal_synapses              (const Target & SRC);

        /*! Create a target of all the cells that are both connected to
            and receive connections from the source target cells. */
        Target find_reciprocal_synapses              (std::string SRC);
        //@}


        //! \name Simple Retrieval
        //@{
        /*! Returns a target with a single cell taken from the "beginning"
            of the source target.  Note that the ordering of cells withing
            a target is not meaningful, so this is basically just a 
            "non-stochastic single sample" of the target. */
        Target select_first                          (const Target &SRC);

        /*! Returns a target with a single cell taken from the "beginning"
            of the source target.  Note that the ordering of cells withing
            a target is not meaningful, so this is basically just a 
            "non-stochastic single sample" of the target. */
        Target select_first                          (std::string SRC);
        //@}


        //! \name Random operations
        //@{
        /*! The Random probability functions find a random subset of the
            given target, returning a percentage of the input Target. */
        /*! At the moment, this uses the standard rand functions. It may
            be desirable in the future to allow an interface to a generic
            random number generator */
        Target random_probability                    (const Target & LHS,float percent, int seed = 0);

        /*! The Random probability functions find a random subset of the
            given target, returning a percentage of the input Target. */
        /*! At the moment, this uses the standard rand functions. It may
            be desirable in the future to allow an interface to a generic
            random number generator */
        Target random_probability                    (std::string LHS,float percent, int seed = 0);



        /*! The random absolute functions find a random subset of the
            given target, returning a specified number of cells.*/
        /*! At the moment, this uses the standard rand functions. It may
            be desirable in the future to allow an interface to a generic
            random number generator */
        Target random_absolute                       (const Target & LHS, int count, int seed = 0);
        /*! The random absolute functions find a random subset of the
            given target, returning a specified number of cells.*/
        /*! At the moment, this uses the standard rand functions. It may
            be desirable in the future to allow an interface to a generic
            random number generator */
        Target random_absolute                       (std::string LHS, int count, int seed = 0);
        //@}


        //! \name Type Cast operations
        //@{
        /*! The Cast operations (such as cast_to_neuron) create a new a new
            Target whose contents are the same as the source Target, but
            whose type, and the type of all subtargets, have been changed. */
        /*! This is not an intelligent conversion, and should only be used
            if you are absolutely sure that you know what you are doing.
            It is probable that you want to use one of the resolve_to_
            methods instead of the cast method. */
        Target cast_to_neuron                        (const Target & SRC);


        /*! The Cast operations (such as cast_to_neuron) create a new a new
            Target whose contents are the same as the source Target, but
            whose type, and the type of all subtargets, have been changed. */
        /*! This is not an intelligent conversion, and should only be used
            if you are absolutely sure that you know what you are doing.
            It is probable that you want to use one of the resolve_to_
            methods instead of the cast method. */
        Target cast_to_neuron                        (std::string);


        /*! The Cast operations (such as cast_to_section) create a new a new
            Target whose contents are the same as the source Target, but
            whose type, and the type of all subtargets, have been changed. */
        /*! This is not an intelligent conversion, and should only be used
            if you are absolutely sure that you know what you are doing.
            It is probable that you want to use one of the resolve_to_
            methods instead of the cast method. */
        Target cast_to_section                       (const Target & SRC);
        
        /*! The Cast operations (such as cast_to_section) create a new a new
            Target whose contents are the same as the source Target, but
            whose type, and the type of all subtargets, have been changed. */
        /*! This is not an intelligent conversion, and should only be used
            if you are absolutely sure that you know what you are doing.
            It is probable that you want to use one of the resolve_to_
            methods instead of the cast method. */
        Target cast_to_section                       (std::string);


        /*! The Cast operations (such as cast_to_compartment) create a new a new
            Target whose contents are the same as the source Target, but
            whose type, and the type of all subtargets, have been changed. */
        /*! This is not an intelligent conversion, and should only be used
            if you are absolutely sure that you know what you are doing.
            It is probable that you want to use one of the resolve_to_
            methods instead of the cast method. */
        Target cast_to_compartment                   (const Target & SRC);
        
        /*! The Cast operations (such as cast_to_compartment) create a new a new
            Target whose contents are the same as the source Target, but
            whose type, and the type of all subtargets, have been changed. */
        /*! This is not an intelligent conversion, and should only be used
            if you are absolutely sure that you know what you are doing.
            It is probable that you want to use one of the resolve_to_
            methods instead of the cast method. */
        Target cast_to_compartment                   (std::string);
        //@}




        //! \name Resolution operations
        //@{
        /*! The resolve_to_ methods (such as resolve_to_neuron) create a new
            Target of the desired type, whose contents are based on the
            associated elements of the source Target.  So if you resolve
            a Neuron/Cell Target to a Section target, the new Target will
            list all Section IDs for all Sections that are part of the
            Neurons in the source set.
            \todo Implement this method */
        Target resolve_to_neuron                     (const Target & SRC);

        /*! The resolve_to_ methods (such as resolve_to_neuron) create a new
            Target of the desired type, whose contents are based on the
            associated elements of the source Target.  So if you resolve
            a Neuron/Cell Target to a Section target, the new Target will
            list all Section IDs for all Sections that are part of the
            Neurons in the source set.
            \todo Implement this method */
        Target resolve_to_neuron                     (std::string);

        /*! The resolve_to_ methods (such as resolve_to_section) create a new
            Target of the desired type, whose contents are based on the
            associated elements of the source Target.  So if you resolve
            a Neuron/Cell Target to a Section target, the new Target will
            list all Section IDs for all Sections that are part of the
            Neurons in the source set.
            \todo Implement this method */
        Target resolve_to_section                    (const Target & SRC);

        /*! The resolve_to_ methods (such as resolve_to_section) create a new
            Target of the desired type, whose contents are based on the
            associated elements of the source Target.  So if you resolve
            a Neuron/Cell Target to a Section target, the new Target will
            list all Section IDs for all Sections that are part of the
            Neurons in the source set.
            \todo Implement this method */
        Target resolve_to_section                    (std::string);

        /*! The resolve_to_ methods (such as resolve_to_section) create a new
            Target of the desired type, whose contents are based on the
            associated elements of the source Target.  So if you resolve
            a Neuron/Cell Target to a Section target, the new Target will
            list all Section IDs for all Sections that are part of the
            Neurons in the source set.
            \todo Implement this method */
        Target resolve_to_compartment                (const Target & SRC);

        /*! The resolve_to_ methods (such as resolve_to_section) create a new
            Target of the desired type, whose contents are based on the
            associated elements of the source Target.  So if you resolve
            a Neuron/Cell Target to a Section target, the new Target will
            list all Section IDs for all Sections that are part of the
            Neurons in the source set.
            \todo Implement this method */
        Target resolve_to_compartment                (std::string);
        //@}



        //! \name Boolean or Set operations
        //@{
        /*! The add operation returns a target which contains all cells
            from both input targets. */
        /*! This operation is comperable to the set theory Union operation */
        /*! This operation is comperable to the boolean logic OR operation */
        Target add                                   (const Target & LHS, const Target & RHS);

        /*! The add operation returns a target which contains all cells
            from both input targets. */
        /*! This operation is comperable to the set theory Union operation */
        /*! This operation is comperable to the boolean logic OR operation */
        Target add                                   (std::string LHS, const Target & RHS);

        /*! The add operation returns a target which contains all cells
            from both input targets. */
        /*! This operation is comperable to the set theory Union operation */
        /*! This operation is comperable to the boolean logic OR operation */
        Target add                                   (const Target & LHS, std::string RHS);

        /*! The add operation returns a target which contains all cells
            from both input targets. */
        /*! This operation is comperable to the set theory Union operation */
        /*! This operation is comperable to the boolean logic OR operation */
        Target add                                   (std::string LHS, std::string RHS);


        /*! The intersection operation returns a target which contains only
            cells that exist in both of the input targets. */
        /*! This operation is comperable to the set theory Intersection operation */
        /*! This operation is comperable to the boolean logic AND operation */
        Target intersect                             (const Target & LHS, const Target & RHS);

        /*! The intersection operation returns a target which contains only
            cells that exist in both of the input targets. */
        /*! This operation is comperable to the set theory Intersection operation */
        /*! This operation is comperable to the boolean logic AND operation */
        Target intersect                             (std::string LHS, const Target & RHS);

        /*! The intersection operation returns a target which contains only
            cells that exist in both of the input targets. */
        /*! This operation is comperable to the set theory Intersection operation */
        /*! This operation is comperable to the boolean logic AND operation */
        Target intersect                             (const Target & LHS, std::string RHS);

        /*! The intersection operation returns a target which contains only
            cells that exist in both of the input targets. */
        /*! This operation is comperable to the set theory Intersection operation */
        /*! This operation is comperable to the boolean logic AND operation */
        Target intersect                             (std::string LHS, std::string RHS);



        /*! The subtract operation returns a target which contains the
            elements which exist in the first parameter, but not the
            second parameter. Thus it gives the effect of a difference
            between the targets. */
        Target subtract                              (const Target & LHS, const Target & RHS);

        /*! The subtract operation returns a target which contains the
            elements which exist in the first parameter, but not the
            second parameter. Thus it gives the effect of a difference
            between the targets. */
        Target subtract                              (std::string LHS, const Target & RHS);

        /*! The subtract operation returns a target which contains the
            elements which exist in the first parameter, but not the
            second parameter. Thus it gives the effect of a difference
            between the targets. */
        Target subtract                              (const Target & LHS, std::string RHS);

        /*! The subtract operation returns a target which contains the
            elements which exist in the first parameter, but not the
            second parameter. Thus it gives the effect of a difference
            between the targets. */
        Target subtract                              (std::string LHS, std::string RHS);



        /*! The exclusive operation returns a target which contains the
            elements which exist in one and only one of the input
            targets. */
        /*! This operation is comperable to the set theory  */
        /*! This operation is comperable to the boolean logic XOR operation */
        Target exclusive                             (const Target & LHS, const Target & RHS);

        /*! The exclusive operation returns a target which contains the
            elements which exist in one and only one of the input
            targets. */
        /*! This operation is comperable to the set theory  */
        /*! This operation is comperable to the boolean logic XOR operation */
        Target exclusive                             (std::string LHS, const Target & RHS);

        /*! The exclusive operation returns a target which contains the
            elements which exist in one and only one of the input
            targets. */
        /*! This operation is comperable to the set theory  */
        /*! This operation is comperable to the boolean logic XOR operation */
        Target exclusive                             (const Target & LHS, std::string RHS);

        /*! The exclusive operation returns a target which contains the
            elements which exist in one and only one of the input
            targets. */
        /*! This operation is comperable to the set theory  */
        /*! This operation is comperable to the boolean logic XOR operation */
        Target exclusive                             (std::string LHS, std::string RHS);
        //@}


        //! \name Classical Volume operations
        //@{
        /*! The select_cylindrical_volume operation will return a target
            containing those members from the source target which fall within
            the cylinder defined by the input parameters */
        /*! This method is intended to parallel the similar operation from
            the old distiller. */
        Target select_cylindrical_volume             (const Target & SRC, float x, float y, float z, float height, float radius);
        /*! The select_cylindrical_volume operation will return a target
            containing those members from the source target which fall within
            the cylinder defined by the input parameters */
        /*! This method is intended to parallel the similar operation from
            the old distiller. */
        Target select_cylindrical_volume             (std::string SRC, float x, float y, float z, float height, float radius);

        /*! The select_hexagonal_volume operation will return a target
            containing those members from the source target which fall within
            the hexagonal volume defined by the input parameters. */
        /*! This method is intended to parallel the similar operation from
            the old distiller. */
        Target select_hexagonal_volume               (const Target & SRC, float x, float y, float z, float height, float radius);

        /*! The select_hexagonal_volume operation will return a target
            containing those members from the source target which fall within
            the hexagonal volume defined by the input parameters. */
        /*! This method is intended to parallel the similar operation from
            the old distiller. */
        Target select_hexagonal_volume               (std::string SRC, float x, float y, float z, float height, float radius);
        //@}


        //! \name Morphology operations
        //@{
        /*! The list_morphologies operation returns a set of all morphology
            types contained within the input target. 
            \return a list of Labels (std::string) */
        std::set<Label> list_morphologies            (const Target &SRC);

        /*! The list_morphologies operation returns a set of all morphology
            types contained within the input target. 
            \return a list of Labels (std::string) */
        std::set<Label> list_morphologies            (std::string SRC);



        /*! The list_morphologies operation returns a set of all morphology
            types contained within the input target. 
            \return a list of Morphology_Type_ID */
        std::set<Morphology_Type_ID> list_morphology_ids  (const Target &SRC);

        /*! The list_morphologies operation returns a set of all morphology
            types contained within the input target. 
            \return a list of Morphology_Type_ID */
        std::set<Morphology_Type_ID> list_morphology_ids  (std::string SRC);



        /*! The filter_morphology operation will return a new target
            which contains only cells of the given morphology. */
        Target filter_morphology                     (const Target &SRC,Label key);

        /*! The filter_morphology operation will return a new target
            which contains only cells of the given morphology. */
        Target filter_morphology                     (std::string   SRC,Label key);

        /*! The filter_morphology operation will return a new target
            which contains only cells of the given morphology. */
        Target filter_morphology                     (const Target &SRC,Morphology_Type_ID key);

        /*! The filter_morphology operation will return a new target
            which contains only cells of the given morphology. */
        Target filter_morphology                     (std::string   SRC,Morphology_Type_ID key);

        /*! The filter_morphology operation will return a new target
            which contains only cells of the given morphology. */
        Target filter_morphology                     (const Target &SRC,Morphology_Type key);

        /*! The filter_morphology operation will return a new target
            which contains only cells of the given morphology. */
        Target filter_morphology                     (std::string   SRC,Morphology_Type key);
        

        // Unique Morph
        /*! The unique_morphologies operation is used to get a sample of cells
            that spans the morphological space of the given input target.
            In other words, it gets one cell of each morphology type.*/
        Target unique_morphologies                   (const Target & RHS);

        /*! The unique_morphologies operation is used to get a sample of cells
            that spans the morphological space of the given input target.
            In other words, it gets one cell of each morphology type.*/
        Target unique_morphologies                   (std::string RHS);

        /*! The unique_morphologies operation is used to get a sample of cells
            that spans the morphological space of the given input target.
            In other words, it gets one cell of each morphology type.*/
        Targets unique_morphologies                  (const Targets &RHS);

        /*! The unique_morphologies operation is used to get a sample of cells
            that spans the morphological space of the given input target.
            In other words, it gets one cell of each morphology type.*/
        Targets unique_morphologies                  (std::vector<Target> &RHS);
        //@}



        //! \name MorphoElectric operations
        //@{
        /*! The list_morphoelectric_types operation returns a set of all
            morphoelectric types contained within the input target.
            \return a list of Labels (std::string) */
        std::set<Label> list_morphoelectric_types    (const Target &SRC);

        /*! The list_morphoelectric_types operation returns a set of all
            morphoelectric types contained within the input target.
            \return a list of Labels (std::string) */
        std::set<Label> list_morphoelectric_types    (std::string SRC);


        /*! The list_morphoelectric_types operation returns a set of all 
            morpholoelectric types contained within the input target.
            \return a list of Morphoelectric_Type_ID */
        std::set<Electrophysiology_Type_ID> 
            list_morphoelectric_type_ids             (const Target &SRC);

        /*! The list_morphoelectric_types operation returns a set of all 
            morpholoelectric types contained within the input target.
            \return a list of Morphoelectric_Type_ID */
        std::set<Electrophysiology_Type_ID> 
            list_morphoelectric_type_ids             (std::string SRC);


        // Unique ME
        /*! The unique_morphoelectric_types operation is used to get a 
            sample of cells that spans the morphoelectric space of the 
            given input target. In other words, it gets one cell of each 
            morphology type.*/
        Target  unique_morphoelectric_types          (const Target & RHS);
    
        /*! The unique_morphoelectric_types operation is used to get a 
            sample of cells that spans the morphoelectric space of the 
            given input target. In other words, it gets one cell of each 
            morphology type.*/
        Target  unique_morphoelectric_types          (std::string RHS);
    
        /*! The unique_morphoelectric_types operation is used to get a 
            sample of cells that spans the morphoelectric space of the 
            given input target. In other words, it gets one cell of each 
            morphology type.*/
        Targets unique_morphoelectric_types          (const Targets &RHS);
    
        /*! The unique_morphoelectric_types operation is used to get a 
            sample of cells that spans the morphoelectric space of the 
            given input target. In other words, it gets one cell of each 
            morphology type.*/
        Targets unique_morphoelectric_types          (std::vector<Target> &RHS);


        /*! The filter_morphoelectric operation will return a new target
            which contains only cells of the given morphoelectric type. */
        Target filter_morphoelectric                 (const Target &SRC,Label key);

        /*! The filter_morphoelectric operation will return a new target
            which contains only cells of the given morphoelectric type. */
        Target filter_morphoelectric                 (std::string   SRC,Label key);

        /*! The filter_morphoelectric operation will return a new target
            which contains only cells of the given morphoelectric type. */
        Target filter_morphoelectric                 (const Target &SRC,Electrophysiology_Type_ID key);

        /*! The filter_morphoelectric operation will return a new target
            which contains only cells of the given morphoelectric type. */
        Target filter_morphoelectric                 (std::string   SRC,Electrophysiology_Type_ID  key);

        /*! The filter_morphoelectric operation will return a new target
            which contains only cells of the given morphoelectric type. */
        Target filter_morphoelectric                 (const Target &SRC,Electrophysiology_Type key);

        /*! The filter_morphoelectric operation will return a new target
            which contains only cells of the given morphoelectric type. */
        Target filter_morphoelectric                 (std::string   SRC,Electrophysiology_Type key);
        //@}
    

    public:
        /*! Utility functions for the historical volumetric functions.  These
            will never be called by the user.*/
        static bool compare_point_to_cylinder        (float px, float py, float pz, float cx, float cy, float cz, float height, float radius);
        
        /*! Utility functions for the historical volumetric functions.  These
            will never be called by the user.*/
        static bool compare_point_to_hexagon         (float px, float py, float cx, float cy, float height, float radius);
        
        /*! Utility functions for the historical volumetric functions.  These
            will never be called by the user.*/
        static bool compare_point_to_hexagonal       (float px, float py, float pz, float cx, float cy, float cz, float height, float radius);
        
        /*! Utility functions for the historical volumetric functions.  These
            will never be called by the user.*/
        static bool compare_point_to_triangle        (float px, float py, float tx1, float ty1, float tx2, float ty2, float tx3, float ty3);

        //! \name Volume operations
        //@{
        /*!

        */
        Target select_volume_comparator(const Target &,Volume_Comparator &);
        /*!

        */
        Target select_volume_comparator(std::string   ,Volume_Comparator &);
        //@}



    public:
        /*!

        */
        static Target s_add                          (const Target & LHS, const Target & RHS);
        static Target s_intersect                    (const Target & LHS, const Target & RHS);
        static Target s_subtract                     (const Target & LHS, const Target & RHS);
        static Target s_exclusive                    (const Target & LHS, const Target & RHS);


        static Target s_cast_to_neuron               (const Target & SRC);
        static Target s_cast_to_section              (const Target & SRC);
        static Target s_cast_to_compartment          (const Target & SRC);


    private:
        /*! This is an internal class used to prevent repetition of
            some code that may need to be changed.  As of writing this
            we want to garantee (sp?) that all Targets are flat before
            we do most operations.  By having this "preparation" step
            set for all methods we can easily change this behavior,
            depending on future requirements or optimizations. */
        /*! For example.  If in the future the Target class is modified
            to remove our constraint that Targets be flattened first,
            we can change this function to say "return SRC;" and it will
            update all Distiller operations.  Alternatively, if we add
            the constraint that all Distiller operations should require
            that the Target be sorted, we can add a sort step to this
            prep function.  But be careful, since inline functions can
            be tricky to debug! */
        inline Target prep_target(Target SRC)
        {
            return SRC.flatten();
        }

    private:
        /*! This microcircuit is used to access information about the cells,
            such as location, morphology, synapses and connectivity. */
        /*! Technically we could request a microcircuit object with every
            function call, and thus make the Distiller a set of functions,
            relieving us of needing to store any kind of state at all.
            However, since the majority of the distiller operations require
            a microcircuit object, it is much cleaner to use the distiller
            as a class which stores the microcircuit for the duration of its
            lifetime. */
        Microcircuit *_microcircuit;
    };
}


#endif

