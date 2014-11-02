#include <BBP/Model/Microcircuit/Microcircuit.h>
#include <SpatialIndex.h>

#include <iostream>
#include <fstream>
#include <memory>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_array.hpp>

#include "libbbpdias/AABBCylinder.h"
#include "libbbpdias/access_count_visitor.hpp"
#include "libbbpdias/data_segment.hpp"
#include "libbbpdias/segment_stream.hpp"
#include "libbbpdias/mesh_stream.hpp"
#include "libbbpdias/circuit_loader.hpp"
#include "libbbpdias/tools.hpp"
#include "libbtree/unlimited_degree_hilbertbtree.hpp"
#include "libbtree/limited_degree_hilbertbtree.hpp"

using namespace bbp;
using namespace std;
using namespace SpatialIndex;

namespace po = boost::program_options;

const std::string data_type_mesh = "mesh";
const std::string data_type_segments = "segments";

const std::string index_r_str = "r-str";
const std::string index_r_1by1 = "r-1by1";
const std::string index_unlimited_b_hilbert = "unlimited-b-hilbert";
const std::string index_limited_b_hilbert = "limited-b-hilbert";

const std::string default_index_base_name = "rtree";

enum { header_page_id = 1 };

// enum { number_of_queries = 10000 };

static bool hilbert_based_index (const std::string & index)
{
    return (index == index_unlimited_b_hilbert)
        || (index == index_limited_b_hilbert);
}

static bool rtree_based_index (const std::string & index)
{
    return (index == index_r_str) || (index == index_r_1by1);
}

static std::auto_ptr<SpatialIndex::IStatistics>
get_tree_stats (const SpatialIndex::ISpatialIndex & tree)
{
    IStatistics * tree_stat_ptr;
    tree.getStatistics (&tree_stat_ptr);
    return std::auto_ptr<SpatialIndex::IStatistics> (tree_stat_ptr);
}

static void
print_stats (const SpatialIndex::IStatistics & stats)
{
// Thanks to spatialindex design
    const dias::hilbert_btree_base::statistics * h_b_stats
        = dynamic_cast<const dias::hilbert_btree_base::statistics *>(
            &stats);
    if (h_b_stats)
        cout << *h_b_stats;
    else
        cout << stats;
}

