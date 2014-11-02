#ifndef VERTEX_INFO_HPP_
#define VERTEX_INFO_HPP_

namespace dias {


    class Vertex_info
    {
    public:

        vertex_info vi;
        vertex_id  vid;

        Vertex_info()
            {
            }

        Vertex_info (const Vertex_info & other) :
            vi (other.vi),
            vid (other.vid)
            {
            }

    	struct xAsc : public std::binary_function<Vertex_info* const, Vertex_info* const, bool>
    	{
    		bool operator()(Vertex_info* const r1, Vertex_info* const r2)
    		{
    			if (r1->vi.coords[0] < r2->vi.coords[0]) return true;
    			else return false;
    		}
    	};

    	struct yAsc : public std::binary_function<Vertex_info* const, Vertex_info* const, bool>
    	{
    		bool operator()(Vertex_info* const r1, Vertex_info* const r2)
    		{
    			if (r1->vi.coords[1] < r2->vi.coords[1]) return true;
    			else return false;
    		}
    	};

    	struct zAsc : public std::binary_function<Vertex_info* const, Vertex_info* const, bool>
    	{
    		bool operator()(Vertex_info* const r1, Vertex_info* const r2)
    		{
    			if (r1->vi.coords[2] < r2->vi.coords[2]) return true;
    			else return false;
    		}
    	};

        void store(Farhan::TemporaryFile &tf)
            {
                std::vector<byte> buf;
                vi.serialize (vid, buf);

                tf.write(buf.size(),&buf[0]);
            }

        bool load(Farhan::TemporaryFile &tf)
            {
                try
                {
                    vertex_info::size_type length;
                    length = tf.readUInt32 ();
                    vid = tf.readUInt32();
#ifdef VERBOSE
                    std::cerr << ">> sizeof (length) = " << sizeof (length)
                              << " sizeof (vid) = " << sizeof (vid) << '\n';
                    std::cerr << "Vertex length = " << length << " id = " << vid
                              << '\n';
#endif
                    length -= (sizeof(length) + sizeof (vid));
                    byte * buf;
                    tf.readBytes(length,&buf);
                    boost::scoped_array<byte> buf_ptr (buf);

                    vi = vertex_info::make_from_buffer (&buf, (ptrdiff_t)length);

                    return true;
                }
                catch(...)
                {
                    return false;
                }
            }
    };

}

#endif
