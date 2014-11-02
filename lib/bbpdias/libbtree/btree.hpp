#ifndef DIAS_BTREE_HPP_
#define DIAS_BTREE_HPP_

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/throw_exception.hpp>

#include <SpatialIndex.h>

#include "libbbpdias/tools.hpp"
#include "libbbpdias/spatialindex_tools.hpp"

namespace dias {

    template <typename V, typename K = unsigned,
              typename _Compare = std::less<K> >
    class btree
    {
    public:

        typedef K key;
        typedef V value;
        typedef std::pair<key, key> key_range;
        typedef std::pair<key, V> key_value;
        typedef std::vector<key_value> query_answer;
        typedef std::pair<key, SpatialIndex::id_type> key_page_id;

        class node
        {
        public:
            virtual ~node(void);

            unsigned capacity (void) const;

        protected:
            explicit node (unsigned capacity);

        private:
            friend class btree<V, K, _Compare>;

            void update (SpatialIndex::IStorageManager & storage,
                         SpatialIndex::id_type page_id) const;

            SpatialIndex::id_type
            write (SpatialIndex::IStorageManager & storage) const;

        private:
            virtual std::vector<byte> serialize(void) const = 0;

            unsigned _capacity;
        };

        class internal_node : public node
        {
        public:
            virtual ~internal_node (void);

        private:
            friend class btree<V, K, _Compare>;

            typedef typename std::vector<key_page_id>::iterator entry_iterator;
            typedef typename
            std::vector<key_page_id>::const_iterator entry_const_iterator;

            internal_node (unsigned page_size, key split_key,
                           SpatialIndex::id_type lesser_child,
                           SpatialIndex::id_type greater_child);

            internal_node (const internal_node & take_from,
                           internal_node::entry_iterator copy_start);

            void add_entry (key k, SpatialIndex::id_type child_page_id);

            key_page_id choose_child_internal_node (key k, key sentinel) const;
            key_page_id choose_child_leaf (const key k,
                                           const key sentinel) const;

            entry_iterator end (void) const;

            entry_iterator split_point (void) const;

            void
            remove_entries_from (entry_iterator begin,
                                 SpatialIndex::id_type new_greatest_child);

            bool full (void) const;

            static boost::shared_ptr<internal_node>
            read (SpatialIndex::IStorageManager & storage,
                  SpatialIndex::id_type page_id, unsigned page_size);

            void check (
                const btree<V, K, _Compare> & t,
                const key lower_bound, const key upper_bound,
                unsigned subtree_height, SpatialIndex::id_type page_id,
                std::set<SpatialIndex::id_type> & seen_page_ids,
                std::set<SpatialIndex::id_type> & unseen_page_ids) const;

            void dump (std::ostream & out, const btree<V, K, _Compare> & t,
                       SpatialIndex::id_type node_id,
                       unsigned subtree_height) const;

        private:
            explicit internal_node (unsigned page_size);

            entry_const_iterator choose_child (key k) const;

            static unsigned calc_capacity (unsigned page_size, size_t key_size,
                                           size_t page_id_size);

            virtual std::vector<byte> serialize(void) const;

            std::vector <key_page_id> entries;

            SpatialIndex::id_type greatest_child;
        };

        class leaf : public node
        {
        public:
            virtual ~leaf (void);

        private:
            friend class btree<V, K, _Compare>;

            typedef typename std::vector<key_value>::iterator entry_iterator;
            typedef typename std::vector<key_value>::const_iterator
            entry_const_iterator;

            explicit leaf (unsigned page_size);

            leaf (unsigned page_size, entry_const_iterator copy_from_begin,
                  entry_const_iterator copy_from_end,
                  SpatialIndex::id_type next_page_id);

            void add_entry (const key k, const V value);

            void query (const btree<V, K, _Compare> & t, query_answer & result,
                        const key sentinel, const key_range & range) const;

            bool full (void) const;

            entry_iterator end (void) const;

            entry_iterator split_point (void) const;

            void remove_entries_from (entry_iterator begin);

            SpatialIndex::id_type get_next (void) const;

            void set_next (SpatialIndex::id_type _next_leaf);

            static boost::shared_ptr<leaf> read (
                SpatialIndex::IStorageManager & storage,
                SpatialIndex::id_type page_id,
                unsigned page_size);

            void check (const key lower_bound,
                        const key upper_bound,
                        SpatialIndex::id_type node_id,
                        SpatialIndex::id_type parent_id) const;

            void check_leaf_chain (const btree<V, K, _Compare> & t,
                                   SpatialIndex::id_type first_leaf_id) const;

            void dump (std::ostream & out) const;

        private:
            bool visit_next_node (const key sentinel,
                                  const key_range & range) const;

            void add_to_query_answer (query_answer & result,
                                      const key_range & range) const;

            static unsigned calc_capacity (unsigned page_size,
                                           size_t num_entries_size,
                                           size_t key_size,
                                           size_t value_size,
                                           size_t node_id_size);

            virtual std::vector<byte> serialize(void) const;

            std::vector <key_value> entries;

            SpatialIndex::id_type next_leaf;
        };

        btree (SpatialIndex::IStorageManager & _storage,
               unsigned page_size, SpatialIndex::id_type header_page_id);

        btree (SpatialIndex::id_type header_page_id,
               SpatialIndex::IStorageManager & _storage);

        virtual ~btree (void);

        void write_header (void) const;

        unsigned get_height (void) const;
        unsigned get_reads (void) const;
        unsigned get_writes (void) const;
        unsigned get_nodes (void) const;
        unsigned get_data (void) const;

        void insert (const key key, const V value);

