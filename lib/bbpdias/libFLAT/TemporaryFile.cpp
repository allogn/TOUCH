#include "TemporaryFile.hpp"
#include <fstream>
#include <boost/utility.hpp>

namespace FLAT {


 TemporaryFile::TemporaryFile(int bufferSize)
{
#ifdef WIN32
	    char tmpName[7] = "XXXXXX";
	    char * tmp_name_ptr = tmpnam(tmpName);
	    if (tmp_name_ptr[0] == '\\')
	        m_sFile = std::string(tmp_name_ptr + 1);
	    else
	        m_sFile = std::string(tmp_name_ptr);
#else
	char tmpName[11] = "flatXXXXXX";
	if (mktemp(tmpName) == 0)
		throw std::ios_base::failure("TemporaryFile: Cannot create temporary file name.");
	m_sFile = tmpName;
#endif

	m_pFile = new BufferedFileWriter(m_sFile, CREATE,bufferSize);
}

 TemporaryFile::~TemporaryFile()
{
	delete m_pFile;

#ifdef _MSC_VER
	_unlink(m_sFile.c_str());
#else
	std::remove(m_sFile.c_str());
#endif
}

 void TemporaryFile::rewindForReading()
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

 void TemporaryFile::rewindForWriting()
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

 bool TemporaryFile::eof()
{
	return m_pFile->eof();
}

 std::string TemporaryFile::getFileName() const
{
	return m_sFile;
}

 uint8_t TemporaryFile::readUInt8()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readUInt8: file not open for reading.");

	return br->readUInt8();
}

 uint16_t TemporaryFile::readUInt16()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readUInt16: file not open for reading.");

	return br->readUInt16();
}

 uint32_t TemporaryFile::readUInt32()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readUInt32: file not open for reading.");

	return br->readUInt32();
}

 uint64_t TemporaryFile::readUInt64()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readUInt64: file not open for reading.");

	return br->readUInt64();
}

 float TemporaryFile::readFloat()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readFloat: file not open for reading.");

	return br->readFloat();
}

 double TemporaryFile::readDouble()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readDouble: file not open for reading.");

	return br->readDouble();
}

 std::string TemporaryFile::readString()
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readString: file not open for reading.");

	return br->readString();
}

 void TemporaryFile::readBytes(uint32_t u32Len, byte** pData)
{
	BufferedFileReader* br = dynamic_cast<BufferedFileReader*>(m_pFile);
	if (br == 0)
		throw std::ios_base::failure("TemporaryFile::readString: file not open for reading.");

	return br->readBytes(u32Len, pData);
}

 void TemporaryFile::write(uint8_t i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

 void TemporaryFile::write(uint16_t i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

 void TemporaryFile::write(uint32_t i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

 void TemporaryFile::write(uint64_t i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

 void TemporaryFile::write(float i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

 void TemporaryFile::write(double i)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(i);
}

 void TemporaryFile::write(const std::string& s)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(s);
}

 void TemporaryFile::write(uint32_t u32Len, byte* pData)
{
	BufferedFileWriter* bw = dynamic_cast<BufferedFileWriter*>(m_pFile);
	if (bw == 0)
		throw std::ios_base::failure("TemporaryFile::write: file not open for writing.");

	return bw->write(u32Len, pData);
}

}
