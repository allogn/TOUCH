/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    James King
                                Thomas Traenkler
                                Juan Hernando Vieites
                                
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_TARGET_H
#define BBP_TARGET_H

#include <cctype>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include <boost/shared_ptr.hpp>

#include "BBP/Common/Types.h"
#include "BBP/Common/Exception/Exception.h"
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Common/Patterns/copy_on_write.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"


namespace bbp {

// ----------------------------------------------------------------------------

//! Type of the target (e.g. cells, sections, compartments, synapses).
/*!
    A nested target is a target that consists of other subtargets (e.g. a set 
    of minicolumns).

    \ingroup Microcircuit_Target
*/

enum Target_Type 
{
    UNDEFINED_TARGET = 0,
    CELL_TARGET,
    SECTION_TARGET,
    COMPARTMENT_TARGET,
    SYNAPSE_TARGET,
    CONNECTION_TARGET,
    NESTED_TARGET
};

// ----------------------------------------------------------------------------

//! container holding a collection of targets
class Targets;

//! shared pointer used to exchange Target between objects
class Target;

//typedef Target * Target;
//typedef const Target * const Target;

/* typedef boost::shared_ptr<Target> Target; */
/* typedef boost::shared_ptr<const Target> const Target; */

// ----------------------------------------------------------------------------

//! Set of selected parts of a microcircuit (e.g. cells, synapses).
/*! 
    microcircuit selections (selecting a set of neurons, sections, 
    compartments, or synapses)
    \ingroup Microcircuit_Target
    \sa Cell_Target Targets

    \bug What is 'int' representing as the type of a synapse ID?
         Something more explicit and its relationship to the Synapse container
         should be employed. (JH)
*/
class Target : public bbp::copy_on_write<Target>
{
    friend class Targets;
    
public:

public:
    //! Classify section types into certain subsets
    enum Subset{ Soma, Axon, Dend, Apic, All, MaxSubset };
    
public:
    inline Target();
    //! construct a labeled target of specified type
    inline Target(Label name, Target_Type type);
    /*! \todo The "ref" parameter to the Target copy constructor should
              default to false, and the Targets class and other classes
              should be redesigned to allow this work properly.
    */
    Target( const Target& target, bool ref = true);
    Target &operator=(const Target& pTarget);
    
    ~Target();
    
    
public:    
    /*! \brief Get a string representation of a target type as it should
               appear in a .target file. */
    static std::string type_to_string( Target_Type pType );
    
    /*! \brief Get a string representation of a subset type as it
               should appear in a .target file. */
    static std::string subset_to_string( Subset pSubset );

    //! name of this target
    inline const Label & name() const;

    //! type of target contents (e.g. cells, compartments, synapses)
    Target_Type type() const;
    
    //! The number of elements the target contains \deprecated 
    /*! Deprecated due to ambiguity from having separate target and cell 
        lists (JK) */
    size_t size() const;

    /*! \brief Insert a set of Targets as target member of the current target.
    */
    void insert (const Targets & targets);

    /*! \brief Insert a single cell gid, with option to give subset/point 
        info (if subset/compartment target).
        Be aware that neurons are assumed to have the syntax 'a' and a
        number (e.g. a5324), starting at 1.
     */
    void insert( Cell_GID cell, Subset subset = All, 
                 const std::vector<double> &points = std::vector<double>() );

    /*! \brief Insert many cells, with option to give corresponding 
               subset/point info (if subset/compartment target). */
    void insert( const std::vector<Cell_GID> & cells, 
                 const std::vector<Subset> & subsets = std::vector<Subset>(),
                 const std::vector< std::vector<double> > & points = 
                     std::vector<std::vector< double> >() );
    
    /*! \brief Insert single cell gid for synapse target with corresponding 
               synapse ids. */
    void insert( Cell_GID cell, const std::vector<int> &synapseIDs );
    
    //! Insert many cells for a synapse target with corresponding synapse ids.
    void insert( const std::vector<Cell_GID> & cells, 
                 const std::vector< std::vector<int> > & synapseIDs );
    
    /*! \brief Insert a single target pointer, with option to give subset/point
               info (if subset/compartment target). */
    void insert( const Target subtarget, Subset subset = All, 
                 const std::vector<double> &points = std::vector<double>() );

    /*! \brief Insert many target pointers, with option to give corresponding
               subset/point info (if subset/compartment target). */
    void insert( const std::vector<Target> & subtargets, 
                 const std::vector<Subset> & subsets = std::vector<Subset>(),
                 const std::vector< std::vector<double> > &points = 
                     std::vector<std::vector< double> >() );
    
    /*! \brief Insert single target pointer for synapse target with 
               corresponding synapse ids. */
    void insert( const Target  subtarget, 
                 const std::vector<int> & synapseIDs );
    