        std::auto_ptr<internal_node>
        make_internal_node (key split_key, SpatialIndex::id_type lesser_child,
                            SpatialIndex::id_type greater_child) const;

        std::auto_ptr<leaf> make_leaf(void) const;

        query_answer range_query (const key_range & range) const;

        void check (void) const;

        void check_subtree (
            SpatialIndex::id_type subtree_root_id,
            SpatialIndex::id_type subtree_parent_id,
            unsigned subtree_height,
            key & lower_bound,
            key upper_bound,
            std::set<SpatialIndex::id_type> & seen_page_ids,
            std::set<SpatialIndex::id_type> & unseen_page_ids) const;

        void dump (std::ostream & out) const;

    private:

        typedef
        std::pair<SpatialIndex::id_type,
                  boost::shared_ptr <internal_node> > id_node_pair;

        typedef std::vector<id_node_pair> tree_path;

        btree(void);

        query_answer range_query_leaf (const key_page_id & leaf_info,
                                       const key_range & range) const;

        key_page_id
        find_leaf (const key new_key, tree_path * path_from_root) const;

        void
        insert_to_leaf (const key new_key, const V new_value,
                        SpatialIndex::id_type leaf_id,
                        tree_path & path_from_root);

        void make_new_root (const key split_key,
                            SpatialIndex::id_type lesser_child,
                            SpatialIndex::id_type greater_child);

        std::auto_ptr<internal_node>
        make_internal_node (const internal_node & take_from,
                            typename internal_node::entry_iterator copy_start)
            const;

        std::auto_ptr<leaf>
        make_leaf (typename leaf::entry_const_iterator begin,
                   typename leaf::entry_const_iterator end,
                   SpatialIndex::id_type next_page_id) const;

        boost::shared_ptr<internal_node>
        read_internal_node (SpatialIndex::id_type page_id) const;

        boost::shared_ptr<leaf>
        read_leaf (SpatialIndex::id_type page_id) const;

        void update (const node & n, SpatialIndex::id_type page_id);

        SpatialIndex::id_type write (const node & n);

        SpatialIndex::IStorageManager & storage;

        unsigned page_size;

        SpatialIndex::id_type header_page_id;
        SpatialIndex::id_type root_node_id;

        unsigned _height;

        mutable unsigned _reads;
        unsigned _writes;
        unsigned _nodes;
        unsigned _data;

        std::set<SpatialIndex::id_type> written_pages;
    };

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::node::~node(void)
    {
    }

