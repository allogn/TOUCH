#ifndef DIAS_FILING_HPP_
#define DIAS_FILING_HPP_

#include <inttypes.h>
#include <fstream>
#include <boost/utility.hpp>

namespace FLAT
{
typedef uint8_t byte;

enum FileMode
{
	APPEND = 0x0,
	CREATE
};

class Exception
{
public:
	virtual std::string what() = 0;
	virtual ~Exception() {}
};

class EndOfStreamException : public Exception
	{
	public:
	EndOfStreamException(std::string s) : m_error(s)
	{
	}
		virtual ~EndOfStreamException() {}
		std::string what()
		{
			return "EndOfStreamException: " + m_error;
		}

	private:
		std::string m_error;
	}; // EndOfStreamException


class IllegalArgumentException : public Exception
{
public:
	IllegalArgumentException(std::string s) : m_error(s)
	{
	}
	virtual ~IllegalArgumentException() {}
	std::string what()
	{
		return "IllegalArgumentException: " + m_error;
	}

private:
	std::string m_error;
}; // IllegalArgumentException


class BufferedFile : boost::noncopyable
	{
	public:
	inline BufferedFile(size_t stBufferSize = 16384);
	inline virtual ~BufferedFile();
	inline virtual void close();
	inline virtual bool eof();
	inline virtual void rewind() = 0;
	inline virtual void seek(uint64_t offset) = 0;

		std::fstream m_file;
		char* m_buffer;
		size_t m_stBufferSize;
		bool m_bEOF;
	};

	class BufferedFileReader : public BufferedFile
	{
	public:
		inline BufferedFileReader();
		inline BufferedFileReader(const std::string& sFileName, size_t stBufferSize = 32768);
		inline virtual ~BufferedFileReader();

		inline virtual void open(const std::string& sFileName);
		inline virtual void rewind();
		inline virtual void seek(uint64_t offset);
        inline std::fstream::off_type get_file_length (void);

		inline virtual uint8_t readUInt8();
		inline virtual uint16_t readUInt16();
		inline virtual uint32_t readUInt32();
		inline virtual uint64_t readUInt64();
		inline virtual float readFloat();
		inline virtual double readDouble();
		inline virtual bool readBoolean();
		inline virtual std::string readString();
		inline virtual void readBytes(uint32_t u32Len, byte** pData);
	};

	class BufferedFileWriter : public BufferedFile
	{
	public:
		inline BufferedFileWriter();
		inline BufferedFileWriter(const std::string& sFileName, FileMode mode = CREATE, size_t stBufferSize = 32768);
		inline virtual ~BufferedFileWriter();

		inline virtual void open(const std::string& sFileName, FileMode mode = CREATE);
		inline virtual void rewind();
		inline virtual void seek(uint64_t offset);
                inline std::fstream::off_type get_pos (void);

		inline virtual void write(uint8_t i);
		inline virtual void write(uint16_t i);
		inline virtual void write(uint32_t i);
		inline virtual void write(uint64_t i);
		inline virtual void write(float i);
		inline virtual void write(double i);
		inline virtual void write(bool b);
		inline virtual void write(const std::string& s);
		inline virtual void write(uint32_t u32Len, byte* pData);
	};

	//
	// BufferedFile
	//
	inline BufferedFile::BufferedFile(size_t stBufferSize)
	: m_buffer(new char[stBufferSize]), m_stBufferSize(stBufferSize), m_bEOF(true)
	{
	}

	inline BufferedFile::~BufferedFile()
	{
		m_file.close();
		delete[] m_buffer;
	}

	inline void BufferedFile::close()
	{
		m_file.close();
	}

	inline bool BufferedFile::eof()
	{
		return m_bEOF;
	}

	//
	// BufferedFileReader
	//
	inline BufferedFileReader::BufferedFileReader()
	{
	}

	inline BufferedFileReader::BufferedFileReader(const std::string& sFileName, size_t stBufferSize)
	: BufferedFile(stBufferSize)
	{
		open(sFileName);
	}

inline void BufferedFileReader::open(const std::string& sFileName)
	{
		m_bEOF = false;
		m_file.close(); m_file.clear();


		m_file.open(sFileName.c_str(), std::ios_base::in | std::ios_base::binary);
		if (! m_file.good())
			throw std::ios_base::failure("BufferedFileReader::BufferedFileReader: Cannot open file.");

		m_file.rdbuf()->pubsetbuf(m_buffer, m_stBufferSize);
	}

	inline BufferedFileReader::~BufferedFileReader()
	{
	}

	inline void BufferedFileReader::rewind()
	{
		m_file.clear();
		m_file.seekg(0, std::ios_base::beg);
		if (! m_file.good())
			throw std::ios_base::failure("BufferedFileReader::rewind: seek failed.");

		m_bEOF = false;
	}

