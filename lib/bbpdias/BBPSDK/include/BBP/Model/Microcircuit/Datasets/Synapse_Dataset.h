/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2007. All rights reserved.

        Responsible authors:	Sebastien Lasserre
								Thomas Traenkler
		Contributing authors:	Juan Hernando Vieites


    Check and update this table!

        This class stores de data for the synapses in a flat struture.
    each synapse is described by :
      - type                             : 2 bytes (Synapse_Type)
      - morphology type                  : 2 bytes (Synapse_Morphology_Type)
      - local position                  : 3 * 4 bytes (Micron)
      - postsynaptic neuron index		 : 4 bytes (Cell_GID)
      - presynaptic neuron index         : 4 bytes (Cell_GID)
      - postsynaptic section index       : 2 bytes (Section_ID)
      - presynaptic section              : 2 bytes (Section_ID)
      - postsynaptic segment index       : 2 bytes (Segment_ID)
      - presynaptic segment index        : 2 bytes (Segment_ID)
      - postsynaptic relative distance   : 2 bytes (Micron)
      - presynaptic relative distance    : 2 bytes (Micron)
      - conductance                      : 4 bytes (Nanosiemens)
      - utilization                      : 4 bytes (Probability)
      - depression                       : 2 bytes (Micron_Constant)
      - facilitation                     : 2 bytes (Micron_Constant)
      - DTC                              : 4 bytes (Millisecond)
      - section order dendrite           : 2 bytes (Section_Branch_Order)
      - section order axon               : 2 bytes (Section_Branch_Order)
 -----------------------------------------------------------------------------
                                           maximum 56 bytes stored per synapse
  
      1.68 Gigabytes of memory are required to store 30 million synapses with
	  all synapse attributes (without neuron connection information). (SL)

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_SYNAPSE_DATASET_H
#define BBP_SYNAPSE_DATASET_H

#include <boost/shared_ptr.hpp>
#include <vector>
#include "BBP/Model/Microcircuit/Types.h"

namespace bbp 
{   

// ----------------------------------------------------------------------------

class Synapse_Dataset;
typedef boost::shared_ptr<Synapse_Dataset> Synapse_Dataset_Ptr;

// ----------------------------------------------------------------------------

//! Internal dataset storing the synapse information in plain arrays.
/*!
	This dataset is referenced from the microcircuit interface objects
	and used as central internal memory of the loaded synapse information. 

	Currently the whole dataset has to be rebuilt when changing the loaded
	data (e.g. during unloading of loaded or loading of additional data).
	
	DISCLAIMER: This is an implementation detail that may be subject to 
	change, so do not access this dataset directly if you don't want to 
	rewrite your code when the dataset implementation changes.

	\todo Currently this structure is used to keep backwards compatibility with
	Gabriel Studio, but should change for more efficient unloading of data. The
	new version could in the worst case return a backwards compatible copy on
	request. (TT/JH)

    \todo 18.12.07 : changed the neuron => CAUTION : need to update 
					  GabrielStudio with this new GID (SL)
    \todo - 18.12.07 : removed function computeSynapsePositions(), 
					  transformToGlobalReferentiel()
                        => CAUTION : should be implemented in Reader (SL)

    \todo swap() for freeing memory with clear(). (TT)

	\todo The unused dataset locations has an issue: What if two Synapse 
	objects point to the same location? No reference count, will affect
	Synapse_HDF5_File_Reader temp Synapse objects. (TT)

	\todo Check if the storage table above is complete, e.g. delays stored 
            but not included in the calculation. (TT)

    \sa Synapse, Synapses, Neuron, Microcircuit, Dataset_Synapse_Accessor
 */
class Synapse_Dataset
{
	friend class Synapse;
	friend class Synapses;

public :

	// CONSTRUCTION ___________________________________________________________

	//! Create dataset and set it to store specified synapse properties.
	inline Synapse_Dataset(Synapse_Attributes_Flags flags = 
        SYNAPSE_NO_ATTRIBUTES);
	//! Virtual destructor that can be reimplemented for derived classes.
	inline virtual ~Synapse_Dataset() {}

    // ________________________________________________________________________

    //! Get which synapse attributes are stored in this dataset.
	inline Synapse_Attributes_Flags attributes() const;
    //! Set which synapse attributes are stored in this dataset.
	inline void attributes(Synapse_Attributes_Flags new_attributes);

	// CONTAINER ITERATION ____________________________________________________

	//class const_iterator;
	//class iterator;

	//const_iterator begin() const;
	//const const_iterator end() const;
	//
	//iterator begin();
	//const iterator end();

	// CONTAINER MODIFICATION _________________________________________________

