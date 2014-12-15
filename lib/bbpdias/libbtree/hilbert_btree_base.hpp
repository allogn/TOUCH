#ifndef DIAS_LIBBTREE_HILBERT_BTREE_BASE_HPP_
#define DIAS_LIBBTREE_HILBERT_BTREE_BASE_HPP_

#include <cstring>

#include <boost/array.hpp>
#include <boost/scoped_array.hpp>
#include <boost/functional/hash.hpp>

#include <SpatialIndex.h>

#include "libbbpdias/Hilbert.hpp"

namespace dias {

    class hilbert_btree_base : public SpatialIndex::ISpatialIndex
    {
    public:

        class statistics : public SpatialIndex::IStatistics
        {
        public:
            statistics (unsigned h, size_t r, size_t w, size_t n,
                        size_t _data) :
                height (h),
                reads (r),
                writes (w),
                nodes (n),
                data (_data)
                {
                };

            virtual ~statistics () { };

            unsigned getHeight () const
                {
                    return height;
                }

            virtual size_t getReads() const
                {
                    return reads;
                };

            virtual size_t getWrites() const
                {
                    return writes;
                };

            virtual size_t getNumberOfNodes () const
                {
                    return nodes;
                };

            virtual size_t getNumberOfData () const
                {
                    return data;
                };

        private:
            friend class hilbert_btree_base;

            friend std::ostream& operator<<(std::ostream & os,
                                            const statistics & s)
                {
                    os << "Height: " << s.getHeight () << '\n';
                    os << "Reads: " << s.getReads () << '\n';
                    os << "Writes: " << s.getWrites () << '\n';
                    os << "Nodes: " << s.getNumberOfNodes () << '\n';
                    os << "Data: " << s.getNumberOfData () << '\n';
                    return os;
                }

        private:
            unsigned height;
            size_t reads, writes, nodes, data;
        };

        class data : public SpatialIndex::IData
        {
        public:
            data (size_t _data_len, const void * _data) :
                data_len (_data_len),
                d (new byte[_data_len])
                {
                    memcpy (d.get (), _data, data_len);
                }

            data (const data & _d) :
                data_len (_d.data_len),
                d (new byte[_d.data_len])
                {
                    memcpy (d.get (), _d.d.get (), data_len);
                }

            virtual Tools::IObject * clone (void)
                {
                    assert (false);
                    return NULL;
                }

            virtual void getData (size_t & s, byte ** data) const
                {
                    s = data_len;
                    *data = new byte[data_len];
                    memcpy (data, d.get(), data_len);
                }

            virtual SpatialIndex::id_type getIdentifier (void) const
                {
                    assert (false);
                    return static_cast<SpatialIndex::id_type> (-1);
                }

            virtual void
            getShape (SpatialIndex::IShape ** s __attribute__((unused))) const
                {
                    assert (false);
                }

        private:
            friend bool operator == (const hilbert_btree_base::data & d1,
                                     const hilbert_btree_base::data & d2)
                {
                    if (d1.data_len != d2.data_len)
                        return false;

                    for (ptrdiff_t i = 0;
                         i < static_cast<ptrdiff_t>(d1.data_len); i++)
                    {
                        if (d1.d[i] != d2.d[i])
                            return false;
                    }

                    return true;
                }

            friend std::size_t hash_value (const hilbert_btree_base::data & d)
                {
                    std::size_t seed = 0;

                    for (size_t i = 0; i < d.data_len; i++)
                    {
                        boost::hash_combine (seed, d.d[i]);
                    }

                    return seed;
                }

            size_t data_len;
            boost::scoped_array<byte> d;
        };

// FIXME: payload itself is missing!
        struct payload
        {
            SpatialIndex::id_type id;
            size_t data_size;

            payload (SpatialIndex::id_type _id = -1,
                     size_t _data_size = 0) :
                id (_id),
                data_size (_data_size)
                {
                }
        };

        virtual bool deleteData (const SpatialIndex::IShape & shape,
                                 SpatialIndex::id_type shapeIdentifier);

        virtual void containsWhatQuery (const SpatialIndex::IShape & query,
                                        SpatialIndex::IVisitor & v);

        virtual void intersectsWithQuery (const SpatialIndex::IShape& query,
                                          SpatialIndex::IVisitor& v);

        virtual void nearestNeighborQuery (
            uint32_t k,
            const SpatialIndex::IShape& query,
            SpatialIndex::IVisitor& v,
            SpatialIndex::INearestNeighborComparator& nnc);

        virtual void nearestNeighborQuery (uint32_t k,
                                           const SpatialIndex::IShape& query,
                                           SpatialIndex::IVisitor& v);

        virtual void selfJoinQuery (const SpatialIndex::IShape& s,
                                    SpatialIndex::IVisitor& v);

        virtual void queryStrategy (SpatialIndex::IQueryStrategy& qs);

        virtual void getIndexProperties (Tools::PropertySet& out) const;

        virtual void addCommand (SpatialIndex::ICommand* in,
                                 SpatialIndex::CommandType ct);

    protected:
        typedef boost::array<bitmask_t, 3> discrete_point;

        hilbert_btree_base (const SpatialIndex::Region & world_extent,
                            size_t dimension,
                            const std::string & hilbert_value_output_fn = "");

        virtual ~hilbert_btree_base (void);

        discrete_point expand_to_space (const SpatialIndex::Point & p) const;

        bitmask_t
        point_to_hilbert_value (const SpatialIndex::Point & p,
                                size_t bits_per_dimension) const;

        void log_hilbert_value (const bitmask_t hilbert_value);

    private:
        const SpatialIndex::Point world_size;
        const SpatialIndex::Point world_start;

        std::ofstream hilbert_value_log;

    };
}

#endif
