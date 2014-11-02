#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE btree
#include <boost/test/unit_test.hpp>

#include <memory>
#include <set>

#include <boost/shared_ptr.hpp>

#include "btree.hpp"

#include "testutils.hpp"

using namespace dias;
using namespace SpatialIndex;

template <class V, class K>
static void test_nonexisting (const btree<V, K> & t, const K k1, const K k2)
{
    typename btree<V, K>::query_answer query_result
        = t.range_query (typename btree<V, K>::key_range(k1, k2));
    BOOST_CHECK_EQUAL (query_result.size(), 0);
}

template <class V, class K>
static void test_existing (const btree<V, K> & t, const K k1, const K k2,
                           const K expected_key, const V expected_value)
{
    typename btree<V, K>::query_answer query_result
        = t.range_query (typename btree<V, K>::key_range(k1, k2));
    if (query_result.size() != 1)
    {
        std::cerr << "test_existing: expected_key = " << expected_key << '\n';
        BOOST_CHECK_EQUAL (query_result.size(), 1);
    }
    else
    {
        BOOST_CHECK_EQUAL (query_result[0].first, expected_key);
        BOOST_CHECK_EQUAL (query_result[0].second, expected_value);
    }
}

template <class V>
static void test_dupkeys (const btree<V> & t, int k1, int k2,
                          int expected_key,
                          const V expected_value1, const V expected_value2)
{
    typename btree<V>::query_answer query_result
        = t.range_query (typename btree<V>::key_range(k1, k2));
    BOOST_CHECK_EQUAL (query_result.size(), 2);
    BOOST_CHECK_EQUAL (query_result[0].first, expected_key);
    BOOST_CHECK_EQUAL (query_result[0].second, expected_value1);
    BOOST_CHECK_EQUAL (query_result[1].first, expected_key);
    BOOST_CHECK_EQUAL (query_result[1].second, expected_value2);
}

template <class V>
static void test_capacity (btree <V> * t, unsigned page_size)
{
    std::auto_ptr<typename btree<V>::leaf> leaf_node = t->make_leaf();

    const unsigned capacity = leaf_node->capacity();
    BOOST_CHECK_EQUAL (capacity,
                       (page_size - sizeof (SpatialIndex::id_type)
                        - sizeof (typename
                                  std::vector<typename btree<V>::key_value>
                                  ::size_type))
                       / (sizeof (typename btree<V>::key)
                          + sizeof (typename btree<V>::value)));

    for (unsigned i = 0; i < capacity; i++)
    {
        unsigned writes_before = t->get_writes ();
        unsigned reads_before = t->get_reads ();
        t->insert (i, i);
        BOOST_CHECK_EQUAL (t->get_writes () - writes_before, 1);
        BOOST_CHECK_EQUAL (t->get_reads () - reads_before, 1);
    }

    t->check ();

    for (unsigned i = 0; i < capacity; i++)
    {
        unsigned reads_before = t->get_reads ();
        test_existing<V, unsigned> (*t, static_cast<int>(i),
                                    static_cast<int>(i), static_cast<int>(i),
                                    static_cast<V>(i));
        BOOST_CHECK_EQUAL (t->get_reads () - reads_before, 1);
    }
}

BOOST_AUTO_TEST_SUITE(btree_testsuite)

BOOST_FIXTURE_TEST_CASE(constructor_simple_test, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    t.check ();
}

BOOST_FIXTURE_TEST_CASE(max_leaf_node_capacity_test, F)
{
    btree<int> t_int (*storage, page_size, header_page_id);
    test_capacity (&t_int, page_size);
}

BOOST_FIXTURE_TEST_CASE(max_leaf_node_capacity_test_ld, F)
{
    btree<long double> t_ld (*storage, page_size, header_page_id);
    test_capacity (&t_ld, page_size);
}

BOOST_FIXTURE_TEST_CASE(max_leaf_node_capacity_test_large, F)
{
    btree<int> t_int_large (*storage, page_size * 10, header_page_id);
    test_capacity (&t_int_large, page_size * 10);
}

BOOST_FIXTURE_TEST_CASE(query_nonexisting, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    t.check ();
    test_nonexisting<int, unsigned> (t, 1, 1);
}

BOOST_FIXTURE_TEST_CASE(simple_insert_and_query, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    t.insert (1, 2);

    t.check ();

    test_existing<int, unsigned> (t, 1, 1, 1, 2);
}

BOOST_FIXTURE_TEST_CASE(query_nonexisting_after_insert, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    t.insert (5, 2);

    t.check ();

    test_nonexisting<int, unsigned> (t, 1, 1);
    test_nonexisting<int, unsigned> (t, 2, 4);
    test_nonexisting<int, unsigned> (t, 6, 8);
}

BOOST_FIXTURE_TEST_CASE(unordered_inserts_and_queries, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    t.insert (9, 10);
    t.insert (4, 8);

    t.check ();

    test_existing<int, unsigned> (t, 7, 9, 9, 10);
    test_existing<int, unsigned> (t, 4, 6, 4, 8);

    test_nonexisting<int, unsigned> (t, 1, 3);
    test_nonexisting<int, unsigned> (t, 5, 8);
    test_nonexisting<int, unsigned> (t, 10, 13);
}

