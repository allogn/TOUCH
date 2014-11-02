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

#ifndef BBP_VOLUME_TEMPORAL_AVERAGE_H
#define BBP_VOLUME_TEMPORAL_AVERAGE_H

#include "BBP/Common/Exception/Exception.h"
#include "BBP/Common/Filter/Filter_Writer.h"
#include "BBP/Common/Math/Geometry/Volume.h"

namespace bbp {

#ifdef WIN32
#undef max
#endif

// ----------------------------------------------------------------------------

/*!
    \brief Computes an average over time of the values inside the volume.

    Assumes that the volume resolution does not change while running.
    \ingroup Filter
*/
template <typename Volume_Data_Type,
          typename Space_Unit = Micron, 
          typename Time_Unit = Millisecond>
class Volume_Temporal_Average
	: public Filter_Writer< Volume <Volume_Data_Type, Space_Unit, Time_Unit> >
{
public:
	//! Construct filter with defaulting to average of 1000 stream steps.
	Volume_Temporal_Average()
	{
        average_window(1000);
    }

	//! Constructs filter with a set number of steps to average.
	Volume_Temporal_Average(Count average_window_size)
	{
        average_window(average_window_size);
	}

    //! Set the number of steps to average.
    /*!
        \todo This should take into account time, not just steps in order
        to make the notion temporal make sense or the filter should be
        renamed Volume_Stream_Average. (TT)
    */
    void average_window(Count average_window_size)
    {
		if (average_window_size == 0)
        {
            throw_exception(std::runtime_error("Volume Temporal Average Filter"
                ": average window size can not be zero."), WARNING_LEVEL,
                __FILE__, __LINE__);
        }

        this->average_window_size = average_window_size;
        this->summed_steps = 0;
    }

	//! Initializes and starts processing of the filter.
	void start()
	{
        Filter_Writer< Volume <Volume_Data_Type, 
                               Space_Unit, 
                               Time_Unit> >::start();
	}

	/*!
        \brief Computes the temporal average of the volume data for set number 
        of steps.
    */
	inline void process()
	{
		Volume<Volume_Data_Type, Space_Unit, Time_Unit> 
            & input_volume = this->input();

        if (this->initialized == false)
        {
		    volume_resolution = this->input().resolution();
            if (averaged_volume.resolution() != volume_resolution)
            {
		        averaged_volume.resize(	volume_resolution.x(), 
								        volume_resolution.y(), 
								        volume_resolution.z());
            }
            this->initialized = true;
        }

		// add current input volume to summed volume
		for (Count x = 0; x < volume_resolution.x(); ++x)
		for (Count y = 0; y < volume_resolution.y(); ++y)
		for (Count z = 0; z < volume_resolution.z(); ++z)
		{
			// check for potential variable overflow in debug mode only
			bbp_assert (averaged_volume(x, y, z) + input_volume (x, y, z) 
				<=  std::numeric_limits<Volume_Data_Type>::max());

			averaged_volume(x, y, z) += input_volume(x, y, z);
		}
		
		// count number of already summed input volumes
		++summed_steps;

		// average the summed data when average window size reached
		if (summed_steps == average_window_size)
		{
			summed_steps = 0;

			// divide by number of samples that were summed up
			for (Count x = 0; x < volume_resolution.x(); ++x)
			for (Count y = 0; y < volume_resolution.y(); ++y)
			for (Count z = 0; z < volume_resolution.z(); ++z)
			{
				averaged_volume(x, y, z) /= average_window_size;
			}

			try 
			{
				// write result to output buffer
				std::cout << "Writing temporal averaged volume to output "
                    "buffer" << std::endl;
				this->output().write(averaged_volume, 
                    this->_write_interruption.get());
			}
			catch(typename Buffer<
                Volume<Volume_Data_Type, Space_Unit, Time_Unit> >::
                Buffer_Interruption &)
			{
#ifndef NDEBUG
				std::cout << "Volume_Temporal_Average interrupted" << std::endl;
#endif
			}
			catch (...)
			{
				std::cerr << "Exception caught in Compartment_Report_Reader"
                    "::frame" << std::endl;
				// Exception generated inside read_frame.
				Filter_Writer< Volume <
                    Volume_Data_Type, 
                    Space_Unit, 
                    Time_Unit> >::pause();
			}

			// reset average
			for (Count x = 0; x < volume_resolution.x(); ++x)
			for (Count y = 0; y < volume_resolution.y(); ++y)
			for (Count z = 0; z < volume_resolution.z(); ++z)
			{
				averaged_volume(x, y, z) = 0;
			}
		}
	}

private:
	Volume <Volume_Data_Type, Space_Unit, Time_Unit>	
                        averaged_volume;
	Count				average_window_size, 
						summed_steps;
	Tensor_Resolution	volume_resolution;
};

// ----------------------------------------------------------------------------

}
#endif
