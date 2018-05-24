
#include "ImgMagick.h"
#include <wand/magick-wand.h>
#include <string.h>
#include <iostream>
using namespace std;

ImgMagick::ImgMagick()
{
	raw = 0;
	height = 0;
	width = 0;
	channels = 3;
}

ImgMagick::ImgMagick(const ImgMagick &a)
{
	raw = 0;
	operator=(a);
}

ImgMagick& ImgMagick::operator=(const ImgMagick& a)
{
	height = a.height;
	width = a.width;
	channels = a.channels;
	if(raw) delete raw; raw = 0;
	if(a.raw)
	{
		raw = new unsigned char[height*width*channels];
		memcpy(raw, a.raw, height*width*channels);
	}
	return *this;
}

ImgMagick::~ImgMagick()
{
	if(raw) delete raw;
	raw = 0;
}

int ImgMagick::Open(const char *filename)
{
	if(raw) delete raw;
	raw = 0;

	MagickWand *wand = NewMagickWand();
	//Open File	
	FILE *imgFile = fopen(filename,"r");
	if(!imgFile) return -1;
	MagickBooleanType ret = MagickReadImageFile(wand,imgFile);
	fclose(imgFile);
	if(!ret) return -1;

	width = MagickGetImageWidth(wand);
	height = MagickGetImageHeight(wand);
	channels = 3;

	raw = new unsigned char[width*height*channels];
	MagickGetImagePixels(wand,0,0,width,height,"RGB",CharPixel,raw);
	ClearMagickWand(wand);
	return 1;
}

int ImgMagick::Save(const char *filename)
{
	if(!raw) return -1;

	MagickWand *wand = NewMagickWand();
	MagickConstituteImage(wand,
    		width,height,"RGB",
    		CharPixel,raw);

	//Save and clean up
	MagickWriteImage(wand, filename);
	ClearMagickWand(wand);
	return 1;
}

void ImgMagick::Clear()
{
	if(raw) delete raw;
	raw = 0;
}

int ImgMagick::Ready()
{
	return (raw != 0);
}

//Pixel data
double ImgMagick::GetPix(int x, int y, unsigned int channel) const
{
	return raw[channel+x*channels+y*channels*width];
}

void ImgMagick::SetPix(int x, int y, unsigned int channel, double val)
{
	raw[channel+x*channels+y*channels*width] = (char)(val+0.5);
}

//Meta data
int ImgMagick::GetWidth() const
{
	return width;
}

int ImgMagick::GetHeight() const
{
	return height;
}

int ImgMagick::GetNumChannels() const
{
	return channels;
}

int ImgMagick::SetWidth(int val)
{
	if(width == val) return 0;
	if(raw) delete raw; 
	width = val;
	raw = new unsigned char[width*height*channels];
	return 1;
}

int ImgMagick::SetHeight(int val)
{
	if(height == val) return 0;
	if(raw) delete raw; 
	height = val;
	raw = new unsigned char[width*height*channels];
	return 1;
}

int ImgMagick::SetNumChannels(int val)
{
	if(val != 3) {cout << "Only 3 channels are supported, " << val << " requested" << endl; throw(0);}
	channels = 3;
	return 1;
}

int ImgMagick::GetInternalImageType() const 
{
	return IMG_FRAME_TYPE_MAGICKWAND;
}

const void *ImgMagick::GetInternalDataConst() const
{
	return raw;
}