BOOST_FIXTURE_TEST_CASE(duplicate_key, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    t.insert (2, 10);
    t.insert (2, 100);

    t.check ();

    test_dupkeys (t, 2, 2, 2, 10, 100);
    test_nonexisting<int, unsigned> (t, 0, 1);
    test_nonexisting<int, unsigned> (t, 3, 1000);
}

BOOST_FIXTURE_TEST_CASE(smallest_in_the_node, F)
{
    btree <int> t (*storage, page_size, header_page_id);

    std::auto_ptr<btree<int>::leaf> leaf_node = t.make_leaf();
    unsigned i = 0;
    for (i = i; i < leaf_node->capacity() - 1; i++)
    {
        t.insert (i * 2 + 100, i);
    }
    t.insert (1, 1);

    t.check ();

    test_existing<int, unsigned> (t, 1, 1, 1, 1);
    test_nonexisting<int, unsigned> (t, 2, 99);
    for (i = 0; i < leaf_node->capacity() - 1; i++)
    {
        test_existing<int, unsigned> (t, static_cast<int>(i * 2 + 100),
                       static_cast<int>(i * 2 + 100),
                       static_cast<int>(i * 2 + 100), static_cast<int>(i));
        test_nonexisting<int, unsigned> (t, i * 2 + 101, i * 2 + 101);
    }
}

BOOST_FIXTURE_TEST_CASE(node_split, F)
{
    btree<int> t (*storage, page_size, header_page_id);

    std::auto_ptr<btree<int>::leaf> leaf_node = t.make_leaf();

    unsigned i = 0;
    for (i = i; i < leaf_node->capacity(); i++)
    {
        t.insert (i * 2, i);
    }

    unsigned reads_before = t.get_reads();
    unsigned writes_before = t.get_writes();
    t.insert (i * 2, i);
    BOOST_CHECK_EQUAL (t.get_reads(), reads_before + 1);
    BOOST_CHECK_EQUAL (t.get_writes(), writes_before + 3);

    t.check ();

    int extra_reads = 1;
    for (i = 0; i < leaf_node->capacity() + 1; i++)
    {
        reads_before = t.get_reads();
        test_existing<int, unsigned> (t, static_cast<int>(i * 2),
                                      static_cast<int>(i * 2),
                                      static_cast<int>(i * 2),
                                      static_cast<int>(i));
        unsigned reads_after = t.get_reads();
        if (reads_after - reads_before == 3)
        {
            if (extra_reads > 0)
                extra_reads--;
            else
            {
                BOOST_FAIL ("Too many reads");
            }
        }
        else
        {
            BOOST_CHECK_EQUAL (t.get_reads(), reads_before + 2);
        }
        test_nonexisting<int, unsigned> (t, i * 2 + 1, i * 2 + 1);
    }
    BOOST_CHECK_EQUAL (extra_reads, 0);
}

BOOST_FIXTURE_TEST_CASE(node_split_insert_lesser, F)
{
    btree<int> t (*storage, page_size, header_page_id);

    std::auto_ptr<btree<int>::leaf> leaf_node = t.make_leaf();

    unsigned i = 0;
    for (i = i; i < leaf_node->capacity(); i++)
    {
        t.insert (i * 2 + 20, i);
    }

    t.insert (10, 5);

    t.check ();

    test_existing<int, unsigned> (t, 10, 10, 10, 5);
}

BOOST_FIXTURE_TEST_CASE(query_result_across_two_leaves, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    std::auto_ptr<btree<int>::leaf> leaf_node = t.make_leaf();
    assert (leaf_node->capacity() > 22);

    unsigned i = 0;
    unsigned already_inserted = 0;

    for (i = 10; i < 20; i++)
    {
        t.insert (i, i);
        already_inserted++;
    }

    for (i = 1000; i < 1010; i++)
    {
        t.insert (i, i);
        already_inserted++;
    }

    unsigned total_dups = leaf_node->capacity() - already_inserted;
    std::set<int> values;
    for (i = 0; i < total_dups; i++)
    {
        t.insert (500, i);
        values.insert (i);
    }

    t.insert (500, i);
    values.insert (i);
    total_dups++;

    t.check ();

    btree<int>::query_answer query_result
        = t.range_query (btree<int>::key_range (500, 500));

    BOOST_CHECK_EQUAL (query_result.size(), total_dups);
    BOOST_FOREACH (btree<int>::key_value k_v, query_result)
    {
        BOOST_CHECK_EQUAL (k_v.first, 500);
        std::set<int>::size_type values_erased = values.erase (k_v.second);
        BOOST_CHECK_EQUAL (values_erased, 1);
    }
    BOOST_CHECK_EQUAL (values.size(), 0);
}

BOOST_FIXTURE_TEST_CASE (query_result_across_three_leaves, F)
{
// TODO
}

