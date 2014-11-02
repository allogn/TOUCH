#include <BBP/Model/Microcircuit/Microcircuit.h>
#include "AABBCylinder.h"
#include <spatialindex/SpatialIndex.h>
#include "tools.hpp"

#include <iostream>
#include <fstream>
#include <memory>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_array.hpp>

#include "access_count_visitor.hpp"
#include "data_segment.hpp"
#include "segment_stream.hpp"
#include "mesh_stream.hpp"
#include "circuit_loader.hpp"

using namespace bbp;
using namespace std;
using namespace SpatialIndex;

namespace po = boost::program_options;

const std::string data_type_mesh = "mesh";
const std::string data_type_segments = "segments";

const std::string loadby_str = "str";
const std::string loadby_1by1 = "1by1";

const float fill_factor = 0.7;

const unsigned page_size = 4096;
const unsigned data_len_in_the_tree = 0; // TODO change

// Index and leaf node capacities given the page size and asssuming double as
// a basic data type for MBRs
// According to Node::getByteArraySize
const unsigned node_header_double = 3 * sizeof(size_t)
    + 2 * 3 * sizeof(double);
const unsigned node_space_for_entries_double = page_size
    - node_header_double;
const unsigned index_entry_size_double = 2 * 3 * sizeof(double)
    + sizeof(id_type) + sizeof(size_t);
const unsigned leaf_entry_size_double = index_entry_size_double
    + data_len_in_the_tree;
const unsigned index_node_capacity_double = node_space_for_entries_double
    / index_entry_size_double;
const unsigned leaf_node_capacity_double = node_space_for_entries_double
    / leaf_entry_size_double;

// libspatialindex does not support float as a basic data type for MBRs yet
// we want it.  Simulate float by calculating how many float-using nodes would
// fit into our page size, then set the page size to fit the same number of
// nodes, but using double instead.
const unsigned node_header_float = 3 * sizeof(size_t) + 2 * 3 * sizeof(float);
const unsigned node_space_for_entries_float = page_size - node_header_float;
const unsigned index_entry_size_float = 2 * 3 * sizeof(float)
    + sizeof(id_type) + sizeof(size_t);
const unsigned leaf_entry_size_float = index_entry_size_float
    + data_len_in_the_tree;
const unsigned index_node_capacity_float = node_space_for_entries_float
    / index_entry_size_float;
const unsigned leaf_node_capacity_float = node_space_for_entries_float
    / leaf_entry_size_float;

const unsigned page_size_float = node_header_double +
    std::max(index_node_capacity_float * index_entry_size_double,
             leaf_node_capacity_float * leaf_entry_size_double);