    /*! \brief Insert many target pointers for a synapse target with
               corresponding synapse ids. */
    void insert( const std::vector<Target> & subtargets, 
                 const std::vector< std::vector<int> > & synapseIDs );
    
    //! Forward iterator for the Target container to visit all elements
    class iterator;
    //! Forward iterator for the Target container to visit all elements
    typedef iterator const_iterator;

    //! Returns the first element of the container for linear access
    inline iterator begin() const;
    /*! 
        Returns the element succeeding the last element in the container.
        If the Target iterator exceeds the elements in the container, the
        iterator will equal this state.
    */
    inline iterator end() const;
    
    //! Set the name of the target.
    void name( const Label & pName );
    
    //! remove target from member targets by name.
    void remove_target( const Label & pName );
    
    //! @return the name of a target member at the given index.
    const Label & target_member_name( unsigned int index ) const;
 
    //! @return the target object of a target member at the given index
    const Target  target_member( unsigned int index ) const;
   
    //! @return the gid of the cell at the given index.
    const Cell_GID & cell_member_gid( unsigned int index ) const;
    
    //! @return The number of subtargets this target contains.
    size_t target_member_size() const;
    
    //! @return The number of cell members this target contains.
    size_t cell_member_size() const;
    
    /*! @return Reference to the section points for a cell member of
                this Target. */
    const std::vector<double> & 
        cell_member_section_points( unsigned int index ) const;
    
    /*! @return Reference to the compartment points for a cell member
                of this Target. */
    const std::vector<double> & 
        cell_member_compartment_points( unsigned int index ) const;
    
    /*! @return Reference to the synapse IDs for a cell member of this 
                Target. */
    const std::vector<int> & 
        cell_member_synapse_IDs( unsigned int index ) const;
    
    /*! @return Reference to the section points for a target member
                (subtarget) of this Target. */
    const std::vector<double> & 
        target_member_section_points( unsigned int index ) const;
    
    /*! @return Reference to the compartment points for a target member 
                (subtarget) of this Target. */
    const std::vector<double> & 
        target_member_compartment_points( unsigned int index ) const;
    
    /*! @return Reference to the synapse IDs for a target member (subtarget)
                of this Target */
    const std::vector<int> &
        target_member_synapse_IDs( unsigned int index ) const;

    /*! @return The subset a cell member uses (for section or 
                compartment targets). */
    Subset cell_member_subset( unsigned int index ) const;
    
    /*! @return The subset a target member (subtarget) uses (for section 
                or compartment targets). */
    Subset target_member_subset( unsigned int index ) const;


public:
    // STL Assistant functions
    typedef std::vector<Cell_GID>::const_iterator cell_iterator;
    inline cell_iterator cell_begin() const;
    inline cell_iterator cell_end() const;

    // Distiller assitant operations
public:
    Target add          (const Target &RHS) const;
    Target subtract     (const Target &RHS) const;
    Target intersect    (const Target &RHS) const;
    Target exclusive    (const Target &RHS) const;

public:
    /*! @return A Target which contains all of the same values as in the source
                Target, but with all of the values from subTargets pulled
                into the root Target, and with all of the subTargets removed.
     */
    Target flatten() const;

    /*! \brief Creates a cell_target object of the same contents
     */
    inline Cell_Target cell_target();
    inline operator Cell_Target();

    /*! \brief Writes the entire Target with all subtargets in an indented
               way, so that it is easy to view Targets while debugging
     */
    void dump_debug() const;

private:
    /* \brief This Operator -> is intentionally private, and should not be used
              The proper way of accessing members of a Target class is through
              use of the dot operator.
     */
    inline Target *operator->();
    

private:
    
    // ------------------------------------------------------------------------
    // Private methods
    // ------------------------------------------------------------------------
    
    /*!
     * Convenience function provided to allow all cell gids a target covers 
     * (i.e. including gids from subtargets) to be placed into the supplied 
     * vector
     *
     * @param collection Vector to be filled with the gids of the target and 
     *        its subtargets
     */
    void get_all_gids( std::vector<Cell_GID> &collection ) const;

    // \todo Document me!
    void _flatten_self();


//private: // I REALLY want to find a way to make operator new be private
//public:  // but that breaks the Java bindings of swig. JLK 17.11.08
//    void *operator new(size_t);
    
protected:

    friend class bbp::copy_on_write_core;
    struct Target_Impl
    {
        Target_Impl()
        : _type(UNDEFINED_TARGET)
        {}
        
        Target_Impl(Label name, Target_Type type)
        : _name(name),
          _type(type),
          _rebuild_lookup(true)
        {}

