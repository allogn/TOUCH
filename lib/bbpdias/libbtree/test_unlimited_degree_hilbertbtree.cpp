#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE unlimited_degree_hilbert_btree
#include <boost/test/unit_test.hpp>

#include "unlimited_degree_hilbertbtree.hpp"

#include "testutils.hpp"

using namespace dias;
using namespace SpatialIndex;

const double small_world_low[number_of_dimensions] = {0.0, 0.0, 0.0};
const double small_world_high[number_of_dimensions] = {2.0, 2.0, 2.0};

const SpatialIndex::Region small_world (small_world_low, small_world_high,
                                        number_of_dimensions);

BOOST_AUTO_TEST_SUITE(unlimited_degree_hilbert_btree_testsuite)

BOOST_FIXTURE_TEST_CASE(factory_simple_test, F)
{
    boost::shared_ptr<ISpatialIndex> t
        = unlimited_degree_hilbert_btree::make_unlimited_degree_hilbert_btree (
            *storage, page_size, header_page_id, small_world,
            number_of_dimensions);
}

// 3D point at (1.0) all dimensions
static double pLow[] = {1.0, 1.0, 1.0};

BOOST_FIXTURE_TEST_CASE(simple_insert_and_point_query_test, F)
{
    boost::shared_ptr<ISpatialIndex> t
        = unlimited_degree_hilbert_btree::make_unlimited_degree_hilbert_btree (
            *storage, page_size, header_page_id, small_world,
            number_of_dimensions);

    SpatialIndex::Point p (pLow, number_of_dimensions);
    SpatialIndex::id_type obj_id = 0;
    t->insertData (0, NULL, p, obj_id);

    assert (t->isIndexValid ());

    boost::unordered_multiset<hilbert_btree_base::data> expected_results;
    hilbert_btree_base::payload pld (0, 0);
    expected_results.insert (hilbert_btree_base::data (sizeof (pld), &pld));

    std::auto_ptr<IVisitor>
        v (new test_visitor(expected_results));

    t->pointLocationQuery (Point(pLow, number_of_dimensions), *v);
}

BOOST_AUTO_TEST_SUITE_END()
