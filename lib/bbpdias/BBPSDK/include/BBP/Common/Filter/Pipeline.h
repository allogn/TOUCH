/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute
        Blue Brain Project & Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Responsible author:     Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_PIPELINE_H
#define BBP_PIPELINE_H

// standard template library __________________________________________________

#include <vector>
#include <iostream>
#include <typeinfo>
#include <stdexcept>

// boost library ______________________________________________________________

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>

#include <boost/lexical_cast.hpp>

#include "Process_Control_Thread.h"

// BBP SDK includes ___________________________________________________________

#include "BBP/Common/Exception/Exception.h"
#include "BBP/Common/Patterns/shared_ptr_helper.h"

// pipeline includes __________________________________________________________

#include "Control/Filter_Data_Interface.h"
#include "Control/Filter_Interface.h"
#include "Control/Playback_Control_Interface.h"

#include "Filter_Data.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! A stream pipeline is a linear sequence of stream filtering stages.
/*!
    The Pipeline class represents a processing stream with serial 
    processing 
    stages. A simple stream could for example threshold and average the input
    data. Each of these stages contains a processor called a 'filter'. In 
    addition to the given set of predefined filters, you can also roll
    your own filters. See the Filter_Interface class and the 
    Filter class template for more information. Note that the stream 
    destructor does not deallocate memory for filters and data objects.
    \sa Filter_Interface Filter
    \ingroup Pipeline
    \todo review with regard to correct shared_ptr use (TT)
    \todo change Filter_Data_Interface * to _Ptr, remove * for filter 
    and data (TT)
*/
//template <typename Input, typename Output = Input>
class Pipeline :
    public virtual Filter_Interface
{

public:

    //! standard constructor
    inline Pipeline();
    //! standard destructor
    inline ~Pipeline();

    // processing control functions
    /*!
        \name Control
        Process control of the pipeline.
    */
    //@{
    //! start pipeline processing (starts all filters, readers, and writers)
    inline void start();
    //! pause pipeline processing (pauses all filters, readers, and writers)
    inline void pause();
    //! stopping pipeline processing (stops all filters, readers, and writers)
    inline void stop();
    //! a single processing step of the pipeline
    inline void step();

//#ifdef BBP_THREADS_SUPPORTED
    /*! 
        \brief The control thread waits until pipeline processing is finished.
        If the pipeline is stopped this function returns immediately. 
        
        In any other case it blocks until the internal thread stops spontaneously,
        which happens when the end of the dataset been processeed is reached
        within any of the contained filters.
    */
    inline void wait_until_finished();
//#endif
    //@}

    // pipeline configuration 
    /*!
        \name Configuration
        Setup of the pipeline filters.
    */
    //@{
    //! name "Pipeline"
    inline Label name() const;
    //! Sets the mode of playback (single frame trigger, frame based or time 
    //! based stream processing)
    inline void mode(Playback_Mode playback_mode);
    //! connects a new filter to the end of the stream if types compatible
    /** DISCLAIMER: The pipeline will store a reference to the filter object. 
        It's the user responsability keep this object allocated durint the
        pipeline's lifecycle. */
    inline void push_back(Filter_Interface & new_filter);
    //! connects a new filter to the end of the stream if types compatible
    inline void push_back(const Filter_Interface_Ptr & new_filter);

    //! connects a new data object to the end of the stream
    inline void push_back(Filter_Data_Interface & new_data);

    //! connects two pipelines
    inline void push_back(Pipeline & new_pipeline);
    //! create a new pipeline with this filter as first element
    /** DISCLAIMER: The pipeline will store a reference to the filter object. 
        It's the user responsability keep this object allocated durint the
        pipeline's lifecycle. */

    inline Pipeline & operator = (Filter_Interface & rhs);
    //! create a new pipeline with this filter as first element
    inline Pipeline & operator = (const Filter_Interface_Ptr & rhs);

    //! connect stream data to last filter in pipeline
    inline Pipeline & operator + (Filter_Data_Interface & rhs);

    //! connect stream filter to end of pipeline
    /** DISCLAIMER: The pipeline will store a reference to the filter object. 
        It's the user responsability keep this object allocated durint the
        pipeline's lifecycle. */
    inline Pipeline operator + (Filter_Interface & rhs);
    //! connect stream filter to end of pipeline
    inline Pipeline operator + (const Filter_Interface_Ptr & rhs);

    //! removes all the pipeline filters
    inline void clear();
    //! number of filters in the stream
    inline size_t size() const;

    //! Forward iterator for the filters in the pipeline to visit all elements
    class iterator;
    //! Returns the first element of the container for linear access
    inline iterator begin(void);
    //! indicates the end of the filter chain has been reached
    inline iterator end(void);
    //! print configuration status information
    inline void print() const;
    //@}

    //! if filter is connected to input data
    bool input_connected() const
    {
        if (filters.empty())
        {
            return false;
        }
        else
        {
            return filters.front()->input_connected();
        }
    }

    //! if filter is connected to output data
    bool output_connected() const
    {
        if (filters.empty())
        {
            return false;
        }
        else
        {
            return filters.back()->output_connected();
        }    
    }

private:
    //! indicates if pipeline is configured properly
    mutable bool                         configured;
    //! Playback mode of the stream pipeline decides if processing is 
    //! done manually, time or frame based.
    Playback_Mode                        playback_mode;
//#ifdef BBP_THREADS_SUPPORTED
    //! thread driving the pipeline processing
    Process_Control_Thread <Pipeline>    thread;
//#endif
    //! linear pipeline of connected filters
    std::vector <Filter_Interface_Ptr>   filters;

private:

    //! Trigger for a processing step.
    inline void process() 

	throw(End_Of_Stream);

    //! check if pipeline is configured properly before processing
    inline bool is_configured() const;
    //! Check if the pipeline end is connected to an output data stream.
    inline bool is_connected() const;

    //! access the input object
    /*
        \sa Filter_Interface Pipeline
    */
    inline Filter_Data_Interface * input_ptr();
    //! Get the stream filter output.
    /*
        \sa Filter_Interface Pipeline
    */
    inline Filter_Data_Interface * output_ptr();
    //! Set the stream filter input.
    /*
        \sa Filter_Interface Pipeline
    */
    inline void input_ptr(Filter_Data_Interface * data);
    //! Set the stream filter output.
    /*
        \sa Filter_Interface Pipeline
    */
    inline void output_ptr(Filter_Data_Interface * data);
    //! Get the stream filter output of the last filter in the pipeline.
    /*
        \sa Filter_Interface Pipeline
    */
    inline Filter_Data_Interface * back_output_pointer();
    //! Set the stream filter output of the last filter in the pipeline.
    /*
        \sa Filter_Interface Pipeline
    */
    inline void back_output_pointer
        (Filter_Data_Interface * new_output_pointer);

    inline void stop_thread_and_filters();
};