	//! Add one synapse to the container
	/*!
		Looks for the first free place in the container and registers new
		synapse there.
		\return Returns the array index at which the synapse was inserted.
	*/
	inline Synapse_Index			insert();
	//! Add one synapse to the container
	/*!
		Adds one element to the end of all activated arrays and registers
		synapse there.
		\return Returns the array index at which the synapse was inserted.
	*/
	inline Synapse_Index			push_back();
	
    //! \todo Commented during the SWIG wrapping because not implmented (SL)
    // //! Labels dataset storage associated with one synapse as unused.
	// /*!
	/*	Note this does not free memory, only mark the location as unused
		so it can be recycled. 
		
		\todo A defrag() function could help, but when
		changing the array sizes, the indices would change and would need
		to be updated, thus freeing memory for a part of the synapses is 
		currently not supported - use clear() instead, but this is also 
		currently not guaranteed to free memory, a free() function should
		be added that swaps the arrays. This should wait until we decided
		on the final synapse dataset design. Potential candidates are two
		dimensional arrays (neuron index, synapse index) or a hash table.
		(TT)*/
	// */
	//void erase(Synapse_Index index);

	//! Resize the synapse dataset (can only increase size of dataset).
	/*!
		If you would like to free memory, use clear().		
		\sa Synapse, Synapses, Neuron
	*/
    inline void						reserve(size_t dataset_size);
	//! Clears the synapse dataset (drops all synapse information).
	inline void						clear();


	// CONTAINER STATUS _______________________________________________________

	//! Get number of synapses that the dataset can hold currently.
	/*!
		This number is the size of all arrays that are loaded
		\sa Synapse_Attributes
		\return number of synapses stored in the dataset
	*/
    inline size_t					size() const; 

	// READ-ONLY ACCESS _______________________________________________________

	//! Get array with types for all loaded synapses.
	inline const Synapse_Type_ID *		types()const;    
	//! Get array with synapse conductance constants for all loaded synapses.
	inline const Nanosiemens *		conductances() const;
	//! Get array with utilization probability constants for all loaded synapses.
    inline const Probability *		utilizations() const;
	//! Get array with synapse short term plasticity depression time constants 
	//! for all loaded synapses.
	inline const Millisecond_Time_Constant * depressions() const;
	//! Get array with synapse short term plasticity facilitation time constants 
	//! for all loaded synapses.
    inline const Millisecond_Time_Constant * facilitations() const;
    //! Get array with absolute synaptic efficacies for synapses loaded.
    inline const Millivolt *            efficacies() const;
    //! Get array with decay time constants for the post synaptic potential
	//! for all loaded synapses.
	inline const Millisecond *		    decays() const;
    //! Get array with transmission delays for all loaded synapses.
	/*!
		\return Array of synapse transmission delay in milliseconds.
				The size of the array is defined by size().
		\sa size()
	*/
	inline const Millisecond *		    delays() const;

	// STRUCTURE ______________________________________________________________

    ////! Get array with local 3D positions for synapses loaded.
    //inline const Vector_3D<Micron> *	positions() const;
	////! Get Micron array with local 3D positions for all loaded synapses.
	///*
	//	\return Array of local 3D coordinates in micron.
	//			Format: xyzxyzxyz... with length = 3 * size().
	//*/
	//   inline const Micron *			    raw_positions() const;    


    //! Get array with presynaptic neurons for all loaded synapses. 
	/*!
		\return Array of global cell identifiers of length size().
	*/
    inline const Cell_GID *			presynaptic_neuron_of_synapses() const;
    //! Get array of section indices on the presynaptic neuron.
	/*!
		\return Array of section indices on the presynaptic neuron 
				where the synapse is located on for all loaded synapses.
	*/
    inline const Section_ID *	presynaptic_section_ids() const;
	//! Get array with segment point indices of the synapse on the presynaptic 
	//! neuron for all loaded synapses. 
    inline const Morphology_Point_ID * presynaptic_segment_ids() const;
    //! Get array with micron distance in the segment of the presynaptic
	//! neuron for all loaded synapses.
    inline const Micron * presynaptic_segment_distances()
		const;
    
	//! Get array with postsynaptic neurons for all loaded synapses.
	/*!
		\return Array of global cell identifiers of length size().
	*/
    inline const Cell_GID *			postsynaptic_neuron_of_synapses() const;
	//! Get array of section indices on the postsynaptic neuron.
	/*
		\return Array of section indices on the presynaptic neuron 
				where the synapse is located on for all loaded synapses.
	*/
    inline const Section_ID *	postsynaptic_section_ids() const;
   	//! Get array with segment point indices of the synapse on the postsynaptic 
	//! neuron for all loaded synapses. 
    inline const Morphology_Point_ID * postsynaptic_segment_ids() const;
   	//! Get array with micron distance in the segment of the postsynaptic
	//! neuron for all loaded synapses.
    inline const Micron * postsynaptic_segment_distances()
        const;

