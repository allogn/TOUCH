/*
 *  Target.cpp
 *  bc_extBBP-SDK
 *
 *  Created by James King on 4/2/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <stack>
#include <deque>
#include <set>
#include <algorithm>
using namespace std;


#include "BBP/Model/Microcircuit/Targets/Target.h"
#include "BBP/Model/Microcircuit/Targets/Targets.h"

namespace {
void mismatch_error()
{
    // Raise an exception or something here.
}

}


namespace bbp {



//-----------------------------------------------------------------------------
// Static Member functions
//-----------------------------------------------------------------------------

std::string Target::type_to_string( Target_Type pType )
{
    switch( pType )
    {
        case CELL_TARGET:
            return "Cell";
        case SECTION_TARGET:
            return "Section";
        case COMPARTMENT_TARGET:
            return "Compartment";
        case SYNAPSE_TARGET:
            return "Synapse";
        default:
            return "Dontcare";
    }
}

//-----------------------------------------------------------------------------

std::string Target::subset_to_string( Subset pSubset )
{
    switch( pSubset )
    {
        case All:
            return "";
        case Soma:
            return "soma";
        case Axon:
            return "axon";
        case Apic:
            return "apic";
        case Dend:
            return "dend";
        default:
            return "undefined";
    }
}

//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Target::Target( const Target &pTarget, bool ref)
    : bbp::copy_on_write<Target>(pTarget,ref)
{

    _impl = pTarget._impl;

// Pre-copy-on-write implementation    
//     _type = pTarget._type;
//     _name = pTarget._name;
    
//     _target_members = pTarget._target_members;
//     _target_subsets = pTarget._target_subsets;
//     _target_points = pTarget._target_points;
//     _target_synapse_IDs = pTarget._target_synapse_IDs;
    
//     _cell_members = pTarget._cell_members;
//     _cell_subsets = pTarget._cell_subsets;
//     _cell_points = pTarget._cell_points;
//     _cell_synapse_IDs = pTarget._cell_synapse_IDs;
    
//     _rebuild_lookup = true;

}

//-----------------------------------------------------------------------------
// Assignment operator
//-----------------------------------------------------------------------------

Target &Target::operator=( const Target &pTarget)
{
    _impl = pTarget._impl;
    return *this;
}




//-----------------------------------------------------------------------------
// Destructors
//-----------------------------------------------------------------------------

Target::~Target()
{
}
    
//-----------------------------------------------------------------------------
// Instance Member functions
//-----------------------------------------------------------------------------

void Target::insert( const Targets& targets )
{
    check_and_copy(this);
    
    _impl->_target_members.insert(_impl->_target_members.end(),
                                  targets.begin(), targets.end());
}

//-----------------------------------------------------------------------------

void Target::insert( Cell_GID cell, Subset subset, const std::vector<double> &points )
{
    check_and_copy(this);
    
    _impl->_rebuild_lookup = true;
    _impl->_cell_members.push_back( cell );
    
    if( _impl->_type == SECTION_TARGET || _impl->_type == COMPARTMENT_TARGET )
    {
        _impl->_cell_subsets.push_back( subset );
        _impl->_cell_points.push_back( points );
    }
}

//-----------------------------------------------------------------------------

void Target::insert( const std::vector<Cell_GID> &cells, const std::vector<Subset> &subsets,
    const std::vector< std::vector<double> > &points )
{
    check_and_copy(this);
    
    _impl->_rebuild_lookup = true;
    
    _impl->_cell_members.insert( _impl->_cell_members.end(), cells.begin(), cells.end() );
    if( _impl->_type == SECTION_TARGET || _impl->_type == COMPARTMENT_TARGET )
    {
        _impl->_cell_subsets.insert( _impl->_cell_subsets.end(), 
                              subsets.begin(), subsets.end() );
        _impl->_cell_points.insert( _impl->_cell_points.end(), 
                             points.begin(), points.end() );
    }
}

//-----------------------------------------------------------------------------

void Target::insert( Cell_GID cell, const std::vector<int> &synapseIDs )
{
    check_and_copy(this);

    
    _impl->_rebuild_lookup = true;
    _impl->_cell_members.push_back( cell );
    _impl->_cell_synapse_IDs.push_back( synapseIDs );
}

//-----------------------------------------------------------------------------

void Target::insert( const std::vector<Cell_GID> &cells, 
                     const std::vector< std::vector<int> > &synapseIDs )
{
    check_and_copy(this);

    _impl->_rebuild_lookup = true;
    _impl->_cell_members.insert( _impl->_cell_members.end(), cells.begin(), cells.end() );
    _impl->_cell_synapse_IDs.insert( _impl->_cell_synapse_IDs.end(), 
                              synapseIDs.begin(), synapseIDs.end() );
}

//-----------------------------------------------------------------------------

void Target::insert( const Target subtarget, Subset subset, 
                     const std::vector<double> &points )
{
    check_and_copy(this);

    _impl->_target_members.push_back( subtarget );
    
    if( _impl->_type == SECTION_TARGET || _impl->_type == COMPARTMENT_TARGET )
    {
        _impl->_target_subsets.push_back( subset );
        _impl->_target_points.push_back( points );
    }
}

//-----------------------------------------------------------------------------

void Target::insert( const std::vector<Target > &subtargets, 
                     const std::vector<Subset> &subsets,
    const std::vector< std::vector<double> > &points )
{
    check_and_copy(this);

    _impl->_target_members.insert( _impl->_target_members.end(),
                            subtargets.begin(), subtargets.end() );
    
    if( _impl->_type == SECTION_TARGET || _impl->_type == COMPARTMENT_TARGET )
    {
        _impl->_target_subsets.insert( _impl->_target_subsets.end(), 
                                subsets.begin(), subsets.end() );
        _impl->_target_points.insert( _impl->_target_points.end(), 
                               points.begin(), points.end() );
    }
}

//-----------------------------------------------------------------------------

void Target::insert( const Target  subtarget, 
                     const std::vector<int> &synapseIDs )
{
    check_and_copy(this);

    _impl->_target_members.push_back( subtarget );
    _impl->_target_synapse_IDs.push_back( synapseIDs );
}

//-----------------------------------------------------------------------------

void Target::insert( const std::vector<Target> &subtargets, 
                     const std::vector< std::vector<int> > &synapseIDs )
{
    check_and_copy(this);

    _impl->_target_members.insert( _impl->_target_members.end(), subtargets.begin(), 
                            subtargets.end() );
    _impl->_target_synapse_IDs.insert( _impl->_target_synapse_IDs.end(), 
                                synapseIDs.begin(), synapseIDs.end() );
}

//-----------------------------------------------------------------------------

void Target::remove_target( const Label &pName )
{
    check_and_copy(this);

    //I would not expect a target to be a member multiple time - but nothing is prohibiting it.  Unless/until
    // such a restriction is put in place, always search the vector fully
    std::vector<Target>::iterator subtarget = _impl->_target_members.begin();
    while( subtarget != _impl->_target_members.end() )
    {
        if( (*subtarget)->name() == pName )
            _impl->_target_members.erase(subtarget);
        else
            ++subtarget;
    }
}

// ----------------------------------------------------------------------------

void Target::get_all_gids( std::vector<Cell_GID> &collection ) const
{

    //copy cell members to vector
    collection.insert( collection.end(), _impl->_cell_members.begin(), _impl->_cell_members.end() );
    
    //have all subtargets do the same
    for( unsigned int targetIndex=0; targetIndex<_impl->_target_members.size(); targetIndex++ )
        _impl->_target_members[targetIndex]->get_all_gids( collection );
}

// ----------------------------------------------------------------------------

const Label& Target::target_member_name( unsigned int index ) const
{

    //if( index < _target_members.size() )
    return _impl->_target_members[index]->name();
    //else, should throw exception
}

// ----------------------------------------------------------------------------

const Target  Target::target_member( unsigned int index ) const
{

    //if( index < _targetMembers.size() )
    return _impl->_target_members[index];
    //else, should throw exception
}

// ----------------------------------------------------------------------------

const Cell_GID& Target::cell_member_gid( unsigned int index ) const
{
    //if( index < _cell_members.size() )
    return _impl->_cell_members[index];
    //else, should throw exception
}

// ----------------------------------------------------------------------------

const std::vector<double>& Target::cell_member_section_points( unsigned int index ) const
{
    return _impl->_cell_points.at(index);
}

// ----------------------------------------------------------------------------

const std::vector<double>& Target::cell_member_compartment_points( unsigned int index ) const
{
    return cell_member_section_points(index);
}

// ----------------------------------------------------------------------------

const std::vector<int>& Target::cell_member_synapse_IDs( unsigned int index ) const
{
    return _impl->_cell_synapse_IDs.at(index);
}

// ----------------------------------------------------------------------------

const std::vector<double>& Target::target_member_section_points( unsigned int index ) const
{
    return _impl->_target_points.at(index);
}

// ----------------------------------------------------------------------------

const std::vector<double>& Target::target_member_compartment_points( unsigned int index ) const
{
    return target_member_section_points(index);
}

// ----------------------------------------------------------------------------

const std::vector<int>& Target::target_member_synapse_IDs( unsigned int index ) const
{
    return _impl->_target_synapse_IDs.at(index);
}

// ----------------------------------------------------------------------------

Target::Subset Target::target_member_subset( unsigned int index ) const
{
    return _impl->_target_subsets.at(index);
}

// ----------------------------------------------------------------------------

Target::Subset Target::cell_member_subset( unsigned int index ) const
{
    return _impl->_cell_subsets.at(index);
}

// ----------------------------------------------------------------------------



Target Target::add          (const Target &RHS) const
{
    Target ret;
    Target left;
    Target right;
    
    set<Cell_GID> result;

    if( _impl->_type != RHS._impl->_type){
        mismatch_error();
    }


    left = flatten();
    right = RHS.flatten();

    set_union(   left._impl->_cell_members.begin(),    left._impl->_cell_members.end(),
                right._impl->_cell_members.begin(),   right._impl->_cell_members.end(),
                inserter(result,result.begin()));

    ret.name(this->name() + "_plus_" + RHS.name());
    ret._impl->_type = this->_impl->_type;
    ret._impl->_cell_members.assign(result.begin(),result.end());

    return ret;
}


Target Target::subtract     (const Target &RHS) const
{
    Target ret;
    Target left;
    Target right;

    set<Cell_GID> result;

    if( _impl->_type != RHS._impl->_type){
        mismatch_error();
    }

    left = flatten();
    right = RHS.flatten();
    
    sort( left._impl->_cell_members.begin(), left._impl->_cell_members.end());
    sort(right._impl->_cell_members.begin(),right._impl->_cell_members.end());
    
    set_difference(   
        left.cell_begin(),    left.cell_end(),
        right.cell_begin(),   right.cell_end(),
        inserter(result,result.begin()));
    
    ret.name(this->name() + "_minus_" + RHS.name());
    ret._impl->_type = this->_impl->_type;
    ret._impl->_cell_members.assign(result.begin(),result.end());
 
    return ret;
}


Target Target::intersect    (const Target &RHS) const
{
    Target ret;
    Target left;
    Target right;

    set<Cell_GID> result;

    if( _impl->_type != RHS._impl->_type){
        mismatch_error();
    }


    left = flatten();
    right = RHS.flatten();
    
    sort( left._impl->_cell_members.begin(), left._impl->_cell_members.end());
    sort(right._impl->_cell_members.begin(),right._impl->_cell_members.end());
    
    set_intersection(   
        left._impl->_cell_members.begin(),    left._impl->_cell_members.end(),
        right._impl->_cell_members.begin(),   right._impl->_cell_members.end(),
        inserter(result,result.begin()));
    
    ret.name(this->name() + "_intersect_" + RHS.name());
    ret._impl->_type = this->_impl->_type;
    ret._impl->_cell_members.assign(result.begin(),result.end());

    return ret;
}


Target Target::exclusive    (const Target &RHS) const
{
    Target ret;
    Target left;
    Target right;

    set<Cell_GID> result;

    if( _impl->_type != RHS._impl->_type){
        mismatch_error();
    }


    left = flatten();
    right = RHS.flatten();
    
    sort( left._impl->_cell_members.begin(), left._impl->_cell_members.end());
    sort(right._impl->_cell_members.begin(),right._impl->_cell_members.end());
    
    set_symmetric_difference(   
        left.cell_begin(),    left.cell_end(),
        right.cell_begin(),   right.cell_end(),
        inserter(result,result.begin()));
    
    ret.name(this->name() + "_exclusive_" + RHS.name());
    ret._impl->_type = this->_impl->_type;
    ret._impl->_cell_members.assign(result.begin(),result.end());
   
    return ret;
}




Target Target::flatten() const
{
    Target ret;
    typedef std::stack<Target> tstack;
    tstack stack;
    ret.name(name() + "_flat");
    ret._impl->_type = type();

    stack.push(*this);
    
    while(!stack.empty()){
        Target p = stack.top();
        stack.pop();

        for(size_t jj = 0; jj < p->cell_member_size(); jj++)
        {
            ret.insert(p->cell_member_gid(jj));
        }
        
        for(std::vector<Target>::const_iterator ii = 
                p->_impl->_target_members.begin(); 
            ii != p->_impl->_target_members.end(); 
            ii++)
        {
            stack.push(*ii);
        }
    }

    return ret;
}

void Target::dump_debug() const
{
    typedef std::pair<Target,int> tpair;
    typedef std::stack<tpair> tstack;
    tstack stack;

    stack.push(tpair(*this,0));
    
    while(!stack.empty()){
        Target p = stack.top().first;
        int depth = 4 * stack.top().second;
        stack.pop();

        cout << string(depth,' ') << "++" << p->name() 
             << "  [" << p->cell_member_size() << "]" << endl;
        
        for(size_t jj = 0; jj < p->cell_member_size(); jj++)
        {
            cout << string(depth,' ') << " | " << p->cell_member_gid(jj) << " " 
                 << endl;
        }
        
        for(std::vector<Target>::const_iterator ii 
                = p->_impl->_target_members.begin(); 
            ii != p->_impl->_target_members.end(); 
            ii++)
        {
            stack.push(tpair(*ii,depth+1));
        }
    }
}


}// end namespace bbp
