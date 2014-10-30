#ifndef VTK_FILE_HPP
#define VTK_FILE_HPP

#include "Box.hpp"
#include <sstream>

using namespace std;

namespace FLAT
{
enum ColorType
{
	WHITE_COLOR,
	BLACK_COLOR,
	RED_COLOR,
	GREEN_COLOR,
	BLUE_COLOR,
	YELLOW_COLOR,
	GREY_COLOR
};

class Color
{
public:
	unsigned char rgb[3];
	Color(ColorType c);
	Color(unsigned char R,unsigned char G,unsigned char B);
};


	class vtkDataFile
	{
	private:
		string _filename;
		string _mode;
		bool   _saved;

		vector<Vertex> _points;
		std::ofstream _file;

		int _pointCount;

		int _polyCount;
		int _lineCount;
		int _vertexCount;

		int _polySize;
		int _lineSize;
		int _vertexSize;

		stringstream _pointSection;
		stringstream _polySection;
		stringstream _verticesSection;
		stringstream _linesSection;

		void writeHeader()
		{
			_file << "# vtk DataFile Version 2.0"  <<endl;
			_file << "Farhan's VTK file writer"  <<endl;
			if (_mode=="ascii")
				_file << "ASCII"  <<endl;
			else
				_file << "BINARY"  <<endl;
			_file << "DATASET POLYDATA"  <<endl;

		}

		void writeFile()
		{
			_file << "POINTS "  << _pointCount << " float" <<endl;
			_file << _pointSection.str();

			if (_polyCount>0)
			{
				_file << "POLYGONS "  << _polyCount << " " << _polySize << endl;
				_file << _polySection.str();
			}


			if (_lineCount>0)
			{
				_file << "LINES " << _lineCount << " " << _lineSize << endl;
				_file << _linesSection.str();
			}

			if (_vertexCount>0)
			{
				_file << "VERTICES " << _vertexCount << " " << _vertexSize << endl;
				_file << _vertexCount << _verticesSection.str() << endl;
			}

			_file.flush();
		}

	public:
		vtkDataFile(string fileName,string fileMode)
		{
			_saved = false;
			_pointCount = 0;
			_polyCount = 0;
			_vertexCount =0;
			_lineSize =0;
			_polySize =0;
			_lineSize =0;
			_vertexSize =1;
			_filename = fileName;
			_mode = fileMode;
			_file.open (_filename.c_str(),std::ios::out |std::ios::trunc);
			_verticesSection.clear();
			_linesSection.clear();
			_polySection.clear();
			_pointSection.clear();
		}

		~vtkDataFile()
		{
			if (!_saved) save();
			_file.close();
		}

		void save()
		{
			writeHeader();
			writeFile();
			_saved = true;
		}

		void DrawVertex(const Vertex& v, const Color& c)
		{
			_pointSection << v[0] << " " << v[1] << " " << v[2] << endl;
			_verticesSection << " " << _pointCount;
			_pointCount++;
			_vertexCount++;
			_vertexSize++;
		}

		void DrawLine(const Vertex& v1,const Vertex& v2, const Color& c)
		{
			_pointSection << v1[0] << " " << v1[1] << " " << v1[2] << endl;
			_pointSection << v2[0] << " " << v2[1] << " " << v2[2] << endl;
			_linesSection << "2 " << _pointCount << " " << _pointCount+1 << endl;
			_pointCount+=2;
			_lineCount++;
			_lineSize+=3;
		}

		void DrawSquare(const Vertex& v1,const Vertex& v2,const Vertex& v3,const Vertex& v4,const Color& c)
		{
			_pointSection << v1[0] << " " << v1[1] << " " << v1[2] << endl;
			_pointSection << v2[0] << " " << v2[1] << " " << v2[2] << endl;
			_pointSection << v3[0] << " " << v3[1] << " " << v3[2] << endl;
			_pointSection << v4[0] << " " << v4[1] << " " << v4[2] << endl;
			_polySection << "4 " << _pointCount << " " << _pointCount+1 << " " << _pointCount+2 << " " << _pointCount+3 << endl;
			_pointCount+=4;
			_polyCount++;
			_polySize+=5;
		}

		void DrawBox(const Box& b, const Color& c)
		{
			Vertex v0,v1,v2,v3,v4,v5,v6,v7;

			v0 = b.low;
			v1 = v0; v1[0] = b.high[0];
			v2 = v1; v2[1] = b.high[1];
			v3 = v2; v3[0] = b.low[0];

			v4 = v0; v4[2] = b.high[2];
			v5 = v1; v5[2] = b.high[2];
			v6 = v2; v6[2] = b.high[2];
			v7 = v3; v7[2] = b.high[2];

			_pointSection << v0[0] << " " << v0[1] << " " << v0[2] << endl;
			_pointSection << v1[0] << " " << v1[1] << " " << v1[2] << endl;
			_pointSection << v2[0] << " " << v2[1] << " " << v2[2] << endl;
			_pointSection << v3[0] << " " << v3[1] << " " << v3[2] << endl;
			_pointSection << v4[0] << " " << v4[1] << " " << v4[2] << endl;
			_pointSection << v5[0] << " " << v5[1] << " " << v5[2] << endl;
			_pointSection << v6[0] << " " << v6[1] << " " << v6[2] << endl;
			_pointSection << v7[0] << " " << v7[1] << " " << v7[2] << endl;

			_polySection << "4 " << _pointCount+0 << " " << _pointCount+1 << " " << _pointCount+2 << " " << _pointCount+3 << endl;
			_polySection << "4 " << _pointCount+4 << " " << _pointCount+5 << " " << _pointCount+6 << " " << _pointCount+7 << endl;
			_polySection << "4 " << _pointCount+0 << " " << _pointCount+1 << " " << _pointCount+5 << " " << _pointCount+4 << endl;
			_polySection << "4 " << _pointCount+2 << " " << _pointCount+3 << " " << _pointCount+7 << " " << _pointCount+6 << endl;
			_polySection << "4 " << _pointCount+0 << " " << _pointCount+4 << " " << _pointCount+7 << " " << _pointCount+3 << endl;
			_polySection << "4 " << _pointCount+1 << " " << _pointCount+2 << " " << _pointCount+6 << " " << _pointCount+5 << endl;

			_pointCount+=8;
			_polyCount+=6;
			_polySize+=30;
		}

	};

}







#endif
