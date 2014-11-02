/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors:    Juan Hernando Vieites
                    Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_EXCEPTION_H
#define BBP_EXCEPTION_H

#include <stdlib.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstring>
#ifdef __GNUC__
#include <cxxabi.h>
//#elif __WIN32__
//#include <windows.h>
#endif
#ifndef WIN32
#include "BBP/Common/System/environ.h"
#endif


/* bbp_assert
 * The bbp_assert macro is a replacement for C asserts, which uses the
 * C++ exception system instead of system dependant messages.  The reason
 * for this is that the boost unit test system does catch classic C
 * asserts, and so requires this change to an exception based system.
*/

#ifndef NDEBUG
#ifdef WIN32
#define bbp_assert(a) assert(a)
#else
#define bbp_assert(a) do { if(!(a)) { throw_exception(std::runtime_error("bbp_assert failed:  < " #a " >"),FATAL_LEVEL,__FILE__, __LINE__); } } while (0)
#endif
#else
#define bbp_assert(a) (static_cast<void>(0))
#endif

namespace bbp
{

// ----------------------------------------------------------------------------

//! Exception log verbosity levels.
enum Verbosity_Level
{
    //! non recoverable problem, the application must stop
    FATAL_LEVEL = 0,
    //! indicates a problem that the programmer needs to deal with
    SEVERE_LEVEL,
    //! may indicate a problem, but the application can continue
    WARNING_LEVEL,
    //! messages informing about the state of the application
    INFORMATIVE_LEVEL,
    //! developer information for debugging applications
    DEBUG_LEVEL
};

// ----------------------------------------------------------------------------
#ifndef SWIG

//! Abstraction for compiler specific demangling function
template <typename T>
std::string demangled_type_name(const T &);

// ----------------------------------------------------------------------------

//! Throws and exception and logs the message if required
/**
   Code compiled with BBP_NOT_VERBOSE will log exceptions if the
   variable BBP_VERBOSE_EXCEPTIONS is defined. If BBP_NOT_VERBOSE wasn't
   defined exceptions will always be be logged.
   
   Logging will be affected by the value.

    \todo For now in the beta phase, we need to always print the error message 
    for users not aware of exception handling or our implementation details so 
    the user is not exposed with an undescriptive abort() message on the SGI.

*/
template <typename T>
inline void throw_exception(const T & exc, Verbosity_Level level = FATAL_LEVEL,
                            const char * source = "", int line = 0);

//! Logging of messages to std::cerr or to file.
/**
   Two environmental variables control the behaviour of this function:
   - BBP_VERBOSE_LEVEL: only if verbosity_level is higher or equal to the
     verbosity level set by this varaible the message will be printed.
     If BBP_VERBOSE_LEVEL is not defined or cannot be parsed into a number,
     the established level will be 0.
     The different levels are:
     * Fatal error (should be printed always).
     * Severe error (thought recuperable a user might want to be informed).
     * Warning.
     * Informative.
     * Debug output.
   - BBP_LOG_FILENAME: path to the log file or an empty string is console
     output is desired. If undefined the name "error.log" will be used.

   Additionally, a verbosity level different from 0 and source filename
   and line can be provided.

   If source is compiled without NDEBUG, messages will be output through 
   std::cerr unconditionally.
 */
inline void log_message(const std::string & message, 
                        Verbosity_Level level = FATAL_LEVEL,
                        const char * source = "", int line = 0);

#endif

// ----------------------------------------------------------------------------

//! Runtime exception class template.
/*
    Specify exception name as a text parameter ("example").
    The severity of an exception depends on the calling and throwing contexts,
    that's the reason to not provide severity as a template parameter.
*/
template <class Tag, class Base = std::exception, bool tag_has_prefix = true>
class Exception;

template <class Tag>
class Exception<Tag, std::exception, true> 
    : public std::exception
{
public:
    Exception() throw() : _message(std::string(Tag::prefix)) {}

    Exception(const std::string & error_message) throw()
        : _message(std::string(Tag::prefix) + ": " + error_message)
    {}   

    virtual ~Exception() throw() {}

    virtual const char *what() const throw()
    {
        return _message.c_str();
    }

protected:
    std::string _message;
};

// ----------------------------------------------------------------------------

template <class Tag>
class Exception<Tag, std::exception, false> : public std::exception
{
public:
    Exception() 
        throw() 
        : _message(demangled_type_name(*this)) 
    {}

