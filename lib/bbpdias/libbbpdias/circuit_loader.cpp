#include "circuit_loader.hpp"

circuit_loader::circuit_loader (const std::string & blue_config_file_name,
				bool load_mesh)
{
    experiment.open(blue_config_file_name);
    microcircuit = &experiment.microcircuit();
    const bbp::Targets & targets = experiment.targets();
    const bbp::Cell_Target target = targets.cell_target("Column");
    bbp::Microcircuit_Loading_Flag_Set to_load = bbp::NEURONS
	| bbp::MORPHOLOGIES;
    if (load_mesh)
	to_load |= bbp::MESHES;
    microcircuit->load(target, to_load);
}
