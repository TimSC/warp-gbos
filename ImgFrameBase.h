#ifndef IMGFRAMEBASE_H_
#define IMGFRAMEBASE_H_

#include <stdlib.h>
//#include <iostream>
//using namespace std;

#define IMG_FRAME_TYPE_UNKNOWN 0
#define IMG_FRAME_TYPE_CIMG_DOUBLE 1
#define IMG_FRAME_TYPE_CIMG_DISPLAY 2

class ImgFrameBase
{
public:
    ImgFrameBase() {};
    ImgFrameBase(const ImgFrameBase &a) {ImgFrameBase::operator=(a);};
	virtual ~ImgFrameBase() {};
	virtual ImgFrameBase& operator=(const ImgFrameBase &a)
	{
		int width = a.GetWidth();
		SetWidth(width);
		int height = a.GetHeight();
		SetHeight(height);
		int channels = a.GetNumChannels();
		SetNumChannels(channels);
		
		if(width < 0 || height < 0 || channels < 0) return *this;
		for(int i=0;i<width;i++)
			for(int j=0;j<height;j++)
				for(int k=0;k<channels;k++)
				{
					double val = a.GetPix (i,j,k);
					SetPix(i,j,k,val);	
				}
		
		return *this;	
	}

	virtual int Open(const char *filename)=0;// {return -1;};

	//Pixel data
	virtual double GetPix(int x, int y, unsigned int channel) const = 0;
	virtual void SetPix(int x, int y, unsigned int channel, double val) {};

	//Meta data
	virtual int GetWidth() const {return -1;};
	virtual int GetHeight() const {return -1;};
	virtual int GetNumChannels() const {return -1;};
	
	virtual int SetWidth(int val) {return -1;};
	virtual int SetHeight(int val) {return -1;};
	virtual int SetNumChannels(int val) {return -1;};

	virtual int GetInternalImageType() const {return IMG_FRAME_TYPE_UNKNOWN;};
	virtual const void *GetInternalDataConst() const {return 0;};
};

class ImgFrameBaseUtils
{
public:
	static void ToGreyScale(class ImgFrameBase &src,
			class ImgFrameBase &dst)
	{
		int width = src.GetWidth();
		dst.SetWidth(width);
		int height = src.GetHeight();
		dst.SetHeight(height);
		dst.SetNumChannels(1);
		int numChan = src.GetNumChannels();
		
		if(width < 0 || height < 0) return;
		for(int i=0;i<width;i++)
			for(int j=0;j<height;j++)
			{
				
				double val = 0.0;
				for(int k=0;k<numChan;k++)
					val += src.GetPix (i,j,k);
				dst.SetPix(i,j,0,val / numChan);	
				//cout << val << endl;
			}
	};

	static void CopyChannel(class ImgFrameBase &src,
			unsigned int srcChannel,
			class ImgFrameBase &dst,
			unsigned int dstChannel)
	{
		int width = src.GetWidth();
		dst.SetWidth(width);
		int height = src.GetHeight();
		dst.SetHeight(height);
		
		if(width < 0 || height < 0) return;
		for(int i=0;i<width;i++)
			for(int j=0;j<height;j++)
			{
				double val = src.GetPix (i,j,srcChannel);
				dst.SetPix(i,j,dstChannel,val);	
				//cout << val << endl;
			}
	};

	static bool WithinImageBounds(class ImgFrameBase &src, 
		int x, int y, unsigned int channel)
	{
		int outOfBounds = 0;
		if(channel < 0 && (int)channel >= src.GetNumChannels()) outOfBounds = 1;
		if(x < 0 || x >= src.GetWidth()) outOfBounds = 1;
		if(y < 0 || y >= src.GetHeight()) outOfBounds = 1;
		return !outOfBounds;
	};

	static double GetIntensityNoBounds(class ImgFrameBase &src, 
		int x, int y, unsigned int channel)
	{
		int outOfBounds = !WithinImageBounds(src,x,y,channel);
		if(outOfBounds) return (double)(rand()%256);
		
		//try
		//{
			return src.GetPix(x,y,channel);
		//}
		//catch(int e)
		//{
			return (double)(rand()%256);
			//return 255.0;
		//}
	};

	static void GetSubImage(class ImgFrameBase &src,
		int x, int y,
		int widthIn, int heightIn,
		class ImgFrameBase &out)
	{
		int width = src.GetWidth();		
		int height = src.GetHeight();
		int channels = src.GetNumChannels();
		
		out.SetWidth(widthIn);
		out.SetHeight(heightIn);
		out.SetNumChannels(channels);
		
		if(width < 0 || height < 0 || channels < 0) return;
		for(int i=0;i<widthIn;i++)
			for(int j=0;j<heightIn;j++)
				for(int k=0;k<channels;k++)
					if(x+i >= 0 && x+i < width && y+j >= 0 && y+j < height)
					{
						double val = src.GetPix (x+i,y+j,k);
						out.SetPix(i,j,k,val);	
					}
		
		return;	
	};
};

class ImgSeqBase : public ImgFrameBase
{
public:
	ImgSeqBase() : ImgFrameBase() {};
	virtual ~ImgSeqBase() {};
	virtual int ReloadCurrentFrame() {return -1;};
	virtual int NextFrameReady() {return -1;};
	virtual int GetNextFrame() {return -1;};

	virtual int SeekFrameReady() {return -1;};
	virtual int SeekToFrame(unsigned int frame) {return -1;};
	virtual int GetFrameNumber() {return -1;};
	virtual int GetDuration() {return -1;};
};

#endif /*IMGFRAMEBASE_H_*/

