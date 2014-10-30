#include "Bitmap.hpp"

using namespace std;
namespace FLAT
{

	Color::Color(ColorType c)
	{
		switch(c)
		{
			case BLACK_COLOR: rgb[0]=0;rgb[1]=0;rgb[2]=0; break;
			case WHITE_COLOR: rgb[0]=255;rgb[1]=255;rgb[2]=255; break;
			case RED_COLOR: rgb[0]=255;rgb[1]=0;rgb[2]=0; break;
			case GREEN_COLOR: rgb[0]=0;rgb[1]=255;rgb[2]=0; break;
			case BLUE_COLOR: rgb[0]=0;rgb[1]=0;rgb[2]=255; break;
			case YELLOW_COLOR: rgb[0]=255;rgb[1]=255;rgb[2]=0; break;
			case GREY_COLOR: rgb[0]=125;rgb[1]=125;rgb[2]=125; break;
			default: rgb[0]=0;rgb[1]=0;rgb[2]=0;
		}
	}
	Color::Color(unsigned char R,unsigned char G,unsigned char B)
	{
		rgb[0] = R;
		rgb[1] = G;
		rgb[2] = B;
	}


	Bitmap::Bitmap (int dim,int ch,const spaceUnit res,Box& dom,const Color& c)
	{
		domain = dom;
		channel = ch;
		Vertex length = dom.high - dom.low;
		int a = Box::LargestDimension(dom);
		for (int i=0;i<dim;i++)
			resolution[i] = res*length[i]/length[a];
		dimension = dim;
		domainExtent = domain.high - domain.low;

		if (dimension==2)
		{
			bitmap = cimg_library::CImg<unsigned char>((int)resolution[0],(int)resolution[1],1,channel);
			bitmap.draw_fill(1,1,c.rgb);
		}
		else if (dimension==3)
		{
			bitmap = cimg_library::CImg<unsigned char>((int)resolution[0],(int)resolution[1],(int)resolution[2],channel);
			bitmap.draw_fill(1,1,1,c.rgb);
		}
		else
			cout << "Donot Support bitmaps other than 2 or 3 dimensions";
	}

	Bitmap::Bitmap (string filename)
	{
		cout << "Bitmap Load() Not Implemented Yet!";
	}

	Bitmap::Bitmap ()
	{
		cout << "Bitmap Load() Not Implemented Yet!";
	}

	Bitmap::~Bitmap()
	{
	}

	void Bitmap::save(string filename)
	{
		bitmap.save(filename.c_str());
	}

	void Bitmap::drawLine(const Vertex& l1,const Vertex & l2,const Color& c)
	{
		Vertex normalized1,normalized2;

		for (int i=0;i<dimension;i++)
		{
			normalized1[i] = (l1[i]-domain.low[i]) / domainExtent[i] * resolution[i];
			normalized2[i] = (l2[i]-domain.low[i]) / domainExtent[i] * resolution[i];
		}

		if (dimension==2)
			bitmap.draw_line((int)normalized1[0],(int)(resolution[1]-normalized1[1]),(int)normalized2[0],(int)(resolution[1]-normalized2[1]),c.rgb);
		else
			bitmap.draw_line((int)normalized1[0],(int)(resolution[1]-normalized1[1]),(int)normalized1[2],(int)normalized2[0],(int)(resolution[1]-normalized2[1]),(int)normalized2[2],c.rgb);
	}

	void Bitmap::drawArrow(const Vertex& l1,const Vertex & l2,const Color& c)
	{
		Vertex normalized1,normalized2;

		for (int i=0;i<dimension;i++)
		{
			normalized1[i] = (l1[i]-domain.low[i]) / domainExtent[i] * resolution[i];
			normalized2[i] = (l2[i]-domain.low[i]) / domainExtent[i] * resolution[i];
		}

		if (dimension==2)
			bitmap.draw_arrow((int)normalized1[0],(int)(resolution[1]-normalized1[1]),(int)normalized2[0],(int)(resolution[1]-normalized2[1]),c.rgb,1,25,15);
		else
			cout << "Cannot draw Arrow in 3D";
	}

	void Bitmap::drawPoint(const Vertex& p,const Color& c)
	{
		Vertex normalized;
		for (int i=0;i<dimension;i++)
			normalized[i] = (p[i]-domain.low[i]) / domainExtent[i] * resolution[i];

		if (dimension==2)
			bitmap.draw_point((int)normalized[0],(int)(resolution[1]-normalized[1]),c.rgb);
		else
			bitmap.draw_point((int)normalized[0],(int)(resolution[1]-normalized[1]),(int)normalized[2],c.rgb);
	}

	void Bitmap::drawRect(const Box& b,const Color& c)
	{
		Vertex normalized1,normalized2;
		for (int i=0;i<dimension;i++)
		{
			normalized1[i] = (b.low[i]-domain.low[i]) / domainExtent[i] * resolution[i];
			normalized2[i] = (b.high[i]-domain.low[i]) / domainExtent[i] * resolution[i];
		}

		if (dimension==2)
			bitmap.draw_rectangle((int)normalized1[0],(int)(resolution[1]-normalized1[1]),(int)normalized2[0],(int)(resolution[1]-normalized2[1]),c.rgb,1,~0U);
		else
			bitmap.draw_rectangle((int)normalized1[0],(int)(resolution[1]-normalized1[1]),(int)normalized1[2],(int)normalized2[0],(int)(resolution[1]-normalized2[1]),(int)normalized2[2],c.rgb,1,0);
	}

	void Bitmap::drawFilledRect(const Box& b,const Color& c)
	{
		Vertex normalized1,normalized2;
		for (int i=0;i<dimension;i++)
		{
			normalized1[i] = (b.low[i]-domain.low[i]) / domainExtent[i] * resolution[i];
			normalized2[i] = (b.high[i]-domain.low[i]) / domainExtent[i] * resolution[i];
		}

		if (dimension==2)
			bitmap.draw_rectangle((int)normalized1[0],(int)(resolution[1]-normalized1[1]),(int)normalized2[0],(int)(resolution[1]-normalized2[1]),c.rgb);
		else
			bitmap.draw_rectangle((int)normalized1[0],(int)(resolution[1]-normalized1[1]),(int)normalized1[2],(int)normalized2[0],(int)(resolution[1]-normalized2[1]),(int)normalized2[2],c.rgb);
	}

	void Bitmap::drawCircle(const Vertex& center, spaceUnit radius, const Color& c)
	{
		Vertex normalized;
		for (int i=0;i<dimension;i++)
			normalized[i] = (center[i]-domain.low[i]) / domainExtent[i] * resolution[i];

		bitmap.draw_circle((int)normalized[0],(int)(resolution[1]-normalized[1]),(int)radius,c.rgb);
	}

	void Bitmap::show()
	{
		bitmap.display();
	}


}
