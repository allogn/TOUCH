#ifndef DATA_SEGMENT_H_
#define DATA_SEGMENT_H_

#include <cstring>

#include <boost/scoped_array.hpp>

#include <SpatialIndex.h>

/// A BBP Segment encapsulated for spatialindex R-Tree
class data_segment : public SpatialIndex::IData
{
private:
    SpatialIndex::id_type id;
    SpatialIndex::Region mbr;
    size_t data_len;
    boost::scoped_array<byte> data;

public:
    data_segment (SpatialIndex::id_type _id, const SpatialIndex::Region & _mbr,
		  size_t _data_len,  const void * _data) :
	id(_id), mbr(_mbr), data_len(_data_len), data(new byte[_data_len])
	{
	    memcpy (data.get(), _data, data_len);
	}

    virtual IObject* clone()
	{
	    abort(); // FIXME
	}

    virtual SpatialIndex::id_type getIdentifier() const
	{
	    return id;
	}

    virtual void getShape (SpatialIndex::IShape ** shape) const
	{
	    *shape = new SpatialIndex::Region (mbr);
	}

    virtual void getData (size_t & s, byte ** data) const
	{
	    s = data_len;
	    *data = new byte[data_len];
	    memcpy (data, this->data.get(), data_len);
	}
};

#endif