// ----------------------------------------------------------------------------

inline Pipeline operator + (const Filter_Interface_Ptr & left_filter,
                            const Filter_Interface_Ptr & right_filter)
{
    Pipeline pipeline;
    pipeline = left_filter;
    return (pipeline + right_filter);    
}

// ----------------------------------------------------------------------------

inline Pipeline operator + (const Filter_Interface_Ptr & left_filter,
                            Filter_Interface & right_filter)
{
    Pipeline pipeline;
    pipeline = left_filter;
    return (pipeline + right_filter);    
}

// ----------------------------------------------------------------------------

//std::ostream & operator << 
//(std::ostream & lhs, Stream_Pipeline_Interface & rhs)
//{
//  lhs << rhs.name() << std::endl;
//  lhs << L"Input: " << * rhs.input_ptr();
//  lhs << L"Output: " << * rhs.output_ptr();
//  return lhs;
//}

// ----------------------------------------------------------------------------

Pipeline::Pipeline() 
    : configured(false), 
//#ifdef BBP_THREADS_SUPPORTED
      playback_mode(PROCESS_FRAME_BASED)
//#else
//    playback_mode(PROCESS_SINGLE_STEP_TRIGGER)
//#endif
{}

// ----------------------------------------------------------------------------

Pipeline::~Pipeline()
{
#ifdef BBP_DEBUG
    //! \todo remove this debug output.
    std::cout << "Pipeline::~Pipeline()" 
        << std::endl;
#endif
    log_message("Deleting pipeline.", DEBUG_LEVEL, __FILE__);
    if (stream_state != STREAM_STOPPED)
    {
        stop();
    }
}

// ----------------------------------------------------------------------------

