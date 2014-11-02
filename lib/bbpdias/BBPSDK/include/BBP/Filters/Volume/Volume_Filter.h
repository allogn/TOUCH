/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MICROCIRCUIT_VOLUME_FILTER_H
#define BBP_MICROCIRCUIT_VOLUME_FILTER_H

#include "BBP/Common/Types.h"
#include "BBP/Common/Math/Geometry/Volume.h"
#include "BBP/Common/Filter/Filter.h"


namespace bbp {

/*!
    This is a volume filter class template that assumes two volumes of same 
    dimensions. To operate on volume data, you only need to implement a
    function that knows how to operate on a single volume element (voxel).
    \ingroup Filter
*/
template <typename Input, typename Output = Input>
class Volume_Filter
    : public Filter<Volume<Input, Micron, Millisecond>, 
                    Volume<Output, Micron, Millisecond> >
{
public:
    Volume_Filter();

    //! start volume filter
    inline void start();
    //! stream processing step for whole volume
    inline void process() = 0;

    //! access the input object \sa{Pipeline} \sa{Filter_Interface}
    inline Filter_Data<Volume<Input, Micron, Millisecond> > * input_ptr();
    //! access the output object \sa{Pipeline} \sa{Filter_Interface}
    inline Filter_Data<Volume<Output, Micron, Millisecond> > * output_ptr();  
    //! assign the filter input object \sa{Pipeline} \sa{Filter_Interface}
    inline void input_ptr(
        Filter_Data_Interface * new_input_pointer);
    //! assign the filter output object \sa{Pipeline} \sa{Filter_Interface}
    inline void output_ptr(
        Filter_Data_Interface * new_output_pointer);
};


// ----------------------------------------------------------------------------

template <typename Input, typename Output>
Volume_Filter <Input, Output>::Volume_Filter()
{
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
Filter_Data<Volume<Input, Micron, Millisecond> > * 
Volume_Filter <Input, Output>::input_ptr()
{
    return & this->_input;
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
Filter_Data<Volume<Output, Micron, Millisecond> > * 
Volume_Filter <Input, Output>::output_ptr() 
{
    return & this->_output;
}
    
// ----------------------------------------------------------------------------

template <typename Input, typename Output>
void Volume_Filter <Input, Output>::input_ptr(
    Filter_Data_Interface * new_input_pointer)
{
    Filter<Volume<Input, Micron, Millisecond>, 
        Volume<Output, Micron, Millisecond> >::input_ptr(new_input_pointer);

    /*!
        \bug This could have undesired behavior if output already 
        connected. (TT)
    */
    if (input_ptr()->is_connected() == true && 
        output_ptr()->is_connected() == true)
    {
        Tensor_Resolution volume_resolution;
        volume_resolution = this->input().resolution();

        this->output().resize(volume_resolution.x(), 
                               volume_resolution.y(), 
                               volume_resolution.z());
    }
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
void Volume_Filter <Input, Output>::output_ptr(
    Filter_Data_Interface * new_output_pointer)
{
    Filter <
        Volume <Input, Micron, Millisecond>, 
        Volume <Output, Micron, Millisecond> >::output_ptr(new_output_pointer);
    if (input_ptr()->is_connected() == true && 
        output_ptr()->is_connected() == true)
    {
        Tensor_Resolution volume_resolution;
        volume_resolution = this->input().resolution();

        this->output().resize(
            volume_resolution.x(), 
            volume_resolution.y(), 
            volume_resolution.z());
    }
}

// ----------------------------------------------------------------------------

template <typename Input, typename Output>
void Volume_Filter<Input, Output>::start()
{
    /*!
        \todo Make sure the output is resized, but now with changed model
        due to Juan's needs, that is not possible right now and needs
        reconsideration. Currently done manually in the process function
        by the filter implementor. (TT)
    */
    Filter<Volume<Input, Micron, Millisecond>, 
        Volume<Output, Micron, Millisecond> >::start();
}

// ----------------------------------------------------------------------------

}
#endif