    template <typename V, typename K, typename _Compare>
    inline unsigned btree<V, K, _Compare>::node::capacity (void) const
    {
        return _capacity;
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::node::node (unsigned capacity) :
        _capacity (capacity)
    {
    }

    template <typename V, typename K, typename _Compare>
    inline void
    btree<V, K, _Compare>::node::update (
        SpatialIndex::IStorageManager & storage,
        SpatialIndex::id_type page_id) const
    {
        std::vector<byte> serialized = serialize();
        SpatialIndex::id_type result_page = page_id;
        storage.storeByteArray (result_page, serialized.size(),
                                &serialized[0]);
        assert (result_page == page_id);
    }

    template <typename V, typename K, typename _Compare>
    inline SpatialIndex::id_type
    btree<V, K, _Compare>::node::write (
        SpatialIndex::IStorageManager & storage) const
    {
        std::vector<byte> serialized = serialize();
        SpatialIndex::id_type result_page
            = SpatialIndex::StorageManager::NewPage;
        storage.storeByteArray (result_page, serialized.size(),
                                &serialized[0]);
        return result_page;
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::internal_node::~internal_node(void)
    {
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::internal_node::internal_node (
        unsigned page_size,
        key split_key,
        SpatialIndex::id_type lesser_child,
        SpatialIndex::id_type greater_child) :
        node (calc_capacity (page_size, sizeof (key),
                             sizeof (SpatialIndex::id_type))),
        entries (1, key_page_id (split_key, lesser_child)),
        greatest_child (greater_child)
    {
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::internal_node::internal_node (
        const btree<V, K, _Compare>::internal_node & take_from,
        internal_node::entry_iterator copy_start) :
        node (take_from.capacity()),
        entries (copy_start, take_from.end ()),
        greatest_child (take_from.greatest_child)
    {
    }

    template <typename V, typename K, typename _Compare>
    bool btree<V, K, _Compare>::internal_node::full (void) const
    {
        return entries.size() == node::capacity();
    }

    template <typename V, typename K, typename _Compare>
    inline void
    btree<V, K, _Compare>::internal_node::add_entry (
        key k,
        SpatialIndex::id_type child_page_id)
    {
        assert (entries.size() + 1 <= node::capacity());

        _Compare comparator;
        if (comparator (entries.back().first, k))
        {
            entries.push_back (std::make_pair (k, greatest_child));
            greatest_child = child_page_id;
        }
        else
        {
            entry_iterator insertion_place
                = std::lower_bound
                (entries.begin(), entries.end(), k,
                 boost::bind (comparator,
                              boost::bind(&key_page_id::first, _1),
                              k));
            SpatialIndex::id_type sibling_child_id = insertion_place->second;
            insertion_place->second = child_page_id;
            entries.insert (insertion_place,
                            std::make_pair(k, sibling_child_id));
        }

        assert (entries.size() <= node::capacity());
    }

    template <typename V, typename K, typename _Compare>
    inline typename btree<V, K, _Compare>::key_page_id
    btree<V, K, _Compare>::internal_node::choose_child_internal_node (
        key k,
        key sentinel) const
    {
#ifndef NDEBUG
        _Compare comparator;
        assert (!comparator (sentinel, k));
#endif

        entry_const_iterator child_ptr = choose_child (k);

        return (child_ptr == entries.end())
            ? std::make_pair (sentinel, greatest_child) : *child_ptr;
    }

    template <typename V, typename K, typename _Compare>
    inline typename btree<V, K, _Compare>::key_page_id
    btree<V, K, _Compare>::internal_node::choose_child_leaf (
        const key k, const key sentinel) const
    {
        entry_const_iterator child_ptr = choose_child (k);

        return (child_ptr == entries.end ())
            ? std::make_pair (sentinel, greatest_child) : *child_ptr;
    }

    template <typename V, typename K, typename _Compare>
    inline typename btree<V, K, _Compare>::internal_node::entry_const_iterator
    btree<V, K, _Compare>::internal_node::choose_child (key k) const
    {
        _Compare comparator;

        entry_const_iterator child_ptr
            = std::lower_bound (entries.begin(), entries.end(), k,
                                boost::bind (comparator,
                                             boost::bind(&key_page_id::first,
                                                         _1),
                                             k));

        assert (((child_ptr == entries.end())
                 && !comparator (k, entries.back().first))
                || !comparator (child_ptr->first, k));

        return child_ptr;
    }

    template <typename V, typename K, typename _Compare>
    inline typename btree<V, K, _Compare>::internal_node::entry_iterator
    btree<V, K, _Compare>::internal_node::end (void) const
    {
        return const_cast<std::vector<key_page_id> *>(&entries)->end();
    }

    template <typename V, typename K, typename _Compare>
    inline typename btree<V, K, _Compare>::internal_node::entry_iterator
    btree<V, K, _Compare>::internal_node::split_point (void) const
    {
        entry_iterator result
            = const_cast<std::vector<key_page_id> *>(&entries)->begin()
            + entries.size() / 2;

        assert ((((result - entries.begin())
                  - (end() - result)) <= 1)
                && (((result - entries.begin())
                     - (end() - result)) >= -1));

        return result;
    }

    template <typename V, typename K, typename _Compare>
    inline void btree<V, K, _Compare>::internal_node::remove_entries_from (
        entry_iterator begin,
        SpatialIndex::id_type new_greatest_child)
    {
        entries.erase (begin, entries.end ());
        greatest_child = new_greatest_child;
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::internal_node::internal_node (
        unsigned page_size) :
        node (calc_capacity (page_size, sizeof (key),
                             sizeof (SpatialIndex::id_type))),
        entries (),
        greatest_child (SpatialIndex::StorageManager::EmptyPage)
    {
    }

    template <typename V, typename K, typename _Compare>
    inline unsigned btree<V, K, _Compare>::internal_node::calc_capacity (
        unsigned page_size, size_t key_size, size_t page_id_size)
    {
        return (page_size - page_id_size)
            / (key_size + page_id_size);
    }

    template <typename V, typename K, typename _Compare>
    std::vector<byte> btree<V, K, _Compare>::internal_node::serialize (void)
        const
    {
        std::vector<byte> result (sizeof (entries.size()) + entries.size()
                                  * (sizeof (key)
                                     + sizeof (SpatialIndex::id_type))
                                  + sizeof (SpatialIndex::id_type));
        byte * ptr = &result[0];

        dias::serialize (entries.size (), &ptr);
        BOOST_FOREACH (key_page_id entry, entries)
        {
            dias::serialize (entry.first, &ptr);
            dias::serialize (entry.second, &ptr);
        }
        dias::serialize (greatest_child, &ptr);

        assert (ptr == (&result.back ()) + 1);
        return result;
    }

    template <typename V, typename K, typename _Compare>
    inline boost::shared_ptr<typename btree<V, K, _Compare>::internal_node>
    btree<V, K, _Compare>::internal_node::read (
        SpatialIndex::IStorageManager & storage,
        SpatialIndex::id_type page_id,
        unsigned page_size)
    {
        size_t length;
        boost::shared_array<byte> bytes
            = dias::read_byte_array (storage, page_id, length);
        assert (length <= page_size);
        byte * c_bytes = bytes.get ();

        size_t num_of_entries;
        dias::unserialize (num_of_entries, &c_bytes);

        boost::shared_ptr<internal_node> result(new internal_node(page_size));

        while (num_of_entries > 0)
        {
            key k;
            dias::unserialize (k, &c_bytes);

            SpatialIndex::id_type page_id;
            dias::unserialize (page_id, &c_bytes);

            result->entries.push_back (std::make_pair(k, page_id));
            num_of_entries--;
        }

        dias::unserialize (result->greatest_child, &c_bytes);

        assert (c_bytes == bytes.get() + length);
        return result;
    }

    template <typename V, typename K, typename _Compare>
    void btree<V, K, _Compare>::internal_node::check (
        const btree<V, K, _Compare> & t,
        const key lower_bound, const key upper_bound,
        unsigned subtree_height, SpatialIndex::id_type page_id,
        std::set<SpatialIndex::id_type> & seen_page_ids,
        std::set<SpatialIndex::id_type> & unseen_page_ids) const
    {
        if (entries.size() > node::capacity())
        {
            BOOST_THROW_EXCEPTION (std::logic_error
                                   ("Too many entries in an internal node"));
        }

        _Compare comparator;
        if (std::adjacent_find (
                entries.begin (), entries.end (),
                boost::bind (comparator,
                             boost::bind (&key_page_id::first, _2),
                             boost::bind (&key_page_id::first, _1)))
            != entries.end ())
        {
            BOOST_THROW_EXCEPTION (std::logic_error
                                   ("Unsorted internal node entries"));
        }

        if (comparator (entries.front().first, lower_bound))
        {
            std::string error
                = (boost::format ("Internal node (id = %1%) "
                                  "smallest key (= %2%) is smaller than "
                                  "the lower bound (= %3%)")
                   % page_id % entries.front().first % lower_bound).str();
            BOOST_THROW_EXCEPTION (std::logic_error (error));
        }

        if (comparator (upper_bound, entries.back().first))
        {
            std::string error
                = (boost::format ("Internal node (id = %1%) "
                                  "largest key (= %2%) is larger than "
                                  "the upper bound (= %3%)")
                   % page_id % entries.back().first % upper_bound).str();
            BOOST_THROW_EXCEPTION (std::logic_error (error));
        }

        key subtree_lower_bound = lower_bound;
        int i = 0;
        BOOST_FOREACH (key_page_id k_p, entries)
        {
            t.check_subtree (k_p.second, page_id, subtree_height - 1,
                             subtree_lower_bound, k_p.first, seen_page_ids,
                             unseen_page_ids);
            subtree_lower_bound = k_p.first;
            i++;
        }
        t.check_subtree (greatest_child, page_id, subtree_height - 1,
                         subtree_lower_bound, upper_bound, seen_page_ids,
                         unseen_page_ids);
    }

    template <typename V, typename K, typename _Compare>
    void btree<V, K, _Compare>::internal_node::dump (
        std::ostream & out,
        const btree<V, K, _Compare> & t, SpatialIndex::id_type node_id,
        unsigned subtree_height) const
    {
        out << "Internal node, id = " << node_id << ", subtree height = "
            << subtree_height << ", size = " << entries.size() << "\n";
        BOOST_FOREACH (key_page_id k_p, entries)
        {
            out << "  (in " << node_id << ") key = " << k_p.first
                << ", page = " << k_p.second << '\n';
            if (subtree_height == 1)
            {
                boost::shared_ptr<typename btree<V, K, _Compare>::leaf> leaf_node
                    = t.read_leaf (k_p.second);
                leaf_node->dump (out);
            }
            else
            {
                boost::shared_ptr<typename btree<V, K, _Compare>::internal_node>
                    intl_node = t.read_internal_node (k_p.second);
                intl_node->dump (out, t, k_p.second, subtree_height - 1);
            }
        }
        out << boost::format("  (in %1%) greatest child page = %2%\n")
            % node_id % greatest_child;
        if (subtree_height == 1)
        {
            boost::shared_ptr<typename btree<V, K, _Compare>::leaf> greatest_leaf
                = t.read_leaf (greatest_child);
            greatest_leaf->dump (out);
        }
        else
        {
            boost::shared_ptr<typename btree<V, K, _Compare>::internal_node>
                g_child = t.read_internal_node (greatest_child);
            g_child->dump (out, t, greatest_child, subtree_height - 1);
        }
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::leaf::~leaf(void)
    {
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::leaf::leaf (unsigned page_size) :
        node (calc_capacity (page_size,
                             sizeof (typename
                                     std::vector<key_value>::size_type),
                             sizeof (typename btree<V, K, _Compare>::key),
                             sizeof (typename btree<V, K, _Compare>::value),
                             sizeof (SpatialIndex::id_type))),
        entries (),
        next_leaf (SpatialIndex::StorageManager::EmptyPage)
    {
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::leaf::leaf (
        unsigned page_size,
        entry_const_iterator copy_from_begin,
        entry_const_iterator copy_from_end,
        SpatialIndex::id_type next_page_id) :
        node (calc_capacity (page_size,
                             sizeof (typename
                                     std::vector<key_value>::size_type),
                             sizeof (typename btree<V, K, _Compare>::key),
                             sizeof (typename btree<V, K, _Compare>::value),
                             sizeof (SpatialIndex::id_type))),
        entries (copy_from_begin, copy_from_end),
        next_leaf (next_page_id)
    {
    }


    template <typename V, typename K, typename _Compare>
    inline void btree<V, K, _Compare>::leaf::add_entry (const key k,
                                                        const V value)
    {
        _Compare comparator;
        entries.insert (std::lower_bound
                        (entries.begin(), entries.end(), k,
                         boost::bind (comparator,
                                      boost::bind (&key_value::first, _1), k)),
                        std::make_pair(k, value));
        assert (entries.size() <= node::capacity());
    }

    template <typename V, typename K, typename _Compare>
    inline void
    btree<V, K, _Compare>::leaf::query (
        const btree<V, K, _Compare> & t,
        query_answer & result,
        const key sentinel,
        const key_range & range) const
    {
        add_to_query_answer (result, range);

        if (!visit_next_node (sentinel, range))
            return;

        boost::shared_ptr<leaf> next_node (t.read_leaf (next_leaf));
        while (true)
        {
            next_node->add_to_query_answer (result, range);

            if (!next_node->visit_next_node (sentinel, range))
                break;

            next_node = t.read_leaf (next_node->next_leaf);
        }
    }

    template <typename V, typename K, typename _Compare>
    inline bool btree<V, K, _Compare>::leaf::visit_next_node (
        const key sentinel,
        const key_range & range) const
    {
        _Compare comparator;
        return (next_leaf != SpatialIndex::StorageManager::EmptyPage)
            && !comparator (sentinel, entries.back().first)
            && !comparator (range.second, sentinel);
    }

    template <typename V, typename K, typename _Compare>
    inline void btree<V, K, _Compare>::leaf::add_to_query_answer (
        query_answer & result,
        const key_range & range) const
    {
        _Compare comparator;

        entry_const_iterator result_begin
            = std::find_if (entries.begin(), entries.end(),
                            !boost::bind (
                                comparator,
                                boost::bind (&key_value::first, _1),
                                range.first)
                            && !boost::bind (
                                comparator,
                                range.second,
                                boost::bind (&key_value::first, _1)));

        entry_const_iterator result_end
            = std::find_if (result_begin, entries.end(),
                            boost::bind (
                                comparator,
                                range.second,
                                boost::bind (&key_value::first, _1)));

        result.insert (result.end (), result_begin, result_end);
    }

    template <typename V, typename K, typename _Compare>
    inline bool btree<V, K, _Compare>::leaf::full (void) const
    {
        return entries.size() == node::capacity();
    }

    template <typename V, typename K, typename _Compare>
    inline typename btree<V, K, _Compare>::leaf::entry_iterator
    btree<V, K, _Compare>::leaf::end (void) const
    {
        return const_cast<std::vector<key_value> *>(&entries)->end();
    }

    template <typename V, typename K, typename _Compare>
    inline typename btree<V, K, _Compare>::leaf::entry_iterator
    btree<V, K, _Compare>::leaf::split_point (void) const
    {
        entry_iterator result
            = const_cast<std::vector<key_value> *>(&entries)->begin()
            + entries.size() / 2;

        assert ((((result - entries.begin())
                 - (end() - result)) <= 1)
                && (((result - entries.begin())
                     - (end() - result)) >= -1));

        return result;
    }

    template <typename V, typename K, typename _Compare>
    inline void
    btree<V, K, _Compare>::leaf::remove_entries_from (entry_iterator begin)
    {
        entries.erase (begin, entries.end ());
    }

    template <typename V, typename K, typename _Compare>
    inline SpatialIndex::id_type
    btree<V, K, _Compare>::leaf::get_next (void) const
    {
        return next_leaf;
    }

    template <typename V, typename K, typename _Compare>
    inline void
    btree<V, K, _Compare>::leaf::set_next (SpatialIndex::id_type _next_leaf)
    {
        next_leaf = _next_leaf;
    }

    template <typename V, typename K, typename _Compare>
    inline boost::shared_ptr<typename btree<V, K, _Compare>::leaf>
    btree<V, K, _Compare>::leaf::read (SpatialIndex::IStorageManager & storage,
                                       SpatialIndex::id_type page_id,
                                       unsigned page_size)
    {
        size_t length;
        boost::shared_array<byte> bytes
            = read_byte_array (storage, page_id, length);
        assert (length <= page_size);
        byte * c_bytes = bytes.get();

        size_t num_of_entries;

        dias::unserialize (num_of_entries, &c_bytes);

        boost::shared_ptr<leaf> result(new leaf(page_size));

        while (num_of_entries > 0)
        {
            key k;
            dias::unserialize (k, &c_bytes);

            V v;
            dias::unserialize (v, &c_bytes);

            result->add_entry (k, v);
            num_of_entries--;
        }

        SpatialIndex::id_type next_leaf;
        dias::unserialize (next_leaf, &c_bytes);
        result->set_next (next_leaf);

#ifndef NDEBUG
        if (c_bytes != bytes.get() + length)
        {
            result->dump (std::cerr);
            std::cerr << "Page id = " << page_id << '\n';
            std::cerr << "Consumed: " << c_bytes - bytes.get () << '\n'
                      << "Should have consumed = " << length << '\n';
            assert (c_bytes == bytes.get() + length);
        }
#endif
        return result;
    }

    template <typename V, typename K, typename _Compare>
    std::vector<byte> btree<V, K, _Compare>::leaf::serialize (void) const
    {
        std::vector<byte> result (sizeof (entries.size()) + entries.size()
                                  * (sizeof (key) + sizeof (V))
                                  + sizeof (next_leaf));
        byte * ptr = &result[0];

        dias::serialize (entries.size (), &ptr);
        BOOST_FOREACH (key_value entry, entries)
        {
            dias::serialize (entry.first, &ptr);
            dias::serialize (entry.second, &ptr);
        }
        dias::serialize (next_leaf, &ptr);

        assert (ptr == (&result.back ()) + 1);
        return result;
    }

    template <typename V, typename K, typename _Compare>
    inline unsigned btree<V, K, _Compare>::leaf::calc_capacity (
        unsigned page_size, size_t num_entries_size,
        size_t key_size, size_t value_size, size_t node_id_size)
    {
        return (page_size - node_id_size - num_entries_size)
            / (key_size + value_size);
    }

    template <typename V, typename K, typename _Compare>
    void btree<V, K, _Compare>::leaf::check (
        const key lower_bound,
        const key upper_bound,
        SpatialIndex::id_type node_id,
        SpatialIndex::id_type parent_id) const
    {
        if (entries.size() == 0)
            return; // FIXME: only root
        if (entries.size() > node::capacity())
        {
            std::string error
                = (boost::format ("Too many entries in a leaf node id = %1%")
                   % node_id).str ();
            BOOST_THROW_EXCEPTION (std::logic_error (error));
        }

        _Compare comparator;
        if (std::adjacent_find (entries.begin(), entries.end(),
                                boost::bind (
                                    comparator,
                                    boost::bind (&key_value::first, _2),
                                    boost::bind (&key_value::first, _1)))
            != entries.end())
        {
            BOOST_THROW_EXCEPTION (std::logic_error
                                   ("Unsorted leaf entries"));
        }
        if (comparator (entries.front().first, lower_bound))
        {
            std::string error
                = (boost::format ("Smallest leaf node (id = %1%) "
                                  "entry (= %2%) is smaller than "
                                  "the discriminator (= %3%) in "
                                  "the parent node (id = %4%)")
                   % node_id % entries.front().first % lower_bound
                   % parent_id).str();
            BOOST_THROW_EXCEPTION (std::logic_error (error));
        }
        if (comparator (upper_bound, entries.back().first))
        {
            std::string error
                = (boost::format ("Largest leaf node (id = %1%) "
                                  "entry (= %2%) is larger than "
                                  "the discriminator (= %3%) in "
                                  "the parent node (=%4%)")
                   % node_id % entries.back().first % upper_bound
                   % parent_id).str();
            BOOST_THROW_EXCEPTION (std::logic_error (error));
        }
    }

    template <typename V, typename K, typename _Compare>
    void
    btree<V, K, _Compare>::leaf::check_leaf_chain (
        const btree<V, K, _Compare> & t,
        SpatialIndex::id_type first_leaf_id) const
    {
        _Compare comparator;

        if (entries.size() == 0)
            return;

        key last_node_key = entries.back().first;
        SpatialIndex::id_type current_leaf_id = first_leaf_id;
        SpatialIndex::id_type next_leaf_id = next_leaf;
        std::set<SpatialIndex::id_type> seen_ids;
        seen_ids.insert (next_leaf_id);
        while (next_leaf_id != SpatialIndex::StorageManager::EmptyPage)
        {
            boost::shared_ptr<leaf> next = t.read_leaf (next_leaf_id);
            if (comparator (next->entries.front().first, last_node_key))
            {
                std::cout << current_leaf_id;
                std::cout << last_node_key;
                std::cout << next->entries.front ().first;
                std::cout << next_leaf_id;
                std::string error
                    = (boost::format ("Largest leaf (id=%1%) "
                                      "node entry (=%2%)  is "
                                      "larger than the smallest leaf node "
                                      "entry (=%3%) in the next leaf (id=%4%)")
                       % current_leaf_id % last_node_key
                       % next->entries.front().first % next_leaf_id).str ();
                BOOST_THROW_EXCEPTION (std::logic_error (error));
            }
            current_leaf_id = next_leaf_id;
            next_leaf_id = next->next_leaf;
            assert (seen_ids.find (next_leaf_id) == seen_ids.end ());
            seen_ids.insert (next_leaf_id);
            last_node_key = next->entries.back().first;
        }
    }

    template <typename V, typename K, typename _Compare>
    void btree<V, K, _Compare>::leaf::dump (std::ostream & out) const
    {
        out << "Leaf node, size = " << entries.size() << '\n';
        if (entries.size() == 0)
            return; // FIXME: only empty root
        out << "Smallest key = " << entries.front().first << '\n';
        out << "Largest key = " << entries.back().first << '\n';
        out << "Next leaf id = " << next_leaf << '\n';
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::btree(
        SpatialIndex::IStorageManager & _storage,
        unsigned _page_size, SpatialIndex::id_type _header_page_id) :
        storage (_storage),
        page_size (_page_size),
        header_page_id (_header_page_id),
        root_node_id (SpatialIndex::StorageManager::NewPage),
        _height (0),
        _reads (0),
        _writes (0),
        _nodes (0),
        _data (0),
        written_pages ()
    {
        leaf empty_root (page_size);
        root_node_id = write (empty_root);
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::btree (
        SpatialIndex::id_type header_page_id,
        SpatialIndex::IStorageManager & _storage) :
        storage (_storage),
        page_size (0),
        root_node_id (),
        _height (0),
        _reads (0),
        _writes (0),
        _nodes (0),
        _data (0),
        written_pages ()
    {
        size_t length;
        boost::shared_array<byte> bytes
            = dias::read_byte_array (storage, header_page_id, length);
        byte * c_bytes = bytes.get ();

        dias::unserialize (root_node_id, &c_bytes);
        dias::unserialize (page_size, &c_bytes);
        dias::unserialize (_height, &c_bytes);
        dias::unserialize (_nodes, &c_bytes);
        dias::unserialize (_data, &c_bytes);
    }

    template <typename V, typename K, typename _Compare>
    inline btree<V, K, _Compare>::~btree (void)
    {
        write_header ();
    }

    template <typename V, typename K, typename _Compare>
    inline void
    btree<V, K, _Compare>::write_header (void) const
    {
        std::vector<byte> serialized_header (page_size, 0);
        byte * ptr = &serialized_header[0];

        dias::serialize (root_node_id, &ptr);
        dias::serialize (page_size, &ptr);
        dias::serialize (_height, &ptr);
        dias::serialize (_nodes, &ptr);
        dias::serialize (_data, &ptr);

        SpatialIndex::id_type actual_page_id = header_page_id;
        storage.storeByteArray (actual_page_id, serialized_header.size (),
                                 &serialized_header[0]);
        assert (actual_page_id == header_page_id);
    }

    template <typename V, typename K, typename _Compare>
    inline unsigned btree<V, K, _Compare>::get_height (void) const
    {
        return _height;
    }

    template <typename V, typename K, typename _Compare>
    inline unsigned btree<V, K, _Compare>::get_reads (void) const
    {
        return _reads;
    }

    template <typename V, typename K, typename _Compare>
    inline unsigned btree<V, K, _Compare>::get_writes (void) const
    {
        return _writes;
    }

    template <typename V, typename K, typename _Compare>
    inline unsigned btree<V, K, _Compare>::get_nodes (void) const
    {
        return _nodes;
    }

    template <typename V, typename K, typename _Compare>
    inline unsigned btree<V, K, _Compare>::get_data (void) const
    {
        return _data;
    }

    template <typename V, typename K, typename _Compare>
    inline void
    btree<V, K, _Compare>::insert (const key new_key, const V new_value)
    {
        tree_path path_from_root;

        typename btree<V, K, _Compare>::key_page_id insertion_leaf
            = find_leaf (new_key, &path_from_root);

        insert_to_leaf (new_key, new_value, insertion_leaf.second,
                        path_from_root);

        _data++;
    }

    template <typename V, typename K, typename _Compare>
    inline typename btree<V, K, _Compare>::key_page_id
    btree<V, K, _Compare>::find_leaf (const key new_key,
                                      tree_path * path_from_root) const
    {
        // FIXME: numeric_limits means that we cannot use e.g. string keys
        key_page_id result = std::make_pair (std::numeric_limits<key>::max(),
                                             root_node_id);

        if (_height == 0)
            return result;

        for (unsigned height = _height; height > 1; height--)
        {
            boost::shared_ptr<internal_node> n
                = read_internal_node (result.second);
            if (path_from_root)
                path_from_root->push_back (std::make_pair (result.second, n));
            result = n->choose_child_internal_node (new_key, result.first);
        }

        boost::shared_ptr<internal_node> n
            = read_internal_node (result.second);
        if (path_from_root)
            path_from_root->push_back (std::make_pair (result.second, n));

        result = n->choose_child_leaf (new_key, result.first);
        return result;
    }

    template <typename V, typename K, typename _Compare>
    inline void
    btree<V, K, _Compare>::insert_to_leaf (
        const key new_key, const V new_value,
        SpatialIndex::id_type leaf_id, tree_path & path_from_root)
    {
        SpatialIndex::id_type root_path_node_id = leaf_id;
        boost::shared_ptr<leaf> leaf_node = read_leaf (root_path_node_id);

        if (!leaf_node->full())
        {
            leaf_node->add_entry (new_key, new_value);
            update (*leaf_node, leaf_id);
            return;
        }

        typename leaf::entry_iterator split_point = leaf_node->split_point ();
        key split_key = split_point->first;

        std::auto_ptr<leaf> new_sibling = make_leaf (split_point,
                                                     leaf_node->end (),
                                                     leaf_node->get_next ());
        _nodes++;
        leaf_node->remove_entries_from (split_point);

        _Compare comparator;
        if (comparator (new_key, split_key))
        {
            leaf_node->add_entry (new_key, new_value);
        }
        else
        {
            new_sibling->add_entry (new_key, new_value);
        }


        SpatialIndex::id_type new_sibling_id = write (*new_sibling);
        leaf_node->set_next (new_sibling_id);
        update (*leaf_node, root_path_node_id);

        while (path_from_root.size () > 0)
        {
            id_node_pair parent_info = path_from_root.back ();
            path_from_root.pop_back ();
            root_path_node_id = parent_info.first;
            boost::shared_ptr<internal_node> parent = parent_info.second;

            if (!parent->full ())
            {
                if (comparator (split_key, parent->entries.front().first))
                {
                    parent->add_entry (split_key, new_sibling_id);
                    path_from_root.clear ();
                }
                else
                {
                    parent->add_entry (split_key, new_sibling_id);
                    path_from_root.clear ();
                }

                update (*parent, root_path_node_id);
                return;
            }

            typename internal_node::entry_iterator split_point
                = parent->split_point ();
            key previous_split_key = split_key;
            split_key = split_point->first;
            SpatialIndex::id_type new_greatest_child = split_point->second;
            typename internal_node::entry_iterator to_move = split_point + 1;
            std::auto_ptr<internal_node> new_sibling
                = make_internal_node (*parent, to_move);
            _nodes++;
            parent->remove_entries_from (split_point, new_greatest_child);

            if (comparator (previous_split_key, split_key))
            {
                assert (!parent->full ());
                parent->add_entry (previous_split_key, new_sibling_id);
            }
            else
            {
                assert (!new_sibling->full ());
                new_sibling->add_entry (previous_split_key, new_sibling_id);
            }

            new_sibling_id = write (*new_sibling);
            update (*parent, root_path_node_id);
        }

        make_new_root (split_key, root_path_node_id, new_sibling_id);
     }

    template <typename V, typename K, typename _Compare>
    inline void btree<V, K, _Compare>::make_new_root (
        const key split_key,
        SpatialIndex::id_type lesser_child,
        SpatialIndex::id_type greater_child)
    {
        std::auto_ptr<internal_node> new_root
            = make_internal_node (split_key, lesser_child, greater_child);
        root_node_id = write (*new_root);
        _nodes++;
        _height++;
    }

    template <typename V, typename K, typename _Compare>
    inline typename btree<V, K, _Compare>::query_answer
    btree<V, K, _Compare>::range_query (const key_range & range) const
    {
        key_page_id leaf_info = (_height == 0)
            ? std::make_pair(std::numeric_limits<key>::max(), root_node_id)
            : find_leaf (range.first, NULL);
        return range_query_leaf (leaf_info, range);
    }

    template <typename V, typename K, typename _Compare>
    inline typename btree<V, K, _Compare>::query_answer
    btree<V, K, _Compare>::range_query_leaf (const key_page_id & leaf_info,
                                             const key_range & range) const
    {
        typename btree<V, K, _Compare>::query_answer result;
        boost::shared_ptr<typename btree<V, K, _Compare>::leaf> node
            = read_leaf (leaf_info.second);
        node->query (*this, result, leaf_info.first, range);
        return result;
    }

    template <typename V, typename K, typename _Compare>
    inline std::auto_ptr<typename btree<V, K, _Compare>::internal_node>
    btree<V, K, _Compare>::make_internal_node (key split_key,
                                  SpatialIndex::id_type lesser_child,
                                  SpatialIndex::id_type greater_child) const
    {
        return std::auto_ptr<internal_node>
            (new internal_node (page_size, split_key,
                                lesser_child, greater_child));
    }

    template <typename V, typename K, typename _Compare>
    inline std::auto_ptr<typename btree<V, K, _Compare>::internal_node>
    btree<V, K, _Compare>::make_internal_node (
        const typename btree<V, K, _Compare>::internal_node & take_from,

        typename btree<V, K, _Compare>::internal_node::entry_iterator
        copy_start) const
    {
        return std::auto_ptr<internal_node> (new internal_node (take_from,
                                                                copy_start));
    }

    template <typename V, typename K, typename _Compare>
    inline std::auto_ptr<typename btree<V, K, _Compare>::leaf>
    btree<V, K, _Compare>::make_leaf (void) const
    {
        return std::auto_ptr<leaf> (new leaf (page_size));
    }

    template <typename V, typename K, typename _Compare>
    inline std::auto_ptr<typename btree<V, K, _Compare>::leaf>
    btree<V, K, _Compare>::make_leaf (
        typename leaf::entry_const_iterator begin,
        typename leaf::entry_const_iterator end,
        SpatialIndex::id_type next_page_id) const
    {
        return std::auto_ptr<leaf> (new leaf (page_size, begin, end,
                                              next_page_id));
    }

    template <typename V, typename K, typename _Compare>
    inline boost::shared_ptr<typename btree<V, K, _Compare>::internal_node>
    btree<V, K, _Compare>::read_internal_node (
        SpatialIndex::id_type page_id) const
    {
        boost::shared_ptr<internal_node> result
            = internal_node::read (storage, page_id, page_size);
        _reads++;
        return result;
    }

    template <typename V, typename K, typename _Compare>
    inline boost::shared_ptr<typename btree<V, K, _Compare>::leaf>
    btree<V, K, _Compare>::read_leaf (SpatialIndex::id_type page_id) const
    {
        boost::shared_ptr<leaf> result
            = leaf::read (storage, page_id, page_size);
        _reads++;
        return result;
    }

    template <typename V, typename K, typename _Compare>
    inline void
    btree<V, K, _Compare>::update (const node & n,
                                   SpatialIndex::id_type page_id)
    {
        n.update (storage, page_id);
        _writes++;
    }

    template <typename V, typename K, typename _Compare>
    inline SpatialIndex::id_type
    btree<V, K, _Compare>::write (const node & n)
    {
        SpatialIndex::id_type result = n.write (storage);
        assert (written_pages.find (result) == written_pages.end ());
        written_pages.insert (result);
        _writes++;
        return result;
    }

    template <typename V, typename K, typename _Compare>
    inline void btree<V, K, _Compare>::check (void) const
    {
        unsigned old_reads = _reads; // Strictly speaking, not exception safe.

        std::set<SpatialIndex::id_type> seen_page_ids;
        std::set<SpatialIndex::id_type> unseen_page_ids (written_pages);

        key lower_bound = std::numeric_limits<key>::min ();
        key upper_bound = std::numeric_limits<key>::max ();

        check_subtree (root_node_id, SpatialIndex::StorageManager::EmptyPage,
                       _height, lower_bound, upper_bound, seen_page_ids,
                       unseen_page_ids);

        key_page_id first_leaf = find_leaf (std::numeric_limits<key>::min (),
                                            NULL);
        boost::shared_ptr<leaf> leaf_node = read_leaf (first_leaf.second);
        leaf_node->check_leaf_chain (*this, first_leaf.second);

        if (unseen_page_ids.size () > 0)
        {
            std::string error
                = (boost::format ("Disconnected pages: %1% total")
                   % unseen_page_ids.size ()).str ();
            BOOST_THROW_EXCEPTION (std::logic_error (error));
        }

        _reads = old_reads;
    }

    template <typename V, typename K, typename _Compare>
    void btree<V, K, _Compare>::check_subtree (
        SpatialIndex::id_type subtree_root_id,
        SpatialIndex::id_type subtree_parent_id,
        unsigned subtree_height,
        key & lower_bound,
        key upper_bound,
        std::set<SpatialIndex::id_type> & seen_page_ids,
        std::set<SpatialIndex::id_type> & unseen_page_ids) const
    {
        size_t r = unseen_page_ids.erase (subtree_root_id);
        assert (r == 1);
        (void)r;

        if (seen_page_ids.find (subtree_root_id) != seen_page_ids.end ())
        {
            std::string error
                = (boost::format ("More than one entry pointing to "
                                  "page id %1% found")
                   % subtree_root_id).str();
            BOOST_THROW_EXCEPTION (std::logic_error (error));
        }

        seen_page_ids.insert (subtree_root_id);

        assert (subtree_root_id != SpatialIndex::StorageManager::EmptyPage);
        if (subtree_height == 0)
        {
            boost::shared_ptr<typename btree<V, K, _Compare>::leaf> leaf_node
                = read_leaf (subtree_root_id);
            leaf_node->check (lower_bound, upper_bound, subtree_root_id,
                              subtree_parent_id);
            lower_bound = upper_bound;
        }
        else
        {
            boost::shared_ptr<typename btree<V, K, _Compare>::internal_node> n
                = read_internal_node (subtree_root_id);
            n->check (*this, lower_bound, upper_bound, subtree_height,
                      subtree_root_id, seen_page_ids, unseen_page_ids);
        }
    }

    template <typename V, typename K, typename _Compare>
    inline void btree<V, K, _Compare>::dump (std::ostream & out) const
    {
        out << "B+ tree, height = " << _height << '\n';
        if (_height == 0)
        {
            boost::shared_ptr<leaf> root_node = read_leaf (root_node_id);
            root_node->dump (out);
            return;
        }
        boost::shared_ptr<internal_node> root_node
            = read_internal_node (root_node_id);
        root_node->dump (out, *this, root_node_id, _height);
    }
}

#endif