int main(int argc, char* argv[])
{
    std::string data_type;
    unsigned limit;
    unsigned limited_hilbert_key_size;
    unsigned number_of_queries;
    std::string indexing_method;
    std::string graph_output_fn;
    std::string root_entries_output_fn;
    std::string index_base_name;
    std::string hilbert_values_fn;
    std::string input_fn;

    po::options_description desc;
    desc.add_options()
        ("help", "print usage")
        ("nocheck", "skip index consistency checks")
        ("noqueries", "do not run queries")
        ("input", po::value<std::string>(&input_fn),
         "config file for building index")
        ("loadindex", po::value<std::string>(&index_base_name),
        "File name stem for loading index")
        ("output",
         po::value<std::string>(&index_base_name),
         "File name stem for the index file")
        ("data",
         po::value<std::string>(&data_type)->default_value(data_type_segments),
         "load mesh or segments")
        ("index",
         po::value<std::string>(&indexing_method)->default_value(index_r_1by1),
         "Indexing method: r-str, r-1by1, limited-b-hilbert,"
         " unlimited-b-hilbert")
        ("graphoutput",
         po::value<std::string>(&graph_output_fn)->default_value(""),
         "output results for gnuplot to file")
        ("limit", po::value<unsigned>(&limit)->default_value(4000000000),
         "limit for number of objects to load")
        ("rootentries",
         po::value<std::string>(&root_entries_output_fn)->default_value(""),
         "output root entry MBRs to file (only for R-tree indexes)")
        ("hilbertvalues",
         po::value<std::string>(&hilbert_values_fn)->default_value(""),
         "output Hilbert values to file (only for Hilbert B-tree index)")
        ("keysize",
         po::value<unsigned>(&limited_hilbert_key_size)->default_value(32),
         "Size in bits of Hilbert keys in the limited-degree Hilbert B-tree")
// FIXME
//        ("worldspace",
//         po::value<std::vector<std::string> > ()->multitoken (),
//         "Coordinates for the world space: lowx lowy lowz highx highy highz\n"
//         "Used for Hilbert curve indexing methods to set up mapping from "
//         "world space to Hilbert curve space.  Ignored for other indexing "
//         "methods")
        ("queryspace",
         po::value<std::vector<std::string> > ()->multitoken (),
         "Coordinates of the query space: lowx lowy lowz highx highy highz\n"
         "If not specified, the whole neuron space and then neuron center "
         "space is used for queries")
        ("numqueries",
         po::value<unsigned>(&number_of_queries),
         "Number of queries to perform")
        ;

//    po::positional_options_description p;
//    p.add ("input", -1);

    po::variables_map vm;
//    po::store (po::command_line_parser (argc, argv).options (desc).
//              positional (p).run (), vm);
    po::store (po::parse_command_line (argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count ("help")) {
        std::cout << desc << '\n';
        return 0;
    }

    bool create_index = vm.count ("input");
    bool load_index = vm.count ("loadindex");
    if (!(create_index ^ load_index))
    {
        std::cerr << "One and only one of --input and --loadindex options can "
            "be specified!\n";
        return 1;
    }

    if (create_index)
    {
        std::cout << "Will create new index using config file" << input_fn
                  <<'\n';
    }
    if (load_index)
    {
        std::cout << "Will load index from " << index_base_name << '\n';
    }


    bool check_index = !vm.count ("nocheck");
    if (check_index) {
        std::cout << "Will check index internal structure\n";
    }
    else {
        std::cout << "Will not check index internal structure\n";
    }

    bool run_queries = !vm.count ("noqueries");
    if (run_queries) {
        std::cout << "Will run queries\n";
    }
    else {
        std::cout << "Will not run queries\n";
    }

    if ((root_entries_output_fn != "") && !rtree_based_index (indexing_method))
    {
        std::cerr
            << "Root entry MBRs are meaningful only for R-tree indexes\n";
        return 1;
    }

    dias::Box world_space;
// FIXME
//    if (vm.count ("worldspace")) {
        if (!hilbert_based_index (indexing_method)) {
            std::cout << "WARNING: specified world space ignored, not "
                "using Hilbert-based index\n";
        }
        else {
// FIXME!!!!!!!!!!
            std::vector<std::string> world_size_str;
            world_size_str.push_back ("-670");
            world_size_str.push_back ("179");
            world_size_str.push_back ("-262");
            world_size_str.push_back ("987");
            world_size_str.push_back ("1863");
            world_size_str.push_back ("1224");

//            world_space = dias::Box::make_box (
//                vm["worldspace"].as<std::vector<std::string> > ());
            world_space = dias::Box::make_box (world_size_str);
            std::cout << "World space: " << world_space << '\n';
        }
/*    }
    else if (hilbert_based_index (indexing_method)) {
        std::cerr << "World space needs to be specified for "
            "Hilbert-based indexing method\n";
        return 1;
    }
*/
    bool user_query_space = vm.count ("queryspace");
    if (user_query_space && !run_queries) {
        std::cerr << "User-specified query space is meaningless "
                     "if queries are not run\n";
        return 1;
    }

    dias::Box query_space;
    if (user_query_space)
    {
        query_space = dias::Box::make_box (
            vm["queryspace"].as<std::vector<std::string> >());
        std::cout << "Query space: " << query_space << '\n';
    }

    std::cout << "Disk page size in bytes: " << dias::default_page_size
              << '\n';

// TODO delete   const std::string input_fn = vm["input"].as<std::string>();

    if (rtree_based_index (indexing_method)) {
        std::cout << "Tree index node capacity: "
                  << dias::index_node_capacity << '\n';
        std::cout << "Tree leaf node capacity: "
                  << dias::leaf_node_capacity << '\n';
    }

    try {
        std::auto_ptr<std::ofstream> output;
        if (graph_output_fn.length() > 0)
            output.reset(new std::ofstream(graph_output_fn.c_str(),
                                           std::ios_base::app));

        boost::shared_ptr<IStorageManager> diskfile;
        boost::shared_ptr<ISpatialIndex> tree;
        id_type indexIdentifier = 1;

        if (create_index) {
            circuit_loader loader(input_fn, data_type == "mesh");
            const Neurons & myNeurons = loader.get_neurons();

            diskfile.reset(StorageManager::createNewDiskStorageManager(
                               index_base_name,
                               dias::default_page_size));

            std::auto_ptr<data_stream> data;

            if (data_type == data_type_mesh)
                data.reset(new mesh_stream(&myNeurons, limit));
            else if (data_type == data_type_segments)
                data.reset(new segment_stream(&myNeurons, limit));
            else {
                std::cerr << "Unknown data type!\n";
                abort();
            }

            SpatialIndex::id_type header_page_id;
            if (hilbert_based_index (indexing_method)) {
                header_page_id = SpatialIndex::StorageManager::NewPage;
                std::vector<byte> serialized_header (dias::default_page_size,
                                                     0);
                diskfile->storeByteArray (header_page_id,
                                          serialized_header.size (),
                                          &serialized_header[0]);
            }

            if (indexing_method == index_r_str) {
                std::cout << "Bulk-loading R-tree using STR...\n";
                tree.reset(RTree::createAndBulkLoadNewRTree(
                               RTree::BLM_STR, *data, *diskfile,
                               dias::fill_factor, dias::index_node_capacity,
                               dias::leaf_node_capacity, 3, RTree::RV_RSTAR,
                               indexIdentifier));
            }
            else if (indexing_method == index_r_1by1) {
                std::cout << "R-tree, inserting one-by-one..\n";
                SpatialIndex::id_type obj_id = 1;
                tree.reset(RTree::createNewRTree (*diskfile, dias::fill_factor,
                                                  dias::index_node_capacity,
                                                  dias::leaf_node_capacity, 3,
                                                  RTree::RV_RSTAR,
                                                  indexIdentifier));
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
            else if (indexing_method == index_unlimited_b_hilbert) {
                std::cout
                    << "B-tree with Hilbert curve, unlimited degree...\n";
                std::cout << "Index and leaf node capacities ignored, "
                    "using page size = " << dias::default_page_size << '\n';
                tree = dias::unlimited_degree_hilbert_btree::
                    make_unlimited_degree_hilbert_btree (
                        *diskfile, dias::default_page_size, header_page_id,
                        world_space.as_region (), 3, hilbert_values_fn);
                SpatialIndex::id_type obj_id = 1;
                while (data->hasNext ())
                {
                    std::auto_ptr<IData> obj (data->getNext ());
                    IShape * shape_ptr;
                    obj->getShape (&shape_ptr);
                    std::auto_ptr<IShape> shape(shape_ptr);
                    Point center;
                    shape->getCenter (center);

                    tree->insertData (0, NULL, center, obj_id);
                    obj_id++;
                }
            }
            else if (indexing_method == index_limited_b_hilbert) {
                std::cout << "B-tree with Hilbert curve, key size = "
                          << limited_hilbert_key_size << '\n';
                std::cout << "Index and leaf node capacities ignored, "
                    "using page size = " << dias::default_page_size << '\n';
                switch (limited_hilbert_key_size)
                {
                case 16:
                    tree = dias::limited_degree_hilbert_btree<uint16_t>::
                        make_limited_degree_hilbert_btree (
                            *diskfile, dias::default_page_size, header_page_id,
                            world_space.as_region (),
                            3, hilbert_values_fn);
                    break;
                case 32:
                    tree = dias::limited_degree_hilbert_btree<uint32_t>::
                        make_limited_degree_hilbert_btree (
                            *diskfile, dias::default_page_size, header_page_id,
                            world_space.as_region (),
                            3, hilbert_values_fn);
                    break;
                case 64:
                    tree = dias::limited_degree_hilbert_btree<uint64_t>::
                        make_limited_degree_hilbert_btree (
                            *diskfile, dias::default_page_size, header_page_id,
                            world_space.as_region (),
                            3, hilbert_values_fn);
                    break;
                }
                SpatialIndex::id_type obj_id = 1;
                while (data->hasNext ())
                {
                    std::auto_ptr<IData> obj (data->getNext ());
                    IShape * shape_ptr;
                    obj->getShape (&shape_ptr);
                    std::auto_ptr<IShape> shape(shape_ptr);
                    Point center;
                    shape->getCenter (center);

                    tree->insertData (0, NULL, center, obj_id);
                    obj_id++;
                }
            }
            else {
                std::cerr << "Unknown load method!\n";
                abort();
            }
        }
        else if (load_index)
        {
            std::cout << "Opening " << index_base_name << " ... ";
            diskfile.reset(StorageManager::loadDiskStorageManager(
                               index_base_name));
            std::cout << "OK\n";
            if (rtree_based_index (indexing_method)) {
                std::cout << "Loading the R-tree... ";
                tree.reset (RTree::loadRTree (*diskfile, indexIdentifier));
            }
            else if (indexing_method == index_unlimited_b_hilbert) {
                std::cout << "Loading the unlimited degree Hilbert B-tree... ";
                tree = dias::unlimited_degree_hilbert_btree::
                    load_unlimited_degree_hilbert_btree (
                        *diskfile, /* header_page_id */ 0, world_space.as_region (), 3,
                        hilbert_values_fn);
            }
            else if (indexing_method == index_limited_b_hilbert) {
                std::cout << "Loading the limited degree Hilbert B-tree"
                          << ", using key size = "
                          << limited_hilbert_key_size << " ...";
                switch (limited_hilbert_key_size)
                {
                case 16:
                    tree = dias::limited_degree_hilbert_btree<uint16_t>::
                        load_limited_degree_hilbert_btree (
                            *diskfile, header_page_id,
                            world_space.as_region (), 3, hilbert_values_fn);
                    break;
                case 32:
                    tree = dias::limited_degree_hilbert_btree<uint32_t>::
                        load_limited_degree_hilbert_btree (
                            *diskfile, header_page_id,
                            world_space.as_region (),
                            3, hilbert_values_fn);
                    break;
                case 64:
                    tree = dias::limited_degree_hilbert_btree<uint64_t>::
                        load_limited_degree_hilbert_btree (
                            *diskfile, header_page_id,
                            world_space.as_region (), 3,
                            hilbert_values_fn);
                    break;
                }
            }
            else {
                std::cerr << "Unknown indexing method!\n";
                return 1;
            }
        }
        else
            abort ();
        std::cout << "done.\n";

        if (check_index) {
            std::cout << "Checking the index structure... ";
            if (!tree->isIndexValid())
                std::cerr << "Internal index checks failed!\n";
            std::cout << "OK\n";
        }

        std::auto_ptr<IStatistics> tree_stats = get_tree_stats (*tree);
        print_stats (*tree_stats);

        if (run_queries && user_query_space)
        {
            cout << "Performing queries in user-specified space...\n";
            double l[3];
            l[0] = query_space.low.Vector [0];
            l[1] = query_space.low.Vector [1];
            l[2] = query_space.low.Vector [2];
            double h[3];
            h[0] = query_space.high.Vector [0];
            h[1] = query_space.high.Vector [1];
            h[2] = query_space.high.Vector [2];
            Region r (Point (l, 3), Point (h, 3));
            float avg_io = dias::perform_random_point_queries (
                tree.get (), r, number_of_queries);
            if (graph_output_fn.length () > 0)
            {
//                *output << data->objs_loaded () << ' ';
                *output << avg_io << '\n';
            }
            tree_stats = get_tree_stats (*tree);
            print_stats (*tree_stats);
        }
        else
        {
#if 0
            Region world_region = data->getWorldExtent();
            Region neuron_center_region
                = data->getNeuronCenterWorldExtent();
            cout << "World extent according to the stream: "
                 << world_region << '\n';
            cout << "World extent between neuron centers: "
                 << neuron_center_region << '\n';

            Region shifted_world_region = world_region;
            shifted_world_region.m_pHigh[0] -= world_region.m_pLow[0];
            shifted_world_region.m_pHigh[1] -= world_region.m_pLow[1];
            shifted_world_region.m_pHigh[2] -= world_region.m_pLow[2];
            shifted_world_region.m_pLow[0] = 0;
            shifted_world_region.m_pLow[1] = 0;
            shifted_world_region.m_pLow[2] = 0;
            cout << "Non-negative world extent: " << shifted_world_region
                 << '\n';

            if (run_queries) {
                cout << "Performing queries on the whole world...\n";
                float avg_io = dias::perform_random_point_queries(
                    tree.get(), world_region, number_of_queries);

                if (graph_output_fn.length() > 0) {
                    *output << data->objs_loaded() << ' ';
                    *output << avg_io;
                }

                cout << "Performing queries between neuron centers...\n";
                avg_io = dias::perform_random_point_queries(
                    tree.get(), neuron_center_region, number_of_queries);

                if (graph_output_fn.length() > 0) {
                    *output << ' ' << avg_io << ' ' << '\n';
                }
            }
#endif

            if (root_entries_output_fn.length() > 0)
            {
                cout << "Outputting root node entry MBRs to "
                     << root_entries_output_fn << "...\n";
                std::ofstream root_entries_output (
                    root_entries_output_fn.c_str(),
                    std::ios_base::app);

                const std::vector<SpatialIndex::Region> mbrs
                    = tree->getRootEntryMBRs();
                BOOST_FOREACH (const SpatialIndex::Region & mbr, mbrs)
                {
                    root_entries_output << mbr << '\n';
                }
            }
        }
    }

    catch (boost::exception & e)
    {
        std::cerr << "boost::exception caught!\n";
        std::cerr << boost::diagnostic_information (e) << '\n';
        return 2;
    }
    catch (std::exception & e)
    {
        std::cerr << "std::exception caught!\n";
        std::cerr << boost::diagnostic_information (e) << '\n';
        return 2;
    }
    return 0;
}
