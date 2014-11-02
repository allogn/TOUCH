#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <BBP/Model/Experiment/Experiment.h>
#include <BBP/Model/Experiment/Readers/File/Blue_Config_File_Reader.h>
#include "File/Target_File_Reader.h"
#include "Microcircuit_MVD_File_Reader.h"

#ifdef BBP_USE_BOOST_SERIALIZATION
// MS compatible compilers support #pragma once

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#endif

using namespace std;
using namespace bbp;

    //int main(int argc, const char* argv[])
int main2(int argc, const char* argv[])
{
// OBJECTS

	Experiment experiment; 
	Cell_Target l4ss, minicolumn, minicolumn_l4ss, neocortical_column, myTarget;

	// BLUE CONFIG FILE LOADING
	
	Blue_Config_File_Reader blue_config_file_reader("/bglscratch/bbp/release/13.08.07/simulations/BasicTests/Probe1.cfg");
	blue_config_file_reader.read(experiment);

	// TARGET FILE LOADING

	//Target_File_Reader target_file_reader(experiment._user_target_source);
	std::cout << "read start.target" << std::endl;
	//target_file_reader.read(blue_config_file_reader.nrn_directory + "/start.target");
	//std::cout << "read user.target" << std::endl;
	//target_file_reader.read(blue_config_file_reader.target_filename);
	//l4ss = experiment.targets.cell_target("L4SS");
	//minicolumn = experiment.targets.cell_target("MiniColumn_34");
	//neocortical_column = experiment.targets.cell_target("Column");
	//myTarget = experiment.targets.cell_target("L4SS");
	//myTarget = experiment.targets.cell_target("Column");

	Microcircuit_MVD_File_Reader microcircuit_reader(blue_config_file_reader.microcircuit_directory + "/circuit.mvd2");
	std::cout << "read circuit (mvd)" << std::endl;
	microcircuit_reader.load(experiment.)read(neocortical_column);
		
	
	// MORPHOLOGY HDF5 FILES LOADING
	std::cout << "morphologies read" << std::endl;
	Morphology_HDF5_File_Reader		morphology_reader(blue_config_file_reader.morphologies_directory);
	std::cout << "read morphologies" << std::endl;
	morphology_reader.read(experiment.microcircuit.morphologies);
	std::cout << "Morphologies read, printing experiment : " << std::endl;
	experiment.microcircuit.print();

	std::cout << "Creating neurons" << endl;
    //Neurons myNeurons = experiment.microcircuit.neurons & myTarget;
    Neurons myNeurons = experiment.microcircuit.neurons;
    std::cout << "Starting to iterate" << endl;

    Neurons::iterator myNeuronsEnd = myNeurons.end(); 
    for (Neurons::iterator i = myNeurons.begin(); i != myNeuronsEnd; ++i) {
		std::cout << "Neuron a" << i->gid << " with morphology " << i->morphology().name() << std::endl;
	}
	return 0;
}


