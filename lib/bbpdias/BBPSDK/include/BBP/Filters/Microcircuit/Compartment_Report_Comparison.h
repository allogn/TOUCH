/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

		Authors: Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_COMPARTMENT_REPORT_COMPARISON_H
#define BBP_COMPARTMENT_REPORT_COMPARISON_H

#include "BBP/Common/Filter/Filter.h"
#include "BBP/Common/Dataset/Containers/Buffer.h"
#include "BBP/Model/Experiment/Compartment_Report_Frame.h"

namespace bbp {

/* ! NOTE THIS IS AN EXPERIMENTAL FILTER ! */

template <typename Stream_Data_Type = Millivolt>
struct Compartment_Report_Frame_Buffer_Duo
{
	Filter_Data <Buffer<Compartment_Report_Frame<Millivolt> > > first;
	Filter_Data <Buffer<Compartment_Report_Frame<Millivolt> > > second;
};

template <typename T>
std::ostream & operator << (std::ostream & lhs, Compartment_Report_Frame_Buffer_Duo<T> & rhs)
{
	return lhs;
}



//! draft of a comparison filter - currently computes difference of two reports
template <typename Stream_Data_Type = Millivolt>
class Compartment_Report_Comparison 
	: public Filter < 
		Compartment_Report_Frame_Buffer_Duo <Stream_Data_Type>,
		Compartment_Report_Frame <Stream_Data_Type> >
{
public:

	Compartment_Report_Comparison() 
		: advance_pending_first_buffer(false),
		  advance_pending_second_buffer(false)
	{
		write_interruption_first.reset(this->input->first->create_write_interruption());
		read_interruption_first.reset(this->input->first->create_read_interruption());
		write_interruption_second.reset(this->input->second->create_write_interruption());
		read_interruption_second.reset(this->input->second->create_read_interruption());
	}
	
	~Compartment_Report_Comparison() {}

	//! start process
	inline virtual void start()
	{
		bbp_assert (first_input_frame_size == second_input_frame_size);

		Compartment_Report_Frame <Stream_Data_Type> & first_frame
			= this->input->first->read(read_interruption_first.get());

		Compartment_Report_Frame <Stream_Data_Type> & second_frame
			= this->input->second->read(read_interruption_second.get());

		first_input_frame_size = first_frame.size();
		second_input_frame_size = second_frame.size();
		* this->output = first_frame;
		this->output->allocate(first_input_frame_size);

		//this->output_frame_size = this->output->size();

		this->stream_state = STREAM_STARTED;
	}

	inline virtual void process()
	{
		// Removing from buffer last object used if needed
		if (advance_pending_first_buffer)
		{
			this->input->first->advance_read_position();
		}

		if (advance_pending_second_buffer)
		{
			this->input->second->advance_read_position();
		}


		try
		{
			Compartment_Report_Frame <Stream_Data_Type> & first_frame
				= this->input->first->read(read_interruption_first.get());
			advance_pending_first_buffer = true;

			Compartment_Report_Frame <Stream_Data_Type> & second_frame
				= this->input->second->read(read_interruption_second.get());
			advance_pending_second_buffer = true;


			Stream_Data_Type * first_input_frame = first_frame.frame();
			Stream_Data_Type * second_input_frame = second_frame.frame();
			Stream_Data_Type * output_frame = this->output->frame();
			

			size_t i = 0;
			//for (size_t i = 0; i < first_input_frame_size; ++i)
			{
				output_frame[i] = first_input_frame[i] - second_input_frame[i];
#ifndef NDEBUG
				std::cout << "Frame A(" << first_frame.time() << " ms) : "
                          << first_input_frame[i] << " - " 
                          << "Frame B(" << second_frame.time() << " ms) : " 
                          << second_input_frame[i] << " = " 
                          << output_frame[i] << std::endl;
#endif
			}		
		}
		catch (...)
		{
			// Read was interrupted don't have to advance next call.
			advance_pending_first_buffer = false;
			advance_pending_second_buffer = false;
		}
	}

	void stop()
	{
		if (this->stream_state != STREAM_STOPPED)
		{
			// Cancel reads and writes to buffer.
//#ifdef BBP_THREADS_SUPPORTED
			read_interruption_first->signal();
			write_interruption_first->signal();
			read_interruption_second->signal();
			write_interruption_second->signal();
//#endif
			advance_pending_first_buffer = false;
			advance_pending_second_buffer = false;
			this->stream_state = STREAM_STOPPED;
		}
	}
protected:
	bool advance_pending_first_buffer, 
		 advance_pending_second_buffer;
    typedef std::auto_ptr<
        Buffer<Compartment_Report_Frame<Millivolt> >::Interruption>
    Buffer_Interruption_Ptr;
    Buffer_Interruption_Ptr read_interruption_first;
    Buffer_Interruption_Ptr write_interruption_first;
    Buffer_Interruption_Ptr read_interruption_second;
    Buffer_Interruption_Ptr write_interruption_second;

	size_t first_input_frame_size;
	size_t second_input_frame_size; 
	size_t output_frame_size;

};

}
#endif
