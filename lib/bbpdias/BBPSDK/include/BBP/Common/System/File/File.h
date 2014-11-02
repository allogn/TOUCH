/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2008. All rights reserved.

        Authors:    Rajnish Ranjan
                    Sebastien Lasserre
                    Juan Hernando Vieites
                    James King
                    Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_FILE_H
#define BBP_FILE_H

#include <vector>
#include <string>
#include <cstring>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <algorithm>
#include <cstdio>


#include "BBP/Common/Types.h"
#include "BBP/Common/Exception/Exceptions.h"

namespace bbp {

//! name of a file in a directory of a filesystem without full path
typedef boost::filesystem::path        Filename;
//! directory in the file system
typedef boost::filesystem::path        Directory;
//! path in the file system
typedef boost::filesystem::path        Filepath;

/*!
    Converts URI to file path.

   Naive implementation of URI decoding into a filepath. No reencoding is 
   performed for special URI characters or escaped constructs.
   @param uri
   @return The filepath from the URI or an empty string if no conversion
   could be performed.
*/
inline boost::filesystem::path uri_to_filename(const URI &uri);


// ----------------------------------------------------------------------------

//! Converts an URI string to a filesystem path.
inline boost::filesystem::path uri_to_filename(const URI &uri)
{
    if (uri.length() == 0)
        return "";

    URI::size_type path_start;
    // Validating URI scheme.
    if (uri.compare(0, 5, "file:") == 0)
    {
        if (uri.length() < 6)
            return "";

        if (uri.compare(5, 2, "//") == 0)
        {
            // Skipping host part.
            if ((path_start = uri.find_first_of("/", 7)) == URI::npos)
            {
                return "";
            }
            if (path_start != 7)
            {
                std::cerr << "Skipping '" << uri.substr(7, path_start)
                          << "' converting URI into filepath" << std::endl;
            }
        }
        else if (uri[5] != '/')
        {
            // Not following generic syntax as in rfc2396.
            return "";
        }
        else
        {
            path_start = 5;
        }
    }
    else if (uri.find_first_of("/") < uri.find_first_of(":") ||
             uri.find_first_of(":") == URI::npos)
    {
        // localhost absolute and relative file URIs.
        path_start = 0;
    }
    // if windows drive letter
    //! \todo This is not unicode safe! (TT)
    else if ((((uri.at(0) >= 'A') && (uri.at(0) <= 'Z' )) 
        || ((uri.at(0) >= 'a') && (uri.at(0) <= 'z' )))
        && (uri.at(1) == ':'))
    {
        // localhost absolute file URIs.
        path_start = 0;
     }
    else
    {
        // wrong scheme or bad URI for file
        //! \todo An exception should be thrown. (TT)
        return "";
    }
        
    return uri.substr(path_start);
}

// ----------------------------------------------------------------------------

enum File_Format
{
    CHAR_FILE_FORMAT,
    UNICODE_FILE_FORMAT,
    BINARY_FILE_FORMAT
};

enum File_Access
{
    FULL_FILE_ACCESS,
    READ_FILE_ACCESS,
    WRITE_FILE_ACCESS
};

// ----------------------------------------------------------------------------

class File
{
public:
    File(const Filepath & filepath)
    {
        open(filepath);
    }

    //! Open the file
    void open(const Filepath & filepath, 
              File_Access access = FULL_FILE_ACCESS,
              File_Format format = CHAR_FILE_FORMAT)
    {
        _path = filepath;

        // set file open mode
        std::ios_base::openmode mode;
        if (access == READ_FILE_ACCESS)
            mode = std::fstream::in;
        else if (access == WRITE_FILE_ACCESS)
            mode = std::fstream::out;
        else // if(access == FULL_FILE_ACCESS)
            mode = std::fstream::in | std::fstream::out;

        if (format == BINARY_FILE_FORMAT)
            mode = mode | std::fstream::binary;

        // open file
        _file.open(filepath.string().c_str(), mode);
    }

    //! Close the file.
    void close()
    {
        _file.close();
    }

