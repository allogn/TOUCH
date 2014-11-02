/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Authors: Thomas Traenkler
                 James King

*/


#include "BBP/Model/Microcircuit/Targets/Targets.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"

namespace bbp {

//-----------------------------------------------------------------------------

Targets::~Targets()
{
    // Smart pointers should automatically handle deallocation here - JLK
}

//-----------------------------------------------------------------------------

Target & Targets::get_target (const Label & label)
{
    std::map<Label, unsigned int>::iterator i = _target_lookup.find(label);
    if (i == _target_lookup.end())
    {
        std::string error("Could not find target label: ");
        throw Target_Not_Found(error + label);
    }
    else
    {
        return (_targets[i->second]);
    }
}

//-----------------------------------------------------------------------------

const Target & Targets::get_target (const Label & label) const
{
    std::map<Label, unsigned int>::const_iterator i = 
        _target_lookup.find(label);
    if (i == _target_lookup.end())
    {
        throw Target_Not_Found(("Error: Could not find target label: " +
                                    label));
    }
    else
    {
        return (_targets[i->second]);
    }
}


//-----------------------------------------------------------------------------

/*
Impletmeneted to allow for the migration to smart pointers.
 - JLK 08.10.08
 */
Targets::const_iterator Targets::find (const Label & label) const
{
    std::map<Label, unsigned int>::const_iterator i = _target_lookup.find(label);
    
    if (i == _target_lookup.end())
    {
        std::string error("Could not find target label: ");
        throw Target_Not_Found(error + label);
    }
    else
    {
	std::vector<Target >::const_iterator ret;
	ret = _targets.begin() + i->second;
	
 	return Targets::const_iterator(ret);	
    }
}

//-----------------------------------------------------------------------------

void Targets::push_back(const Target & target)
{
    //make sure this target doesn't already exist
    std::map<Label, unsigned int>::iterator exists = 
        _target_lookup.find( target.name() );
    if( exists != _target_lookup.end() ) {
        std::string msg = "Multiple definitions of target ";
        std::runtime_error( msg.append( target.name() ) );
    }

    _targets.push_back( target );
    _target_lookup.insert( std::pair<Label,unsigned int>(target.name(), 
        _targets.size()-1) );
}

//-----------------------------------------------------------------------------

Target Targets::insert (const Label & label, Target_Type type)
{
    if( exists(label) )
    {
        std::runtime_error(
            "Targets container already contains this target label");
    }
    
    //    _targets.push_back( Target( new Target( label, type) ) );
    _targets.push_back( Target( label, type ) );
    _target_lookup.insert( 
        std::pair<Label,unsigned int>(label, _targets.size()-1) );
    return _targets.back();
}

//-----------------------------------------------------------------------------

bool Targets::exists (const Label & label) const
{
    std::map<Label, unsigned int>::const_iterator i = 
        _target_lookup.find(label);
    if (i == _target_lookup.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}

//-----------------------------------------------------------------------------

void Targets::erase (const Label & label)
{
/*!
    \todo This operation will probably be slow, especially if targets added 
    early are deleted first, information needs to be updated in this Targets 
    object (indices in lookup map) second, all targets will need to be check 
    whether they use the target about to be deleted. Of course, it should be a
    rare operation; however, if it is used a lot, then we can look into ways 
    to improve performance.
*/
    // get the index of the target to be erased - all targets after it must 
    // be updated with a new index value
    std::map<Label, unsigned int>::iterator targetIndex = 
        _target_lookup.find(label);
    if( targetIndex == _target_lookup.end() )
    {
        throw Target_Not_Found(("Error: Could not find target " +
                                    label));
    }
        
    //update lookup map
    for( unsigned int updateIndex=targetIndex->second; 
        updateIndex < _targets.size(); updateIndex++ )
        _target_lookup[_targets[updateIndex]->name()] = updateIndex;
    
    //iterate through all targets removing all references to the lost target
    std::vector<Target >::iterator it = _targets.begin();
    while( it != _targets.end() )
    {
        // have this target check for usage of the deleted (still pending) 
        // target
        if( (*it)->name() != label ) 
            (*it)->remove_target( label );
        it++;
    }
    
    //lastly, remove from map and vector
    _target_lookup.erase(label);
    _targets.erase( _targets.begin()+targetIndex->second );
}

//-----------------------------------------------------------------------------

void Targets::clear ()
{
    _target_lookup.clear();
    _targets.clear();
}

//-----------------------------------------------------------------------------

size_t Targets::size () const
{
    return _targets.size();
}

//-----------------------------------------------------------------------------

Cell_Target Targets::cell_target(const Label & label) const
{
    const Target & target = get_target(label);

    // check if target compatible with cells object
    if (target.type() != CELL_TARGET && target.type() != NESTED_TARGET)
    {
        throw Target_Not_Found(
            "Conversion of Target object to Cell_Target object failed. "
            "Target type incompatible with a Neurons object");
    }

    Cell_Target cells(label);
    
    // insert the cell ids of the cell target into cells object
    std::vector<Cell_GID> cell_members;
    target.get_all_gids( cell_members );
    
    for ( unsigned int cellIndex=0; 
        cellIndex < cell_members.size(); cellIndex++ )
    {
        cells.insert( cell_members[cellIndex] );
    }
    
    return cells;
}

//-----------------------------------------------------------------------------

}
