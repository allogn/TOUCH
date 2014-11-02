/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Juan Hernando Vieites
                                Sebastien Lasserre

*/


#ifndef BBP_MICROCIRCUIT_H
#define BBP_MICROCIRCUIT_H

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "Types.h"
#include "BBP/Common/Exception/Exception.h"
#include "BBP/Model/Microcircuit/Containers/Meshes.h"
#include "BBP/Model/Microcircuit/Targets/Targets.h"
#include "BBP/Model/Microcircuit/Datasets/Microcircuit_Dataset.h"
#include "BBP/Common/Patterns/shared_ptr_helper.h"
#include "Recipe_XML_File_Parser.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Experiment;
class Microcircuit;
class Neuron;
class Neurons;
class Synapse;
class Morphologies;
class Mesh;

class Distiller;
class Experiments;
class Synapses;

class Microcircuit_Dataset;
class Structure_Dataset;
class Dynamics_Dataset;
class Microcircuit_Reader;

//! smart pointer to a synapses container
typedef boost::shared_ptr<Synapses> Synapses_Ptr;
//! smart pointer to a neurons container
typedef boost::shared_ptr<Neurons> Neurons_Ptr;
//! smart pointer to a const neuron
typedef boost::shared_ptr<Neuron> Neuron_Ptr;
//! smart pointer to a neuron
typedef boost::shared_ptr<const Neuron> Const_Neuron_Ptr;
//! smart pointer to a microcircuit object
typedef boost::shared_ptr<Microcircuit> Microcircuit_Ptr;
//! smart weak pointer to a microcircuit object
typedef boost::weak_ptr<Microcircuit> Microcircuit_Weak_Ptr;
//! smart pointer to a read only microcircuit object
typedef boost::shared_ptr<const Microcircuit> Const_Microcircuit_Ptr;
//! smart pointer to a microcircuit reader object
typedef boost::shared_ptr<Microcircuit_Reader> Microcircuit_Reader_Ptr;
//! smart pointer to a morphologies container
typedef boost::shared_ptr<Morphologies> Morphologies_Ptr;



// ----------------------------------------------------------------------------