	inline void BufferedFileReader::seek(uint64_t offset)
	{
		m_bEOF = false;
		m_file.clear();
#ifdef WIN32
		m_file.seekg(0, std::ios_base::beg);
		uint32_t n4GBSeeks = offset / 0xFFFFFFFF;
		for(uint32_t i = 0; i < n4GBSeeks; i++)
			m_file.seekg(0xFFFFFFFF, std::ios_base::cur);
		m_file.seekg(offset % 0xFFFFFFFF, std::ios_base::cur);
#else
		m_file.seekg(offset, std::ios_base::beg);
#endif
		if (! m_file.good())
			throw std::ios_base::failure("BufferedFileReader::rewind: seek failed.");
	}

inline std::fstream::off_type BufferedFileReader::get_file_length (void)
{
    m_file.clear ();
    std::fstream::off_type old_position = m_file.tellg ();
    if (old_position == -1)
        throw std::ios_base::failure ("BufferedFileReader::get_file_length: 1st tellg failed.");
    m_file.seekg (0, std::ios::end);
    if (!m_file.good ())
        throw std::ios_base::failure ("BufferedFileReader::get_file_length: seekg to file end failed.");
    std::fstream::off_type result = m_file.tellg ();
    if (old_position == -1)
        throw std::ios_base::failure ("BufferedFileReader::get_file_length: 2nd tellg failed.");
    m_file.seekg (old_position, std::ios::beg);
    if (!m_file.good ())
        throw std::ios_base::failure ("BufferedFileReader::get_file_length: seekg to old position failed.");
    return result;
}

        inline uint8_t BufferedFileReader::readUInt8()
	{
		if (m_bEOF) throw EndOfStreamException("");

		uint8_t ret;
		m_file.read(reinterpret_cast<char*>(&ret), sizeof(uint8_t));
		if (! m_file.good())
		{
			m_bEOF = true;
			throw EndOfStreamException("");
		}
		return ret;
	}

	inline uint16_t BufferedFileReader::readUInt16()
	{
		if (m_bEOF) throw EndOfStreamException("");

		uint16_t ret;
		m_file.read(reinterpret_cast<char*>(&ret), sizeof(uint16_t));
		if (! m_file.good())
		{
			m_bEOF = true;
			throw EndOfStreamException("");
		}
		return ret;
	}

	inline uint32_t BufferedFileReader::readUInt32()
	{
		if (m_bEOF) throw EndOfStreamException("");

		uint32_t ret;
		m_file.read(reinterpret_cast<char*>(&ret), sizeof(uint32_t));
		if (! m_file.good())
		{
			m_bEOF = true;
			throw EndOfStreamException("");
		}
		return ret;
	}

	inline uint64_t BufferedFileReader::readUInt64()
	{
		if (m_bEOF) throw EndOfStreamException("");

		uint64_t ret;
		m_file.read(reinterpret_cast<char*>(&ret), sizeof(uint64_t));
		if (! m_file.good())
		{
			m_bEOF = true;
			throw EndOfStreamException("");
		}
		return ret;
	}

	inline float BufferedFileReader::readFloat()
	{
		if (m_bEOF) throw EndOfStreamException("");

		float ret;
		m_file.read(reinterpret_cast<char*>(&ret), sizeof(float));
		if (! m_file.good())
		{
                    m_bEOF = true;
			throw EndOfStreamException("");
		}
		return ret;
	}

	inline double BufferedFileReader::readDouble()
	{
		if (m_bEOF) throw EndOfStreamException("");
                if (! m_file.good ())
                {
                    m_bEOF = true;
                    throw EndOfStreamException ("");
                }

		double ret;
		m_file.read(reinterpret_cast<char*>(&ret), sizeof(double));
		if (! m_file.good())
		{
			m_bEOF = true;
			throw EndOfStreamException("");
		}
		return ret;
	}

	inline bool BufferedFileReader::readBoolean()
	{
		if (m_bEOF) throw EndOfStreamException("");

		bool ret;
		m_file.read(reinterpret_cast<char*>(&ret), sizeof(bool));
		if (! m_file.good())
		{
			m_bEOF = true;
			throw EndOfStreamException("");
		}
		return ret;
	}

	inline 	std::string BufferedFileReader::readString()
	{
		if (m_bEOF) throw EndOfStreamException("");

		std::string::size_type len;
		m_file.read(reinterpret_cast<char*>(&len), sizeof(std::string::size_type));
		if (! m_file.good())
		{
			m_bEOF = true;
			throw EndOfStreamException("");
		}

		std::string::value_type* buf = new std::string::value_type[len];
		m_file.read(reinterpret_cast<char*>(buf), len * sizeof(std::string::value_type));
		if (! m_file.good())
		{
			delete[] buf;
			m_bEOF = true;
			throw EndOfStreamException("");
		}

		std::string ret(buf, len);
		delete[] buf;

		return ret;
	}

	inline void BufferedFileReader::readBytes(uint32_t u32Len, byte** pData)
	{
		if (m_bEOF) throw EndOfStreamException("");

		*pData = new byte[u32Len];
		m_file.read(reinterpret_cast<char*>(*pData), u32Len);
		if (! m_file.good())
		{
			delete[] *pData;
			m_bEOF = true;
			throw EndOfStreamException("");
		}
	}