	// DYNAMICS _______________________________________________________________

	//! Get synapse type.
	inline Synapse_Type_ID & type(size_t index);    
	//! Get synapse conductance constant.
	inline Nanosiemens & conductance(size_t index);
	//! Get utilization probability constant.
    inline Probability & utilization(size_t index);
	//! Get synapse short term plasticity depression time constant.
	inline Millisecond_Time_Constant & depression(size_t index);
	//! Get synapse short term plasticity facilitation time constant. 
	inline Millisecond_Time_Constant & facilitation(size_t index);
    //! Get absolute synaptic efficacy.
    inline Millivolt & efficacy(size_t index);
	//! Get array with decay time constants for the post synaptic potential
	//! for all loaded synapses.
	inline Millisecond & decay(size_t index);
	//! Get array with delays for all loaded synapses.
	/*!
		\return Array of synapse transmission delay in milliseconds.
				The size of the array is defined by size().
		\sa size()
	*/
	inline Millisecond & delay(size_t index);


	// STRUCTURE ______________________________________________________________

    ////! Get soma relative 3D position of synapse (not translated or rotated).
    //inline Vector_3D<Micron> & position(size_t index);

    //! Get array with presynaptic neurons for all loaded synapses. 
	/*!
		\return Array of global cell identifiers of length size().
	*/
    inline Cell_GID & presynaptic_neuron_of_synapse(size_t index);
    //! Get array of section indices on the presynaptic neuron.
	/*
		\return Array of section indices on the presynaptic neuron 
				where the synapse is located on for all loaded synapses.
	*/
    inline Section_ID & presynaptic_section_id(size_t index);
    //! Get array with segment point indices of the synapse on the presynaptic 
	//! neuron for all loaded synapses. 
    inline Morphology_Point_ID & presynaptic_segment_id(size_t index);
    //! Get array with micron distance in the segment of the presynaptic
	//! neuron for all loaded synapses.
    inline Micron & presynaptic_segment_distance(size_t index);
	
	// ________________________________________________________________________

	//! Get array with postsynaptic neurons for all loaded synapses.
	/*!
		\return Array of global cell identifiers of length size().
	*/
    inline Cell_GID & postsynaptic_neuron_of_synapse(size_t index);
   	//! Get array of section indices on the postsynaptic neuron.
	/*
		\return Array of section indices on the postsynaptic neuron 
				where the synapse is located on for all loaded synapses.
	*/
    inline Section_ID & postsynaptic_section_id(size_t index);
   	//! Get array with segment point indices of the synapse on the postsynaptic
	//! neuron for all loaded synapses. 
    inline Morphology_Point_ID & postsynaptic_segment_id(size_t index);
   	//! Get array with micron distance in the segment of the postsynaptic
	//! neuron for all loaded synapses.
    inline Micron & postsynaptic_segment_distance(size_t index);

protected :

	// DATASET MANAGEMENT _____________________________________________________

	////! Register that keeps track which array locations are free.
	///*!
	//	If false, the memory location is unused, otherwise it is used.

	//	DISCLAIMER: The std::vector<bool> stores boolean values as individual
	//	bits which has some side effects when not properly used as it is not 
	//	a real container. In this case it is desired (as it reduces the memory
	//	needed to keep track of 40 million synapses for a single column by a 
	//	significant factor (1 bit compared to 1 byte - factor of 8). (TT)
	//*/
	//std::vector<bool>						_used_memory_locations;
	//! Defines which flags are stored in the dataset.
	Synapse_Attributes_Flags                _attributes;
    //! Total number of synapses the dataset can store currently.
	/*!
		All synapse arrays are the same size or of size zero if attribute 
		not loaded.
	*/
    size_t                                  _dataset_size;
	////! References all microcircuits that store synapse info in this dataset.
	//std::vector<Microcircuit *>			_microcircuits;

	// NEURON CONNECTIONS _____________________________________________________


	// STRUCTURE ______________________________________________________________

 //   //! position of each synapse. Size = numSynapses*3, stored x,y,z,x,y,z, ...
	//std::vector<Vector_3D<Micron> >			_positions;

    // SYNAPSE LEVEL CONNECTION

    // PRESYNAPTIC
	//! presynaptic GID of the neuron connected to the synapes
    std::vector<Cell_GID>					_presynaptic_connection_of_synapses;
    //! relative section for each synapse in the presynaptic neuron
    std::vector<Section_ID>					_presynaptic_section_ids;
    //! relative segment in a section for each synapse in preynaptic neuron
    std::vector<Segment_ID>					_presynaptic_segment_ids;
    //! micron distance of synapse from start of the presynaptic segment 
    std::vector<Micron>                     _presynaptic_segment_distances;