void Pipeline::start()
{
    log_message("Starting pipeline.", DEBUG_LEVEL, __FILE__, __LINE__);

    // check if pipeline is properly configured
    if (is_configured() == true)
    {
        // start all the filters
        for (Count filter_counter = 0; filter_counter < size(); 
             ++filter_counter)
        {
            filters[filter_counter]->start();
            //! \todo below boost::format produced security warning on VC9 (TT)
            //log_message(boost::str(boost::format("Started filter %1%") %
            log_message(std::string("Started filter ") 
                + boost::lexical_cast<std::string>(filter_counter),
                        DEBUG_LEVEL, __FILE__, __LINE__);
        }

        // manual triggering of frames
        if (playback_mode == PROCESS_SINGLE_STEP_TRIGGER) 

        {
            log_message("Stream pipeline started in single frame trigger mode",
                        INFORMATIVE_LEVEL);
        }
//#ifdef BBP_THREADS_SUPPORTED
        else if (playback_mode == PROCESS_FRAME_BASED)
        {
            // attach the stream processing to the thread if not yet done
            if (thread.is_attached() == false)
            {
                // Cancel function not needed since thread is destructed after
                // the pipeline is completely stopped.
                thread.attach(this, &Pipeline::step, 0);
            }

            // then start the pipeline thread
            thread.start();
            log_message("Started pipeline processing.", DEBUG_LEVEL, 
                        __FILE__, __LINE__);
        }
        else if (playback_mode == PROCESS_TIME_BASED)
        {
            std::cerr << "Time based processing is not implemented for"
                         " the pipeline at this time." << std::endl;
            abort();
        }
        else if (playback_mode == PROCESS_REALTIME_BASED)
        {
            std::cerr << "Realtime based processing is not implemented for"
                         " the pipeline at this time." << std::endl;
            abort();
        }
//#endif 
        else
        {
            throw_exception(std::logic_error("The specified pipeline"
                                             " processing mode is not valid."),
                            SEVERE_LEVEL, __FILE__, __LINE__);
        }

        stream_state = STREAM_STARTED;
    }
    else
    {
        log_message("Pipeline start failed. Pipeline is not configured.",
                    INFORMATIVE_LEVEL);
    }
}

// ----------------------------------------------------------------------------

void Pipeline::pause()
{
    log_message("Starting pipeline.", DEBUG_LEVEL, __FILE__, __LINE__);
//#ifdef BBP_THREADS_SUPPORTED
    thread.pause();
//#endif

    for (Count filter_counter = 0; filter_counter < size(); ++filter_counter)
    {
        filters[filter_counter]->pause();
        //! \todo below boost::format produced security warning on VC9 (TT)
        log_message(std::string("Paused filter ") 
            + boost::lexical_cast<std::string>(filter_counter),
                    DEBUG_LEVEL, __FILE__, __LINE__);
    }

    stream_state = STREAM_PAUSED;
}

// ----------------------------------------------------------------------------

void Pipeline::stop()
{
    stop_thread_and_filters();
//#ifdef BBP_THREADS_SUPPORTED
    log_message("Waiting for pipeline thread to be suspended", 
                DEBUG_LEVEL, __FILE__, __LINE__);
    // Waiting for the internal thread to be stopped.
    thread.wait_for_suspend();
//#endif

    stream_state = STREAM_STOPPED;
    log_message("Pipeline stopped", DEBUG_LEVEL, __FILE__, __LINE__);
}

// ----------------------------------------------------------------------------

void Pipeline::step()
{
    try
    {
        process();
    }
    catch (std::exception & e)
    {
        // When a End_Of_Stream or any other unknown exception is received
        // we stop the pipeline. The stop function will not be used, it would
        // lead to a dead-lock.
        stop_thread_and_filters();
        stream_state = STREAM_STOPPED;
        log_message("Pipeline stopped", DEBUG_LEVEL, __FILE__, __LINE__);
        std::cerr << e.what() << std::endl;
        throw;
    }
    catch (...)
    {
        // When a End_Of_Stream or any other unknown exception is received
        // we stop the pipeline. The stop function will not be used, it would
        // lead to a dead-lock.
        stop_thread_and_filters();
        stream_state = STREAM_STOPPED;
        log_message("Pipeline stopped", DEBUG_LEVEL, __FILE__, __LINE__);
        throw;
    }
}

// ----------------------------------------------------------------------------

