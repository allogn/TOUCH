#ifndef QUERY
#define QUERY


#include <vector>
#include "libbbpdias/Box.hpp"
#include "Data.cpp"
#include "StopWatch.cpp"

#define LOW_MEMORY true

namespace dias {
/*
 * Query structure for Spatial Indexing
 * Its Random Creation
 * Result Validation for Accuracy
 * and Performance Statistics
 *
 * Author: Farhan Tauheed
 */
    class Query
    {
    public:
        //Query
        Box box;

        // Results
        std::vector<Data> bruteForce;   // List of Data in Query by Brute Force
        std::vector<Data> algorithm;    // List of Data in Query by Algorithm

        // Data Statistics
        bigSpaceUnit volume;
        bigSpaceUnit density;
        int points;
        int dataInNode;
        StopWatch sw;
        Data seed;

        // Algorithm Statistics
        int nodeAccesses;
        int containerAccesses;
        int leafAccesses;
        int rangeNodeAccesses;
        int rangeLeafAccesses;

    Query() :
        volume (0),
        density (0),
        points (0),
        dataInNode (0),
        nodeAccesses (0),
        containerAccesses (0),
        leafAccesses (0),
        rangeNodeAccesses (0),
        rangeLeafAccesses (0)
            {
            }

        Query (const Box & b) :
            box (b),
            volume (0),
            density (0),
            points (0),
            dataInNode (0),
            nodeAccesses (0),
            containerAccesses (0),
            leafAccesses (0),
            rangeNodeAccesses (0),
            rangeLeafAccesses (0)
            {
            }

	void VisitLeaf(const Data& data);
	void VisitNode(int dataCount);
	void VisitRangeLeaf(const Data& data);
	void VisitRangeNode();
	void VisitValidateLeaf(const Data& data);

	int  ValidateSeeding();
	void ValidateSeedingDetail();
	void ProcessQuery();
	std::vector<Data> getQueryResult();

	friend std::ostream & operator << (std::ostream & lhs, const Query & rhs);
	static void randomQuery(const Box& world,const int percentageVolume,Query& random);
};

void Query::VisitValidateLeaf(const Data& data)
	{

   	if (LOW_MEMORY==false)
  			algorithm.push_back(data);

	}

void Query::VisitRangeLeaf(const Data& data)
	{
	rangeLeafAccesses++;

	if (LOW_MEMORY==false)
			algorithm.push_back(data);
	}

void Query::VisitRangeNode()
	{
	rangeNodeAccesses++;
	}

void Query::VisitLeaf(const Data& data)
	{
	points++;
	algorithm.push_back(data);

	leafAccesses++;
	seed = data;

	}



void Query::VisitNode(int dataCount)
	{
	nodeAccesses++;
	dataInNode += dataCount;
	}

void Query::ValidateSeedingDetail()
{
	if (algorithm.empty()==true && bruteForce.empty()==false)
		{
                    std::cout <<"\n" << *this <<"\n Error: Algorithm List Empty.. BruteForce List has items:" <<bruteForce.size() << "\n";
		int c=0;
                for (std::vector<Data>::const_iterator i = bruteForce.begin(); i != bruteForce.end(); ++i)
		 {
                     std::cout << "\t" << *i << "\n";
                     c++;
                     if (c==40)break;
		 }
	    return;
		}

	if (algorithm.empty()==false && bruteForce.empty()==true)
		{
                    std::cout <<"\n" << *this <<"\n Error: Algorithm List has item.. BruteForce List is Empty\n";
                    for (std::vector<Data>::const_iterator i = algorithm.begin(); i != algorithm.end(); ++i)
	    	std::cout << "\t" << *i << "\n";
	    return;
		}

	if (algorithm.empty()==true && bruteForce.empty()==true)
		{
		std::cout <<"\n"<< *this << "\n Correct: both Algorithm List and BruteForce List are Empty\n";
		return;
		}

	for (std::vector<Data>::const_iterator i = bruteForce.begin(); i != bruteForce.end(); ++i)
		for (std::vector<Data>::const_iterator j = algorithm.begin(); j != algorithm.end(); ++j)
			if (*i==*j)
			{
				std::cout <<"\n"<< *this << "\n Correct: Algorithm Result Found in BruteForce List.. Size of Results=" << algorithm.size() << " Data:" << *i << "\n" ;
				return;
			}

	std::cout <<"\n"<< *this <<" Error: Algorithm Results not Found in BruteForce List\n ALGORITHM LIST: \n";
    for (std::vector<Data>::const_iterator i = algorithm.begin(); i != algorithm.end(); ++i)
    	std::cout << "\t" << *i << "\n";

	std::cout << " BRUTE FORCE Items: " <<bruteForce.size() << "\n";
	int c=0;
	for (std::vector<Data>::const_iterator i = bruteForce.begin(); i != bruteForce.end(); ++i)
	{
		    	std::cout << "\t" << *i << "\n";
		    	c++;
		    	if (c==40)break;
	 }
}

int Query::ValidateSeeding() //Check accuracy for Seeding not for Range Query
	{
	if (algorithm.empty()==true && bruteForce.empty()==false)
		{
		int c=0;
		for (std::vector<Data>::const_iterator i = bruteForce.begin(); i != bruteForce.end(); ++i)
			{
			c++;
			if (c==40)break;
			}
	    return 0;
		}

	if (algorithm.empty()==false && bruteForce.empty()==true)
		return 0;

	if (algorithm.empty()==true && bruteForce.empty()==true)
		return 1;

	for (std::vector<Data>::const_iterator i = bruteForce.begin(); i != bruteForce.end(); ++i)
		for (std::vector<Data>::const_iterator j = algorithm.begin(); j != algorithm.end(); ++j)
			if (*i==*j)
				return 1;

	return 0;
	}

std::vector<Data> Query::getQueryResult() {
	return algorithm;
}

void Query::ProcessQuery()
	{
	this->volume = Box::volume(box);
	if (volume>0)
		density = points/volume;
	}

std::ostream & operator << (std::ostream & lhs,const Query & rhs)
	{
	lhs << rhs.box.low[0] << "," << rhs.box.low[1] << "," << rhs.box.low[2] << "," << rhs.box.high[0] << "," << rhs.box.high[1] << "," << rhs.box.high[2];
	lhs << "," << rhs.density << "," << rhs.points << "," << rhs.volume;
	return lhs;
	}

std::istream & operator >> (std::istream & lhs __attribute__((__unused__)),
                            Query & rhs __attribute__((unused)))
	{
	//lhs >> "(" >> rhs.box.low[0] >> "," >> rhs.box.low[1] >> "," >> rhs.box.low[2] >> ")(" >> rhs.box.high[0] >> "," >> rhs.box.high[1] >> "," >> rhs.box.high[2] >> ")";
//	lhs >> "  " >> rhs.density >> "  " >> rhs.points >> "  " >> rhs.volume;
	return lhs;
	}

}
#endif
