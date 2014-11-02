#ifndef LIMITED_DEGREE_HILBERTBTREE_
#define LIMITED_DEGREE_HILBERTBTREE_

#include "hilbertbtree.hpp"
#include "btree.hpp"

namespace dias
{
    template <class T>
    class limited_degree_hilbert_btree : public hilbert_btree<T>
    {
    public:
        limited_degree_hilbert_btree (
            SpatialIndex::IStorageManager & storage,
            unsigned page_size,  SpatialIndex::id_type header_page_id,
            const SpatialIndex::Region & world_extent,
            size_t dimension,
            const std::string & hilbert_value_output_fn = "");

        limited_degree_hilbert_btree (
            SpatialIndex::IStorageManager & storage,
            SpatialIndex::id_type header_page_id,
            const SpatialIndex::Region & world_extent, size_t dimension,
            const std::string & hilbert_value_output_fn = "");

        virtual void insertData (size_t len, const byte* pData,
                                 const SpatialIndex::IShape & shape,
                                 SpatialIndex::id_type shapeIdentifier);

        virtual void pointLocationQuery (const SpatialIndex::Point& query,
                                         SpatialIndex::IVisitor& v);

        static boost::shared_ptr<SpatialIndex::ISpatialIndex>
        make_limited_degree_hilbert_btree (
            SpatialIndex::IStorageManager & storage,
            unsigned page_size, SpatialIndex::id_type header_page_id,
            const SpatialIndex::Region & world_extent,
            size_t dimension,
            const std::string & hilbert_value_output_fn = "");

        static boost::shared_ptr<SpatialIndex::ISpatialIndex>
        load_limited_degree_hilbert_btree (
            SpatialIndex::IStorageManager & storage,
            SpatialIndex::id_type, const SpatialIndex::Region & world_extent,
            size_t dimension,
            const std::string & hilbert_value_output_fn = "");
    };

    template <class T>
    void limited_degree_hilbert_btree<T>::insertData (
        size_t len,
        const byte* pData __attribute__((unused)),
        const SpatialIndex::IShape & shape,
        SpatialIndex::id_type shapeIdentifier)
    {
        const SpatialIndex::Point * point
            = dynamic_cast<const SpatialIndex::Point *> (&shape);

        T hilbert_value = static_cast<T>
            (this->point_to_hilbert_value (*point, sizeof (T) / 3));
        log_hilbert_value (hilbert_value);

        this->tree.insert (hilbert_value,
                           hilbert_btree_base::payload (shapeIdentifier, len));
    }

    template <class T>
    void limited_degree_hilbert_btree<T>::pointLocationQuery (
        const SpatialIndex::Point& query,
        SpatialIndex::IVisitor& v)
    {
        T search_hilbert_value = static_cast<T>
            (this->point_to_hilbert_value (query, sizeof (T) / 3));

        query_and_visit_query_results (search_hilbert_value, v);
    }

    template <class T>
    limited_degree_hilbert_btree<T>::limited_degree_hilbert_btree (
        SpatialIndex::IStorageManager & storage,
        unsigned page_size, SpatialIndex::id_type header_page_id,
        const SpatialIndex::Region & world_extent,
        size_t dimension,
        const std::string & hilbert_value_output_fn) :
        hilbert_btree<T> (storage, page_size, header_page_id, world_extent,
                          dimension, hilbert_value_output_fn)
    {
    }

    template <class T>
    limited_degree_hilbert_btree<T>::limited_degree_hilbert_btree (
        SpatialIndex::IStorageManager & storage,
        SpatialIndex::id_type header_page_id,
        const SpatialIndex::Region & world_extent, size_t dimension,
        const std::string & hilbert_value_output_fn) :
        hilbert_btree<T> (storage, header_page_id, world_extent, dimension,
                          hilbert_value_output_fn)
    {
    }

    template <class T>
    boost::shared_ptr<SpatialIndex::ISpatialIndex>
    limited_degree_hilbert_btree<T>::make_limited_degree_hilbert_btree (
        SpatialIndex::IStorageManager & storage,
        unsigned page_size, SpatialIndex::id_type header_page_id,
        const SpatialIndex::Region & world_extent,
        size_t dimension,
        const std::string & hilbert_value_output_fn)
    {
        return boost::shared_ptr<SpatialIndex::ISpatialIndex> (
            new limited_degree_hilbert_btree<T> (storage, page_size,
                                                 header_page_id,
                                                 world_extent, dimension,
                                                 hilbert_value_output_fn));
    }

    template <class T>
    boost::shared_ptr<SpatialIndex::ISpatialIndex>
    limited_degree_hilbert_btree<T>::load_limited_degree_hilbert_btree (
        SpatialIndex::IStorageManager & storage,
        SpatialIndex::id_type header_page_id,
        const SpatialIndex::Region & world_extent, size_t dimension,
        const std::string & hilbert_value_output_fn)
    {
        return boost::shared_ptr<SpatialIndex::ISpatialIndex> (
            new limited_degree_hilbert_btree<T> (storage, header_page_id,
                                                 world_extent, dimension,
                                                 hilbert_value_output_fn));
    }
}

#endif
