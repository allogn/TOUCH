/*

		Ecole Polytechnique Federale de Lausanne
		Brain Mind Institute,
		Blue Brain Project
		(c) 2006-2007. All rights reserved.

		Authors: Juan Hernando Vieites
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_MICROCIRCUIT_MVD_FILE_READER_H
#define BBP_MICROCIRCUIT_MVD_FILE_READER_H

#include "BBP/Common/System/File/File.h"
#include "BBP/Model/Microcircuit/Targets/Cell_Target.h"
#include "BBP/Model/Microcircuit/Readers/Microcircuit_Composition_Reader.h"

namespace bbp
{

/*!
*/
class Microcircuit_MVD_File_Reader 
    : public Microcircuit_Composition_Reader,
      public Neurons_Accessor
{
public:
    /*!
      Creates a reader to extract data from the given mvd filename.
    */
    inline Microcircuit_MVD_File_Reader(const Filepath & circuit_filename);

    /*!
      Returns the data source this reader is bound to.
    */
    inline virtual URI source() const;

    virtual void open();

    virtual void close();

    virtual void load(Neurons & neurons,
                      const Cell_Target & target, 
                      const Structure_Dataset_Ptr & structure);

protected:
    Filepath _circuit_filename;
};

Microcircuit_MVD_File_Reader::Microcircuit_MVD_File_Reader
(const Filepath & circuit_filename) :
    _circuit_filename(circuit_filename)
{
}

URI Microcircuit_MVD_File_Reader::source() const
{
    // TODO: Improve this conversion
    return _circuit_filename.string();
}

}
#endif
