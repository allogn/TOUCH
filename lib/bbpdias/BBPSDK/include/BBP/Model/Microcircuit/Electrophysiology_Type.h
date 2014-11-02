/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Anirudh Vij
*/
#ifndef BBP_ELECTROPHYSIOLOGY_TYPE_H
#define BBP_ELECTROPHYSIOLOGY_TYPE_H

#include "Types.h"
#include "Datasets/Structure_Dataset.h"

namespace bbp {

// ----------------------------------------------------------------------------

class Microcircuit_MVD_File_Reader;

// ----------------------------------------------------------------------------

//! Classification of the electrophysiology (electrical behavior) of a cell.
/*!
  \todo default constructor links a Structure_Dataset, but make sure the way
  the Morphologies are created in the Structure_Dataset during loading 
  share the dataset (AV).
*/
class Electrophysiology_Type
{
    friend class Microcircuit_MVD_File_Reader;

public:
    Electrophysiology_Type()
    {
    }

    Electrophysiology_Type(Electrophysiology_Type_ID id,
                           const Structure_Dataset_Ptr & dataset) 
    :
        _id(id),
        _dataset(dataset) 
    {}

    //! Get identifier of the e-type.
    Electrophysiology_Type_ID id() const
    {
        return _id; 
    }

    //! Get name of the e-type.
    inline const Label name() const
    {
        if (_id >= _dataset->Electrophysiology_Types_Array.size()) {
            log_message("Possible inconsistency detected. Found an "
                        "electrophysiology ID for which there is no "
                        "known name", SEVERE_LEVEL);
            return "Unknown";
        }
        return (_dataset->Electrophysiology_Types_Array[_id]);
    }

    //! Set identifier of the e-type.
    void id(Electrophysiology_Type_ID i)
    {
        _id = i;
    }

private:
    Electrophysiology_Type_ID   _id;
    Structure_Dataset_Ptr       _dataset;
};

// ----------------------------------------------------------------------------

inline std::ostream & operator << (std::ostream & lhs, 
                                   const Electrophysiology_Type & rhs)
{
    return lhs << rhs.name();
}

// ----------------------------------------------------------------------------
/*
//! Get the electrophysiology type for an electrophysiology type ID.
inline Electrophysiology_Type electrophysiology_type(Electrophysiology_Type_ID id)
{
return Electrophysiology_Type(id);
}
*/
}
// ----------------------------------------------------------------------------
#endif
