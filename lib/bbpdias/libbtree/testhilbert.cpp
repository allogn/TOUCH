#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE hilbert_btree
#include <boost/test/unit_test.hpp>

#include <boost/unordered_set.hpp>

#include "hilbertbtree.hpp"

#include "testutils.hpp"

using namespace dias;
using namespace SpatialIndex;

enum { number_of_dimensions = 3 };

BOOST_AUTO_TEST_SUITE(hilbert_btree_testsuite)

BOOST_FIXTURE_TEST_CASE(factory_simple_test, F)
{
    boost::shared_ptr<ISpatialIndex> t
        = hilbert_btree::make_hilbert_btree (*storage, page_size,
                                             number_of_dimensions);
}

// 3D point at (1.0) all dimensions
static double pLow[] = {1.0, 1.0, 1.0};

class test_visitor : public IVisitor
{
public:
    test_visitor (const boost::unordered_multiset<hilbert_btree::data>
                  & _expected_results) :
        expected_results (_expected_results)
        {
        }

private:
    boost::unordered_multiset<hilbert_btree::data> expected_results;

    void visitNode (const INode & in __attribute__((unused)))
        {
            assert (false);
        };

    void visitData (const IData & in)
        {
            const hilbert_btree::data * in_data
                = dynamic_cast<const hilbert_btree::data *>(&in);
            BOOST_CHECK_EQUAL (expected_results.erase (*in_data), 1);
        };

    void visitData (std::vector<const IData*>& v __attribute__((unused)))
        {
            assert (false);
        };
};

BOOST_FIXTURE_TEST_CASE(simple_insert_and_point_query_test, F)
{
    boost::shared_ptr<ISpatialIndex> t
        = hilbert_btree::make_hilbert_btree (*storage, page_size,
                                             number_of_dimensions);

    SpatialIndex::Point p (pLow, number_of_dimensions);
    SpatialIndex::id_type obj_id = 0;
    t->insertData (0, NULL, p, obj_id);

    assert (t->isIndexValid ());

    boost::unordered_multiset<hilbert_btree::data> expected_results;
    hilbert_btree::payload pld (0, 0);
    expected_results.insert (hilbert_btree::data (sizeof (pld), &pld));

    std::auto_ptr<IVisitor>
        v (new test_visitor(expected_results));

    t->pointLocationQuery (Point(pLow, number_of_dimensions), *v);
}

BOOST_AUTO_TEST_SUITE_END()
