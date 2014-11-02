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

#ifndef BBP_REPORT_READER_H
#define BBP_REPORT_READER_H

#include <cmath>

#ifdef WIN32
#undef min		// macros collide with std::min/max
#undef max
#endif

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "BBP/Common/Math/Round.h"
#include "BBP/Model/Microcircuit/Types.h"
#include "BBP/Model/Experiment/Report_Specification.h"

namespace bbp {
    
// ----------------------------------------------------------------------------

class Report;
typedef boost::shared_ptr<Report> Report_Ptr;
class Report_Reader;
typedef boost::shared_ptr<Report_Reader> Report_Reader_Ptr;

class Compartment_Report;
class Report_Frame;

// Report_Reader interface class _____________________________________________________

//! Abstract class for report read access.
/*!
    This class provides the common report interface to be used for reading
    data. Different functions are provided to enable full, per frame and
    stream based loading. Derived classes are specialized for different 
    types of reports.

    While most of the cases may have been covered with a float data
    type (single precision floating point, typically 32 bits),
    reported data may have different units or even report structures
    or objects. The interface on the one hand is prepared for
    such data types and on the other hand by using unit specific
    classes as types (e.g. Millivolt class) enforces type safety
    during conversions. Further, it allows to access the unit of the
    variable in the static unit member variable).
    
    Compartment_Report_Reader and Synapse_Report_Reader (once it exists) class
    templates derive from this class.
*/
class Report_Reader 
	: private boost::noncopyable
{
protected:
	//! Construct an uninitialized report reader.
    inline Report_Reader() {};
	//! Construct and initialize a report reader with report specification.
    inline Report_Reader(const Report_Specification & specs);

public:
    virtual ~Report_Reader() {}

    //! Report specification.
    inline const Report_Specification & specs() const;

    //! Opens report data source and prepares it for loading.
    /*! If the report is already open it will be implicitly closed first.
        Before actually loading any data the target of interest has to be
        set. This is report specific, for example, for Compartment_Report
        a Cell_Target must be specified.

       The time window is initialized to the [start, end) interval specified 
       in the specs. */
    virtual void open() = 0
        /* throw (Bad_Data, IO_Error) */;

    //! Opens report data source and prepares it for loading.
    /* Overlodaded function provided for convenience. It acts just as
       the function above but adding report specification replacament.

       The time window is initialized to the [start, end) interval specified 
       in the specs. */
    inline void open(const Report_Specification & specs)
        /* throw (IO_Error, Bad_Data)*/;

    //! Closes the report data source.
    /* All resources allocated by this object will be freed */
    virtual void close() = 0;

    //! Sets the window of interest.
    /*! This function selects the time window that will be relevant for all
        the loading functions and for frame count calculation.
        The default time window is the whole report.

        Timestamps given are rounded to the nearest multiple of the original
        report timestep and clamped to the original start and end times.
        The time interval must be considered open on the right. This means 
        that the following holds:
        \f$(end\_time - start\_time) / specs.timestep = frame\_count\f$

        Setting the interest window implicitly performs a reset to frame 0
        so it requires that method to be supported in the final reader. 

        \sa Report_Specification::end_time(). */
    inline void time_window(Millisecond start_time, Millisecond end_time)
        /* throw (Unsupported, Bad_Data) */;

    //!  Returns the number of frames of the current time window.
    inline virtual Frame_Number window_frame_count() const;

    //! Returns the current interest window start time
    inline virtual Millisecond start_time() const;

    //! Returns the current interest window end time
    inline virtual Millisecond end_time() const;

    // DATA ACCESS INTERFACE --------------------------------------------------
    
    // The following functions must be only used after opening the report

    //! Loads the full report in memory
    /*! The time window loaded depends on the current interest window. Further
        access to the data depends on the final derived classes.
        Data loaded can be invalidated by target selection funtions of derived
        classes (refer to the relevant report type class for details).
        Data sources are not required to support this feature.
        \sa time_window() */
    virtual void load_full_report() = 0
        /* throw (IO_Error, Unsupported) */;

    /*! \brief Returns true if and only if the given timestamp is accessible 
        in a preloaded buffer. 
        If this function returns true is not guarantee that the data is still
        there at the moment of reading it. \sa load_full_report. 
		\todo This needs clarification of the conditions that lead to
		unavailable data if returned true previously. (TT)
	*/
    virtual bool preloaded(Millisecond timestamp) const = 0;

    //! Loads data for a single frame with random access.
    /*! Buffer allocation will be managed by the final implementation.

        If this operation is supported the next frame returned by 
        load_next_frame will be the frame_number given here increased by
        the current frame skip or rate.

        If the current time window has already been loaded with
        load_full_report no additional data will be loaded and no memory
        will be allocated.

        @param frame_number A frame numbers relative to the current interest 
        window. If the frame number is out of the interest window an 
        exception is thrown. 
        @param frame Pass a report frame here that will be filled with
        the new data.
     */
    virtual void load_frame(Frame_Number frame_number, Report_Frame & frame)
        /* throw (IO_Error, Unsupported) */ = 0;

    //! Loads data for a single frame with random access.
    /*! This is an overloaded function provided for convenience. 
        Its specification is basically the same as the function above but 
        applied to timestamps. */
    inline virtual void load_frame(Millisecond timestamp, Report_Frame & frame)
        /* throw (IO_Error, Unsupported) */;

    //! Loads data for a single frame with sequential access.
    /*! For a report just opened the next frame will be frame 0.
        This is the way to access information from network streams.
        Allocation will be managed by the final implementation.
        This method will work for *all* data sources.
        \return This method returns false if before reading anything the data
        source is detected to have reached the end and the configuration is 
        not wait for new data. It returns true when some data has been 
        read. Note frame_skip() will set what is considered the next frame.
        @param frame Pass a report frame here that will be filled with
        the new data.

        \todo Provide the same specification that load_frame with regard to
        preloaded buffers. This is implementable with some minor extensions
        inside derived classes in without interfering with final reader and
        streamer classes.
        \sa Report_Reader::frame_skip(double skip_rate)
    */
    virtual bool load_next_frame(Report_Frame & frame)
        /* throw (IO_Error) */ = 0;

    //! Resets the next frame to load to a given one.
    /*! This function reinitializes the data source and sets the cursor for
        load_next_frame to the given frame number.
        When no agument is provided the source is reset to a default state
        which depends on the source type. If the source supports it, the
        frame cursor will be set to the first frame of the interest window.
        Sources are only required to support the no argument version.
        Frame numbers are relative to the current interest window and
        are not necessarily checked for correctness. */
    virtual void reset(Frame_Number frame_number = UNDEFINED_FRAME_NUMBER)
        /* throw (IO_Error, Unsupported) */ = 0;

    //! Resets the next frame to load to the nearest one to the timestamp.
    /*! This is an overloaded function provided for convenience. 
        Its specification is basically the same as the function above but 
        applied to timestamps. */
    inline virtual void reset(Millisecond timestamp)
        /* throw (IO_Error, Unsupported) */;

    //! Unloads all data from memory.
    /*! This operation unload any data internally stored or cached. This 
        method never throws. Subsequent access to the supposedly permanent 
        storage (as specified in derived classes) will be undefined. */
    virtual void unload() = 0;

    //! Tells load_next_frame to skip some amount frames each time is called.
    /*! The default frame skip for any reader will be 1. Non integer numbers
         are allowed.
         @param skip_rate A non zero real number with the number of frames 
         that must be increased the frame cursor after each call to 
         load_next_frame.
         Frame numbers returned by load_next_frame should be rounded to the 
         nearest integer. For example given a frame_skip of 1.5 load_next_frame
         will output the following sequence of framestamps: 
         0, 2, 3, 5, 6, ... */
    virtual void frame_skip(double skip_rate)
        /* throw (Unsupported) */ = 0;

protected:
    /** \cond SHOW_IN_DEVELOPER_REFERENCE */

    //! Derived classes may override this function to update the time window
    /*! The time window passed here will be already discretized with the
        timestep. */
    virtual void time_window_impl(Millisecond start __attribute__((__unused__)),
                                  Millisecond end  __attribute__((__unused__)))
//#ifdef WIN32
 //       throw(...) {}
//#else
        throw (Unsupported, Bad_Data) {}
//#endif
    
    /*! Implementation function that converts timestamps to frame numbers
        in the current interest window, clamping performed if needed.
        Report implementors must note that this function is not 
        thread-safe. */
    inline Frame_Number relative_frame_for_timestamp(Millisecond timestamp) 
        const;

    /*! Implementation function that converts timestamps to frame numbers
        in the current absolute reference window from the original report 
        specs, clamping performed if needed. */
    inline Frame_Number absolute_frame_for_timestamp(Millisecond timestamp) 
        const;

    //! Discretizes a given timestep using the time step of the report.
	/*!
		@return Nearest time sample in the report to the given time.
	*/
    inline Millisecond discretize(Millisecond time) const;

    virtual void check_reader_compatibility() 
        /* throw (Bad_Data) */ const = 0;

protected:
    Report_Specification     _specification;

    Millisecond      _start_time; /*!< Current interest window start time.
                                       To be set by window() */
    Millisecond      _end_time;   /*!< Current interest window end time.
                                       To be set by window() */

    /** \endcond SHOW_IN_DEVELOPER_REFERENCE */
};

// ----------------------------------------------------------------------------

// TODO: Review reasonable functionality interfaces. (TT)
//! A moment in time in the report.
/*!
    \ingroup Report
*/
class Report_Frame
{
public:
    virtual ~Report_Frame() 
    {}
};

// ----------------------------------------------------------------------------
    
Report_Reader::Report_Reader(const Report_Specification & specs) :
    _specification(specs)
{
    _start_time = specs.start_time();
    _end_time = specs.end_time();
}
    
// ----------------------------------------------------------------------------

const Report_Specification & Report_Reader::specs() const
{
    return _specification;
}
    
// ----------------------------------------------------------------------------

void Report_Reader::open(const Report_Specification &specs)
{
    _specification = specs;
    _start_time = specs.start_time();
    _end_time = specs.end_time();
    open();
}
    
// ----------------------------------------------------------------------------

void Report_Reader::time_window(Millisecond start, Millisecond end)
{
    if (start < _specification.start_time() ||
        end > _specification.end_time())
    {
        throw_exception(Bad_Data("Report_Reader::time_window: Window"
                                 " out of bounds"), SEVERE_LEVEL);
    }

    _start_time = discretize(start);
    _end_time = discretize(end);
    
    unload();

    time_window_impl(start, end);

    reset(Frame_Number(0)); // This may throw
}

// ----------------------------------------------------------------------------

Frame_Number Report_Reader::window_frame_count() const
{
    return (Frame_Number) round((_end_time - _start_time) / 
                                _specification.timestep());
}

// ----------------------------------------------------------------------------

Millisecond Report_Reader::start_time() const
{
    return _start_time;
}

// ----------------------------------------------------------------------------

Millisecond Report_Reader::end_time() const
{
    return _end_time;
}

// ----------------------------------------------------------------------------

void Report_Reader::load_frame(Millisecond timestamp, Report_Frame & frame)
{
    bbp_assert(timestamp >= _start_time && timestamp < _end_time);

    load_frame(relative_frame_for_timestamp(timestamp), frame);
}

// ----------------------------------------------------------------------------

void Report_Reader::reset(Millisecond timestamp)
{
    if (timestamp == UNDEFINED_MILLISECOND)
        reset();
    else
        reset(relative_frame_for_timestamp(timestamp));
}

// ----------------------------------------------------------------------------

Frame_Number Report_Reader::relative_frame_for_timestamp
(Millisecond timestamp) const
{
    timestamp = std::max(Millisecond(0.0), timestamp - _start_time);
    Frame_Number frame =
        std::min(Frame_Number(round((_end_time - _start_time) / 
                                    _specification.timestep())) - 1,
                 Frame_Number(round(timestamp / _specification.timestep())));
    return frame;
}

// ----------------------------------------------------------------------------

Frame_Number Report_Reader::absolute_frame_for_timestamp
(Millisecond timestamp) const
{
    timestamp = std::max(Millisecond(0.0), 
                         timestamp - _specification.start_time());
    Frame_Number frame =
        std::min(Frame_Number(round((_specification.end_time() - 
                                     _specification.start_time()) / 
                                    _specification.timestep())) - 1,
                 Frame_Number(round(timestamp / _specification.timestep())));
    return frame;
}

// ----------------------------------------------------------------------------

Millisecond Report_Reader::discretize(Millisecond time) const
{
    return (_specification.start_time() + 
            absolute_frame_for_timestamp(time) * _specification.timestep());
}

}
#endif
