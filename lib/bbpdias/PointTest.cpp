#include <vector>
#include <iomanip>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include "Rtree.cpp"
#include "cpu_timer.cpp"

#define QUERIES 500
static boost::mt11213b generator (42u);
using namespace std;

class pointVisitor : public SpatialIndex::IVisitor
{
public:
	int nodeAccesses;
	int leaves;

    pointVisitor()
		{
    	nodeAccesses=0;
    	leaves=0;
		}

    virtual void visitNode (const SpatialIndex::INode& in)
		{
    	nodeAccesses++;
		}

    virtual void visitData (const SpatialIndex::IData& in)
		{
    	leaves++;
		}

    virtual void visitData (std::vector<const SpatialIndex::IData *>& v
                            __attribute__((__unused__)))
		{
		}
};

int main(int argc, const char* argv[])
{
	dias::Rtree* myTree = new dias::Rtree(argv[1],false);
	pointVisitor visitor;
	double neuron_low[3] = {98.2538, 1005.14, 92.9046};
	double neuron_high[3] = {452.301, 1385.56, 456.743};

	for (int i=0;i<QUERIES;i++)
	{
		double p[3];
		{
		boost::uniform_real<> uni_dist (neuron_low[0],neuron_high[0]);
		boost::variate_generator<boost::mt11213b &,boost::uniform_real<> > uni(generator, uni_dist);
		p[0] = uni();
		}
		{
		boost::uniform_real<> uni_dist (neuron_low[1],neuron_high[1]);
		boost::variate_generator<boost::mt11213b &,boost::uniform_real<> > uni(generator, uni_dist);
		p[1] = uni();
		}
		{
		boost::uniform_real<> uni_dist (neuron_low[2],neuron_high[2]);
		boost::variate_generator<boost::mt11213b &,boost::uniform_real<> > uni(generator, uni_dist);
		p[2] = uni();
		}
		//std::cout<< p[0] <<"," << p[1] << "," << p[2] << endl;

		SpatialIndex::Point point(p,3);

		myTree->tree->pointLocationQuery(point,visitor);
	}

	std::cout << "Total I/Os"<< visitor.nodeAccesses << endl;

	delete myTree;

	/*

	ofstream writeFile;
    writeFile.open (argv[3],ios::out |ios::trunc);




    if (mode.compare("seed")==0)
    {
        writeFile << "Seeding Evaluation" << endl;
        Rtree* myTree = new Rtree(argv[1],false);
        writeFile << "Rtree Loaded: " << argv[1] << endl;
        QueryGenerator qg;
        qg.Load(argv[2]);
        writeFile << "Queries Loaded: " << argv[2] << " (" << qg.queries.size() << ") " << endl;
        writeFile << "\n\n" << "Results,I/Os,PointsRetreived,Sec\n\n";
        int count=1;
        int totalPoints=0;
        int totalIos=0;
        int totalRetrieved=0;
    	tpie::cpu_timer c;
    	c.start();

        for (vector<Query>::iterator it = qg.queries.begin(); it!=qg.queries.end();++it)
        {
        	myTree->Seed(*it);
        	if (count%QUERIES==0)
        	{
        		c.stop();
        		totalPoints += it->points;
        		totalIos+=it->nodeAccesses;
        		totalRetrieved=0;

        		writeFile << totalPoints << "," << totalIos << "," << totalRetrieved << "," << c << endl;

        		c.reset();
        		totalPoints=0;
        		totalIos=0;
        		totalRetrieved=0;
        		c.start();
        	}
        	else
        	{
        		totalPoints += it->points;
        		totalIos+=it->nodeAccesses;
        		totalRetrieved+=it->dataInNode;
        	}
        }

        delete myTree;
    }

    else
    {
        writeFile << "RangeQuery Evaluation" << endl;
        Rtree* myTree = new Rtree(argv[1],false);
        writeFile << "Rtree Loaded: " << argv[1] << endl;
        QueryGenerator qg;
        qg.Load(argv[2]);
        writeFile << "Queries Loaded: " << argv[2] << " (" << qg.queries.size() << ") " << endl;
        writeFile << "\n\n" << "Results,I/Os,PointsRetreived,Sec\n\n";
        int count=1;
        int totalPoints=0;
        int totalIos=0;
        int totalRetrieved=0;
    	tpie::cpu_timer c;
    	c.start();

        for (vector<Query>::iterator it = qg.queries.begin(); it!=qg.queries.end();++it,count++)
        {
        	myTree->RangeSearch(*it);
        	if (count%QUERIES==0)
        	{
        		c.stop();
        		totalPoints += it->points;
        		totalIos+=it->nodeAccesses;
        		totalRetrieved=0;

        		writeFile << totalPoints << "," << totalIos << "," << totalRetrieved << "," << c << endl;

        		c.reset();
        		totalPoints=0;
        		totalIos=0;
        		totalRetrieved=0;
        		c.start();
        	}
        	else
        	{
        		totalPoints += it->points;
        		totalIos+=it->nodeAccesses;
        		totalRetrieved+=it->dataInNode;
        	}
        }


    }

    writeFile.close();
    */
   return 0;
}