//#ifdef BBP_THREADS_SUPPORTED
void Pipeline::wait_until_finished()
{
    thread.wait_for_stop();
}
//#endif

// ----------------------------------------------------------------------------

Label Pipeline::name() const
{
    return "Pipeline";
}

// ----------------------------------------------------------------------------

void Pipeline::mode(Playback_Mode playback_mode)
{
    this->playback_mode = playback_mode;
}

// ----------------------------------------------------------------------------

Pipeline & Pipeline::operator + (Filter_Data_Interface & rhs)
{
    push_back(rhs);
    return * this;
}

// ----------------------------------------------------------------------------

Pipeline Pipeline::operator + (Filter_Interface & rhs)
{
    push_back(rhs);
    return * this;
}

// ----------------------------------------------------------------------------

Pipeline Pipeline::operator + (const Filter_Interface_Ptr & rhs)
{
    push_back(rhs);
    return * this;
}

// ----------------------------------------------------------------------------

void Pipeline::push_back(Pipeline & new_pipeline)
{
    // if pipeline empty
    if (filters.empty() == true)
    {
        // add pipeline to end of pipeline
        for (Count i = 0; i < new_pipeline.size(); ++i)
        {
            push_back(new_pipeline.filters[i]);
        }
    }
    // if pipeline not empty
    else
    {
        if (filters.back()->output_ptr()->is_connected() == false)
        {
            if (new_pipeline.input_ptr()->is_connected() == false)
            {
                //! \todo throw_exception()
                throw std::runtime_error("Pipelines miss a shared "
                                         "stream data object");
            }
            else
            {
                filters.back()->output_ptr(new_pipeline.input_ptr());
            }
        }
        else
        {
            if (new_pipeline.input_ptr()->is_connected() == false)
            {
                new_pipeline.input_ptr(filters.back()->output_ptr());
                try
                {
                    if (new_pipeline.output_ptr()->is_connected() == false)
                        new_pipeline.output_ptr
                            (filters.back()->output_ptr());
                }
                catch (Stream_Data_Incompatible e)
                {
                }
            }
            else
            {
                filters.back()->output_ptr(new_pipeline.input_ptr());
                try
                {
                    if (new_pipeline.output_ptr()->is_connected() == false)
                        new_pipeline.output_ptr
                            (filters.back()->output_ptr());
                }
                catch (Stream_Data_Incompatible e)
                {
                }

            }
        }
    }
}

// ----------------------------------------------------------------------------

void Pipeline::push_back(Filter_Data_Interface & data)
{
    // if pipeline empty
    if (filters.empty() == true)
    {
        input_ptr(& data);
    }
    // if pipeline not empty
    else
    {
        filters.back()->output_ptr(& data);
    }
}

// ----------------------------------------------------------------------------

void Pipeline::push_back(Filter_Interface & new_filter)
{
    push_back(Filter_Interface_Ptr(& new_filter, shared_ptr_non_deleter));
}

// ----------------------------------------------------------------------------

void Pipeline::push_back(const Filter_Interface_Ptr & new_filter)
{
    if (filters.empty())
    {
        filters.push_back(new_filter);
    }
    else
    {
        if (new_filter->input_ptr()->is_connected() == false)
        {
            if (filters.back()->output_ptr()->is_connected() == false)
            {
                throw std::runtime_error(
                    "No stream data between filters, "
                    "could not add filter to pipeline");
            }
            else
            {
                new_filter->input_ptr(
                    filters.back()->output_ptr());
                try
                {
                    //if (new_filter.output_ptr()->is_connected() == false)
                    new_filter->output_ptr(
                        filters.back()->output_ptr());
                }
                catch (Stream_Data_Incompatible e)
                {
                }

            }
        }
        else
        {
            if (filters.back()->output_ptr()->is_connected() == false)
            {
                filters.back()->output_ptr(new_filter->input_ptr());
            }
            else
            {
                new_filter->input_ptr(filters.back()->output_ptr());
                try
                {
                    //if (new_filter.output_ptr()->is_connected() == false)
                    new_filter->output_ptr(
                        filters.back()->output_ptr());
                }
                catch (Stream_Data_Incompatible e)
                {
                }
            }
        }
        filters.push_back(new_filter);
    } 
}

// ----------------------------------------------------------------------------

