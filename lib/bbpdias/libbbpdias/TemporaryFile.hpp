#ifndef DIAS_TEMPORARYFILE_HPP_
#define DIAS_TEMPORARYFILE_HPP_

#include <fstream>
#include <boost/utility.hpp>

#include "Filing.hpp"

namespace Farhan
{

class TemporaryFile : boost::noncopyable
{
public:
	inline TemporaryFile();
	inline TemporaryFile(std::string filename,bool create);
	inline virtual ~TemporaryFile();

	inline void rewindForReading();
	inline void rewindForWriting();
	inline bool eof();
	inline std::string getFileName() const;

	inline uint8_t readUInt8();
	inline uint16_t readUInt16();
	inline uint32_t readUInt32();
	inline uint64_t readUInt64();
	inline float readFloat();
	inline double readDouble();
	inline std::string readString();
	inline void readBytes(uint32_t u32Len, byte** pData);

	inline void write(uint8_t i);
	inline void write(uint16_t i);
	inline void write(uint32_t i);
	inline void write(uint64_t i);
	inline void write(float i);
	inline void write(double i);
	inline void write(const std::string& s);
	inline void write(uint32_t u32Len, byte* pData);

	std::string m_sFile;
        Farhan::BufferedFile* m_pFile;
public:
        inline TemporaryFile& operator=(const TemporaryFile &rhs)
	{
			m_sFile = rhs.m_sFile;
			m_pFile = rhs.m_pFile;

		return *this;
	}

};


//
// TemporaryFile
//
inline TemporaryFile::TemporaryFile(std::string filename,bool create)
{
	m_sFile = filename;
	if (create)
		m_pFile = new BufferedFileWriter(m_sFile, CREATE);
	else
		m_pFile = new BufferedFileReader(filename);
}

inline TemporaryFile::TemporaryFile()
{
#ifdef _MSC_VER
	char tmpName[L_tmpnam_s];
	errno_t err = tmpnam_s(tmpName, L_tmpnam_s);
	if (err)
		throw std::ios_base::failure("TemporaryFile: Cannot create temporary file name.");

	if (tmpName[0] == '\\')
		m_sFile = std::string(tmpName + 1);
	else
		m_sFile = std::string(tmpName);
#else
    char tmpName[7] = "XXXXXX";

    char * tmp_name_ptr = tmpnam(tmpName);

//XXX:
//if (mktemp(tmpName) == 0)
//	throw std::ios_base::failure("TemporaryFile: Cannot create temporary file name.");
    if (tmp_name_ptr[0] == '\\')
        m_sFile = std::string(tmp_name_ptr + 1);
    else
        m_sFile = std::string(tmp_name_ptr);

#endif

	m_pFile = new BufferedFileWriter(m_sFile, CREATE);
}


inline TemporaryFile::~TemporaryFile()
{
	delete m_pFile;

#ifdef _MSC_VER
	_unlink(m_sFile.c_str());
#else
	std::remove(m_sFile.c_str());
#endif
}

inline void TemporaryFile::rewindForReading()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br != 0)
		br->rewind();
	else
	{
		delete m_pFile;
		m_pFile = new BufferedFileReader(m_sFile);
	}
}

inline void TemporaryFile::rewindForWriting()
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw != 0)
		bw->rewind();
	else
	{
		delete m_pFile;
		m_pFile = new BufferedFileWriter(m_sFile);
	}
}

inline bool TemporaryFile::eof()
{
	return m_pFile->eof();
}

inline std::string TemporaryFile::getFileName() const
{
	return m_sFile;
}

inline uint8_t TemporaryFile::readUInt8()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readUInt8: file not open for reading.");

	return br->readUInt8();
}

inline uint16_t TemporaryFile::readUInt16()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readUInt16: file not open for reading.");

	return br->readUInt16();
}

inline uint32_t TemporaryFile::readUInt32()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readUInt32: file not open for reading.");

	return br->readUInt32();
}

inline uint64_t TemporaryFile::readUInt64()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readUInt64: file not open for reading.");

	return br->readUInt64();
}

inline float TemporaryFile::readFloat()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readFloat: file not open for reading.");

	return br->readFloat();
}

inline double TemporaryFile::readDouble()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readDouble: file not open for reading.");

	return br->readDouble();
}

inline std::string TemporaryFile::readString()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readString: file not open for reading.");

	return br->readString();
}

inline void TemporaryFile::readBytes(uint32_t u32Len, byte** pData)
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readString: file not open for reading.");

	return br->readBytes(u32Len, pData);
}

inline void TemporaryFile::write(uint8_t i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

inline void TemporaryFile::write(uint16_t i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

inline void TemporaryFile::write(uint32_t i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

inline void TemporaryFile::write(uint64_t i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

inline void TemporaryFile::write(float i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

inline void TemporaryFile::write(double i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

inline void TemporaryFile::write(const std::string& s)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(s);
}

inline void TemporaryFile::write(uint32_t u32Len, byte* pData)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(u32Len, pData);
}
}
#endif
