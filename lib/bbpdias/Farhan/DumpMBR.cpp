#include <fstream>
#include <iostream>

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <BBP/Model/Experiment/Experiment.h>
#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <BBP/Model/Microcircuit/Containers/Neurons.h>
#include <BBP/Model/Microcircuit/Containers/Sections.h>
#include <BBP/Model/Microcircuit/Containers/Segments.h>

#include "circuit_loader.hpp"
#include "tools.hpp"

namespace po = boost::program_options;

static void dump_segment_mbrs (std::ostream & output,
			       const bbp::Transform_3D<bbp::Micron> & gtrafo,
			       const bbp::Morphology & morphology)
{
  BOOST_FOREACH (bbp::Section section, morphology.all_sections())
    {
      bbp::Segments::const_iterator segment = section.segments().begin();
      bbp::Segments::const_iterator end = section.segments().end();
      while (segment != end)
	{
	  vect plow, phigh;
	  get_segment_mbr (*segment, gtrafo, &plow, &phigh);
	  output_mbr (output, plow, phigh);
	  segment++;
	}
    }
}

static void dump_mesh (std::ostream & output,
		       const bbp::Transform_3D<bbp::Micron> & gtrafo,
		       const bbp::Morphology & morphology)
{
  if (!morphology.mesh_loaded())
    abort();
  const bbp::Mesh & mesh = morphology.mesh();
  bbp::Array<bbp::Vertex_Index>::const_iterator t = mesh.triangles().begin();
  bbp::Array<bbp::Vertex_Index>::const_iterator e = mesh.triangles().end();
  const bbp::Array<bbp::Vector_3D<bbp::Micron> > & vertices
    = mesh.vertices();
  while (t != e)
    {
      vect plow, phigh;
      triangle_ids tids = { {*t++, *t++, *t++} };

      get_triangle_mbr (tids, vertices, gtrafo, &plow, &phigh);
      output_mbr (output, plow, phigh);
    }
}

int main (int argc, char * argv[])
{
  po::options_description desc;
  desc.add_options()
    ("input", po::value<std::string>(), "config file")
    ("output", po::value<std::string>(), "dump file")
    ("segments", "dump segments") // FIXME: only one of segments and mesh at same time!
    ("mesh", "dump mesh")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  const std::string input_fn = vm["input"].as<std::string>();
  const std::string output_fn = vm["output"].as<std::string>();

  std::cout << "Input: " << input_fn << '\n';
  std::cout << "Output: " << output_fn << '\n';

  std::ofstream output(output_fn.c_str());

  bool do_dump_segments = vm.count("segments");
  bool do_dump_mesh = vm.count("mesh");

  circuit_loader loader(input_fn, do_dump_mesh);
  bbp::Neurons neurons = loader.get_neurons();

  BOOST_FOREACH (bbp::Neuron neuron, neurons)
    {
      std::cout
	<< boost::format("Loading neuron id = %1% with morphology %2%\n")
	% neuron.gid() % neuron.morphology().label();
      if (do_dump_segments)
	dump_segment_mbrs(output, neuron.global_transform(),
			  neuron.morphology());
      if (do_dump_mesh)
	dump_mesh(output, neuron.global_transform(), neuron.morphology());
    }
  output.close();
}
