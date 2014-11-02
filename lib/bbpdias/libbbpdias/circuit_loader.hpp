#ifndef CIRCUIT_LOADER_HPP_
#define CIRCUIT_LOADER_HPP_

#include <string>

#include <boost/utility.hpp>

#include <BBP/Model/Experiment/Experiment.h>

class circuit_loader : boost::noncopyable
{
private:
    bbp::Experiment experiment;
    bbp::Microcircuit * microcircuit;
public:
    circuit_loader (const std::string & blue_config_file_name, bool load_mesh);

    const bbp::Neurons & get_neurons(void) const
	{
	    return microcircuit->neurons();
	}
};

#endif