    //! Comparison if the content of two files are identical.
    /*!
        This will close the files if they were open. Note the files must have
        valid file paths before using this function.
    */
    inline bool diff(File & rhs)
    {
        File & lhs = * this;
        if (lhs._path.empty() == true || rhs._path.empty() == true)
        {
            //! \todo Readd this.
            //throw_exception(bbp::logic_error("Files need to have a path set to "
            //   "compare them"), SEVERE_LEVEL, __FILE__, __LINE__);
            throw std::logic_error("Files need to have a path set to "
                    "compare them");
        }
        if (lhs.is_open() == true)
        {
            lhs.close();
        }
        if (rhs.is_open() == true)
        {
            rhs.close();
        }
        lhs.open(lhs._path, READ_FILE_ACCESS, CHAR_FILE_FORMAT);
        rhs.open(rhs._path, READ_FILE_ACCESS, CHAR_FILE_FORMAT);

        if (lhs._file.is_open() == false || rhs._file.is_open() == false)
        {
            if (lhs.is_open() == false)
            {
                lhs.close();
            }
            if (rhs.is_open() == false)
            {
                rhs.close();
            }
            //! \todo Readd this.
            /*throw_exception(logic_error("At least one file could not be "
               "opened"), SEVERE_LEVEL, __FILE__, __LINE__);*/
            throw std::logic_error("At least one file could not be opened.");
        }

        char lhs_read_buffer[1024], 
             rhs_read_buffer[1024];

        Count current_line = 1;
        while(lhs._file.good() && rhs._file.good())
        {
            lhs._file.getline(lhs_read_buffer, 1024);
            std::streamsize lhs_characters_read = lhs._file.gcount();
            rhs._file.getline(rhs_read_buffer, 1024);
            std::streamsize rhs_characters_read = rhs._file.gcount();

            if (lhs._file.bad() || rhs._file.bad() )
            {
                std::cout << "File bad at line " 
                    << current_line << " " << std::endl
                    << "file " << _path.string() << ": " << std::endl
                    << lhs_read_buffer << std::endl
                    << "file " << rhs._path.string() << ": " << std::endl
                    << rhs_read_buffer << std::endl;
                lhs._file.close();
                rhs._file.close();
                return false;
            }
            else if (lhs_characters_read != rhs_characters_read)
            {
                std::cout << "File line length differs in line " 
                    << current_line << " " << std::endl
                    << "file " << lhs._path.string() << ": " << std::endl
                    << lhs_characters_read << " characters read: " 
                    << lhs_read_buffer << std::endl
                    << "file " << rhs._path.string() << ": " << std::endl
                    << rhs_characters_read << " characters read: "
                    << rhs_read_buffer << std::endl;

                lhs._file.close();
                rhs._file.close();
                return false;
            }

            for (std::streamsize i = 0; i < lhs_characters_read; ++i)
            {
                if (lhs_read_buffer[i] != rhs_read_buffer[i])
                {
                    std::cout << "Files differ in line " 
                        << current_line << " " << std::endl
                        << "file " << lhs._path.string() << ": " << std::endl
                        << lhs_read_buffer << std::endl
                        << "file " << rhs._path.string() << ": " << std::endl
                        << rhs_read_buffer << std::endl;
                    lhs._file.close();
                    rhs._file.close();
                    return false;
                }
            }
            ++current_line;
        }
        lhs._file.close();
        rhs._file.close();
        return true;
    }


    bool is_open() const
    {
        return _file.is_open();
    }


private:
    Filepath     _path;
    mutable std::fstream _file;
};

//! Create a unique file path for a temporary file.
/*!
    The file is placed in the standard directory for temporary files - e.g. 
    on Windows Vista this may be "C:\Users\Username\AppData\Local\Temp\".

    Be aware this code is not thread safe, and a race condition may occur
    if not used with caution (a filepath previously unique might alread be
    claimed and a file created from another thread).
*/ 
inline Filepath create_temporary_filepath(const Filename & prefix)
{
#ifdef WIN32
    char * filename = _tempnam(0, prefix.string().c_str());
#else
    //char * filename = tempnam(0, prefix.string().c_str());
    int f;
    char filename[] = "/tmp/bbp_tmp_XXXXXX";
    f = mkstemp(filename);
    close(f);

#endif
    return Filepath(filename);
}

// ----------------------------------------------------------------------------

} // bbp namespace

// ----------------------------------------------------------------------------

#ifndef WIN32

#include <cstdio>
#include <string>
#include <sys/dir.h>

namespace bbp {

extern char GstrExt[10];

// ----------------------------------------------------------------------------

#ifdef __APPLE__
// For some reason, Apple's version of scandir wants the filter function to 
// pass a non-const pointer.
inline int file_select( struct direct   *entry){
#else
inline int file_select(const struct direct   *entry)
{
#endif
    char *ptr;
    
    // std::cerr << "file_select : entry->d_name = " << entry->d_name << 
    // std::endl  ;
    // std::cerr << "string to compare = " << GstrExt << std::endl  ;
    
    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name,"..") == 0))
        return (0);
    else {        
        if(strcmp(GstrExt, "\0")) {
            
	    ptr = rindex(const_cast<char *>(entry->d_name), '.');
            
            if ((ptr != NULL) && ((strcmp(ptr, GstrExt) == 0)))
        {
                //std::cerr << "file_select : entry->d_name correct !! = " << 
                //entry->d_name << std::endl  ;
        return (1);
        }
            else
                return(0);
        }else
            return (1);
    }
}

// ----------------------------------------------------------------------------

inline void scanDirWithFilter
    (const char *dir,const char *filter,std::vector<std::string> *res)
{
    struct direct **files;
    int nCount, i ;
    
    res->clear() ;
    
    strcpy(GstrExt, filter);
    nCount = scandir(dir, &files, file_select, NULL) ;    
    if (nCount == -1 )
    {
        std::cerr << "scanDirWithFilter : CAUTION - cannot find the directory "
            "or permission issue !!!!! - " << dir << std::endl  ;
        return ;
    }
    for ( i = 0 ; i < nCount ; i++ )
    {
        res->push_back(files[i]->d_name) ;
    }
}

// ----------------------------------------------------------------------------


} // end of namespace bbp

#endif // WIN32

#endif  // end of if defined BBP_FILE_H