    // POSTSYNAPTIC
	//! postsynaptic GID of the neuron connected to the synapes
	std::vector<Cell_GID>               _postsynaptic_connection_of_synapses;
    //! relative section for each synapse in the postsynaptic neuron
    std::vector<Section_ID>				_postsynaptic_section_ids;
    //! relative segment in a section for each synapse in postynaptic neuron
    std::vector<Segment_ID>				_postsynaptic_segment_ids;
    //! micron distance of synapse from start of the postsynaptic segment 
    std::vector<Micron>                 _postsynaptic_segment_distances;

    // NEURON LEVEL CONNECTION
    //! Total number of neurons dataset can store connections for currently.
	/*!
		All connection arrays are the same size or of size zero if attribute 
		not loaded.
	*/
	size_t									_number_of_neurons;
	//! Array of presynaptic neurons relative to specific neurons.
	std::vector<Cell_GID>					_presynaptic_connection_of_neurons;
	//! Array of postsynaptic neurons relative to specific neurons.
	std::vector<Cell_GID>					_postsynaptic_connection_of_neurons;

    // DYNAMICS _______________________________________________________________

	//! type of each synapse
	std::vector<Synapse_Type_ID>				_types;
	//! conductance constant for each synapse
	std::vector<Nanosiemens>				_conductances;
    //! probability of synaptic neurotransmitter release
	std::vector<Probability>				_utilizations;
    //! depression values of the synapses (integer value in the range 40-700)
	std::vector<Millisecond_Time_Constant>	_depressions;
    //! facilitation values of the synapses (integer value in the range 40-700)
    std::vector<Millisecond_Time_Constant>	_facilitations;
    //! absolute synaptic efficacies for synapses loaded.
    std::vector<Millivolt>                  _efficacies;
    //! postsynaptic potential decay time constant
	std::vector<Millisecond>			    _decays;
    //! delay 
	std::vector<Millisecond>			    _delays;

};


// ----------------------------------------------------------------------------

inline Synapse_Dataset::Synapse_Dataset(Synapse_Attributes_Flags flags)
: _attributes(flags),
  _dataset_size(0)
{
}

// ----------------------------------------------------------------------------

inline Synapse_Attributes_Flags Synapse_Dataset::attributes() const
{
    return _attributes;
}

// ----------------------------------------------------------------------------

inline void Synapse_Dataset::attributes(Synapse_Attributes_Flags
                                        new_attributes)
{
    _attributes = new_attributes;
}

// ----------------------------------------------------------------------------

//! \todo This function should take a synapse object as argument later on. (TT)
Synapse_Index Synapse_Dataset::push_back()
{
	//// Check which attributes are stored in the dataset and resize
	//// the corresponding arrays to the new size (all the same).
	//_used_memory_locations.push_back(true);

    // DYNAMICS _______________________________________________________________

    // type
	if (_attributes & SYNAPSE_TYPE)
		_types.push_back(UNDEFINED_SYNAPSE_TYPE_ID);
    // conductance
	if (_attributes & SYNAPSE_CONDUCTANCE)
		_conductances.push_back(UNDEFINED_NANOSIEMENS);
    // utilization
	if (_attributes & SYNAPSE_UTILIZATION)
		_utilizations.push_back(UNDEFINED_PROBABILITY);
    // facilitation
	if (_attributes & SYNAPSE_SHORT_TERM_FACILITATION)
		_facilitations.push_back(UNDEFINED_MILLISECOND_TIME_CONSTANT);
    // depression
	if (_attributes & SYNAPSE_SHORT_TERM_DEPRESSION)
		_depressions.push_back(UNDEFINED_MILLISECOND_TIME_CONSTANT);
    // absolute synaptic efficacy
	if (_attributes & SYNAPSE_ABSOLUTE_SYNAPTIC_EFFICACY)
		_efficacies.push_back(UNDEFINED_MILLIVOLT);
    // delay
	if (_attributes & SYNAPSE_DELAY)
		_delays.push_back(UNDEFINED_MILLISECOND);
    // postsynaptic potential decay time constant
    if (_attributes & SYNAPSE_DECAY)
        _decays.push_back(UNDEFINED_MILLISECOND);

    // STRUCTURE ______________________________________________________________

 //   // position
	//if (_attributes & SYNAPSE_POSITION)
	//	_positions.push_back(UNDEFINED_VECTOR_3D_MICRON);

    // PRESYNAPTIC
    // presynaptic neuron
    if (_attributes & SYNAPSE_PRESYNAPTIC_NEURON)
		_presynaptic_connection_of_synapses.push_back(UNDEFINED_CELL_GID);
    if (_attributes & SYNAPSE_PRESYNAPTIC_SECTION)
        _presynaptic_section_ids.push_back(UNDEFINED_SECTION_ID);
    // presynaptic morphology segment
    if (_attributes & SYNAPSE_PRESYNAPTIC_SEGMENT)
        _presynaptic_segment_ids.push_back(
        UNDEFINED_SEGMENT_ID);
    // presynaptic morphology segment distance
    if (_attributes & SYNAPSE_PRESYNAPTIC_SEGMENT_DISTANCE)
        _presynaptic_segment_distances.push_back(
		UNDEFINED_MICRON);

    // POSTSYNAPTIC
    // postsynaptic neuron
    if (_attributes & SYNAPSE_POSTSYNAPTIC_NEURON)
		_postsynaptic_connection_of_synapses.push_back(UNDEFINED_CELL_GID);
    // postsynaptic morphology section
    if (_attributes & SYNAPSE_POSTSYNAPTIC_SECTION)
        _postsynaptic_section_ids.push_back(
        UNDEFINED_SECTION_ID);
    // postsynaptic morphology segment
    if (_attributes & SYNAPSE_POSTSYNAPTIC_SEGMENT)
        _postsynaptic_segment_ids.push_back(
        UNDEFINED_SEGMENT_ID);
    // postsynaptic morphology segment distance
    if (_attributes & SYNAPSE_POSTSYNAPTIC_SEGMENT_DISTANCE)
        _postsynaptic_segment_distances.push_back(
        UNDEFINED_MICRON);

    // ________________________________________________________________________

	// Update total number of synapses stored in the dataset.
	++_dataset_size;

	//! \todo Check which attributes are loaded and compare to arrays. (TT)
	// Check if dataset sizes are consistent.
    bbp_assert(_types.size() == _dataset_size
        || _types.size() == 0);
    bbp_assert(_conductances.size() == _dataset_size 
        || _conductances.size() == 0);
    bbp_assert(_utilizations.size() == _dataset_size 
        || _utilizations.size() == 0);
    bbp_assert(_depressions.size() == _dataset_size 
        || _depressions.size() == 0);
	bbp_assert(_facilitations.size() == _dataset_size 
        || _facilitations.size() ==0);
	bbp_assert(_efficacies.size() == _dataset_size 
        || _efficacies.size() == 0);
	//bbp_assert(_positions.size() == _dataset_size 
    //      || _positions.size() == 0);
    bbp_assert(_presynaptic_connection_of_neurons.size() == _number_of_neurons 
        || _presynaptic_connection_of_neurons.size() == 0);
    bbp_assert(_presynaptic_connection_of_synapses.size() == _dataset_size
        || _presynaptic_connection_of_synapses.size() == 0);
	bbp_assert(_presynaptic_section_ids.size() == _dataset_size 
        || _presynaptic_section_ids.size() == 0);
	bbp_assert(_presynaptic_segment_ids.size() == _dataset_size 
        || _presynaptic_segment_ids.size() == 0);
	bbp_assert(_presynaptic_segment_distances.size() == _dataset_size 
        || _presynaptic_segment_distances.size() == 0);
	bbp_assert(_postsynaptic_connection_of_neurons.size() == _number_of_neurons 
        || _postsynaptic_connection_of_neurons.size() == 0);
	bbp_assert(_postsynaptic_connection_of_synapses.size() == _dataset_size 
        || _postsynaptic_connection_of_synapses.size() == 0);
	bbp_assert(_postsynaptic_section_ids.size() == _dataset_size 
        || _postsynaptic_section_ids.size() == 0);
	bbp_assert(_postsynaptic_segment_ids.size() == _dataset_size 
        || _postsynaptic_segment_ids.size() == 0);
	bbp_assert(_postsynaptic_segment_distances.size() == _dataset_size 
        || _postsynaptic_segment_distances.size() == 0);
	bbp_assert(_decays.size() == _dataset_size 
        || _decays.size() == 0);
    bbp_assert(_delays.size() == _dataset_size 
        ||  _delays.size() == 0);

	return _dataset_size - 1;
}


// ----------------------------------------------------------------------------

//! \todo This should take a Synapse object as argument later on. (TT)
inline Synapse_Index Synapse_Dataset::insert()
{
	//// Find an unused array location in the allocated synapse dataset.	
	//for (size_t i = 0; i < _used_memory_locations.size(); ++i)
	//{
	//	if (_used_memory_locations[i] == false)
	//	{
	//		_used_memory_locations[i] = true;
	//		return i;
	//	}
	//}

	// Otherwise allocate new storage and add synapse to end of dataset
	return push_back();
}

// ----------------------------------------------------------------------------

//// \todo This should as option take a Synapse object as argument later on. (TT)
//inline void Synapse_Dataset::erase(Synapse_Index index)
//{
//	if (_used_memory_locations.size() > index)
//		_used_memory_locations[index] = false;
//	else
//	{
//		std::cerr << "WARNING: Could not erase synapse "
//		  << "(index is out of bounds). Continue without change" << std::endl;
//	}
//}

// ----------------------------------------------------------------------------

inline void Synapse_Dataset::reserve(size_t	dataset_size)
{
	//// Check which attributes are stored in the dataset and resize
	//// the corresponding arrays to the new size (all the same).
	//_used_memory_locations.resize(dataset_size, false);

    // DYNAMICS

    if (_attributes & SYNAPSE_TYPE)
		_types.reserve(dataset_size);
	if (_attributes & SYNAPSE_CONDUCTANCE)
		_conductances.reserve(dataset_size);
	if (_attributes & SYNAPSE_UTILIZATION)
		_utilizations.reserve(dataset_size);
	if (_attributes & SYNAPSE_SHORT_TERM_DEPRESSION)
		_depressions.reserve(dataset_size);
	if (_attributes & SYNAPSE_SHORT_TERM_FACILITATION)
		_facilitations.reserve(dataset_size);
	if (_attributes & SYNAPSE_ABSOLUTE_SYNAPTIC_EFFICACY)
		_efficacies.reserve(dataset_size);
	if (_attributes & SYNAPSE_DECAY)
		_decays.reserve(dataset_size);
    if (_attributes & SYNAPSE_DELAY)
		_delays.reserve(dataset_size);

    // STRUCTURE

  //  if (_attributes & SYNAPSE_POSITION)
		//_positions.reserve(dataset_size);

    // presynaptic attributes
	if (_attributes & SYNAPSE_PRESYNAPTIC_NEURON)
		_presynaptic_connection_of_synapses.reserve(dataset_size);
	if (_attributes & SYNAPSE_PRESYNAPTIC_SECTION)
		_presynaptic_section_ids.reserve(dataset_size);
	if (_attributes & SYNAPSE_PRESYNAPTIC_SEGMENT)
		_presynaptic_segment_ids.reserve(dataset_size);
	if (_attributes & SYNAPSE_PRESYNAPTIC_SEGMENT_DISTANCE)
		_presynaptic_segment_distances.reserve(dataset_size);

    // postsynaptic attributes
    if (_attributes & SYNAPSE_POSTSYNAPTIC_NEURON)
		_postsynaptic_connection_of_synapses.reserve(dataset_size);
	if (_attributes & SYNAPSE_POSTSYNAPTIC_SECTION)
		_postsynaptic_section_ids.reserve(dataset_size);
	if (_attributes & SYNAPSE_POSTSYNAPTIC_SEGMENT)
		_postsynaptic_segment_ids.reserve(dataset_size);
	if (_attributes & SYNAPSE_POSTSYNAPTIC_SEGMENT_DISTANCE)
		_postsynaptic_segment_distances.reserve(dataset_size);
}

// ----------------------------------------------------------------------------

// \todo swap to free memory (TT)
void Synapse_Dataset::clear()
{
	// Set synapse and neuron counters to zero.
	_dataset_size		= 0;
    _number_of_neurons	= 0;

	//// Set all connection and synapse arrays to size zero.
	//_used_memory_locations.clear();

    // Clear connection attributes.
    std::vector<Cell_GID> temp_presynaptic_connection_of_neurons;
    _presynaptic_connection_of_neurons.swap(
        temp_presynaptic_connection_of_neurons);
    
    std::vector<Cell_GID> temp_postsynaptic_connection_of_neurons;
	_postsynaptic_connection_of_neurons.swap(
        temp_postsynaptic_connection_of_neurons);

    // DYNAMICS

    // Clear synapse attributes.
    std::vector<Synapse_Type_ID> temp_types;
    _types.swap(temp_types);
    
    std::vector<Nanosiemens> temp_conductances;
	_conductances.swap(temp_conductances);
    
    std::vector<Probability> temp_utilizations;
	_utilizations.swap(temp_utilizations);
    
    std::vector<Millisecond_Time_Constant> temp_depressions;
    _depressions.swap(temp_depressions);
    
    std::vector<Millisecond_Time_Constant> temp_facilitations;
    _facilitations.swap(temp_facilitations);
    
    std::vector<Millivolt> temp_efficacies;
    _efficacies.swap(temp_efficacies);
    
    std::vector<Millisecond> temp_decays;
    _decays.swap(temp_decays);
    
    std::vector<Millisecond> temp_delays;
	_delays.swap(temp_delays);

    // STRUCTURE

    // Clear presynaptic synapse attributes.
    std::vector<Cell_GID> temp_presynaptic_connection_of_synapses;
    _presynaptic_connection_of_synapses.swap(
        temp_presynaptic_connection_of_synapses);
    
    std::vector<Section_ID> temp_presynaptic_section_ids;
    _presynaptic_section_ids.swap(temp_presynaptic_section_ids);
    
    std::vector<Segment_ID> temp_presynaptic_segment_ids;
	_presynaptic_segment_ids.swap(temp_presynaptic_segment_ids);
    
    std::vector<Micron> temp_presynaptic_segment_distances;
	_presynaptic_segment_distances.swap(temp_presynaptic_segment_distances);

    // Clear postsynaptic synapse attributes.
    std::vector<Cell_GID> temp_postsynaptic_connection_of_synapses;
    _postsynaptic_connection_of_synapses.swap(
       temp_postsynaptic_connection_of_synapses);
    
    std::vector<Section_ID> temp_postsynaptic_section_ids;
    _postsynaptic_section_ids.swap(temp_postsynaptic_section_ids);
    
    std::vector<Segment_ID> temp_postsynaptic_segment_ids;
    _postsynaptic_segment_ids.swap(temp_postsynaptic_segment_ids);

    std::vector<Micron> temp_postsynaptic_segment_distances;
    _postsynaptic_segment_distances.swap(temp_postsynaptic_segment_distances);
}

// ----------------------------------------------------------------------------

inline size_t Synapse_Dataset::size() const
{
	return _dataset_size;
}

// ----------------------------------------------------------------------------

inline const Synapse_Type_ID * Synapse_Dataset::types()const
{
	if (_types.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _types[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Nanosiemens * Synapse_Dataset::conductances() const
{
	if (_conductances.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _conductances[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Probability * Synapse_Dataset::utilizations() const
{
	if (_utilizations.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _utilizations[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Millisecond_Time_Constant * Synapse_Dataset::depressions() const
{
	if (_depressions.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _depressions[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Millisecond_Time_Constant * Synapse_Dataset::facilitations() const
{
	if (_facilitations.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _facilitations[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Millivolt * Synapse_Dataset::efficacies() const
{
	if (_efficacies.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _efficacies[0] ;
	}
}

// ---------------------------------------------------------------------------- 

inline const Millisecond * Synapse_Dataset::delays() const
{
	if (_delays.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _delays[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Millisecond * Synapse_Dataset::decays() 
    const
{
	if (_decays.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _decays[0] ;
	}
}

// ----------------------------------------------------------------------------

//inline const Vector_3D<Micron> * Synapse_Dataset::positions() const
//{
//	if (_positions.size() == 0)
//	{
//		return 0;
//	}
//	else
//	{
//		return & _positions[0] ;
//	}
//}

// ----------------------------------------------------------------------------

//inline const Micron * Synapse_Dataset::raw_positions() const
//{
//	if (_positions.size() == 0)
//	{
//		return 0;
//	}
//	else
//	{
//        /*! \todo: test code should be written to check if reinterpret_cast is 
//            supported on each platform we run this on (TT) */
//		return reinterpret_cast<const Micron*>(& _positions[0]);
//	}
//}

// ----------------------------------------------------------------------------

inline const Cell_GID * Synapse_Dataset::
	presynaptic_neuron_of_synapses() const
{
	if (_presynaptic_connection_of_synapses.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _presynaptic_connection_of_synapses[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Section_ID * Synapse_Dataset::
    presynaptic_section_ids() const
{
	if (_presynaptic_section_ids.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _presynaptic_section_ids[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Morphology_Point_ID * Synapse_Dataset::
    presynaptic_segment_ids() const
{
	if (_presynaptic_segment_ids.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _presynaptic_segment_ids[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Micron * Synapse_Dataset::
	presynaptic_segment_distances() const
{
	if (_presynaptic_segment_distances.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _presynaptic_segment_distances[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Cell_GID * Synapse_Dataset::
	postsynaptic_neuron_of_synapses() const
{
	if (_postsynaptic_connection_of_synapses.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _postsynaptic_connection_of_synapses[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Section_ID * Synapse_Dataset::postsynaptic_section_ids() 
	const
{
	if (_postsynaptic_section_ids.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _postsynaptic_section_ids[0] ;
	}
}

// ----------------------------------------------------------------------------

inline const Morphology_Point_ID * Synapse_Dataset::
	postsynaptic_segment_ids() const
{
	if (_postsynaptic_segment_ids.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _postsynaptic_segment_ids[0] ;
	}
}           

// ----------------------------------------------------------------------------

inline const Micron * Synapse_Dataset::
	postsynaptic_segment_distances() const
{
	if (_postsynaptic_segment_distances.size() == 0)
	{
		return 0;
	}
	else
	{
		return & _postsynaptic_segment_distances[0] ;
	}
}  

// GET SINGLE ELEMENTS ________________________________________________________


Synapse_Type_ID & Synapse_Dataset::type(size_t index)
{
	return _types[index];
}

// ----------------------------------------------------------------------------

Nanosiemens & Synapse_Dataset::conductance(size_t index)
{
	return _conductances[index];
}

// ----------------------------------------------------------------------------

Probability & Synapse_Dataset::utilization(size_t index)
{
	return _utilizations[index];
}

// ----------------------------------------------------------------------------

Millisecond_Time_Constant & Synapse_Dataset::depression(size_t index)
{
	return _depressions[index];
}

// ----------------------------------------------------------------------------

Millisecond_Time_Constant & Synapse_Dataset::facilitation(size_t index)
{
	return _facilitations[index];
}

// ----------------------------------------------------------------------------

Millivolt & Synapse_Dataset::efficacy(size_t index)
{
    return _efficacies[index];
}

// ----------------------------------------------------------------------------

Millisecond & Synapse_Dataset::decay(size_t index)
{
	return _decays[index];
}


// ----------------------------------------------------------------------------

Millisecond & Synapse_Dataset::delay(size_t index)
{
	return _delays[index];
}

// ----------------------------------------------------------------------------

//Vector_3D<Micron> & Synapse_Dataset::position(size_t index)
//{
//	return _positions[index];
//}

// ----------------------------------------------------------------------------

Cell_GID & Synapse_Dataset::presynaptic_neuron_of_synapse(size_t index)
{
	return _presynaptic_connection_of_synapses[index];
}

// ----------------------------------------------------------------------------

Section_ID & Synapse_Dataset::presynaptic_section_id(size_t index)
{
	return _presynaptic_section_ids[index];
}

// ----------------------------------------------------------------------------

Morphology_Point_ID & Synapse_Dataset::
	presynaptic_segment_id(size_t index)
{
	return _presynaptic_segment_ids[index];
}

// ----------------------------------------------------------------------------

Micron & Synapse_Dataset::
	presynaptic_segment_distance(size_t index)
{
	return _presynaptic_segment_distances[index];
}

// ----------------------------------------------------------------------------

Cell_GID & Synapse_Dataset::postsynaptic_neuron_of_synapse(size_t index)
{
	return _postsynaptic_connection_of_synapses[index];
}

// ----------------------------------------------------------------------------

Section_ID & Synapse_Dataset::postsynaptic_section_id(size_t index)
{
	return _postsynaptic_section_ids[index];
}

// ----------------------------------------------------------------------------

Morphology_Point_ID & Synapse_Dataset::
	postsynaptic_segment_id(size_t index)
{
	return _postsynaptic_segment_ids[index];
}

// ----------------------------------------------------------------------------

Micron & Synapse_Dataset::
	postsynaptic_segment_distance(size_t index)
{
	return _postsynaptic_segment_distances[index];
}

// ----------------------------------------------------------------------------

// \todo Review commented code below - comment not by me. The method should
//		 probably be implemented in the Synapses container or Microcircuit
//		 for returning these as dataset statistics. (TT)
/**
    Method to return the number of pure afferent, pure efferent
    and shared synapses stored as far as all the neurons of the
    MVD are not loaded (because of ther universe target
    definition), some synapses stored don't have pre- or
    post-synaptic neuron.  This method should be implemented in
    the SynapseClass. (SL)
*/
/*void Synapse_Dataset::getSynapseCounts(
    Synapse_Index& nbSynapses,
    Synapse_Index& nbPureAffererentSynapses, 
    Synapse_Index& nbPureEfferentSynapses, 
    Synapse_Index& nbSharedSynapses
)  const
{
    nbSynapses = _synapseCount ;
    nbPureAfferentSynapses = 0 ;
    nbPureEfferentSynapses = 0 ;
    nbSharedSynapses = 0 ;
    for ( Synapse_ID i = 0 ; i < _synapseCount ; i++ )
    {
        if (_arrayPresynapticNeuronID[i]==UNDEFINED_NEURON_ID)
            nbPureAfferentSynapses++ ;
        else if (_arrayPostsynapticNeuronID[i]==UNDEFINED_NEURON_ID)
            nbPureEfferentSynapses++ ;
        else
            nbSharedSynapses++ ;
    }
}*/

// ----------------------------------------------------------------------------

} 
#endif // BBP_DATASET_SYNAPSE_H
