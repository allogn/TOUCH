/*
	
	Ecole Polytechnique Federale de Lausanne
	Brain Mind Institute
	Blue Brain Project
	(c) 2006-2007. All rights reserved.

	Responsible authors: Thomas Traenkler

*/


#include <bbp.h>

using namespace bbp;

// ----------------------------------------------------------------------------

/*!
    \todo Clarify that spatio_temporal_average specifies milliseconds from 
    simulation start, not from simulation time window that is reported in 
    the file. also produce an error message if interval not in file. Should 
    actually probably go in the Stream_Playback_Control then.
*/
void spatiotemporal_average (URI            blue_config_filename, 
							 Filename		volume_compartment_mapping_filename,
							 Filename		output_filename_prefix,
							 Millisecond	begin_time,
							 Millisecond	end_time,
							 Millisecond	average_time_window,
                             bool           portable = false)
{
	Count frames_per_second(10);

    // experiment
    Experiment  experiment;
    experiment.open(blue_config_filename);
	Cell_Target circuit_target = experiment.cell_target("Column");

    std::cout << "Microcircuit: loading target Column" << std::endl;
    Count j = 0;

    for (Cell_Target::const_iterator i = circuit_target.begin();
        i != circuit_target.end(); ++i)
    {
        Neuron_Ptr new_neuron(new Neuron(experiment.microcircuit(), *i , j));
	    experiment.microcircuit().neurons().insert(new_neuron);
        ++j;
    }
	
	Volume_Temporal_Average<Millivolt, Micron, Millisecond>	
		temporal_average(Count (average_time_window * frames_per_second));

    Filter_Data<Microcircuit> microcircuit(experiment.microcircuit_ptr());
    
    // pipeline setup
    Pipeline    pipeline;
        
    std::cout << "Loading compartment report..." << std::endl;
    Reports_Specification::const_iterator result = 
        experiment.reports().find("allCompartments");
    if (result == experiment.reports().end())
    {
        std::cerr << "No \"allCompartments\" report found" << std::endl;
        exit(1);
    }
    Compartment_Report_Stream_Reader<Millivolt> binary_report_reader(* result, 2);
    binary_report_reader.cell_target(circuit_target);
    Compartment_Report_Printer<Millivolt>   printer;
    Compartment_Voltages_to_Microcircuit    report_link;
    Compartment_Voltage_Voxel_Average voltage_volume_average(volume_compartment_mapping_filename, false);
    Volume_Voltage_to_Byte voltage_to_byte;

	binary_report_reader.time_window(begin_time, end_time);
	binary_report_reader.speed(1.0);
	voltage_to_byte.dynamic_range(-65.0, -20.0);

	std::cout << "Pipeline 1: Setup spatio-temporal average." << std::endl;
	pipeline = binary_report_reader
		  + printer
		  + report_link
		  + microcircuit
		  + voltage_volume_average
		  + temporal_average;

	std::cout << "Pipeline 1: Started processing spatio-temporal average." 
		<< std::endl;
	pipeline.start();
	pipeline.wait_until_finished();
	pipeline.stop();
	std::cout << "Pipeline 1: Temporal average finished." << std::endl;


    std::cout << "Pipeline 2: Setup NRRD volume file writer." << std::endl;

    Pipeline    pipeline2;
    Filter_Reader<Volume<Millivolt, Micron, Millisecond> >   volume_buffer_reader;
    volume_buffer_reader.input_ptr(temporal_average.output_ptr());
	Volume_NRRD_File_Stream_Writer nrrd_file_writer(output_filename_prefix);

    pipeline2 = volume_buffer_reader 
                + voltage_to_byte 
                + nrrd_file_writer;

    std::cout << "Pipeline 2: Started writing volumes to disk." << std::endl;
	pipeline2.start();
	pipeline2.wait_until_finished();
	std::cout << "Pipeline 2: Written volumes to disk." << std::endl;
	
}

// ----------------------------------------------------------------------------

int main(int argc, const char* argv[])
{
	if (argc != 7 )
	{
		std::cout
			<< std::endl
			<< "SPATIO-TEMPORAL AVERAGE" << std::endl
			<< "______________________________________________________" 
			<< std::endl
			<< std::endl
			<< "DESCRIPTION: Produces a sequence of NRRD files with "
			   "spatio-temporal averaged volumes of voltages of the "
			   "microcircuit simulation. Time unit is millisecond. "
			   "The mapping file will be searched in the circuit "
			   "directory of the blue config file." 
			<< std::endl 
			<< std::endl

			<< "SYNTAX: "
			<< "spatiotemporal_average "
			<< "blue_config_filename "
			<< "volume_compartment_mapping_filename "
			<< "output_filename_prefix "
			<< "begin_time "
			<< "end_time "
			<< "average_time_window " 
			<< std::endl
			<< std::endl

			<< "EXAMPLE: "
			<< "spatiotemporal_average "
			<< "/bglscratch/bbp/release/20.08.07/simulations/"
			   "BasicTests.298/Probe14.cfg "
			<< "volume_compartment_mapping "
			<< "output "
			<< "55.0 "
			<< "85.0 "
			<< "10.0 " 
			<< std::endl
			<< std::endl;
		exit(1);
	}
	
	URI      	blue_config_filename				(argv[1]);
	Filename	volume_compartment_mapping_filename (argv[2]);
	Filename	output_filename_prefix				(argv[3]);
	Millisecond begin_time			(boost::lexical_cast<Millisecond>(argv[4])), 
				end_time			(boost::lexical_cast<Millisecond>(argv[5])),
				average_time_window	(boost::lexical_cast<Millisecond>(argv[6]));

	spatiotemporal_average (blue_config_filename,
							volume_compartment_mapping_filename,
							output_filename_prefix,
							begin_time,
							end_time,
							average_time_window);
    return 0;
}