BOOST_FIXTURE_TEST_CASE (insert_after_split, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    std::auto_ptr<btree<int>::leaf> leaf_node = t.make_leaf();

    unsigned i = 0;
    for (i = i; i < leaf_node->capacity(); i++)
    {
        t.insert (i * 2, i);
        t.check ();
    }

    t.insert (i * 2, i);
    t.check ();
    i++;

    t.insert (i * 2, i);
    i++;

    t.check ();

    for (unsigned j = 0; j < i; j++)
    {
        test_existing<int, unsigned> (t, static_cast<int>(j * 2),
                                      static_cast<int>(j * 2),
                                      static_cast<int>(j * 2),
                                      static_cast<int>(j));
    }
}

BOOST_FIXTURE_TEST_CASE (insert_new_internal_node_entry, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    std::auto_ptr<btree<int>::leaf> leaf_node = t.make_leaf();

    // Fill root
    unsigned i = 0;
    for (i = i; i < leaf_node->capacity(); i++)
    {
        t.insert (i * 2, i);
    }

    // Split root
    t.insert (i * 2, i);
    i++;

    t.check ();

    // Split one of the new nodes
    unsigned old_i = i;
    for (i = old_i; i < old_i + leaf_node->capacity(); i++)
    {
        t.insert (i * 2, i);
        t.check ();
    }

    t.check ();

    int extra_reads = 3;
    for (unsigned j = 0; j < i; j++)
    {
        unsigned reads_before = t.get_reads();
        test_existing<int, unsigned> (t, static_cast<int>(j * 2),
                                      static_cast<int>(j * 2),
                                      static_cast<int>(j * 2),
                                      static_cast<int>(j));
        unsigned reads_after = t.get_reads();
        if (reads_after - reads_before == 3)
        {
            if (extra_reads > 0)
                extra_reads--;
            else
            {
                BOOST_FAIL ((boost::format("Too many reads, key = %1%")
                             % (j * 2)).str());
            }
        }
        else
        {
            BOOST_CHECK_EQUAL (reads_after, reads_before + 2);
        }
        test_nonexisting<int, unsigned> (t, i * 2 + 1, i * 2 + 1);
    }
    BOOST_CHECK_EQUAL (extra_reads, 0);
}

BOOST_FIXTURE_TEST_CASE (three_levels, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    std::auto_ptr<btree<int>::leaf> leaf_node = t.make_leaf();

    // Fill root
    unsigned i = 0;
    for (i = i; i < leaf_node->capacity(); i++)
    {
        t.insert (i * 2, i);
    }

    // Fill and split leaf level.
    for (i = i; i < leaf_node->capacity() * leaf_node->capacity(); i++)
    {
        t.insert (i * 2, i);
    }

    t.check ();

    for (unsigned j = 0; j < i; j++)
    {
        test_existing<int, unsigned> (t, static_cast<int>(j * 2),
                                      static_cast<int>(j * 2),
                                      static_cast<int>(j * 2),
                                      static_cast<int>(j));
        test_nonexisting<int, unsigned> (t, i * 2 + 1, i * 2 + 1);
    }
}

BOOST_FIXTURE_TEST_CASE (four_levels, F)
{
    btree<int> t (*storage, 208, header_page_id);
    unsigned leaf_capacity = (t.make_leaf ())->capacity ();

    unsigned i = 0;
    for (i = i; i < leaf_capacity; i++)
    {
        t.insert (i * 2, i);
    }

    for (i = i; i < leaf_capacity * leaf_capacity; i++)
    {
        t.insert (i * 2, i);
    }

    for (i = i; i < leaf_capacity * leaf_capacity * leaf_capacity; i++)
    {
        t.insert (i * 2, i);
    }

    t.check ();

    for (unsigned j = 0; j < i; j++)
    {
        unsigned reads_before = t.get_reads ();
        test_existing<int, unsigned> (t, static_cast<int>(j * 2),
                                      static_cast<int>(j * 2),
                                      static_cast<int>(j * 2),
                                      static_cast<int>(j));
        if ((t.get_reads () != reads_before + 4)
            && (t.get_reads () != reads_before + 5))
            BOOST_FAIL ("Wrong number of reads");
        test_nonexisting<int, unsigned> (t, i * 2 + 1, i * 2 + 1);
    }
}

BOOST_FIXTURE_TEST_CASE (float_keys, F)
{
    btree<int, float> t (*storage, page_size, header_page_id);
    t.insert (1.0, 5);
    t.insert (2.0, 10);
    test_existing<int, float> (t, 0.5, 1.5, 1.0, 5);
    test_existing<int, float> (t, 1.5, 2.5, 2.0, 10);
    test_nonexisting<int, float> (t, 3.0, 30.0);
}

BOOST_FIXTURE_TEST_CASE (split_smallest_child, F)
{
    btree<int> t (*storage, page_size, header_page_id);
    unsigned leaf_capacity = (t.make_leaf ())->capacity ();

    unsigned i;
    for (i = 0; i <= leaf_capacity; i++)
    {
        t.insert (i * 2, i);
        t.check ();
    }

    for (i = 0; i <= leaf_capacity / 2; i++)
    {
        t.insert (i * 2 + 1, i);
        t.check ();
        t.insert (i * 2 + 1, i);
        t.check ();
    }

    t.check ();
}

BOOST_AUTO_TEST_SUITE_END()
