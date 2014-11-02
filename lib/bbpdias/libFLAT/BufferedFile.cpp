#include "BufferedFile.hpp"

using namespace std;
namespace FLAT
{

	BufferedFile::BufferedFile()
	{
		eof = true;
		temporary = false;
	}

	BufferedFile::~BufferedFile()
	{
		close();
		if (temporary)
		{
			try
			{
			remove(filename.c_str());
			}
			catch(...)
			{
#ifdef FATAL
		cout << "Cannot Delete Temporary File: " << filename << "\n";
#endif
			}
		}
	}

	/*
	 * not fixed for large files
	 */
	void BufferedFile::seekEnd(uint64 offset)
	{
		try
		{
			eof = false;
			file.clear();
			file.seekg(offset*-1, ios_base::end);
			if (!file.good()) throw 1;
		}
		catch(...)
		{
			eof = true;
	#ifdef FATAL
			cout << "Cannot Seek file" << filename << " to:" << offset << "\n";
	#endif
		}
	}

	void BufferedFile::close()
	{
		eof=true;
		file.clear();
		if (file.is_open())
			file.close();
		delete[] buffer;
		buffer  = NULL;
	}

	void BufferedFile::open(string filename)
	{
		try
		{
			this->filename = filename;
			close();
			eof=false;
			temporary=false;
			buffer = new char[FILE_BUFFER_SIZE];

			file.open(filename.c_str(), ios_base::in | ios_base::binary);
			file.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);

			if (!file.good()) throw 1;
		}
		catch(...)
		{
			eof = true;
	#ifdef FATAL
			cout << "Cannot Open file: " << filename << "\n";
	#endif
		}
	}

	void BufferedFile::create(string filename)
	{
		try
		{
			this->filename = filename;
			close();
			eof = false;
			temporary = false;
			buffer = new char[FILE_BUFFER_SIZE];

			file.open(filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
			file.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);

			if (!file.good()) throw 1;
		}
		catch(...)
		{
			eof = true;
	#ifdef FATAL
			cout << "Cannot Create file: " << filename << "\n";
	#endif
		}
	}

	void BufferedFile::maketemporary()
	{
#ifdef WIN32
	    char tmpName[11] = "tempXXXXXX";
	    char * tmp_name_ptr = tmpnam(tmpName);
	    if (tmp_name_ptr[0] == '\\')
	        filename = string(tmp_name_ptr + 1);
	    else
	    	filename = string(tmp_name_ptr);
#else
	    char tmpName[11] = "tempXXXXXX";
	    if (mktemp(tmpName) == 0)
#ifdef FATAL
		cout << "Cannot Make Temporary filename: \n";
#endif
	    filename = tmpName;
#endif
	    create (filename);
	    temporary = true;
	}

	void BufferedFile::rewind()
	{
		try
		{
			file.clear();
			file.seekg(0, ios_base::beg);
			if (! file.good())
				throw 1;
			eof = false;
		}
		catch(...)
		{
			eof = true;
	#ifdef FATAL
			cout << "Cannot Rewind file" << filename << "\n";
	#endif
		}
	}

	uint32 BufferedFile::readUInt32()
	{
		uint32 ret;
		try
		{
		file.read(reinterpret_cast<char*>(&ret), sizeof(uint32));
		if (!file.good()) throw 1;
		}
		catch(...)
		{
			eof = true;
			#ifdef FATAL

			cout << "Cannot Read uint32 from file: " << filename << "\n";
	#endif
		}
		return ret;
	}

	void BufferedFile::writeUInt32(uint32 i)
	{
		try
		{
		file.write(reinterpret_cast<const char*>(&i), sizeof(uint32));
		if (! file.good()) throw 1;
		}
		catch(...)
		{
			eof = true;
	#ifdef FATAL
			cout << "Cannot Write uint32 to file: " << filename << "\n";
	#endif
		}
	}

	uint64 BufferedFile::readUInt64()
	{
		uint64 ret;
		try
		{
		file.read(reinterpret_cast<char*>(&ret), sizeof(uint64));
		if (!file.good()) throw 1;
		}
		catch(...)
		{
			eof = true;
	#ifdef FATAL
			cout << "Cannot Read uint64 from file: " << filename << "\n";
	#endif
		}
		return ret;
	}

	void BufferedFile::writeUInt64(uint64 i)
	{
		try
		{
		file.write(reinterpret_cast<const char*>(&i), sizeof(uint64));
		if (! file.good()) throw 1;
		}
		catch(...)
		{
			eof = true;
	#ifdef FATAL
			cout << "Cannot Write uint64 to file: " << filename << "\n";
	#endif
		}
	}
}
