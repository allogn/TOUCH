#ifndef DIAS_SFCBTREE_HPP_
#define DIAS_SFCBTREE_HPP_

#include <functional>

#include <boost/tuple/tuple.hpp>

#include "hilbert_btree_base.hpp"
#include "btree.hpp"

#include <SpatialIndex.h>

namespace dias {

    template <typename K, typename _Compare = std::less<K> >
    class hilbert_btree : public hilbert_btree_base
    {
    public:
        virtual void getStatistics (SpatialIndex::IStatistics** out) const;

        virtual bool isIndexValid (void);

    protected:
        hilbert_btree (SpatialIndex::IStorageManager & storage,
                       unsigned page_size, SpatialIndex::id_type header_page_id,
                       const SpatialIndex::Region & world_extent,
                       size_t dimension,
                       const std::string & hilbert_value_output_fn = "");

        hilbert_btree (SpatialIndex::IStorageManager & storage,
                       SpatialIndex::id_type header_page_id,
                       const SpatialIndex::Region & world_extent,
                       size_t dimension,
                       const std::string & hilbert_value_output_fn = "");

        virtual ~hilbert_btree (void);

        void query_and_visit_query_results (K search_value,
                                            SpatialIndex::IVisitor &v) const;

        typedef btree<payload, K, _Compare> tree_type;
        tree_type tree;
    };

    template <typename K, typename _Compare>
    hilbert_btree<K, _Compare>::hilbert_btree (
        SpatialIndex::IStorageManager & storage,
        unsigned page_size, SpatialIndex::id_type header_page_id,
        const SpatialIndex::Region & world_extent,
        size_t dimension,
        const std::string & hilbert_value_output_fn) :
        hilbert_btree_base (world_extent, dimension, hilbert_value_output_fn),
        tree (storage, page_size, header_page_id)
    {
    }

    template <typename K, typename _Compare>
    hilbert_btree<K, _Compare>::hilbert_btree (
        SpatialIndex::IStorageManager & storage,
        SpatialIndex::id_type header_page_id,
        const SpatialIndex::Region & world_extent,
        size_t dimension,
        const std::string & hilbert_value_output_fn) :
        hilbert_btree_base (world_extent, dimension, hilbert_value_output_fn),
        tree (header_page_id, storage)
    {
    }

    template <typename K, typename _Compare>
    hilbert_btree<K, _Compare>::~hilbert_btree (void)
    {
    }

    template <typename K, typename _Compare>
    void
    hilbert_btree<K, _Compare>::query_and_visit_query_results (
        K search_value, SpatialIndex::IVisitor &v) const
    {
        typename tree_type::query_answer results
            = this->tree.range_query (std::make_pair (search_value,
                                                      search_value));
        BOOST_FOREACH (typename tree_type::key_value r, results)
        {
// FIXME: no node visiting
            v.visitData (data (sizeof (r.second), &r.second));
        }
    }

    template <typename K, typename _Compare>
    void hilbert_btree<K, _Compare>::getStatistics (
        SpatialIndex::IStatistics** out) const
    {
        *out = new statistics (tree.get_height (), tree.get_reads (),
                               tree.get_writes (), tree.get_nodes (),
                               tree.get_data ());
    }

    template <typename K, typename _Compare>
    bool
    hilbert_btree<K, _Compare>::isIndexValid (void)
    {
        try {
            tree.check ();
        }
        catch (std::logic_error & e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
        return true;
    }

}

#endif