Pipeline & Pipeline::operator = (Filter_Interface & rhs)
{
    clear();
    Pipeline::operator + (rhs);
    return * this;
}

// ----------------------------------------------------------------------------

Pipeline & Pipeline::operator = (const Filter_Interface_Ptr & rhs)
{
    clear();
    Pipeline::operator + (rhs);
    return * this;
}

// ----------------------------------------------------------------------------

size_t Pipeline::size() const
{
    return filters.size();
}

// ----------------------------------------------------------------------------

void Pipeline::clear()
{
    stop();
    filters.clear();
    configured = false;
}

// ----------------------------------------------------------------------------

void Pipeline::print() const
{
    std::cout << "Pipeline (" << size() << " filters) = { ";
    for (Count i = 0; i < size(); ++i)
    {
        std::cout << filters[i]->name() << " (";
        if (filters[i]->input_ptr()->is_connected() == true)
        {
            std::cout << * filters[i]->input_ptr() << ", ";
        }
        else
        {
            std::cout << "-, ";
        }

        if (filters[i]->output_ptr()->is_connected() == true)
        {
            std::cout << * filters[i]->output_ptr() << ")";
        }
        else
        {
            std::cout << "-)";
        }
        if (i < size() - 1)
        {
            std::cout << " + ";
        }
        else
        {
            std::cout << " }" << std::endl;
        }
    }
}

#ifndef SWIG // ---------------------------------------------------------------

class Pipeline::iterator :
    public std::iterator<std::forward_iterator_tag, Filter_Interface>
{
public:     
    iterator(std::vector<Filter_Interface_Ptr>::iterator 
             pipeline_iterator)
        : current(pipeline_iterator)
    {}

    iterator() {}

    ~iterator () {}

public:
    iterator & operator = (const iterator & righthand_side)
    {
        current = righthand_side.current;
        return (*this);
    }

    // ==
    bool operator == (const iterator & righthand_side) const
    {
        return (current == righthand_side.current);
    }

    // !=
    bool operator != (const iterator & righthand_side) const
    {
        return (current != righthand_side.current);
    }

    // ++prefix
    iterator & operator ++ ()
    {
        ++current;
        return *this;
    }

    // postfix++
    iterator operator ++ (int)
    {
        iterator tmp(*this);
        current++;
        return(tmp);
    }

    // *
    Filter_Interface & operator * ()
    {
        return  ** current;
    }

    // ->
    Filter_Interface * operator -> ()
    {
        return (& * (iterator) *this);
    }

    const Filter_Interface_Ptr & ptr()
    {
        return * current;
    }

private:
    std::vector<Filter_Interface_Ptr>::iterator current;

};

#endif // SWIG ----------------------------------------------------------------

Pipeline::iterator Pipeline::begin(void)
{
    return iterator ( filters.begin() );
}

// ----------------------------------------------------------------------------

Pipeline::iterator Pipeline::end(void)
{
    return iterator ( filters.end() );
}

// ----------------------------------------------------------------------------

Filter_Data_Interface * Pipeline::input_ptr()
{
    if (filters.empty())
    {
        throw std::runtime_error("Pipeline is empty therefore "
                                 "no input object");
    }
    else
    {
        return filters.front()->input_ptr();
    }
}

// ----------------------------------------------------------------------------

Filter_Data_Interface * Pipeline::output_ptr()
{
    if (filters.empty())
    {
        throw std::runtime_error("Pipeline is empty therefore "
                                 "no output object");
    }
    else
    {
        return filters.back()->output_ptr();
    }
}

// ----------------------------------------------------------------------------

void Pipeline::input_ptr(Filter_Data_Interface * data)
{
    if (filters.empty())
    {
        throw std::runtime_error("Pipeline is empty therefore"
                                 " no input object");
    }
    else
    {
        filters.front()->input_ptr(data);
    }
}

// ----------------------------------------------------------------------------

void Pipeline::output_ptr(Filter_Data_Interface * data)
{
    if (filters.empty())
    {
        throw std::runtime_error("Pipeline is empty therefore"
                                 " no output object");
    }
    else
    {
        filters.back()->output_ptr(data);
    }
}


// ----------------------------------------------------------------------------

Filter_Data_Interface * Pipeline::back_output_pointer()
{
    if (filters.empty())
    {
        throw std::runtime_error("Pipeline is empty therefore "
                                 "no output object");
    }
    else
    {
        return filters.back()->output_ptr();
    }
}

