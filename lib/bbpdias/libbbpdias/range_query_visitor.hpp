#ifndef RANGE_QUERY_VISITOR_H_
#define RANGE_QUERY_VISITOR_H_
#include <iostream>
#include <SpatialIndex.h>

class range_query_visitor : public SpatialIndex::IVisitor
{
private:
    unsigned node_accesses;
    unsigned data_accesses;
    unsigned number_of_queries;
    unsigned neurons;

public:
    range_query_visitor() :
	node_accesses(1),
	data_accesses(0),
	number_of_queries(1),
	neurons(1)
	{
	}

    virtual void
    visitNode (const SpatialIndex::INode& in __attribute__((unused)))
        {
            node_accesses++;
        }

    virtual void visitData (const SpatialIndex::IData& in)
	{
	    data_accesses++;

	    byte *b;
	    size_t l;
	    in.getData(l,&b);

	    std::cout << "Data Found for Neuron: a" << neurons << " Section-Segment (" << (char *)b <<")\n" ;
	}

    virtual void
    visitData (std::vector<const SpatialIndex::IData *>& v
               __attribute__((unused)))
	{
	    abort(); // FIXME
	}

    void new_query()
	{
	    number_of_queries++;
	}

    void inc_neuron()
    {
    	neurons++;
    }

    void reset_stats()
    {
    	node_accesses=1;
    	data_accesses=0;
    	neurons=1;
    }

    double get_avg_io() const
	{
	    return (static_cast<double>(node_accesses) / number_of_queries);
	}

    void print_stats() const
	{
	    std::cout << "Total queries: " << number_of_queries << "\n";
	    std::cout << "Total node accesses: " << node_accesses << "\n";
	    std::cout << "Total data accesses: " << data_accesses << "\n";
	}
};

#endif
