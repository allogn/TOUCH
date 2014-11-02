#include "DataFileReader.hpp"
//#include "PayLoad.hpp"
#include "Box.hpp"
#include "Cone.hpp"

using namespace FLAT;
using namespace std;

class CrawlStatistics
{
public:
	int SeedIOs;
	int MetaDataIOs;
	int PayLoadIOs;
	int MetaDataEntries;
	int ConsideredPoints; // Used in BFS maybe Result or maybe Rejected
	int RejectedPoints;   // Used in BFS but not inside query
	int ResultPoints;     // Used in BFS and inside Query
	int UseLessPoints;    // Never used in BFS but were in page
	int UselessPage;
	int ResultPage;
	int MixedPage;

	CrawlStatistics()
	{
		SeedIOs=0;
		PayLoadIOs=0;
		MetaDataIOs=0;
		MetaDataEntries=0;

		UselessPage=0;
		ResultPage=0;
		MixedPage=0;

		ConsideredPoints=0;
		UseLessPoints=0;
		ResultPoints=0;
		RejectedPoints=0;
	}

	void print()
	{
            std::cout  << ResultPoints << "\t" <<  UseLessPoints << std::endl;
	}

	void add(CrawlStatistics* cs)
	{
		this->SeedIOs+=cs->SeedIOs;
		this->MetaDataIOs+=cs->MetaDataIOs;
		this->PayLoadIOs+=cs->PayLoadIOs;
		this->MetaDataEntries+=cs->MetaDataEntries;
		this->ConsideredPoints+=cs->ConsideredPoints;
		this->RejectedPoints+=cs->RejectedPoints;
		this->ResultPoints+=cs->ResultPoints;
		this->UseLessPoints+=cs->UseLessPoints;
		this->UselessPage+=cs->UselessPage;
		this->ResultPage+=cs->ResultPage;
		this->MixedPage+=cs->MixedPage;
	}
};

class Query
{
public:
	Box box;
	int seedID;

	Query()
	{
		seedID = -1;
	}
};

void tokenize(const std::string& str,std::vector<std::string>& tokens,const std::string& delimiters = " ")
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

vector<Query> loadQueries(const std::string& filename)
{
	vector<Query> queries;
	std::ifstream readFile;
	readFile.open(filename.c_str(),std::ios::in);

	if (readFile.is_open())
	{
		while (!readFile.eof())
		{
			std::vector<std::string> tokens;
			std::string line;
			getline (readFile,line);
			if (readFile.eof()) break;
			tokenize(line, tokens);
			Query temp;
			temp.box.low[0]   = atof(tokens.at(0).c_str());
			temp.box.low[1]   = atof(tokens.at(1).c_str());
			temp.box.low[2]   = atof(tokens.at(2).c_str());
			temp.box.high[0]  = atof(tokens.at(3).c_str());
			temp.box.high[1]  = atof(tokens.at(4).c_str());
			temp.box.high[2]  = atof(tokens.at(5).c_str());
			queries.push_back(temp);
		}
		readFile.close();
	}

	return queries;
}


int main(int argc, const char* argv[])
{
	string dataFile  = argv[1];
	string queryFile  = argv[2];
    DataFileReader* input = new DataFileReader(dataFile);
    input->loadHeader();

    //std::cout  << "\n\nResults" << "\t" <<  "UseLess" << std::endl;

	CrawlStatistics* totalStats = new CrawlStatistics();
	vector<Query> queries = loadQueries(queryFile);
	for (vector<Query>::iterator query = queries.begin(); query != queries.end();query++)
	{
		CrawlStatistics* cs = new CrawlStatistics();
		while (input->hasNext())
		{
			SpatialObject* object = input->getNext();
			if (Box::overlap(query->box, object->getMBR()))
				cs->ResultPoints++;
			else
				cs->UseLessPoints++;
			delete object;
		}
		input->rewind();
		cs->print();
		totalStats->add(cs);
		delete cs;
	}
	cout <<"\n";
	totalStats->print();
	delete totalStats;
	delete input;
}
