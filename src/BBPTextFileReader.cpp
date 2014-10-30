#include "BBPTextFileReader.hpp"
#include "SpatialObjectFactory.hpp"

using namespace std;

namespace FLAT
{
BBPTextFileReader::BBPTextFileReader(string& fileName,string& slicename)
	{
		try
		{
			inputFile.open(fileName.c_str(),std::ios::in);
			counter=0;
			loadHeader(slicename);
		}
		catch(...)
		{
#ifdef FATAL
			cout << "Cannot load InputFile" << endl;
#endif
			exit(0);
		}
	}

BBPTextFileReader::~BBPTextFileReader()
	{
		inputFile.close();
	}

void tokenize1(const std::string& str,std::vector<std::string>& tokens,const std::string& delimiters = " ")
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

	void BBPTextFileReader::loadHeader(string& filename )
	{
		objectType      = FLAT::VERTEX;
		objectByteSize  = SpatialObjectFactory::getSize(objectType);         // Size in Bytes of each Object

		try
				{
					std::ifstream readFile;
					readFile.open(filename.c_str(),std::ios::in);
					std::string line1; 	getline (readFile,line1);
					std::string line2; 	getline (readFile,line2);
					std::string line3; 	getline (readFile,line3);
					std::string line4; 	getline (readFile,line4);
					std::string line5; 	getline (readFile,line5);
					std::string line6; 	getline (readFile,line6);
					std::string line7; 	getline (readFile,line7);
					std::string line8; 	getline (readFile,line8);
					std::string line9; 	getline (readFile,line9);
					std::string line10; 	getline (readFile,line10);

					std::vector<std::string> tokens;
					tokenize1(line1,tokens);
					if (tokens.size()>2)
					{
						universe.low[0] = atof(tokens.at(0).c_str());
						universe.high[0] = atof(tokens.at(1).c_str());
					}
					else
					{
#ifdef FATAL
			cout << "Formatting problem : Could not read header from file : " << filename;
#endif
					}
					tokens.clear();
					tokenize1(line2,tokens);
					if (tokens.size()>2)
					{
						universe.low[1] = atof(tokens.at(0).c_str());
						universe.high[1] = atof(tokens.at(1).c_str());
					}
					else
					{
#ifdef FATAL
			cout << "Formatting problem : Could not read header from file : " << filename;
#endif
					}
					tokens.clear();
					tokenize1(line3,tokens);
					if (tokens.size()>2)
					{
						universe.low[2] = atof(tokens.at(0).c_str());
						universe.high[2] = atof(tokens.at(1).c_str());
					}
					else
					{
#ifdef FATAL
			cout << "Formatting problem : Could not read header from file : " << filename;
#endif
					}
					tokens.clear();
					tokenize1(line10,tokens);
					if (tokens.size()>4)
					{
						string temp = tokens.at(3);
						temp = temp.substr(0,temp.size()-1);
						objectCount = atoi(temp.c_str());
					}
					else
					{
#ifdef FATAL
			cout << "Formatting problem : Could not read header from file : " << filename;
#endif
					}

					readFile.close();
				}
		catch(...)
		{
#ifdef FATAL
			cout << "Could not read header from file : " << filename;
#endif
		}

#ifdef INFORMATION
		cout << "\n == INPUT FILE HEADER == \n\n"
		     << "OBJECT TYPE: " << SpatialObjectFactory::getTitle(objectType) << endl
			 << "TOTAL OBJECTS: " << objectCount << endl
			 << "OBJECT BYTE SIZE: " << objectByteSize <<endl
			 << "UNIVERSE BOUNDS: " << universe << endl;
#endif
	}

	bool BBPTextFileReader::hasNext()
	{
		data = SpatialObjectFactory::create(objectType);

		string line;
		getline (inputFile,line);

		if (inputFile.eof() || counter>=objectCount)
		{
			return false;
		}
		else
		{
			counter++;
			if (objectType==VERTEX)
			{
			Vertex v;
			std::vector<std::string> tokens;
			tokenize1(line,tokens);
			v[0] = atof(tokens.at(1).c_str());
			v[1] = atof(tokens.at(2).c_str());
			v[2] = atof(tokens.at(3).c_str());

			data->unserialize((int8*)&(v.Vector));
			//cout << tokens.size() << "  " << v << " " << data->getCenter() << endl;
			}
			else
			{
#ifdef FATAL
			cout << "Object type not supported" << endl;
#endif
			}
			return true;
		}
	}

	SpatialObject* BBPTextFileReader::getNext()
	{
		return data;
	}

	void BBPTextFileReader::rewind()
	{
#ifdef FATAL
		cout << "Rewind not implemented!:" <<endl;
#endif

		counter=0;
	}
}
