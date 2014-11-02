#include "DataFileReader.hpp"
#include "ExternalSort.hpp"
//#include  "STRIndex.hpp"
//#include "RtreeIndex.hpp"


using namespace std;
using namespace FLAT;

int main(int argc, const char* argv[])
{

	///////////////////////// TEST FOR RTREE /////////////////////////////////////////////
	string inputFile  = argv[1];
	string outPutStem = argv[2];

	RtreeIndex* myIndex = new RtreeIndex();
	DataFileReader* input = new DataFileReader(inputFile);
	input->loadHeader();

	myIndex->buildIndex(input,outPutStem);

	delete input;
	delete myIndex;
/*
	///////////////////////// TEST STR INDEX AND TESSELLATION ////////////////////////////

	string inputFile  = argv[1];
	string outPutStem = argv[2];

	STRIndex* myIndex = new STRIndex();
	DataFileReader* input = new DataFileReader(inputFile);
	input->loadHeader();

	myIndex->buildIndex(input,outPutStem);
	//myIndex->loadIndex("testIndex");

	delete input;
	delete myIndex;
*/

	/*
	////////////////////// TEST INPUT READER and EXTERNAL SORTER /////////////////////////
	string inputFile  = argv[1];
	DataFileReader* inputData = new DataFileReader(inputFile);
	inputData->loadHeader();

	ExternalSort* externalSort = new ExternalSort(250,1,inputData->objectType);
	while (inputData->hasNext())
    {
		SpatialObject* sobj = inputData->getNext();
		if (sobj->getType()==VERTEX)
		cout << *((Vertex*)sobj) << endl;
		else if (sobj->getType()==CONE)
			cout << *((Cone*)sobj) << endl;
		else
			cout << *((FLAT::Box*)sobj) << endl;
		externalSort->insert(sobj);
    }

	externalSort->sort();
	externalSort->rewind();
 cout << "\n sorTed\n\n";
	while (externalSort->hasNext())
    {
		SpatialObject* sobj =  externalSort->getNext();
		if (sobj->getType()==VERTEX)
		cout << *((Vertex*)sobj) << endl;
		else if (sobj->getType()==CONE)
			cout << *((Cone*)sobj) << endl;
		else
			cout << *((FLAT::Box*)sobj) << endl;
    }

	externalSort->clean();

	delete externalSort;
	delete inputData;
*/
}