//! Interface to the physical structure and dynamics of a microcircuit.
/*!
    The microcircuit class provides access to static and dynamic simulation 
    data related to the logical microcircuit structure. Inherits the Neurons 
    selection interface.
    \todo In all microcircuit classes evaluate to replace shared_ptr<T> with 
    weak_ptr<T> in microcircuit classes to avoid potential memory leaks. (JH)
    \todo The experiment() function needs to be extended, many experiments 
    could be associated with the same microcircuit. Further, I would
    prefer functions in general to return a reference instead of 
    pointers and throw an exception if the pointer is invalid if at 
    all possible. (TT)
    \todo remove const versions of non const functions since currently we 
    are not to support READ_ONLY for the moment (JH/TT)
    \todo support for sharing a microcircuit between experiments
    \todo test code cleanup
    \todo Check the implications on existing code of the clarification that 
    Dataset_Morphology::_point_relative_distances is a normalized section 
    relative distance, not in micron length unit. Existing filter code using 
    morphology may or may not be affected but we need to review. (TT)
    \todo Move inline specifiers from function declaration to function 
    definition. (TT)
    \todo Profile performance and memory usage of library code. (TT)
    \todo Evaluate Intel compiler benefits and drawbacks. (SL/JH/TT)
    \ingroup Microcircuit Laboratory
*/
class Microcircuit 
    : private boost::noncopyable
{
    friend class Neuron;
    friend class Neurons;
    friend class Morphology;
    friend class Morphologies;
    friend class Section;
    friend class Sections;
    friend class Segment;
    friend class Segments;
    friend class Compartment;
    //friend class Compartments;
    friend class Synapse;
    friend class Synapses;
    friend class Microcircuit_Reader;
    friend std::ostream & operator << 
        (std::ostream & lhs, const Microcircuit & rhs);

public:
    //! Creates a microcircuit returning it with a smart pointer
    /*! The smart pointer is linked internally with a weak pointer. This
        allows some objects to get a shared pointer from the microcircuit
        object directly. */
    static Microcircuit_Ptr create();

    //! Simple constructor.
    /*! This constructor is provided for on stack construction or when
        constructor a default constructor is needed (i.e. some templated 
        code). */
    Microcircuit();

public:
    //! Print microcircuit state information to standard output.
    void                        print() const;


    // DISTILLER --------------------------------------------------------------

    Distiller distiller();

    // STRUCTURE ______________________________________________________________
    
    /*!
        \name Structures
        Access to biological structures of the microcircuit.
    */
    //@{
    //! Get neuron via global identifier (read-only). 
    /*! Exception is thrown if GID not found. */
    const Neuron &              neuron(Cell_GID gid) const;
    /*!
      Returns the cell target in the default target set with the given
      target name.
      Exception thrown is target is not a cell target or it doesn't exist.
     */
    Cell_Target                 cell_target(const Label & target_name) const;
        /* throw Neuron_Not_Found */ 
    //@}

    /*!
        \name Structure containers
        Access to containers of biological structures in the microcircuit.
    */
    //@{
    //! Get all neurons loaded in the microcircuit (read only).
    const Neurons &             neurons() const;
    //! Default circuit targets.
    const Targets &             targets() const;
    //! Get all unique morphologies loaded the microcircuit (read only).
    const Morphologies &        morphologies() const;
    ////! Get all synapses loaded in the microcircuit (read only).
    //const Synapses            synapses() const;
    //! Get all unique meshes loaded in the microcircuit (read only).
    const Meshes &              meshes() const;
    ////! Get experiments associated with this microcircuit.
    //const Experiments & experiments() const;
    //@}


    // MODIFIABLE _____________________________________________________________

    /*!
        \name Structure
        Full access to biological structures of the microcircuit.
    */
    //@{
    //! Get neuron via global identifier (modifiable). 
    /*! Exception is thrown if GID not found. */
    Neuron &                    neuron(Cell_GID gid);
    //@}

    /*!
        \name Structure containers
        Full access to containers of biological structures in the microcircuit.
    */
    //@{
    //! Get all neurons loaded in the microcircuit (modifiable).
    Neurons &               neurons();
    ////! Get all synapses loaded in the microcircuit (modifiable).
    //Synapses              synapses();
    //! Get all unique morphologies loaded in the microcircuit (modifiable).
    Morphologies &          morphologies();
    //! Get all unique meshes loaded in the microcircuit (modifiable).
    Meshes &                meshes();
    //@}

    // READER INTERFACE _______________________________________________________

    /*!
        \name Dataset
        Loads and manages the microcircuit.
    */
    //@{
    //! Opens the microcircuit using the data sources specified in the
    //!  given experiment.
    /*!
      If a microcircuit was already open it is first closed before
      proceeding.
      If any exception occurs this object will be left in the same state
      that the default constructor does.
    */
    void open(const Experiment & experiment);

    //! Opens the microcircuit using a specific reader. 
    /*! 
        This overloaded function works as the Experiment version and is intended 
        to simplify data loading in very specific cases (i.e unit tests).
        Use the Experiment based version as the preferred method for opening
        a microcircuit. 
    */
    void open(const Microcircuit_Reader_Ptr & reader);

    //! Frees all resources used by this object and its contents.
    /*! This microcircuit will remain unusable until open is called again. */
    void close();
  
    //! Loads the datasets specified in the options flag set for given target.
    /*!
        Only shared synapses are loaded (i.e. synapses between the neurons in
        the specified cell target. If all synapses should be loaded, currently
        the whole microcircuit target "Column" needs to be specified, but not
        all attributes need to be loaded then.

        In the future a Connection_Target could help out here that specifies 
        which synapses are to be loaded (e.g. only incoming synapses, outgoing 
        synapses, shared synapses, synapses to layer IV, ...).

        Loading new neurons invalidates the current compartment report frame
        binding.

        \todo Review loading flag system for more robust intuitive usage. (TT)
    */
    void load(const Cell_Target & target,
              Microcircuit_Loading_Flag_Set data_types = MICROCIRCUIT);

    //! Loads the datasets specified in the options flag set for given target.
    /*!
        Equivalent to the old load function, but accepts a Target
        instead of a Cell_Target.
    */
    void load(Target & target,
              Microcircuit_Loading_Flag_Set data_types = MICROCIRCUIT);


    //! Deallocates all internal data structures and objects.
    /*! The microcircuit doesn't need to be open again to load new data,
        however, using any previous data from this microcircuit will have
        undefined behaviour.
        \todo future version will support selective unloading. (JH) 
    */

    
    void unload();

    //! Bind a compartment report frame.
    /*!
        The binding will be valid until new neurons are loaded or unload
        is called.
    */
    void update(const Compartment_Report_Frame<Millivolt> & frame);

    // ________________________________________________________________________

    //! Get low-level access to microcircuit dataset (read-only).
    const Microcircuit_Dataset & dataset() const;
    //! Get low-level access to microcircuit dataset (modifiable).
    Microcircuit_Dataset &       dataset();
    //@}

protected:
    //! Get neuron pointer via global identifier (read-only).
    /*! If neuron is not found a null pointer will be returned */
    // A null pointer is easier to check that try .. catch and not finding
    // a neuron is not necessary a programming error. When we return a 
    // reference it's different since there is not null reference concept (JH).
    Const_Neuron_Ptr                    neuron_ptr(Cell_GID gid) const;
        /* throw Neuron_Not_Found */
    //! Get neuron pointer via global identifier (modifiable).
    /*! If neuron is not found a null pointer will be returned */
    // A null pointer is easier to check that try .. catch and not finding
    // a neuron is not necessary a programming error. When we return a 
    // reference it's different since there is not null reference concept (JH).
    Neuron_Ptr                          neuron_ptr(Cell_GID gid);
        /* throw Neuron_Not_Found */

    //! Dataset linking reports of simulated dynamics.
    Microcircuit_Dataset                _dataset;
    //! Reader interface choosing appropriate reader for specified source.
    Microcircuit_Reader_Ptr             _reader;

    //! All loaded neurons in this microcircuit.
    Neurons_Ptr                         _neurons;
    //! All loaded morphologies in this microcircuit.
    Morphologies_Ptr                    _morphologies;
    //! All loaded meshes in this microcircuit
    Meshes                              _meshes;
    //! Default circuit targets
    Targets                             _targets;

    ////! this vector links experiments done with this microcircuit
    //Experiments                       _experiments;

    Cell_Target                         _currently_bound_cell_target;

    /*! This pointer is used by friend classes and by the implementation to 
        get a shared_pointer from this. This is pointer is always guaranteed
        to return a valid shared_ptr when lock is called. */
    Microcircuit_Weak_Ptr               _shared_this;
    /*! This is a helper for stack constructec object.
        For stack constructed objects, this objects guaratees that
        _shared_this.lock() returns valid a shared_ptr and avoids 
        leaking the shared_count object. For objects created with create()
        it will be a null pointer. 
        The deallocator associated to this pointer never will call the 
        class destructor. */
    Microcircuit_Ptr               _strong_shared_this;
    
};

//! output stream of microcircuit state information
std::ostream & operator << (std::ostream & lhs, const Microcircuit & rhs);


// ----------------------------------------------------------------------------

}
#endif
