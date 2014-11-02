#ifndef ACCESS_COUNT_VISITOR_H_
#define ACCESS_COUNT_VISITOR_H_

#include <iostream>

#include <SpatialIndex.h>

/// Callback for R-tree range query that gathers node access stats
class access_count_visitor : public SpatialIndex::IVisitor
{
private:
    unsigned node_accesses;
    unsigned data_accesses;

    unsigned number_of_queries;

public:
    access_count_visitor() :
	node_accesses(0),
	data_accesses(0),
	number_of_queries(0)
	{
	}

    virtual void
    visitNode (const SpatialIndex::INode& in __attribute__((unused)))
        {
            node_accesses++;
        }

    virtual void
    visitData (const SpatialIndex::IData& in __attribute__((unused)))
        {
            data_accesses++;
        }

    virtual void
    visitData (std::vector<const SpatialIndex::IData *> & v
               __attribute__((unused)))
        {
            abort (); // TODO
        }

    void new_query()
	{
	    number_of_queries++;
	}
    void reset_stats()
    {
    	node_accesses=0;
    	data_accesses=0;
    	number_of_queries=0;
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
	    std::cout << "Node acceses per query: " << get_avg_io() << "\n";
	}
};

#endif
