#include <vector>
#include <iomanip>
#include <unistd.h>
#include "Rtree.cpp"
#include "QueryGenerator.cpp"
#include "cpu_timer.cpp"

#include <boost/lexical_cast.hpp>


using namespace std;

int main(int argc, const char* argv[])
{
    //string mode = argv[4];
    //ofstream writeFile;
    //writeFile.open (argv[3],ios::out |ios::trunc);

    unsigned cache_size = 0;
    if (argc == 6)
    {
        cache_size = boost::lexical_cast<unsigned>(argv[5]);
        std::cout << "Using a cache sized " << cache_size << '\n';
    }

    /*
    if (mode.compare("seed")==0)
    {
        writeFile << "Seeding Evaluation" << endl;
        dias::Rtree* myTree = new dias::Rtree(argv[1], false, cache_size);
        writeFile << "Rtree Loaded: " << argv[1] << endl;
        dias::QueryGenerator qg;
        qg.Load(argv[2]);
        writeFile << "Queries Loaded: " << argv[2] << " (" << qg.queries.size() << ") " << endl;
        writeFile << "\n\n" << "Results,I/Os,PointsRetreived,Sec\n\n";
        int count=1;
        int totalRetrieved=0;
    	tpie::cpu_timer c;
    	c.start();

        for (vector<dias::Query>::iterator it = qg.queries.begin ();
             it != qg.queries.end ();
             ++it)
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
                        totalIosAfterCache = 0;
        		c.start();
        	}
        	else
        	{
        		totalPoints += it->points;
        		totalIos+=it->nodeAccesses;
        		totalRetrieved+=it->dataInNode;
                        totalIosAfterCache = myTree->get_reads () + myTree->get_writes ();
        	}
        }

        delete myTree;
    }
*/
  //  else
  //  {
        //writeFile << "RangeQuery Evaluation" << endl;
        dias::Rtree* myTree = new dias::Rtree(argv[1], false, cache_size);
        cout << "Rtree Loaded: " << argv[1] << endl;
        dias::QueryGenerator qg;
        qg.Load(argv[2]);
        cout << "Queries Loaded: " << argv[2] << " (" << qg.queries.size() << ") " << endl;
        //cout << "\n\n" << "I/Os,Results,Cache I/Os,Sec\n\n";
        int count=1;

        int totalPoints=0;
        int totalIos=0;
        unsigned totalIosAfterCache = 0;
        tpie::cpu_timer c;
        c.start();
        for (vector<dias::Query>::iterator it = qg.queries.begin ();
             it != qg.queries.end (); ++it, count++)
        {
            myTree->RangeSearch(*it);
            totalPoints += it->points;
            totalIos+=it->nodeAccesses;
        }
        c.stop();
        totalIosAfterCache = myTree->get_reads () + myTree->get_writes ();

        delete myTree;
//    }

    std::cout << "Total objects = " << totalPoints << '\n';
    std::cout << "Total I/Os = " << totalIos << '\n';
    std::cout << "Total I/Os after cache = " << totalIosAfterCache << '\n';
    std::cout << "Total time = " << c << '\n';

//    writeFile.close();
    return 0;
}
