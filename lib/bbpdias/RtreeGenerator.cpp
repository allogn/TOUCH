#include <cstring>
#include <vector>
#include <iomanip>
#include <SpatialIndex.h>

#include "libbbpdias/spatialindex_tools.hpp"

using namespace std;

void Tokenize(const string& str,vector<string>& tokens,const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

class data_stream : public SpatialIndex::IDataStream
{

public:
	//Farhan::TemporaryFile* tf;
	int id;
	//bool havenext;
	ifstream readFile;
	string line;


	data_stream (string filename)
	{
		id=0;
		//havenext=true;
		readFile.open(filename.c_str(),ios::in);
		getline (readFile,line);
		getline (readFile,line);
		getline (readFile,line);
		//tf = new Farhan::TemporaryFile(filename,false);
		//tf->rewindForReading();
	}

	virtual ~data_stream()
	{
		readFile.close();
	}

	virtual bool hasNext()
	{
		if (readFile.is_open()==false) return false;
		getline (readFile,line);
		return (readFile.eof()==false);
	}

	virtual size_t size()
	{
		abort(); // FIXME
	}

	virtual void rewind()
	{
		abort(); // FIXME
	}

	virtual SpatialIndex::IData* getNext()
	{
		vector<string> tokens;
		Tokenize(line, tokens);

		double lo[3],hi[3];
		lo[0] = atof(tokens.at(0).c_str());
		lo[1] = atof(tokens.at(1).c_str());
		lo[2] = atof(tokens.at(2).c_str());

		//hi[0] = atof(tokens.at(3).c_str());
		//hi[1] = atof(tokens.at(4).c_str());
		//hi[2] = atof(tokens.at(5).c_str());

		SpatialIndex::Region mbr = SpatialIndex::Region(lo,lo,3);
		SpatialIndex::RTree::Data* ret = new SpatialIndex::RTree::Data(0, 0, mbr, id++);
		return ret;
	}


};

int main(int argc, const char* argv[])
{
    string inputFile = argv[1];
    string outputFile = argv[2];
    string mode = argv[3];

    SpatialIndex::id_type indexIdentifier=0;
    data_stream* ds = new data_stream(inputFile);
    SpatialIndex::IStorageManager* diskfile
        = SpatialIndex::StorageManager::createNewDiskStorageManager(
            outputFile, dias::default_page_size);

    if (mode.compare("str")==0)
    {
        std::cout <<"STR BULK LOAD\n";
        try
        {
            SpatialIndex::ISpatialIndex * tree
                = SpatialIndex::RTree::createAndBulkLoadNewRTree (
                    SpatialIndex::RTree::BLM_STR,
                    *ds,
                    *diskfile,
                    dias::fill_factor, dias::index_node_capacity,
                    dias::leaf_node_capacity, 3,
                    SpatialIndex::RTree::RV_RSTAR,
                    indexIdentifier);
            delete tree;
            delete diskfile;
        }
        catch(...)
        {
            std::cout << ds->id;
        }

    }
    else
    {
        SpatialIndex::ISpatialIndex * tree
            = SpatialIndex::RTree::createNewRTree (
                *diskfile, dias::fill_factor, dias::index_node_capacity,
                dias::leaf_node_capacity, 3, SpatialIndex::RTree::RV_RSTAR,
                indexIdentifier);

        SpatialIndex::Region mbr;
        int id=0;
        int counter=0;
        while (ds->hasNext())
        {
            try
            {
                SpatialIndex::IData* data = ds->getNext();
                SpatialIndex::IShape* mb;
                data->getShape(&mb);
                mb->getMBR(mbr);
                id = data->getIdentifier();
                tree->insertData(0,NULL,mbr,id);
                delete mb;
                delete data;
            }
            catch(...)
            {
                std::cerr << "\nid:" <<id << " counter:" << counter;
                std::cerr << "\n" << mbr.m_pLow[0]<< "," << mbr.m_pLow[1]
                          << "," << mbr.m_pLow[2] <<"   "<< mbr.m_pHigh[0]
                          << "," << mbr.m_pHigh[1] << "," << mbr.m_pHigh[2];
            }
        }
        delete tree;
        delete diskfile;
    }

    return 0;
}
