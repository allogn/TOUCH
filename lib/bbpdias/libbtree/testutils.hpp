#ifndef DIAS_TESTUTILS_HPP_
#define DIAS_TESTUTILS_HPP_

#include <boost/scoped_ptr.hpp>
#include <boost/unordered_set.hpp>

#include <SpatialIndex.h>

#include "hilbert_btree_base.hpp"

enum { page_size = 4096 };

struct F {
    F(void) :
        header_page_id (SpatialIndex::StorageManager::NewPage),
        storage (SpatialIndex::StorageManager::createNewMemoryStorageManager())
        {
            std::vector<byte> serialized_header (page_size, 0);
            storage->storeByteArray (header_page_id, serialized_header.size (),
                                     &serialized_header[0]);
        }

    SpatialIndex::id_type header_page_id;
    boost::scoped_ptr<SpatialIndex::IStorageManager> storage;
};

enum { number_of_dimensions = 3 };

class test_visitor : public SpatialIndex::IVisitor
{
public:
    test_visitor (const boost::unordered_multiset<dias::hilbert_btree_base::data>
                  & _expected_results) :
        expected_results (_expected_results)
        {
        }

private:
    boost::unordered_multiset<dias::hilbert_btree_base::data> expected_results;

    void visitNode (const SpatialIndex::INode & in __attribute__((unused)))
        {
            assert (false);
        };

    void visitData (const SpatialIndex::IData & in)
        {
            const dias::hilbert_btree_base::data * in_data
                = dynamic_cast<const dias::hilbert_btree_base::data *>(&in);
            BOOST_CHECK_EQUAL (expected_results.erase (*in_data), 1);
        };

    void visitData (std::vector<const SpatialIndex::IData*>& v __attribute__((unused)))
        {
            assert (false);
        };
};

#endif