    //! \bug warning C4355: 'this' : used in base member initializer list (TT)
    Exception(const std::string & error_message) 
        throw()
    {
        _message = demangled_type_name(*this) + ": " + error_message;
    }    

    virtual ~Exception() throw() {}

    virtual const char *what() const 
        throw()
    {
        return _message.c_str();
    }

protected:
    std::string _message;
};

// ----------------------------------------------------------------------------

template <class Tag, class Base>
class Exception<Tag, Base, true> : public Base
{
public:
    Exception() 
        throw() : Base(std::string(Tag::prefix))
    {}

    Exception(const std::string & error_message) 
        throw() 
        : Base(std::string(Tag::prefix) + ": " +  error_message)
    {}

    virtual ~Exception() 
        throw() 
    {}
};

// ----------------------------------------------------------------------------

template <class Tag, class Base>
class Exception<Tag, Base, false> : public Base
{
public:
    Exception() throw() : Base(demangled_type_name(*this)) {}

    //! Throw exception (and print to console in verbose mode).
    Exception(const std::string & error_message) throw()
        : Base(demangled_type_name(*this) + ":" + error_message)
    {}

    virtual ~Exception() throw() {}
};

// ----------------------------------------------------------------------------

#ifndef SWIG
template <typename T>
std::string demangled_type_name(const T & x)
{
#ifdef __GNUC__
    int status = -1;
    std::string ret = typeid(x).name();
    char * demangled_name =
        abi::__cxa_demangle(ret.c_str(), 0, 0, &status);
    if (status == 0)
        ret = demangled_name;
    ::free(demangled_name);
    return ret;
//! \todo Not tested and not thread_safe
//#elif __WIN32__
//    char buffer[256];
//    UnDecorateSymbolName(typeid(x).name(), buffer, 256, UNDNAME_COMPLETE);
//    return std::string(buffer);
#else
    return std::string(typeid(x).name());
#endif
}

// ----------------------------------------------------------------------------

template <typename T>
void throw_exception(const T & exc, Verbosity_Level level, 
                     const char * source, int line)
{
    std::string _;
#ifdef BBP_NOT_VERBOSE
    static bool log_errors = bbp::getenv("BBP_VERBOSE_EXCEPTIONS", _);
#else
    static bool log_errors = true;
#endif

    if (log_errors)
    {
        log_message(exc.what(), level, source, line);
    }
    
    throw exc;
} 

// ----------------------------------------------------------------------------

//! Log a message to screen or file.
void log_message(const std::string & message, Verbosity_Level level,
                 const char * source, int line)
{
    static std::string log_filename = "";
    if (log_filename == "")
    {
        log_filename = "error.log";
#ifndef WIN32
        bbp::getenv("BBP_LOG_FILENAME", log_filename);
#else
        log_filename = "";
#endif
    }
    static std::ofstream log_file(log_filename.c_str());

    static int verbosity_level = -1;
    if (verbosity_level < 0)
    {
        std::string level = "0";
#ifndef WIN32
        bbp::getenv("BBP_VERBOSE_LEVEL", level);
#else
        level = "0";
#endif
        verbosity_level = strtol(level.c_str(), 0, 10);
    }

    if (log_filename != "" && !log_file) {
        std::cerr << "WARNING - Couldn't open '" << log_filename 
                  << "' for logging. Logging to standard error instead." 
                  << std::endl;
    }

    if ((int)level <= verbosity_level)
    {
        std::stringstream prefix;
        if (strcmp(source, "") != 0)
        {
            prefix << "[" << source;
            if (line != 0)
                prefix << ":" << line;
            prefix << "] ";
        }

#if !defined NDEBUG || defined SWIG
        // Logging inconditionally to std::cerr.
        std::cerr << prefix.str() << message << std::endl;
#else
        if (!log_file)
            std::cerr << prefix.str() << message << std::endl;
#endif
        if (log_file)
            log_file << prefix.str() << message << std::endl;
    }

}
#endif

// ----------------------------------------------------------------------------

}

#endif
