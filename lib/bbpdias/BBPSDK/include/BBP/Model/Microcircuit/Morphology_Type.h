/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Anirudh Vij
*/
#ifndef BBP_MORPHOLOGY_TYPE_H
#define BBP_MORPHOLOGY_TYPE_H

#include "Types.h"
#include "Datasets/Structure_Dataset.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Microcircuit_MVD_File_Reader;

// ----------------------------------------------------------------------------

//! Classification of the morphology type (shape) of a cell.
/*!
   \todo default constructor should create and link a new Structure_Dataset,
   but make sure the way the Morphologies are created in the 
   Structure_Dataset during loading share the dataset (AV)
   \todo make microcircuit member function private (AV)
 */
class Morphology_Type
{
    friend class Microcircuit_MVD_File_Reader;

public:
    Morphology_Type()
        : _id(UNDEFINED_MORPHOLOGY_TYPE)
    {
    }

    Morphology_Type(Morphology_Type_ID id, 
                    const Structure_Dataset_Ptr & dataset) 
    :
        _id(id),
        _dataset(dataset) 
    {}

    //! Get identifier of the m-type.
    Morphology_Type_ID id() const
    {
        return _id;
    }

    //! Query if morphology is of pyramidal type.
    inline bool is_pyramidal() const
    {
        if(_dataset->Pyramidal_Array[_id] == "PYR")
            return true;
        else
            return false;
    }

    //! Query if morphology is of interneurons type.
    inline bool is_interneuron() const
    {
        return (!is_pyramidal());
    }

    //! Query if morphology is of excitatory type.
    inline bool is_excitatory() const
    {
        if(_dataset->Excitatory_Array[_id] == "EXC")
            return true;
        else
            return false;
    }

    //! Query if morphology is of inhibitory type.
    inline bool is_inhibitory() const
    {
        return (!is_excitatory());
    }

    //! Get name of the m-type.
    inline const Label name() const
    {
        if (_dataset.get() == NULL)
        {
            log_message("Error, This morphology has a null _dataset pointer",
                        SEVERE_LEVEL);
            return "Unknown";
        }

        if (_id >= _dataset->Morphology_Types_Array.size()) {
            log_message("Possible inconsistency detected. Found a "
                        "morphology ID for which there is no "
                        "known name", SEVERE_LEVEL);
            return "Unknown";
        }
        return (_dataset->Morphology_Types_Array[_id]);
    }

    //! Set identifier of the m-type.
    void id(Morphology_Type_ID i)
    {
        _id = i;
    }

    bool operator == (const Morphology_Type & rhs) const
    {
        if (_id == rhs._id && _dataset == rhs._dataset)
            return true;
        else
            return false;
    }

    bool operator != (const Morphology_Type & rhs) const
    {
        return !(*this == rhs);
    }

private:
    Morphology_Type_ID      _id;
    Structure_Dataset_Ptr   _dataset;
};

// ----------------------------------------------------------------------------

inline std::ostream & operator << (std::ostream & lhs, 
                                   const Morphology_Type & rhs)
{
    return lhs << rhs.name();
}

// ----------------------------------------------------------------------------
/*
//! Get the morphology type for an morphology type ID.
inline Morphology_Type morphology_type(Morphology_Type_ID id)
{
return Morphology_Type(id);
}
*/
}
#endif