        Target_Impl( const Target_Impl& pTarget_Impl )
        :   _name               (pTarget_Impl._name),
            _type               (pTarget_Impl._type),
            _target_members     (pTarget_Impl._target_members),
            _target_subsets     (pTarget_Impl._target_subsets),
            _target_points      (pTarget_Impl._target_points),
            _target_synapse_IDs (pTarget_Impl._target_synapse_IDs),
            _cell_members       (pTarget_Impl._cell_members),
            _cell_subsets       (pTarget_Impl._cell_subsets),
            _cell_points        (pTarget_Impl._cell_points),
            _cell_synapse_IDs   (pTarget_Impl._cell_synapse_IDs),
            _rebuild_lookup     (true)
        {}

        ~Target_Impl()
        {}
      
        
        // ---------------------------------------------------------------------
        // Member data
        // ---------------------------------------------------------------------
        //! Name of this target
        Label                              _name;

        //! Type of target contents (e.g. cells, compartments, synapses)
        Target_Type                        _type;

        //! Subtarget members, manually ordered
        std::vector<Target >     _target_members;

        //! For section/compartment targets.
        /*! Subset information corresponding to each target member. */
        std::vector<Subset>                _target_subsets;

        //! For section/compartment targets.
        /*! Normalized point information corresponding to each target member. */
        std::vector< std::vector<double> > _target_points;

        //! For synapse targets.
        /*! Synapse ids corresponding to each target member. */
        std::vector< std::vector<int> >    _target_synapse_IDs;

        //! Cell members, manually ordered.
        std::vector<Cell_GID>              _cell_members;

        //! For section/compartment targets.
        /*! Subset information corresponding to each cell member. */
        std::vector<Subset>                _cell_subsets;

        //! For section/compartment targets.
        /*! normalized point information corresponding to each cell member. */
        std::vector< std::vector<double> > _cell_points;

        //! For synapse targets.
        /*! Synapse ids corresponding to each cell member */
        std::vector< std::vector<int> >    _cell_synapse_IDs;

        //! For faster cell lookup
        /*! Provide a map keyed on gid with value corresponding to the
          index of the cell in the member list. */
        std::map<Cell_GID, unsigned int>   _gid_index_lookup;

        //! Flag to indicate that _gid_index_lookup must be rebuilt
        bool _rebuild_lookup;

    };

    typedef Target_Impl implementation;
    
