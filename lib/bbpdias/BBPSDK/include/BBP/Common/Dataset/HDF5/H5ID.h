/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors: Juan Hernando Vieites
*/

#include <iostream>
#include <hdf5.h>

namespace bbp
{

//! A class that wraps a hid_t from hdf5 library (This class is not user friendly).
/*!
    \todo The interface and documentation of this class needs to be clarified 
    to be understandable. In general I think we should write generic HDF5 code
    snippets that will do standard jobs quicker, and more robustly to speedup
    implementation. For most cases this will be good enough. (TT)
*/
class H5ID
{
public:
    // Constructors
    H5ID() :
        _id(-1),
        _H5close(0)
    {}

    H5ID(hid_t id, herr_t (*close_function)(hid_t)) :
        _id(id),
        _H5close(close_function)
    {}

    // Destructor
    ~H5ID()
    {
        // only close if the id is valid
        if (_id >= 0)
        {
            close();
        }
    }
    
    //! Copy contruction with ownership tranference semantics
    H5ID(H5ID &id)
    {
        *this = id;
    }

private:
    //! Assignment with ownership transference semantics.
    H5ID & operator=(H5ID &id)
    {
        close();
        _id = id._id;
        _H5close = id._H5close;
        id._id = -1;
        id._H5close = 0;

        return *this;
    }
    
public:
    void reset()
    {
        close();
    }

    void reset(hid_t id, herr_t (*close_function)(hid_t))
    {
        close();
        _id = id;
        _H5close = close_function;
    }

    // Conversion to bool
    operator bool() const
    {
        return _id >= 0 || (_H5close == 0 && _id == -1);
    }

    operator hid_t() const
    {
        return _id;
    }

protected:
    void close()
    {
        if (_H5close != 0) {
            if (_H5close(_id) < 0)
            {
                std::cout << "Error closing HDF5 object" << std::endl;
            }
            _id = -1;
            _H5close = 0;
        }
    }

protected:
    hid_t _id;
    herr_t (*_H5close)(hid_t);
};

}
