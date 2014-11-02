#include <cassert>
#include <fstream>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include "libbbpdias/Box.hpp"

namespace po = boost::program_options;

enum { default_num_of_ops = 1000000 };

int main (int argc, char *argv[])
{
    unsigned num_of_ops;

    po::options_description desc ("Allowed options");
    desc.add_options ()
        ("help", "print usage")
        ("output", po::value<std::string> (), "output file")
        ("size",
         po::value<unsigned> (&num_of_ops)->default_value (default_num_of_ops),
         "number of operations in the output")
        ("universe",
         po::value<std::vector<std::string> > ()->multitoken (),
         "Coordinates of the universe in: lowx lowy lowz highx highy highz")
        ;

    po::variables_map vm;
    po::store (po::parse_command_line (argc, argv, desc), vm);
    po::notify (vm);

    if (vm.count ("help"))
    {
        std::cout << desc << '\n';
        return 0;
    }

    if (!vm.count ("output"))
    {
        std::cerr << "Output file not specified!\n";
        std::cerr << "Invoke with --help to see valid options\n";
        return 1;
    }

    const std::string output_name = vm["output"].as<std::string> () ;
    std::cout << "Output file: " << output_name << '\n';

    if (!vm.count ("universe"))
    {
        std::cerr << "Universe size not specified!\n";
        std::cerr << "Invoke with --help to see valid options\n";
        return 1;
    }

    const dias::Box universe
        = dias::Box::make_box (vm["universe"].as<std::vector<std::string> >());
    std::cout << "Universe: " << universe << '\n';

    std::ofstream output (output_name.c_str (),
                          std::ios::binary | std::ios::trunc);

    static boost::mt11213b generator (42u);

    unsigned i = 0;
    while (i < num_of_ops)
    {
        float point[3];
        for (unsigned j = 0; j < 3; j++)
        {
            boost::uniform_real<> uni_dist (universe.low[j], universe.high[j]);
            boost::variate_generator<boost::mt11213b &, boost::uniform_real<> >
                uni (generator, uni_dist);
            point[j] = uni ();
        }
        output << point[0] << ' ' << point[1] << ' ' << point[2] << '\n';
        i++;
    }

    return 0;
}
