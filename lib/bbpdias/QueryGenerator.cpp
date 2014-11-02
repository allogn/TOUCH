#ifndef QUERY_GENERATOR
#define QUERY_GENERATOR

#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "Query.cpp"

namespace dias {

class QueryGenerator
{
public:

	std::ofstream writeFile;
	std::ifstream readFile;
	std::vector<Query> queries;

	QueryGenerator()
	{
		queries.clear();
	}

	~QueryGenerator()
	{
		queries.clear();
	}

	void Generate(const Box &world,int queryCount,float VolumePercentage);
	void Load(const std::string& filename);
	void Save(const std::string& filename);
};


/*
 * Same Volume , Random Queries with equal number of density queries
 *
 */
void QueryGenerator::Generate(const Box &world,int queryCount,const float VolumePercentage)
{
	for (int i=0;i<queryCount;i++)
	{
		Query temp;
		Box::randomBox(world,VolumePercentage,temp.box);
		queries.push_back(temp);
	}
}

void Tokenize(const std::string& str,std::vector<std::string>& tokens,const std::string& delimiters = " ")
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

void QueryGenerator::Save(const std::string &filename)
{
	writeFile.open (filename.c_str(),std::ios::out |std::ios::trunc);

	for (std::vector<Query>::iterator i = queries.begin();i!=queries.end();++i)
            writeFile << i->box.low[0] << " " << i->box.low[1] << " " << i->box.low[2] << " " << i->box.high[0] << " " << i->box.high[1] << " " << i->box.high[2] << " " << std::endl;
	writeFile.close();
}

void QueryGenerator::Load(const std::string& filename)
{
	queries.clear();
	readFile.open(filename.c_str(),std::ios::in);

	if (readFile.is_open())
	{
		while (!readFile.eof())
		{
			std::vector<std::string> tokens;
			std::string line;
			getline (readFile,line);
			if (readFile.eof()) break;
			Tokenize(line, tokens);
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
}

}

#endif
