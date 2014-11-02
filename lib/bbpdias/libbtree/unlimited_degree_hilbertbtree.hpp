#ifndef UNLIMITED_DEGREE_HILBERTBTREE_
#define UNLIMITED_DEGREE_HILBERTBTREE_

#include <boost/shared_ptr.hpp>

#include "hilbertbtree.hpp"
#include "btree.hpp"

#include <SpatialIndex.h>

#include "libbbpdias/tools.hpp"

namespace std
{
    template <>
    class numeric_limits<dias::float_vect>
    {
    public:
        static dias::float_vect max (void)
            {
                dias::float_vect result;
                result[0] = result[1] = result[2]
                    = std::numeric_limits<float>::max ();
                return result;
            }

        static dias::float_vect min (void)
            {
                dias::float_vect result;
                result[0] = result[1] = result[2]
                    = std::numeric_limits<float>::min ();
                return result;
            }
    };
}

namespace dias
{
    class hilbert_less
        : public std::binary_function<dias::float_vect,
                                      dias::float_vect, bool> {
    public:
        bool operator() (const float_vect & x, const float_vect & y) const
            {
                vect double_x;
                double_x[0] = x[0]; double_x[1] = x[1]; double_x[2] = x[2];
                vect double_y;
                double_y[0] = y[0]; double_y[1] = y[1]; double_y[2] = y[2];
                return hilbert_ieee_cmp (3, double_x.data (),
                                         double_y.data ()) < 0;
            }
    };

    class unlimited_degree_hilbert_btree
        : public hilbert_btree<float_vect, hilbert_less>
    {
    public:
        unlimited_degree_hilbert_btree (
            SpatialIndex::IStorageManager & storage,
            unsigned page_size, SpatialIndex::id_type header_page_id,
            const SpatialIndex::Region & world_extent,
            size_t dimension,
            const std::string & hilbert_value_output_fn = "");

        unlimited_degree_hilbert_btree (
            SpatialIndex::IStorageManager & storage,
            SpatialIndex::id_type header_page_id,
            const SpatialIndex::Region & world_extent,
            size_t dimension,
            const std::string & hilbert_value_output_fn = "");

        virtual void insertData (size_t len, const byte* pData,
                                 const SpatialIndex::IShape & shape,
                                 SpatialIndex::id_type shapeIdentifier);

        virtual void pointLocationQuery (const SpatialIndex::Point& query,
                                         SpatialIndex::IVisitor& v);

        static boost::shared_ptr<SpatialIndex::ISpatialIndex>
        make_unlimited_degree_hilbert_btree (
            SpatialIndex::IStorageManager & storage,
            unsigned page_size, SpatialIndex::id_type header_page_id,
            const SpatialIndex::Region & world_extent,
            size_t dimension,
            const std::string & hilbert_value_output_fn = "");

        static boost::shared_ptr<SpatialIndex::ISpatialIndex>
        load_unlimited_degree_hilbert_btree (
            SpatialIndex::IStorageManager & storage,
            SpatialIndex::id_type header_page_id,
            const SpatialIndex::Region & world_extent,
            size_t dimension,
            const std::string & hilbert_value_output_fn = "");
    };
}

#endif