// ----------------------------------------------------------------------------

void Pipeline::back_output_pointer
(Filter_Data_Interface * new_output_pointer)
{
    if (filters.empty())
        throw std::runtime_error("no filters in pipeline");
    else
    {
        filters.back()->output_ptr(new_output_pointer);
        log_message(std::string("Connect output of last filter in pipeline"
                                " to stream data of type ") +
                    typeid(filters.back()->output_ptr()).name(),
                    DEBUG_LEVEL, __FILE__, __LINE__);
    }
}

// ----------------------------------------------------------------------------

void Pipeline::process() 
        throw (End_Of_Stream)

{
    for (Count pipeline_step = 0; 
         pipeline_step < filters.size(); 
         ++pipeline_step)
    {
        try 
        {
            filters[pipeline_step]->process();
        }
        catch (Stream_Interruption &) 
        {
            // No filter will be further processed.
            // Aborting iteration
            break;
        }
        catch (End_Of_Stream &)
        {
            //! \todo below boost::format produced security warning on VC9 (TT)
            log_message(std::string("Filter ") + 
                boost::lexical_cast<std::string>(pipeline_step) + "'" + 
                boost::lexical_cast<std::string>(
                    filters[pipeline_step]->name()) + "'" +
                std::string(" threw End_Of_Stream exception"),
                DEBUG_LEVEL, __FILE__, __LINE__);
            // stop or throw ?
            //throw;
            stop_thread_and_filters();
            stream_state = STREAM_STOPPED;
            break;
        }
        catch (std::exception & e) 
        {
            log_message(std::string("Pipeline::process: unknown exception "
                                    " caught ") + e.what(), 
                        SEVERE_LEVEL, __FILE__, __LINE__);
            break;
        }
        catch (...) 
        {
            log_message(std::string("Pipeline::process: unknown exception "
                                    " caught "), 
                        SEVERE_LEVEL, __FILE__, __LINE__);
            break;
        }
    }
}

// ----------------------------------------------------------------------------

bool Pipeline::is_connected() const
{
    if ((* filters.front()).input_ptr()->is_connected() == true &&
        (* filters.back()).output_ptr()->is_connected() == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

bool Pipeline::is_configured() const
{
    bool check_passed = true;

    for (Count filter_counter = 0; filter_counter < size(); ++filter_counter)
    {
        Filter_Interface & filter = * filters[filter_counter];
        if (filter.is_connected() == false)
        {
            log_message(
                std::string("filter ")
                + boost::lexical_cast<std::string>(filter_counter)
                + std::string("input: ") 
                + typeid(filter.input_ptr()).name() 
                + std::string(" ")
                + boost::lexical_cast<std::string>(
                    filter.input_ptr()->is_connected())
                + std::string(" output: ")
                + typeid(filter.output_ptr()).name() 
                + std::string(" ")
                + boost::lexical_cast<std::string>(
                    filter.output_ptr()->is_connected()),
                DEBUG_LEVEL, __FILE__, __LINE__);
            check_passed = false;
        }
    }
    configured = check_passed;

    if (check_passed == false)
        throw std::runtime_error("Pipeline configuration checked failed "
            "(pipeline not set up properly).");

    return configured;
}

// ----------------------------------------------------------------------------

inline void Pipeline::stop_thread_and_filters()
{
//#ifdef BBP_THREADS_SUPPORTED
    log_message("Stopping pipeline thread.", DEBUG_LEVEL, __FILE__, __LINE__);
    thread.stop();
//#endif
    // Telling every filter to stop processing. If pipeline thread was
    // blocked inside any process function (e.g. waiting for an empty
    // buffer to be pushed) that process will be interrupted and the
    // pipeline thread can be waited for supension.
    for (Count filter_counter = 0; filter_counter < size(); ++filter_counter)
    {
        log_message(std::string("Stopping filter ") 
            + boost::lexical_cast<std::string>(filter_counter),
            DEBUG_LEVEL, __FILE__, __LINE__);
        filters[filter_counter]->stop();
        log_message(std::string("Stopped filter ")
            + boost::lexical_cast<std::string>(filter_counter),
            DEBUG_LEVEL, __FILE__, __LINE__);
    }    
}

// ----------------------------------------------------------------------------


}
#endif