	//
	// BufferedFileWriter
	//
	inline BufferedFileWriter::BufferedFileWriter()
	{
		open("");
	}

	inline BufferedFileWriter::BufferedFileWriter(const std::string& sFileName, FileMode mode, size_t stBufferSize)
	: BufferedFile(stBufferSize)
	{
		open(sFileName, mode);
	}

	inline BufferedFileWriter::~BufferedFileWriter()
	{
		m_file.flush();
	}

	inline void BufferedFileWriter::open(const std::string& sFileName, FileMode mode)
	{
		m_bEOF = false;
		m_file.close(); m_file.clear();

		if (mode == CREATE)
		{
			m_file.open(sFileName.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
			if (! m_file.good())
				throw std::ios_base::failure("BufferedFileWriter::open: Cannot open file.");
		}
		else if (mode == APPEND)
		{
			// Idiotic fstream::open truncates an existing file anyway, if it is only opened
			// for output (no ios_base::in flag)!! On the other hand, if a file does not exist
			// and the ios_base::in flag is specified, then the open fails!!

			m_file.open(sFileName.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::binary);
			if (! m_file.good())
			{
				m_file.clear();
				m_file.open(sFileName.c_str(), std::ios_base::out | std::ios_base::binary);
				if (! m_file.good())
					throw std::ios_base::failure("BufferedFileWriter::open: Cannot open file.");
			}
			else
			{
				m_file.seekp(0, std::ios_base::end);
				if (! m_file.good())
					throw std::ios_base::failure("BufferedFileWriter::open: Cannot open file.");
			}
		}
		else
			throw IllegalArgumentException("BufferedFileWriter::open: Unknown mode.");
	}

	inline void BufferedFileWriter::rewind()
	{
		m_bEOF = false;
		m_file.clear();
		m_file.seekp(0, std::ios_base::beg);
		if (! m_file.good())
			throw std::ios_base::failure("BufferedFileWriter::rewind: seek failed.");
	}

	inline void BufferedFileWriter::seek(uint64_t offset)
	{
		m_bEOF = false;
		m_file.clear();
		m_file.seekp(offset, std::ios_base::beg);
		if (! m_file.good())
			throw std::ios_base::failure("BufferedFileWriter::seek: seek failed.");
	}

        inline std::fstream::off_type BufferedFileWriter::get_pos (void)
        {
            std::fstream::off_type pos = m_file.tellg ();
            if (!m_file.good ())
                throw std::ios_base::failure ("BufferedFileWriter::get_pos: get_pos failed.");
            return pos;
        }

	inline void BufferedFileWriter::write(uint8_t i)
	{
		m_file.write(reinterpret_cast<const char*>(&i), sizeof(uint8_t));
		if (! m_file.good()) throw std::ios_base::failure("");
	}

	inline void BufferedFileWriter::write(uint16_t i)
	{
		m_file.write(reinterpret_cast<const char*>(&i), sizeof(uint16_t));
		if (! m_file.good()) throw std::ios_base::failure("");
	}

	inline void BufferedFileWriter::write(uint32_t i)
	{
		m_file.write(reinterpret_cast<const char*>(&i), sizeof(uint32_t));
		if (! m_file.good()) throw std::ios_base::failure("");
	}

	inline void BufferedFileWriter::write(uint64_t i)
	{
		m_file.write(reinterpret_cast<const char*>(&i), sizeof(uint64_t));
		if (! m_file.good()) throw std::ios_base::failure("");
	}

	inline void BufferedFileWriter::write(float i)
	{
		m_file.write(reinterpret_cast<const char*>(&i), sizeof(float));
		if (! m_file.good()) throw std::ios_base::failure("");
	}

	inline void BufferedFileWriter::write(double i)
	{
		m_file.write(reinterpret_cast<const char*>(&i), sizeof(double));
		if (! m_file.good()) throw std::ios_base::failure("");
	}

	inline void BufferedFileWriter::write(bool b)
	{
		m_file.write(reinterpret_cast<const char*>(&b), sizeof(bool));
		if (! m_file.good()) throw std::ios_base::failure("");
	}

	inline void BufferedFileWriter::write(const std::string& s)
	{
		std::string::size_type len = s.size();
		m_file.write(reinterpret_cast<const char*>(&len), sizeof(std::string::size_type));
		if (! m_file.good()) throw std::ios_base::failure("");
		m_file.write(reinterpret_cast<const char*>(s.c_str()), len * sizeof(std::string::value_type));
		if (! m_file.good()) throw std::ios_base::failure("");
	}

	inline void BufferedFileWriter::write(uint32_t u32Len, byte* pData)
	{
		m_file.write(reinterpret_cast<const char*>(pData), u32Len);
		if (! m_file.good()) throw std::ios_base::failure("");
	}

}
#endif