int main(int argc, char* argv[])
{
    std::string data_type;
    unsigned limit;
    std::string load_method;
    std::string graph_output_fn;
    std::string root_entries_output_fn;

    po::options_description desc;
    desc.add_options()
        ("help", "print usage")
        ("input", po::value<std::string>(), "config file")
        ("data",
         po::value<std::string>(&data_type)->default_value(data_type_segments),
         "load mesh or segments")
        ("load",
         po::value<std::string>(&load_method)->default_value(loadby_str),
         "R-tree load method: str, 1by1")
        ("graphoutput",
         po::value<std::string>(&graph_output_fn)->default_value(""),
         "output results for gnuplot to file")
        ("limit", po::value<unsigned>(&limit)->default_value(4000000000),
         "limit for number of objects to load")
        ("rootentries",
         po::value<std::string>(&root_entries_output_fn)->default_value(""),
         "output root entry MBRs to file")
        ;

    po::positional_options_description p;
    p.add("input", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).
              positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << '\n';
        return 0;
    }

    const std::string input_fn = vm["input"].as<std::string>();

    std::auto_ptr<std::ofstream> output;
    if (graph_output_fn.length() > 0)
        output.reset(new std::ofstream(graph_output_fn.c_str(),
                                       std::ios_base::app));

    circuit_loader loader(input_fn, data_type == "mesh");
    const Neurons & myNeurons = loader.get_neurons();

    std::string baseName = "rtree";

    const int page_size = page_size_float;
    std::cout << "Disk page size in bytes: " << page_size << '\n';
    std::auto_ptr<IStorageManager>
        diskfile(StorageManager::createNewDiskStorageManager(baseName,
                                                             page_size));

    id_type indexIdentifier = 0;

    std::auto_ptr<data_stream> data;

    if (data_type == data_type_mesh)
        data.reset(new mesh_stream(&myNeurons, limit));
    else if (data_type == data_type_segments)
        data.reset(new segment_stream(&myNeurons, limit));
    else {
        std::cerr << "Unknown data type!\n";
        abort();
    }

    std::auto_ptr<ISpatialIndex> tree;

    const unsigned index_node_capacity = index_node_capacity_float;
    const unsigned leaf_node_capacity = leaf_node_capacity_float;

    std::cout << "Tree index node capacity: " << index_node_capacity << '\n';
    std::cout << "Tree leaf node capacity: " << leaf_node_capacity << '\n';

    if (load_method == loadby_str) {
        std::cout << "Bulk-loading using STR...\n";
        tree.reset(RTree::createAndBulkLoadNewRTree(RTree::BLM_STR,
                                                    *data,
                                                    *diskfile, fill_factor,
                                                    index_node_capacity,
                                                    leaf_node_capacity,
                                                    3, RTree::RV_RSTAR,
                                                    indexIdentifier));
    }
    else if (load_method == loadby_1by1) {
        std::cout << "Inserting one-by-one..\n";
        SpatialIndex::id_type obj_id = 1;
        tree.reset(RTree::createNewRTree (*diskfile, fill_factor,
                                          index_node_capacity,
                                          leaf_node_capacity, 3,
                                          RTree::RV_RSTAR, indexIdentifier));
        while (data->hasNext())
        {
            std::auto_ptr<IData> obj(data->getNext());
            IShape * shape_ptr;
            obj->getShape (&shape_ptr);
            std::auto_ptr<IShape> shape(shape_ptr);

            tree->insertData (0, NULL, *shape, obj_id);
            obj_id++;
        }
    }
    else {
        std::cerr << "Unknown load method!\n";
        abort();
    }

    std::cout << "Checking R-tree structure... ";
    if (!tree->isIndexValid())
      std::cerr << "R-tree internal checks failed!\n";
    std::cout << "OK\n";

    std::auto_ptr<IStatistics> tree_stats;
    IStatistics * tree_stat_ptr;
    tree->getStatistics (&tree_stat_ptr);
    tree_stats.reset (tree_stat_ptr);
    cout << *tree_stats;

    Region world_region = data->getWorldExtent();
    Region neuron_center_region = data->getNeuronCenterWorldExtent();
    cout << "World extent according to the stream: " << world_region << '\n';
    cout << "World extent between neuron centers: " << neuron_center_region
         << '\n';

    cout << "Performing queries on the whole world...\n";
    float avg_io = perform_random_point_queries(tree.get(), world_region,
                                                10000);

    if (graph_output_fn.length() > 0) {
        *output << data->objs_loaded() << ' ';
        *output << avg_io;
    }

    cout << "Performing queries between neuron centers...\n";
    avg_io = perform_random_point_queries(tree.get(), neuron_center_region,
                                          10000);

    if (graph_output_fn.length() > 0) {
        *output << ' ' << avg_io << ' ' << '\n';
    }

    if (root_entries_output_fn.length() > 0)
    {
        cout << "Outputting root node entry MBRs to "
             << root_entries_output_fn << "...\n";
        std::ofstream root_entries_output (root_entries_output_fn.c_str(),
                                           std::ios_base::app);
        const std::vector<SpatialIndex::Region> mbrs
            = tree->getRootEntryMBRs();
        BOOST_FOREACH (const SpatialIndex::Region & mbr, mbrs)
        {
            root_entries_output << mbr << '\n';
        }
    }

    return 0;
}
