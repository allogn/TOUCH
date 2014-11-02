#ifndef DIAS_TEMPORARYFILE_HPP
#define DIAS_TEMPORARYFILE_HPP

#include "Filing.hpp"

namespace FLAT
{

class TemporaryFile : boost::noncopyable
{
public:
	 TemporaryFile(int bufferSize);
	 virtual ~TemporaryFile();

	 void rewindForReading();
	 void rewindForWriting();
	 bool eof();
	 std::string getFileName() const;
	 TemporaryFile& operator=(const TemporaryFile &rhs);

	 uint8_t readUInt8();
	 uint16_t readUInt16();
	 uint32_t readUInt32();
	 uint64_t readUInt64();
	 float readFloat();
	 double readDouble();
	 std::string readString();
	 void readBytes(uint32_t u32Len, byte** pData);

	 void write(uint8_t i);
	 void write(uint16_t i);
	 void write(uint32_t i);
	 void write(uint64_t i);
	 void write(float i);
	 void write(double i);
	 void write(const std::string& s);
	 void write(uint32_t u32Len, byte* pData);

	std::string m_sFile;
    BufferedFile* m_pFile;

};

inline TemporaryFile& TemporaryFile::operator=(const TemporaryFile &rhs)
{
		m_sFile = rhs.m_sFile;
		m_pFile = rhs.m_pFile;

	return *this;
}

}
#endif