    boost::shared_ptr<Target_Impl> _impl;

};

// ----------------------------------------------------------------------------

class Target::iterator
    : public std::iterator<std::forward_iterator_tag, Cell_GID>
{
    friend class Target;
public:
    typedef Cell_GID value_type;
public: // 
    explicit iterator();
protected:
    // This is only meant to be used by the Target begin and end methods
    explicit iterator(const Target &RHS);

public: // 
    inline iterator &operator =(const iterator &);
    
    inline bool operator==(const iterator &) const;
    inline bool operator!=(const iterator &) const;
    inline iterator &operator ++();
    inline iterator operator  ++(int);
    inline value_type & operator *();
    inline value_type * operator->();

protected: // internal mechanics
    void        init();
    void        set_end();
    iterator &  next();
    Cell_GID    get();
    bool        is_end();
private: // internal types
    typedef std::pair<
        std::vector<Target> *,
        std::vector<Target>::iterator> target_vector_pair;
private: // members
    const Target *              _my_target;
    Cell_GID                    _current_value;
    std::vector<Target>         _target_sequence;
    int                         _current_target;
    int                         _current_cell;
};

// ----------------------------------------------------------------------------

class UnimplementedException : public std::exception {};


inline             Target::iterator::iterator()
    : _my_target(NULL),
    _current_value(0),
    _current_target(0),
    _current_cell(0)
{
}

inline             Target::iterator::iterator(const Target &RHS)
    : _my_target(&RHS),
    _current_value(0),
    _current_target(0),
    _current_cell(0)
{
}

inline Target::iterator &Target::iterator::operator=(const iterator &RHS)
{
    _my_target          = RHS._my_target;
    _current_value      = RHS._current_value;
    _current_target     = RHS._current_target;
    _current_cell       = RHS._current_cell;
    _target_sequence    = RHS._target_sequence;
    return *this;
}

inline bool         Target::iterator::operator==(const iterator &RHS) const
{
    return (_my_target           == RHS._my_target     ) &&
           (_current_value       == RHS._current_value ) &&
           (_current_target      == RHS._current_target) &&
           (_current_cell        == RHS._current_cell  ) ;
}

inline bool         Target::iterator::operator!=(const iterator &RHS) const
{
    return (_my_target           != RHS._my_target     ) ||
           (_current_value       != RHS._current_value ) ||
           (_current_target      != RHS._current_target) ||
           (_current_cell        != RHS._current_cell  ) ;
}

inline Target::iterator &   Target::iterator::operator ++()
{
    return next();
}

inline Target::iterator     Target::iterator::operator  ++(int)
{
    iterator ret = *this;
    next();
    return ret;
}

inline Target::iterator::value_type & Target::iterator::operator *()
{
    bbp_assert(_current_target < (int)_target_sequence.size());
    bbp_assert(_current_value  != 0);
    bbp_assert(_current_target != -1);
    bbp_assert(_current_cell   != -1);
    return _current_value;
}

inline Target::iterator::value_type * Target::iterator::operator->()
{
    bbp_assert(_current_target < (int)_target_sequence.size());
    return &_current_value;
}

// ----------------

// typedef std::pair<
//         std::vector<Target> *,
//         std::vector<Target>::iterator> target_vector_pair;


inline bool                        Target::iterator::is_end()
{
    bbp_assert(_my_target != NULL);
    if ((_current_value  ==  0) &&
        (_current_target == -1) &&
        (_current_cell   == -1))
       return true;
    return false;
}


inline void                        Target::iterator::init()
{
    typedef std::stack<Target> tstack;
    tstack stack;
  
    stack.push(*_my_target);
    
    while(!stack.empty()){
        Target p = stack.top();
        stack.pop();
        if(p.cell_member_size())
            _target_sequence.push_back(p);
        
        for(std::vector<Target>::reverse_iterator 
            ii  = p->_impl->_target_members.rbegin();
            ii != p->_impl->_target_members.rend();
            ++ii)
        {
            stack.push(*ii);
        }
    }
    if(_target_sequence.empty()){
        log_message("Warning: empty target",INFORMATIVE_LEVEL);
        set_end();
    }else{    
        _current_value = _target_sequence[_current_target]
                         .cell_member_gid(_current_cell);
    }
    
}

inline void                        Target::iterator::set_end()
{
    bbp_assert(_my_target != NULL);

    _current_value  =  0;
    _current_target = -1;
    _current_cell   = -1;
}

inline Target::iterator &          Target::iterator::next()
{
    ++_current_cell;
    if(_current_cell == 
       (int)_target_sequence[_current_target].cell_member_size())
    {
        _current_cell = 0;
        ++_current_target;
    }

    if(_current_target == (int)_target_sequence.size()){
        set_end();
    }else{
        _current_value = _target_sequence[_current_target].cell_member_gid(_current_cell);
    }
    
    return *this;
}

inline Cell_GID                    Target::iterator::get()
{
    return _current_value;
}







// ----------------------------------------------------------------------------

Target::Target()
: _impl(new Target_Impl)
{
}

// ----------------------------------------------------------------------------

Target::Target(Label name, Target_Type type)
: _impl(new Target_Impl(name,type))
{
}

// ----------------------------------------------------------------------------

const Label & Target::name() const
{
   return _impl->_name;
}

// ----------------------------------------------------------------------------

inline Target_Type Target::type() const
{
   return _impl->_type;
}

// ----------------------------------------------------------------------------

inline size_t Target::size () const
{
    std::cerr << "Deprecated.  Use target_member_size() and"
                 " cell_member_size() to get quantitative data\n";
    return _impl->_target_members.size() + _impl->_cell_members.size();
}

// ----------------------------------------------------------------------------

inline size_t Target::cell_member_size() const
{
    return _impl->_cell_members.size();
}

// ----------------------------------------------------------------------------

inline size_t Target::target_member_size() const
{
    return _impl->_target_members.size();
}

// ----------------------------------------------------------------------------

inline void Target::name( const Label &pName )
{
    check_and_copy(this);
    _impl->_name = pName;
}

// ----------------------------------------------------------------------------
inline Target *Target::operator->()
{
    return this;
}
 

// ----------------------------------------------------------------------------
inline std::vector<Cell_GID>::const_iterator Target::cell_begin() const
{
    return _impl->_cell_members.begin();
}

// ----------------------------------------------------------------------------
inline std::vector<Cell_GID>::const_iterator Target::cell_end() const
{
    return _impl->_cell_members.end();
}

// ----------------------------------------------------------------------------

inline Target::iterator Target::begin() const
{
    Target::iterator begin_iterator(*this);
    begin_iterator.init();
    return begin_iterator;
}

inline Target::iterator Target::end() const
{
    Target::iterator end_iterator(*this);
    end_iterator.set_end();
    return end_iterator;
}

inline Cell_Target Target::cell_target()
{
    Cell_Target ret;
    for(Target::iterator i = begin(); i != end(); i++)
    {
        ret.insert(*i);
    }
    return ret;
}

inline Target::operator Cell_Target()
{
    return cell_target();
}





}// namespace BBP



#endif
